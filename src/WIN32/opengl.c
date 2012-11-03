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



void set_pixel_format( HDC hDC )
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
    SetPixelFormat( hDC, format, &pfd );
}



sgui_window* sgui_opengl_window_create( unsigned int width,
                                        unsigned int height, int resizeable )
{
    sgui_window_w32* wnd;
    DWORD style;
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

    wnd->hDC = GetDC( wnd->hWnd );
    set_pixel_format( wnd->hDC );

    wnd->hRC = wglCreateContext( wnd->hDC );
    wglMakeCurrent( wnd->hDC, wnd->hRC );

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
        {
            wglMakeCurrent( NULL, NULL );
            wglMakeCurrent( wnd->hDC, wnd->hRC );
        }

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

