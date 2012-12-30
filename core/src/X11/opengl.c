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
#include "sgui_opengl.h"
#include "sgui_event.h"

#ifndef SGUI_NO_OPENGL
#include <stdio.h>


#define LOAD_GLFUN( name ) glXGetProcAddress( (const GLubyte*)(name) )

#ifndef GLX_VERSION_1_3
    typedef struct GLXFBConfigRec* GLXFBConfig;

    #define GLX_X_RENDERABLE  0x8012
    #define GLX_X_VISUAL_TYPE 0x22
    #define GLX_TRUE_COLOR    0x8002
#endif

#ifndef GLX_VERSION_1_4
    #define GLX_CONTEXT_MAJOR_VERSION_ARB             0x2091
    #define GLX_CONTEXT_MINOR_VERSION_ARB             0x2092
    #define GLX_CONTEXT_FLAGS_ARB                     0x2094
    #define GLX_CONTEXT_PROFILE_MASK_ARB              0x9126

    #define GLX_CONTEXT_DEBUG_BIT_ARB                 0x0001
    #define GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB    0x0002

    #define GLX_CONTEXT_CORE_PROFILE_BIT_ARB          0x00000001
    #define GLX_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB 0x00000002
#endif

typedef GLXFBConfig* (* CHOOSEFBCFGPROC )(Display*,int,const int*,int*);
typedef XVisualInfo* (* GETVISUALFROMFBCPROC )(Display*, GLXFBConfig);
typedef GLXContext   (* CREATECONTEXTATTRIBSPROC )( Display*, GLXFBConfig,
                                                    GLXContext, Bool,
                                                    const int* );


GLXFBConfig get_fb_config( void )
{
    GLXFBConfig fbc, *fbl;
    CHOOSEFBCFGPROC ChooseFBConfig;
    int fbcount;

    int attr[] =
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

    ChooseFBConfig = (CHOOSEFBCFGPROC)LOAD_GLFUN( "glXChooseFBConfig" );

    if( !ChooseFBConfig )
        return NULL;

    fbl = ChooseFBConfig( dpy, DefaultScreen(dpy), attr, &fbcount );

    if( !fbl )
        return NULL;

    fbc = fbl[0];
    XFree( fbl );

    return fbc;
}

XVisualInfo* get_visual_from_fbc( GLXFBConfig fbc )
{
    GETVISUALFROMFBCPROC GetVisualFromFBConfig;

    GetVisualFromFBConfig = (GETVISUALFROMFBCPROC)
    LOAD_GLFUN( "glXGetVisualFromFBConfig" );

    if( !GetVisualFromFBConfig )
        return NULL;

    return GetVisualFromFBConfig( dpy, fbc );
}

XVisualInfo* get_visual_old( void )
{
    int attr[] =
    {
        GLX_RGBA,
        GLX_DOUBLEBUFFER,
        GLX_RED_SIZE,     8,
        GLX_GREEN_SIZE,   8,
        GLX_BLUE_SIZE,    8,
        GLX_ALPHA_SIZE,   8,
        GLX_DEPTH_SIZE,  24,
        GLX_STENCIL_SIZE, 8,
        None
    };

    return glXChooseVisual( dpy, DefaultScreen(dpy), attr );
}

/****************************************************************************/

GLXContext create_context( GLXFBConfig cfg )
{
    CREATECONTEXTATTRIBSPROC CreateContextAttribs;
    GLXContext ctx = 0;
    int attribs[10], major, minor;

    /* try to load context creation function */
    CreateContextAttribs = (CREATECONTEXTATTRIBSPROC)
    LOAD_GLFUN( "glXCreateContextAttribsARB" );

    if( !CreateContextAttribs )
        return NULL;

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
    for( major=4; !ctx && major>=3; --major )
    {
        for( minor=3; !ctx && minor>=0; --minor )
        {
            attribs[1] = major;
            attribs[3] = minor;
            ctx = CreateContextAttribs( dpy, cfg, 0, True, attribs );
        }
    }

    /* try to create 2.x context */
    for( minor=1; !ctx && minor>=0; --minor )
    {
        attribs[1] = 2;
        attribs[3] = minor;
        ctx = CreateContextAttribs( dpy, cfg, 0, True, attribs );
    }

    /* try to create 1.x context */
    for( minor=5; !ctx && minor>=0; --minor )
    {
        attribs[1] = 1;
        attribs[3] = minor;
        ctx = CreateContextAttribs( dpy, cfg, 0, True, attribs );
    }

    return ctx;
}

/****************************************************************************/


sgui_window* sgui_opengl_window_create( unsigned int width,
                                        unsigned int height, int resizeable,
                                        int compatibillity )
{
    sgui_window_xlib* wnd;
    XSizeHints hints;
    XSetWindowAttributes swa;
    XWindowAttributes attr;
    XVisualInfo* vi = NULL;
    GLXFBConfig fbc = NULL;
    Colormap cmap;

    if( !width || !height )
        return NULL;

    /********* allocate space for the window structure *********/
    wnd = add_window( );

    if( !wnd )
        return NULL;

    /******************** create the window ********************/

    /* try to get an XVisualInfo by any means possible */
    fbc = get_fb_config( );

    if( fbc )
        vi = get_visual_from_fbc( fbc );

    if( !vi )
        vi = get_visual_old( );

    if( !vi )
    {
        sgui_opengl_window_destroy( (sgui_window*)wnd );
        return NULL;
    }

    /* get a color map for the visual */
    cmap = XCreateColormap( dpy, RootWindow(dpy, vi->screen),
                            vi->visual, AllocNone );

    if( !cmap )
    {
        sgui_opengl_window_destroy( (sgui_window*)wnd );
        return NULL;
    }

    /* create the window */
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
    if( fbc && !compatibillity )
        wnd->context.gl = create_context( fbc );

    /* fall back to old context creation function */
    if( !wnd->context.gl )
        wnd->context.gl = glXCreateContext( dpy, vi, NULL, GL_TRUE );

    XFree( vi );    /* either way, we don't need the visual info anymore */

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
#endif

