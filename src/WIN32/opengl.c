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
#include "sgui_opengl.h"
#include "internal.h"



typedef HGLRC (* WGLCREATECONTEXTATTRIBSARBPROC )( HDC, HGLRC, const int* );

#define WGL_CONTEXT_MAJOR_VERSION_ARB   0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB   0x2092
#define WGL_CONTEXT_LAYER_PLANE_ARB     0x2093
#define WGL_CONTEXT_FLAGS_ARB           0x2094
#define WGL_CONTEXT_PROFILE_MASK_ARB    0x9126

#define WGL_CONTEXT_DEBUG_BIT_ARB               0x0001
#define WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB  0x0002

#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB          0x00000001
#define WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB 0x00000002



int set_pixel_format( HDC hDC )
{
    PIXELFORMATDESCRIPTOR pfd;
    int format;

    memset( &pfd, 0, sizeof( pfd ) );

    pfd.nSize      = sizeof( pfd );
    pfd.nVersion   = 1;
    pfd.dwFlags    = PFD_DRAW_TO_WINDOW|PFD_SUPPORT_OPENGL|PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 24;
    pfd.cDepthBits = 16;
    pfd.iLayerType = PFD_MAIN_PLANE;

    format = ChoosePixelFormat( hDC, &pfd );

    return SetPixelFormat( hDC, format, &pfd );
}

HGLRC create_context( HDC hDC, int version_major,
                      int version_minor, int flags )
{
    WGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = NULL;
    HGLRC ctx;
    int attribs[10];

    /* load the new context creation function */
    wglCreateContextAttribsARB = (WGLCREATECONTEXTATTRIBSARBPROC)
    wglGetProcAddress( "wglCreateContextAttribsARB" );

    if( !wglCreateContextAttribsARB )
        return NULL;

    /* fill attrib array */
    attribs[0] = WGL_CONTEXT_MAJOR_VERSION_ARB;
    attribs[1] = version_major;
    attribs[2] = WGL_CONTEXT_MINOR_VERSION_ARB;
    attribs[3] = version_minor;
    attribs[4] = WGL_CONTEXT_PROFILE_MASK_ARB;
    attribs[5] = flags & SGUI_OPENGL_COMPATIBILITY_PROFILE ?
                         WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB :
                         WGL_CONTEXT_CORE_PROFILE_BIT_ARB;
    attribs[6] = 0;

    if( flags & ~(SGUI_OPENGL_COMPATIBILITY_PROFILE) )
    {
        attribs[6] = WGL_CONTEXT_FLAGS_ARB;
        attribs[7] = 0;
        attribs[8] = 0;

        if( flags & SGUI_OPENGL_DEBUG )
            attribs[7] |= WGL_CONTEXT_DEBUG_BIT_ARB;

        if( flags & SGUI_OPENGL_FORWARD_COMPATIBLE )
            attribs[7] |= WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB;
    }

    /* try to create the context */
    if( !version_major )
    {
        /* try to create 4.x down to 3.x context */
        for( version_major=4; !ctx && version_major>=3; --version_major )
        {
            for( version_minor=3; !ctx && version_minor>=0; --version_minor )
            {
                attribs[1] = version_major;
                attribs[3] = version_minor;
                ctx = wglCreateContextAttribsARB( hDC, 0, attribs );
            }
        }

        /* try to create 2.x context */
        for( version_minor=1; !ctx && version_minor>=0; --version_minor )
        {
            attribs[1] = 2;
            attribs[3] = version_minor;
            ctx = wglCreateContextAttribsARB( hDC, 0, attribs );
        }

        /* try to create 1.x context */
        for( version_minor=5; !ctx && version_minor>=0; --version_minor )
        {
            attribs[1] = 1;
            attribs[3] = version_minor;
            ctx = wglCreateContextAttribsARB( hDC, 0, attribs );
        }
    }
    else
        ctx = wglCreateContextAttribsARB( hDC, 0, attribs );

    return ctx;
}



sgui_window* sgui_opengl_window_create( unsigned int width,
                                        unsigned int height, int resizeable,
                                        int version_major, int version_minor,
                                        int flags )
{
    sgui_window_w32* wnd;
    DWORD style;
    HGLRC temp, oldctx;
    HDC olddc;
    RECT r;

    if( !width || !height )
        return NULL;

    /*************** allocate space for the window structure ***************/
    wnd = add_window( );

    if( !wnd )
        return NULL;

    /*************************** create a window ***************************/
    style = resizeable ? WS_OVERLAPPEDWINDOW : (WS_CAPTION | WS_SYSMENU);

    SetRect( &r, 0, 0, width, height );
    AdjustWindowRect( &r, style, FALSE );

    wnd->hWnd = CreateWindowEx( 0, wndclass, "", style, 0, 0, r.right-r.left,
                                r.bottom-r.top, 0, 0, hInstance, NULL );

    if( !wnd->hWnd )
    {
        sgui_opengl_window_destroy( (sgui_window*)wnd );
        return NULL;
    }

    SET_USER_PTR( wnd->hWnd, wnd );

    wnd->base.w = width;
    wnd->base.h = height;

    /* create OpenGL context */
    wnd->hDC = GetDC( wnd->hWnd );

    if( !wnd->hDC || !set_pixel_format( wnd->hDC ) )
    {
        sgui_opengl_window_destroy( (sgui_window*)wnd );
        return NULL;
    }

    temp = wglCreateContext( wnd->hDC );

    if( !temp )
    {
        sgui_opengl_window_destroy( (sgui_window*)wnd );
        return NULL;
    }

    oldctx = wglGetCurrentContext( );
    olddc = wglGetCurrentDC( );

    if( !wglMakeCurrent( wnd->hDC, temp ) )
    {
        sgui_opengl_window_destroy( (sgui_window*)wnd );
        return NULL;
    }

    wnd->hRC = create_context(wnd->hDC, version_major, version_minor, flags);

    /* restore the privous context */
    wglMakeCurrent( olddc, oldctx );

    if( wnd->hRC )
        wglDeleteContext( temp );
    else
        wnd->hRC = temp;

    /****************** register implementation functions ******************/
    wnd->base.get_mouse_position = window_w32_get_mouse_position;
    wnd->base.set_mouse_position = window_w32_set_mouse_position;
    wnd->base.set_visible        = window_w32_set_visible;
    wnd->base.set_title          = window_w32_set_title;
    wnd->base.set_size           = window_w32_set_size;
    wnd->base.move_center        = window_w32_move_center;
    wnd->base.move               = window_w32_move;

    return (sgui_window*)wnd;
}

void sgui_opengl_window_destroy( sgui_window* window )
{
    sgui_window_w32* wnd = (sgui_window_w32*)window;
    MSG msg;

    if( wnd )
    {
        sgui_internal_window_fire_event(window, SGUI_API_DESTROY_EVENT, NULL);

        if( wnd->hRC )
            wglDeleteContext( wnd->hRC );

        if( wnd->hDC )
            ReleaseDC( wnd->hWnd, wnd->hDC );

        if( wnd->hWnd )
        {
            DestroyWindow( wnd->hWnd );
            PeekMessage( &msg, wnd->hWnd, WM_QUIT, WM_QUIT, PM_REMOVE );
        }

        remove_window( wnd );
    }
}

void sgui_opengl_window_swap_buffers( sgui_window* window )
{
    if( window )
        SwapBuffers( ((sgui_window_w32*)window)->hDC );
}

void sgui_opengl_window_make_current( sgui_window* window )
{
    if( window )
        wglMakeCurrent( TO_W32(window)->hDC, TO_W32(window)->hRC );
    else
        wglMakeCurrent( NULL, NULL );
}

