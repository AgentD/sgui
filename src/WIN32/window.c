/*
 * window.c
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



#ifndef MAPVK_VSC_TO_VK_EX
    #define MAPVK_VSC_TO_VK_EX 3
#endif

#if defined(__amd64__) || defined(__amd64) || defined(__x86_64__) ||\
    defined(__x86_64) || defined(_M_X64) || defined(_M_AMD64)

    #define SET_USER_PTR( hwnd, ptr )\
            SetWindowLongPtr( hwnd, GWLP_USERDATA, (LONG_PTR)ptr )

    #define GET_USER_PTR( hwnd ) GetWindowLongPtr( hwnd, GWLP_USERDATA )
#else
    #define SET_USER_PTR( hwnd, ptr )\
            SetWindowLong( hwnd, GWL_USERDATA, (LONG)ptr )

    #define GET_USER_PTR( hwnd ) GetWindowLong( hwnd, GWL_USERDATA )
#endif

#define TO_W32( window ) ((sgui_window_w32*)window)



LRESULT CALLBACK WindowProcFun( HWND hWnd, UINT msg, WPARAM wp, LPARAM lp )
{
    sgui_window* wnd;
    sgui_event e;
    PAINTSTRUCT ps;
    HDC hDC;
    WCHAR c[2];
    UINT key;

    wnd = (sgui_window*)GET_USER_PTR( hWnd );

    if( !wnd )
        return DefWindowProc( hWnd, msg, wp, lp );

    switch( msg )
    {
    case WM_DESTROY:
        wnd->visible = 0;
        sgui_internal_window_fire_event( wnd, SGUI_USER_CLOSED_EVENT, NULL );
        break;
    case WM_MOUSEMOVE:
        e.mouse_move.x = LOWORD( lp );
        e.mouse_move.y = HIWORD( lp );
        sgui_internal_window_fire_event( wnd, SGUI_MOUSE_MOVE_EVENT, &e );
        break;
    case WM_MOUSEWHEEL:
        e.mouse_wheel.direction = GET_WHEEL_DELTA_WPARAM( wp )/120;
        sgui_internal_window_fire_event( wnd, SGUI_MOUSE_WHEEL_EVENT, &e );
        break;
    case WM_LBUTTONDOWN:
        e.mouse_press.button = SGUI_MOUSE_BUTTON_LEFT;
        e.mouse_press.x = LOWORD( lp );
        e.mouse_press.y = HIWORD( lp );
        sgui_internal_window_fire_event( wnd, SGUI_MOUSE_PRESS_EVENT, &e );
        break;
    case WM_LBUTTONUP:
        e.mouse_press.button = SGUI_MOUSE_BUTTON_LEFT;
        e.mouse_press.x = LOWORD( lp );
        e.mouse_press.y = HIWORD( lp );
        sgui_internal_window_fire_event( wnd, SGUI_MOUSE_RELEASE_EVENT, &e );
        break;
    case WM_MBUTTONDOWN:
        e.mouse_press.button = SGUI_MOUSE_BUTTON_MIDDLE;
        e.mouse_press.x = LOWORD( lp );
        e.mouse_press.y = HIWORD( lp );
        sgui_internal_window_fire_event( wnd, SGUI_MOUSE_PRESS_EVENT, &e );
        break;
    case WM_MBUTTONUP:
        e.mouse_press.button = SGUI_MOUSE_BUTTON_MIDDLE;
        e.mouse_press.x = LOWORD( lp );
        e.mouse_press.y = HIWORD( lp );
        sgui_internal_window_fire_event( wnd, SGUI_MOUSE_RELEASE_EVENT, &e );
        break;
    case WM_RBUTTONDOWN:
        e.mouse_press.button = SGUI_MOUSE_BUTTON_RIGHT;
        e.mouse_press.x = LOWORD( lp );
        e.mouse_press.y = HIWORD( lp );
        sgui_internal_window_fire_event( wnd, SGUI_MOUSE_PRESS_EVENT, &e );
        break;
    case WM_RBUTTONUP:
        e.mouse_press.button = SGUI_MOUSE_BUTTON_RIGHT;
        e.mouse_press.x = LOWORD( lp );
        e.mouse_press.y = HIWORD( lp );
        sgui_internal_window_fire_event( wnd, SGUI_MOUSE_RELEASE_EVENT, &e );
        break;
    case WM_CHAR:
        c[0] = (WCHAR)wp;
        c[1] = '\0';

        WideCharToMultiByte( CP_UTF8, 0, c, 2,
                             (LPSTR)e.char_event.as_utf8_str, 8, NULL, NULL );

        if( (e.char_event.as_utf8_str[0] & 0x80) ||
            !iscntrl( e.char_event.as_utf8_str[0] ) )
        {
            sgui_internal_window_fire_event( wnd, SGUI_CHAR_EVENT, &e );
        }
        break;
    case WM_SYSKEYUP:
    case WM_SYSKEYDOWN:
    case WM_KEYDOWN:
    case WM_KEYUP:
        key = (UINT)wp;

        if( key==VK_SHIFT || key==VK_CONTROL || key==VK_MENU )
            key = MapVirtualKey( (lp>>16) & 0xFF, MAPVK_VSC_TO_VK_EX );

        if( (lp & 0x1000000) && (key==VK_CONTROL) )
            key = VK_RCONTROL;

        if( (lp & 0x1000000) && (key==VK_MENU) )
            key = VK_RMENU;

        /* Send event */
        e.keyboard_event.code = (SGUI_KEY_CODE)key;

        if( msg==WM_KEYDOWN || msg==WM_SYSKEYDOWN )
            sgui_internal_window_fire_event(wnd, SGUI_KEY_PRESSED_EVENT, &e);
        else
            sgui_internal_window_fire_event(wnd, SGUI_KEY_RELEASED_EVENT, &e);

        /* let DefWindowProc handle system keys, except ALT */
        if( (msg==WM_SYSKEYUP || msg==WM_SYSKEYDOWN) &&
            !(key==VK_MENU || key==VK_LMENU || key==VK_RMENU) )
        {
            return DefWindowProc( hWnd, msg, wp, lp );
        }
        break;
    case WM_SIZE:
        wnd->w = LOWORD( lp );
        wnd->h = HIWORD( lp );

        /* send size change event */
        e.size.new_width  = wnd->w;
        e.size.new_height = wnd->h;

        sgui_internal_window_fire_event( wnd, SGUI_SIZE_CHANGE_EVENT, &e );

        /* resize canvas and redraw everything */
        sgui_canvas_resize( (sgui_canvas_gdi*)wnd->back_buffer,
                            wnd->w, wnd->h );
        sgui_canvas_clear( wnd->back_buffer, NULL );
        sgui_widget_manager_draw_all( wnd->mgr, wnd->back_buffer );
        break;
    case WM_MOVE:
        wnd->x = LOWORD( lp );
        wnd->y = HIWORD( lp );
        break;
    case WM_PAINT:
        hDC = BeginPaint( hWnd, &ps );

        BitBlt( hDC, 0, 0, wnd->w, wnd->h,
                ((sgui_canvas_gdi*)wnd->back_buffer)->dc, 0, 0, SRCCOPY );

        EndPaint( hWnd, &ps );
        break;
    default:
        return DefWindowProc( hWnd, msg, wp, lp );
    }

    return 0;
}

/****************************************************************************/

void window_w32_get_mouse_position( sgui_window* wnd, int* x, int* y )
{
    POINT pos;

    GetCursorPos( &pos );
    ScreenToClient( TO_W32(wnd)->hWnd, &pos );

    *x = pos.x;
    *y = pos.y;
}

void window_w32_set_mouse_position( sgui_window* wnd, int x, int y )
{
    POINT pos = { x, y };

    ClientToScreen( TO_W32(wnd)->hWnd, &pos );
    SetCursorPos( pos.x, pos.y );
}

void window_w32_set_visible( sgui_window* wnd, int visible )
{
    ShowWindow( TO_W32(wnd)->hWnd, visible ? SW_SHOWNORMAL : SW_HIDE );
}

void window_w32_set_title( sgui_window* wnd, const char* title )
{
    SetWindowTextA( TO_W32(wnd)->hWnd, title );
}

void window_w32_set_size( sgui_window* wnd,
                          unsigned int width, unsigned int height )
{
    RECT rcClient, rcWindow;
    POINT ptDiff;

    /* Resize the client rect of the window */
    GetClientRect( TO_W32(wnd)->hWnd, &rcClient );
    GetWindowRect( TO_W32(wnd)->hWnd, &rcWindow );

    ptDiff.x = (rcWindow.right  - rcWindow.left) - rcClient.right;
    ptDiff.y = (rcWindow.bottom - rcWindow.top ) - rcClient.bottom;

    MoveWindow( TO_W32(wnd)->hWnd, rcWindow.left, rcWindow.top,
                (int)width + ptDiff.x, (int)height + ptDiff.y, TRUE );

    wnd->w = width;
    wnd->h = height;

    /* resize the canvas */
    sgui_canvas_resize( (sgui_canvas_gdi*)wnd->back_buffer, wnd->w, wnd->h );
}

void window_w32_move_center( sgui_window* wnd )
{
    RECT desktop, window;
    int w, h, dw, dh;

    GetClientRect( GetDesktopWindow( ), &desktop );
    GetWindowRect( TO_W32(wnd)->hWnd,   &window  );

    w = window.right  - window.left;
    h = window.bottom - window.top;

    dw = desktop.right  - desktop.left;
    dh = desktop.bottom - desktop.top;

    MoveWindow( TO_W32(wnd)->hWnd,
                (dw>>1)-(w>>1), (dh>>1)-(h>>1), w, h, TRUE );
}

void window_w32_move( sgui_window* wnd, int x, int y )
{
    RECT outer, inner;
    int w, h, dx, dy;

    GetClientRect( TO_W32(wnd)->hWnd, &inner );
    GetWindowRect( TO_W32(wnd)->hWnd, &outer );

    w = outer.right  - outer.left;
    h = outer.bottom - outer.top;

    dx = inner.left - outer.left;
    dy = inner.top  - outer.top;

    MoveWindow( TO_W32(wnd)->hWnd, x+dx, y+dy, w, h, TRUE );
}

/****************************************************************************/

void update_window( sgui_window_w32* wnd )
{
    unsigned int i, num;
    MSG msg;
    RECT r;
    sgui_rect sr;

    /* update the widgets, redraw window if there was any change */
    sgui_widget_manager_update( wnd->base.mgr );

    num = sgui_widget_manager_num_dirty_rects( wnd->base.mgr );

    for( i=0; i<num; ++i )
    {
        sgui_widget_manager_get_dirty_rect( wnd->base.mgr, &sr, i );

        SetRect( &r, sr.left, sr.top, sr.right, sr.bottom );
        InvalidateRect( wnd->hWnd, &r, TRUE );
    }

    sgui_widget_manager_draw( wnd->base.mgr, wnd->base.back_buffer );
    sgui_widget_manager_clear_dirty_rects( wnd->base.mgr );

    /* message loop */
    while( PeekMessage( &msg, wnd->hWnd, 0, 0, PM_REMOVE ) )
    {
        TranslateMessage( &msg );
        DispatchMessage( &msg );
    }
}

/****************************************************************************/

sgui_window* sgui_window_create( unsigned int width, unsigned int height,
                                 int resizeable )
{
    sgui_window_w32* wnd;
    DWORD style;
    RECT r;
    unsigned char rgb[3];

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
        sgui_window_destroy( (sgui_window*)wnd );
        return NULL;
    }

    SET_USER_PTR( wnd->hWnd, wnd );

    wnd->base.w = width;
    wnd->base.h = height;

    /**************************** create canvas ****************************/
    wnd->base.back_buffer =
    (sgui_canvas*)sgui_canvas_create( wnd->base.w, wnd->base.h );

    if( !wnd->base.back_buffer )
    {
        sgui_window_destroy( (sgui_window*)wnd );
        return NULL;
    }

    sgui_skin_get_window_background_color( rgb );
    sgui_canvas_set_background_color( wnd->base.back_buffer, rgb );

    sgui_canvas_clear( wnd->base.back_buffer, NULL );

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

void sgui_window_destroy( sgui_window* wnd )
{
    MSG msg;

    if( wnd )
    {
        sgui_internal_window_fire_event( wnd, SGUI_API_DESTROY_EVENT, NULL );

        if( TO_W32(wnd)->hWnd )
        {
            DestroyWindow( TO_W32(wnd)->hWnd );
            PeekMessage(&msg, TO_W32(wnd)->hWnd, WM_QUIT, WM_QUIT, PM_REMOVE);
        }

        if( wnd->back_buffer )
            sgui_canvas_destroy( (sgui_canvas_gdi*)wnd->back_buffer );

        remove_window( (sgui_window_w32*)wnd );
    }
}

