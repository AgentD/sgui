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
typedef GLXContext (* CREATECONTEXTATTRIBSPROC )( Display*, GLXFBConfig,
                                                  GLXContext, Bool,
                                                  const int* );

typedef struct
{
    sgui_context super;

    sgui_window* wnd;
    GLXContext gl;
}
sgui_context_gl;

static const int versions[][2] = { {4,5}, {4,4}, {4,3}, {4,2}, {4,1}, {4,0},
                                                 {3,3}, {3,2}, {3,1}, {3,0} };


static void set_attributes( int* attr, int bpp, int depth, int stencil,
                            int doublebuffer, int samples )
{
#define ADD_ATTR( name, val ) *(attr++) = (name); *(attr++) = (val)
    ADD_ATTR( GLX_BUFFER_SIZE, bpp             );
    ADD_ATTR( GLX_RED_SIZE,    bpp==16 ? 5 : 8 );
    ADD_ATTR( GLX_GREEN_SIZE,  bpp==16 ? 6 : 8 );
    ADD_ATTR( GLX_BLUE_SIZE,   bpp==16 ? 5 : 8 );

    if( bpp==32      ) { ADD_ATTR( GLX_ALPHA_SIZE,   8       ); }
    if( depth        ) { ADD_ATTR( GLX_DEPTH_SIZE,   depth   ); }
    if( stencil      ) { ADD_ATTR( GLX_STENCIL_SIZE, stencil ); }
    if( doublebuffer ) { ADD_ATTR( GLX_DOUBLEBUFFER, True    ); }

    if( samples )
    {
        ADD_ATTR( GLX_SAMPLE_BUFFERS, 1 );
        ADD_ATTR( GLX_SAMPLES, samples );
    }

    ADD_ATTR( None, None );
#undef ADD_ATTR
}
/****************************************************************************/
static sgui_context* gl_context_create_share( sgui_context* super )
{
    sgui_context_gl* this = (sgui_context_gl*)super;
    return gl_context_create( this->wnd, this->wnd->backend, super );
}

static void gl_context_destroy( sgui_context* this )
{
    sgui_internal_lock_mutex( );
    glXDestroyContext( x11.dpy, ((sgui_context_gl*)this)->gl );
    sgui_internal_unlock_mutex( );

    free( this );
}

static void gl_context_make_current( sgui_context* this, sgui_window* wnd )
{
    GLXContext ctx = this ? ((sgui_context_gl*)this)->gl : 0;
    Drawable dst = wnd ? TO_X11(wnd)->wnd : None;

    sgui_internal_lock_mutex( );
    glXMakeContextCurrent( x11.dpy, dst, dst, ctx );
    sgui_internal_unlock_mutex( );
}

static void* gl_context_get_internal( sgui_context* this )
{
    return &(((sgui_context_gl*)this)->gl);
}

static void gl_context_release_current( sgui_context* this )
{
    (void)this;
    sgui_internal_lock_mutex( );
    glXMakeContextCurrent( x11.dpy, None, None, 0 );
    sgui_internal_unlock_mutex( );
}

static sgui_funptr gl_context_load( sgui_context* this, const char* name )
{
    return (this && name) ? LOAD_GLFUN( name ) : NULL;
}
/****************************************************************************/
static void gl_set_vsync( sgui_window* this, int interval )
{
    void(* SwapIntervalEXT )( Display*, GLXDrawable, int );

    sgui_internal_lock_mutex( );

    SwapIntervalEXT = (void(*)(Display*,GLXDrawable,int))
                      LOAD_GLFUN( "glXSwapIntervalEXT" );

    if( SwapIntervalEXT )
        SwapIntervalEXT( x11.dpy, TO_X11(this)->wnd, interval );

    sgui_internal_unlock_mutex( );
}

static void gl_swap_buffers( sgui_window* this )
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
/****************************************************************************/
sgui_context* gl_context_create( sgui_window* wnd, int backend,
                                 sgui_context* share )
{
    CREATECONTEXTATTRIBSPROC CreateContextAttribs;
    sgui_context_gl* this = calloc( 1, sizeof(sgui_context_gl) );
    sgui_context* super = (sgui_context*)this;
    GLXContext sctx = share ? ((sgui_context_gl*)share)->gl : 0;
    int attribs[10];
    unsigned int i;

    if( !this )
        return NULL;

    sgui_internal_lock_mutex( );
    CreateContextAttribs = (CREATECONTEXTATTRIBSPROC)
    LOAD_GLFUN( "glXCreateContextAttribsARB" );

    if( backend==SGUI_OPENGL_CORE && CreateContextAttribs )
    {
        attribs[0] = GLX_CONTEXT_MAJOR_VERSION_ARB;
        attribs[1] = 0;
        attribs[2] = GLX_CONTEXT_MINOR_VERSION_ARB;
        attribs[3] = 0;
        attribs[4] = GLX_CONTEXT_PROFILE_MASK_ARB;
        attribs[5] = GLX_CONTEXT_CORE_PROFILE_BIT_ARB;
        attribs[6] = GLX_CONTEXT_FLAGS_ARB;
        attribs[7] = GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB;
        attribs[8] = None;

        for(i=0; !this->gl && i<sizeof(versions)/sizeof(versions[0]); ++i)
        {
            attribs[1] = versions[i][0];
            attribs[3] = versions[i][1];
            this->gl = CreateContextAttribs( x11.dpy, TO_X11(wnd)->cfg, sctx,
                                             True, attribs );
        }
    }

    if( !this->gl )
        this->gl = glXCreateNewContext( x11.dpy, TO_X11(wnd)->cfg,
                                        GLX_RGBA_TYPE, sctx, GL_TRUE );

    if( !this->gl )
        goto fail;

    this->wnd = wnd;

    wnd->swap_buffers = gl_swap_buffers;
    wnd->set_vsync = gl_set_vsync;

    super->create_share    = gl_context_create_share;
    super->destroy         = gl_context_destroy;
    super->make_current    = gl_context_make_current;
    super->release_current = gl_context_release_current;
    super->load            = gl_context_load;
    super->get_internal    = gl_context_get_internal;
    sgui_internal_unlock_mutex( );
    return super;
fail:
    free( this );
    return NULL;
}

Window create_glx_window( sgui_window* this,
                          const sgui_window_description* desc,
                          Window parent )
{
    int fbcount, attr[20], samples = desc->samples;
    XSetWindowAttributes swa;
    GLXFBConfig* fbl = NULL;
    XVisualInfo* vi = NULL;
    Window wnd = 0;

    while( !fbl && samples>=0 )
    {
        set_attributes( attr, desc->bits_per_pixel, desc->depth_bits,
                        desc->stencil_bits, desc->flags & SGUI_DOUBLEBUFFERED,
                        samples-- );

        fbl = glXChooseFBConfig( x11.dpy, x11.screen, attr, &fbcount );
    }

    if( !fbl )
        return 0;
    if( !fbcount )
        goto outfbl;

    TO_X11(this)->cfg = fbl[0];
    vi = glXGetVisualFromFBConfig( x11.dpy, fbl[0] );

    if( !vi )
        goto outfbl;

    swa.border_pixel = 0;
    swa.colormap = XCreateColormap( x11.dpy, RootWindow(x11.dpy, vi->screen),
                                    vi->visual, AllocNone );

    if( !swa.colormap )
        goto out;

    wnd = XCreateWindow( x11.dpy, parent, 0, 0, desc->width, desc->height, 0,
                         vi->depth, InputOutput, vi->visual,
                         CWBorderPixel|CWColormap, &swa );
out:
    XFree( vi );
outfbl:
    XFree( fbl );
    return wnd;
}
#else
Window create_glx_window( sgui_window* wnd,
                          const sgui_window_description* desc,
                          Window parent )
{
    (void)wnd; (void)desc; (void)parent;
    return 0;
}

sgui_context* gl_context_create( sgui_window* wnd, int backend,
                                 sgui_context* share )
{
    (void)wnd; (void)backend; (void)share;
    return NULL;
}
#endif /* SGUI_NO_OPENGL */

