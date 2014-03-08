/*
 * opengl.c
 * This file is part of sgui
 *
 * Copyright (C) 2012 - David Oberhollenzer
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */
#define SGUI_BUILDING_DLL
#include "internal.h"
#include "sgui_event.h"

#ifndef SGUI_NO_OPENGL
#ifndef GLX_ARB_create_context
    #define GLX_CONTEXT_MAJOR_VERSION_ARB             0x2091
    #define GLX_CONTEXT_MINOR_VERSION_ARB             0x2092
    #define GLX_CONTEXT_FLAGS_ARB                     0x2094

    #define GLX_CONTEXT_DEBUG_BIT_ARB                 0x0001
    #define GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB    0x0002
#endif

#ifndef GLX_ARB_create_context_profile
    #define GLX_CONTEXT_PROFILE_MASK_ARB              0x9126
    #define GLX_CONTEXT_CORE_PROFILE_BIT_ARB          0x00000001
    #define GLX_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB 0x00000002
#endif


typedef GLXContext (* CREATECONTEXTATTRIBSPROC )( Display*, GLXFBConfig,
                                                  GLXContext, Bool,
                                                  const int* );

int glversions[][2] = { {4,4}, {4,3}, {4,2}, {4,1}, {4,0},
                               {3,3}, {3,2}, {3,1}, {3,0} };


static void set_attributes( int* attr, int bpp, int depth, int stencil,
                            int doublebuffer, int samples )
{
    int i=0;

    if( bpp!=16 || bpp!=24 || bpp!=32 )
        bpp = 32;

    attr[ i++ ] = GLX_BUFFER_SIZE;
    attr[ i++ ] = bpp;
    attr[ i++ ] = GLX_RED_SIZE;
    attr[ i++ ] = bpp==16 ? 5 : 8;
    attr[ i++ ] = GLX_GREEN_SIZE;
    attr[ i++ ] = bpp==16 ? 6 : 8;
    attr[ i++ ] = GLX_BLUE_SIZE;
    attr[ i++ ] = bpp==16 ? 5 : 8;

    if( bpp==32 )
    {
        attr[ i++ ] = GLX_ALPHA_SIZE;
        attr[ i++ ] = 8;
    }

    if( depth )
    {
        attr[ i++ ] = GLX_DEPTH_SIZE;
        attr[ i++ ] = depth;
    }

    if( stencil )
    {
        attr[ i++ ] = GLX_STENCIL_SIZE;
        attr[ i++ ] = stencil;
    }

    if( doublebuffer )
    {
        attr[ i++ ] = GLX_DOUBLEBUFFER;
        attr[ i++ ] = True;
    }

    if( samples )
    {
        attr[ i++ ] = GLX_SAMPLE_BUFFERS;
        attr[ i++ ] = 1;
        attr[ i++ ] = GLX_SAMPLES;
        attr[ i++ ] = samples;
    }

    attr[ i++ ] = None;
}

int get_fbc_visual_cmap( GLXFBConfig* fbc, XVisualInfo** vi, Colormap* cmap,
                         const sgui_window_description* desc )
{
    GLXFBConfig* fbl;
    int fbcount, attr[20], samples;

    *fbc  = NULL;
    *vi   = NULL;
    *cmap = 0;

    samples = desc->samples;

    do
    {
        set_attributes( attr, desc->bits_per_pixel, desc->depth_bits,
                        desc->stencil_bits, desc->doublebuffer, samples-- );

        fbl = glXChooseFBConfig( dpy, DefaultScreen(dpy), attr, &fbcount );
    }
    while( !fbl && samples>=0 );

    if( fbl && fbcount )
    {
        *fbc = fbl[0];
        *vi = glXGetVisualFromFBConfig( dpy, fbl[0] );
        XFree( fbl );
    }

    if( !(*vi) )
        return 0;

    /* get a color map for the visual */
    *cmap = XCreateColormap( dpy, RootWindow(dpy, (*vi)->screen),
                             (*vi)->visual, AllocNone );

    if( !(*cmap) )
    {
        XFree( *vi );
        *vi = NULL;
        return 0;
    }

    return 1;
}

/****************************************************************************/

struct sgui_gl_context
{
    GLXContext gl;
};


sgui_gl_context* sgui_gl_context_create( sgui_window* wnd,
                                         sgui_gl_context* share,
                                         int core )
{
    CREATECONTEXTATTRIBSPROC CreateContextAttribs;
    sgui_gl_context* ctx;
    GLXContext sctx;
    int attribs[10];
    unsigned int i;

    if( !wnd )
        return NULL;

    if( wnd->backend!=SGUI_OPENGL_CORE && wnd->backend!=SGUI_OPENGL_COMPAT )
        return NULL;

    if( !(ctx = malloc( sizeof(sgui_gl_context) )) )
        return NULL;

    sgui_internal_lock_mutex( );

    ctx->gl = 0;
    sctx = share ? share->gl : 0;

    /* try to load context creation function */
    CreateContextAttribs = (CREATECONTEXTATTRIBSPROC)
    LOAD_GLFUN( "glXCreateContextAttribsARB" );

    /* try to create core context */
    if( core && CreateContextAttribs )
    {
        /* fill attribute array */
        attribs[0] = GLX_CONTEXT_MAJOR_VERSION_ARB;
        attribs[1] = 0;
        attribs[2] = GLX_CONTEXT_MINOR_VERSION_ARB;
        attribs[3] = 0;
        attribs[4] = GLX_CONTEXT_PROFILE_MASK_ARB;
        attribs[5] = GLX_CONTEXT_CORE_PROFILE_BIT_ARB;
        attribs[6] = GLX_CONTEXT_FLAGS_ARB;
        attribs[7] = GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB;
        attribs[8] = None;

        /* try to create 4.x down to 3.x context */
        for(i=0; !ctx->gl && i<sizeof(glversions)/sizeof(glversions[0]); ++i)
        {
            attribs[1] = glversions[i][0];
            attribs[3] = glversions[i][1];
            ctx->gl = CreateContextAttribs( dpy, TO_X11(wnd)->cfg, sctx,
                                            True, attribs );
        }
    }

    /* fallback: old context creation function */
    if( !ctx->gl )
        ctx->gl = glXCreateNewContext( dpy, TO_X11(wnd)->cfg, GLX_RGBA_TYPE,
                                       sctx, GL_TRUE );

    if( !ctx->gl )
    {
        free( ctx );
        ctx = NULL;
    }

    sgui_internal_unlock_mutex( );

    return ctx;
}

void sgui_gl_context_destroy( sgui_gl_context* ctx )
{
    if( ctx )
    {
        sgui_internal_lock_mutex( );
        glXDestroyContext( dpy, ctx->gl );
        sgui_internal_unlock_mutex( );

        free( ctx );
    }
}

void sgui_gl_context_make_current( sgui_gl_context* ctx, sgui_window* wnd )
{
    sgui_internal_lock_mutex( );

    if( ctx )
    {
        glXMakeContextCurrent( dpy, TO_X11(wnd)->wnd, TO_X11(wnd)->wnd,
                               ctx->gl );
    }
    else
    {
        glXMakeContextCurrent( dpy, 0, 0, 0 );
    }

    sgui_internal_unlock_mutex( );
}

sgui_funptr sgui_gl_context_load( sgui_gl_context* ctx, const char* name )
{
    (void)ctx;
    return LOAD_GLFUN( name );
}

void gl_swap_buffers( sgui_window* this )
{
    sgui_internal_lock_mutex( );

    /*
        For singlebuffered contexts, glXSwapBuffers is a no-op and thus
        doesn't do an implicit flush (on some implementations like Mesa
        or AMD). When the context is never released and no flush happens,
        certain OpenGL implementations freeze and go to 100% CPU.
        This caused me a lot of confusion, so to avoid it for others, I
        added this not-so-implicit flush.
     */
    glFlush( );

    glXSwapBuffers( dpy, TO_X11(this)->wnd );
    sgui_internal_unlock_mutex( );
}
#else
sgui_gl_context* sgui_gl_context_create( sgui_window* wnd,
                                         sgui_gl_context* share, int core )
{
    (void)wnd;
    (void)share;
    (void)core;
    return NULL;
}

void sgui_gl_context_destroy( sgui_gl_context* ctx )
{
    (void)ctx;
}

void sgui_gl_context_make_current( sgui_gl_context* ctx, sgui_window* wnd )
{
    (void)ctx;
    (void)wnd;
}

sgui_funptr sgui_gl_context_load( sgui_gl_context* ctx, const char* name )
{
    (void)ctx;
    (void)name;
    return NULL;
}
#endif

