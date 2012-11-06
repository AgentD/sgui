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
#include "internal.h"
#include "sgui_opengl.h"



XVisualInfo* get_visual( void )
{
    GLXFBConfig* fbl;
    XVisualInfo* vi;
    int fbcount;

    static int attr[] =
    {
        GLX_X_RENDERABLE,   True,
        GLX_X_VISUAL_TYPE,  GLX_TRUE_COLOR,
        GLX_RED_SIZE,       8,
        GLX_GREEN_SIZE,     8,
        GLX_BLUE_SIZE,      8,
        GLX_ALPHA_SIZE,     8,
        GLX_DEPTH_SIZE,     24,
        GLX_STENCIL_SIZE,   8,
        GLX_DOUBLEBUFFER,   True,
        None
    };

    fbl = glXChooseFBConfig( dpy, DefaultScreen(dpy), attr, &fbcount );

    if( !fbl )
        return NULL;

    vi = glXGetVisualFromFBConfig( dpy, fbl[0] );
    XFree( fbl );

    return vi;
}

GLXContext create_context( XVisualInfo* vi )
{
    return glXCreateContext( dpy, vi, NULL, GL_TRUE );
}



sgui_window* sgui_opengl_window_create( unsigned int width,
                                        unsigned int height, int resizeable )
{
    sgui_window_xlib* wnd;
    XSizeHints hints;
    XSetWindowAttributes swa;
    XWindowAttributes attr;
    XVisualInfo* vi;
    Colormap cmap;

    if( !width || !height )
        return NULL;

    /********* allocate space for the window structure *********/
    wnd = add_window( );

    if( !wnd )
        return NULL;

    /******************** create the window ********************/
    vi = get_visual( );

    if( !vi )
    {
        sgui_opengl_window_destroy( (sgui_window*)wnd );
        return NULL;
    }

    cmap = XCreateColormap( dpy, RootWindow(dpy, vi->screen),
                            vi->visual, AllocNone );

    if( !cmap )
    {
        sgui_opengl_window_destroy( (sgui_window*)wnd );
        return NULL;
    }

    swa.colormap          = cmap;
    swa.background_pixmap = None;
    swa.border_pixel      = 0;
    swa.event_mask        = ExposureMask | StructureNotifyMask |
                            SubstructureNotifyMask |
                            KeyPressMask | KeyReleaseMask |
                            PointerMotionMask |
                            ButtonPressMask | ButtonReleaseMask;

    wnd->wnd = XCreateWindow( dpy, RootWindow(dpy, vi->screen), 0, 0,
                              width, height, 0, vi->depth, InputOutput,
                              vi->visual,CWBorderPixel|CWColormap|CWEventMask,
                              &swa );

    if( !wnd->wnd )
    {
        sgui_opengl_window_destroy( (sgui_window*)wnd );
        return NULL;
    }

    /* make the window non resizeable if required */
    if( !resizeable )
    {
        hints.flags      = PSize | PMinSize | PMaxSize;
        hints.min_width  = hints.max_width  = hints.base_width  = (int)width;
        hints.min_height = hints.max_height = hints.base_height = (int)height;

        XSetWMNormalHints( dpy, wnd->wnd, &hints );
    }

    /* tell X11 what events we will handle */
    XSetWMProtocols( dpy, wnd->wnd, &atom_wm_delete, 1 );
    XFlush( dpy );

    /* get the real geometry as the window manager is free to change it */
    XGetWindowAttributes( dpy, wnd->wnd, &attr );

    wnd->base.x = attr.x;
    wnd->base.y = attr.y;
    wnd->base.w = (unsigned int)attr.width;
    wnd->base.h = (unsigned int)attr.height;

    /**************** Create an OpenGL context *****************/
    wnd->context.gl = create_context( vi );
    XFree( vi );

    if( !wnd->context.gl )
    {
        sgui_opengl_window_destroy( (sgui_window*)wnd );
        return NULL;
    }

    /*********** Create an input method and context ************/
    wnd->ic = XCreateIC( im, XNInputStyle,
                         XIMPreeditNothing | XIMStatusNothing, XNClientWindow,
                         wnd->wnd, XNFocusWindow, wnd->wnd, NULL );

    if( !wnd->ic )
    {
        sgui_opengl_window_destroy( (sgui_window*)wnd );
        return NULL;
    }

    /************* store the remaining information *************/
    wnd->resizeable = resizeable;

    wnd->base.get_mouse_position = window_x11_get_mouse_position;
    wnd->base.set_mouse_position = window_x11_set_mouse_position;
    wnd->base.set_visible        = window_x11_set_visible;
    wnd->base.set_title          = window_x11_set_title;
    wnd->base.set_size           = window_x11_set_size;
    wnd->base.move_center        = window_x11_move_center;
    wnd->base.move               = window_x11_move;

    return (sgui_window*)wnd;
}

void sgui_opengl_window_destroy( sgui_window* window )
{
    if( !window )
        return;

    SEND_EVENT( window, SGUI_API_DESTROY_EVENT, NULL );

    if( TO_X11(window)->context.gl )
        glXDestroyContext( dpy, TO_X11(window)->context.gl );

    if( TO_X11(window)->ic )
        XDestroyIC( TO_X11(window)->ic );

    if( TO_X11(window)->wnd )
        XDestroyWindow( dpy, TO_X11(window)->wnd );

    remove_window( TO_X11(window) );
}

void sgui_opengl_window_make_current( sgui_window* window )
{
    if( window )
        glXMakeCurrent( dpy, TO_X11(window)->wnd, TO_X11(window)->context.gl );
    else
        glXMakeCurrent( dpy, 0, 0 );
}

void sgui_opengl_window_swap_buffers( sgui_window* window )
{
    if( window )
        glXSwapBuffers( dpy, TO_X11(window)->wnd );
}

