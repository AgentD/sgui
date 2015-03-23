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
#include "platform.h"



#ifndef SGUI_NO_OPENGL
int glversions[][2] = { {4,5}, {4,4}, {4,3}, {4,2}, {4,1}, {4,0},
                                      {3,3}, {3,2}, {3,1}, {3,0} };


static void set_attributes( int* attr, int bpp, int depth, int stencil,
                            int doublebuffer, int samples )
{
    *(attr++) = GLX_BUFFER_SIZE;
    *(attr++) = bpp;
    *(attr++) = GLX_RED_SIZE;
    *(attr++) = bpp==16 ? 5 : 8;
    *(attr++) = GLX_GREEN_SIZE;
    *(attr++) = bpp==16 ? 6 : 8;
    *(attr++) = GLX_BLUE_SIZE;
    *(attr++) = bpp==16 ? 5 : 8;

    if( bpp!=16 && bpp!=24 )
    {
        *(attr++) = GLX_ALPHA_SIZE;
        *(attr++) = 8;
    }

    if( depth )
    {
        *(attr++) = GLX_DEPTH_SIZE;
        *(attr++) = depth;
    }

    if( stencil )
    {
        *(attr++) = GLX_STENCIL_SIZE;
        *(attr++) = stencil;
    }

    if( doublebuffer )
    {
        *(attr++) = GLX_DOUBLEBUFFER;
        *(attr++) = True;
    }

    if( samples )
    {
        *(attr++) = GLX_SAMPLE_BUFFERS;
        *(attr++) = 1;
        *(attr++) = GLX_SAMPLES;
        *(attr++) = samples;
    }

    *attr = None;
}

int get_fbc_visual_cmap( GLXFBConfig* fbc, XVisualInfo** vi, Colormap* cmap,
                         const sgui_window_description* desc )
{
    int fbcount, attr[20], samples = desc->samples;
    GLXFBConfig* fbl = NULL;

    while( !fbl && samples>=0 )
    {
        set_attributes( attr, desc->bits_per_pixel, desc->depth_bits,
                        desc->stencil_bits, desc->flags & SGUI_DOUBLEBUFFERED,
                        samples-- );

        fbl = glXChooseFBConfig( x11.dpy, x11.screen, attr, &fbcount );
    }

    if( !fbl || !fbcount )
        return 0;

    *fbc = fbl[0];
    *vi = glXGetVisualFromFBConfig( x11.dpy, fbl[0] );
    XFree( fbl );

    if( !(*vi) )
        return 0;

    *cmap = XCreateColormap( x11.dpy, RootWindow(x11.dpy, (*vi)->screen),
                             (*vi)->visual, AllocNone );

    if( !(*cmap) )
    {
        XFree( *vi );
        return 0;
    }

    return 1;
}

/****************************************************************************/

static sgui_context* gl_context_create_share( sgui_context* super )
{
    sgui_context_gl* this = (sgui_context_gl*)super;

    return this ? gl_context_create( this->wnd,
                                     this->wnd->backend==SGUI_OPENGL_CORE,
                                     this ) : NULL;
}

static void gl_context_destroy( sgui_context* this )
{
    if( this )
    {
        sgui_internal_lock_mutex( );
        glXDestroyContext( x11.dpy, ((sgui_context_gl*)this)->gl );
        sgui_internal_unlock_mutex( );

        free( this );
    }
}

static void gl_context_make_current( sgui_context* this, sgui_window* wnd )
{
    if( this && wnd )
    {
        sgui_internal_lock_mutex( );
        glXMakeContextCurrent( x11.dpy, TO_X11(wnd)->wnd, TO_X11(wnd)->wnd,
                               ((sgui_context_gl*)this)->gl );
        sgui_internal_unlock_mutex( );
    }
}

static void* gl_context_get_internal( sgui_context* this )
{
    return this ? &(((sgui_context_gl*)this)->gl) : NULL;
}

static void gl_context_release_current( sgui_context* this )
{
    if( this )
    {
        sgui_internal_lock_mutex( );
        glXMakeContextCurrent( x11.dpy, 0, 0, 0 );
        sgui_internal_unlock_mutex( );
    }
}

static sgui_funptr gl_context_load( sgui_context* this, const char* name )
{
    return (this && name) ? LOAD_GLFUN( name ) : NULL;
}


sgui_context* gl_context_create( sgui_window* wnd, int core,
                                 sgui_context_gl* share )
{
    CREATECONTEXTATTRIBSPROC CreateContextAttribs;
    sgui_context_gl* this = malloc( sizeof(sgui_context_gl) );
    sgui_context* super = (sgui_context*)this;
    GLXContext sctx = share ? share->gl : 0;
    int attribs[10];
    unsigned int i;

    if( !this )
        return NULL;

    sgui_internal_lock_mutex( );
    this->gl = 0;

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
        for(i=0; !this->gl && i<sizeof(glversions)/sizeof(glversions[0]); ++i)
        {
            attribs[1] = glversions[i][0];
            attribs[3] = glversions[i][1];
            this->gl = CreateContextAttribs( x11.dpy, TO_X11(wnd)->cfg, sctx,
                                             True, attribs );
        }
    }

    /* fallback: old context creation function */
    if( !this->gl )
        this->gl = glXCreateNewContext( x11.dpy, TO_X11(wnd)->cfg,
                                        GLX_RGBA_TYPE, sctx, GL_TRUE );

    if( this->gl )
    {
        this->wnd = wnd;

        super->create_share    = gl_context_create_share;
        super->destroy         = gl_context_destroy;
        super->make_current    = gl_context_make_current;
        super->release_current = gl_context_release_current;
        super->load            = gl_context_load;
        super->get_internal    = gl_context_get_internal;
    }
    else
    {
        free( this );
        this = NULL;
    }

    sgui_internal_unlock_mutex( );

    return super;
}

void gl_set_vsync( sgui_window* this, int interval )
{
    void(* SwapIntervalEXT )( Display*, GLXDrawable, int );

    sgui_internal_lock_mutex( );

    SwapIntervalEXT = (void(*)(Display*,GLXDrawable,int))
                      LOAD_GLFUN( "glXSwapIntervalEXT" );

    if( SwapIntervalEXT )
        SwapIntervalEXT( x11.dpy, TO_X11(this)->wnd, interval );

    sgui_internal_unlock_mutex( );
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

    glXSwapBuffers( x11.dpy, TO_X11(this)->wnd );
    sgui_internal_unlock_mutex( );
}
#endif

