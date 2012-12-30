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
#define SGUI_BUILDING_DLL
#include "internal.h"
#include "sgui_event.h"



void sgui_window_get_mouse_position( sgui_window* wnd, int* x, int* y )
{
    POINT pos = { 0, 0 };

    if( wnd )
    {
        GetCursorPos( &pos );
        ScreenToClient( TO_W32(wnd)->hWnd, &pos );
    }

    if( x ) *x = pos.x<0 ? 0 : (pos.x>=(int)wnd->w ? ((int)wnd->w-1) : pos.x);
    if( y ) *y = pos.y<0 ? 0 : (pos.y>=(int)wnd->h ? ((int)wnd->h-1) : pos.y);
}

void sgui_window_set_mouse_position( sgui_window* wnd, int x, int y,
                                     int send_event )
{
    POINT pos;
    sgui_event e;

    if( wnd && wnd->visible )
    {
        x = x<0 ? 0 : (x>=(int)wnd->w ? ((int)wnd->w-1) : x);
        y = y<0 ? 0 : (y>=(int)wnd->h ? ((int)wnd->h-1) : y);

        pos.x = x;
        pos.y = y;
        ClientToScreen( TO_W32(wnd)->hWnd, &pos );
        SetCursorPos( pos.x, pos.y );

        if( send_event )
        {
            e.mouse_move.x = x;
            e.mouse_move.y = y;
            sgui_internal_window_fire_event( wnd, SGUI_MOUSE_MOVE_EVENT, &e );
        }
    }
}

void sgui_window_set_visible( sgui_window* wnd, int visible )
{
    if( wnd && (wnd->visible!=visible) )
    {
        ShowWindow( TO_W32(wnd)->hWnd, visible ? SW_SHOWNORMAL : SW_HIDE );

        wnd->visible = visible;

        if( !visible )
            sgui_internal_window_fire_event( wnd, SGUI_API_INVISIBLE_EVENT,
                                             NULL );
    }
}

void sgui_window_set_title( sgui_window* wnd, const char* title )
{
    if( wnd && title )
        SetWindowTextA( TO_W32(wnd)->hWnd, title );
}

void sgui_window_set_size( sgui_window* wnd,
                           unsigned int width, unsigned int height )
{
    RECT rcClient, rcWindow;
    POINT ptDiff;

    if( !wnd || !width || !height )
        return;

    /* Determine the actual window size for the given client size */
    GetClientRect( TO_W32(wnd)->hWnd, &rcClient );
    GetWindowRect( TO_W32(wnd)->hWnd, &rcWindow );

    ptDiff.x = (rcWindow.right  - rcWindow.left) - rcClient.right;
    ptDiff.y = (rcWindow.bottom - rcWindow.top ) - rcClient.bottom;

    MoveWindow( TO_W32(wnd)->hWnd, rcWindow.left, rcWindow.top,
                (int)width + ptDiff.x, (int)height + ptDiff.y, TRUE );

    wnd->w = width;
    wnd->h = height;

    /* resize the canvas */
    sgui_canvas_resize( wnd->back_buffer, wnd->w, wnd->h );
    sgui_canvas_clear( wnd->back_buffer, NULL );
    sgui_widget_manager_draw_all( wnd->mgr, wnd->back_buffer );
}

void sgui_window_move_center( sgui_window* wnd )
{
    RECT desktop, window;
    int w, h, dw, dh;

    if( wnd )
    {
        GetClientRect( GetDesktopWindow( ), &desktop );
        GetWindowRect( TO_W32(wnd)->hWnd,   &window  );

        w = window.right  - window.left;
        h = window.bottom - window.top;

        dw = desktop.right  - desktop.left;
        dh = desktop.bottom - desktop.top;

        MoveWindow( TO_W32(wnd)->hWnd,
                    (dw>>1)-(w>>1), (dh>>1)-(h>>1), w, h, TRUE );
    }
}

void sgui_window_move( sgui_window* wnd, int x, int y )
{
    RECT outer, inner;
    int w, h, dx, dy;

    if( wnd )
    {
        GetClientRect( TO_W32(wnd)->hWnd, &inner );
        GetWindowRect( TO_W32(wnd)->hWnd, &outer );

        w = outer.right  - outer.left;
        h = outer.bottom - outer.top;

        dx = inner.left - outer.left;
        dy = inner.top  - outer.top;

        MoveWindow( TO_W32(wnd)->hWnd, x+dx, y+dy, w, h, TRUE );

        wnd->x = x;
        wnd->y = y;
    }
}

/****************************************************************************/

int handle_window_events(sgui_window_w32* wnd, UINT msg, WPARAM wp, LPARAM lp)
{
    sgui_window* base = (sgui_window*)wnd;
    unsigned int i, num;
    RECT r;
    sgui_rect sr;
    sgui_event e;
    PAINTSTRUCT ps;
    HDC hDC;
    WCHAR c[2];
    UINT key;

    switch( msg )
    {
    case WM_DESTROY:
        base->visible = 0;
        sgui_internal_window_fire_event( base, SGUI_USER_CLOSED_EVENT, NULL );
        break;
    case WM_MOUSEMOVE:
        e.mouse_move.x = LOWORD( lp );
        e.mouse_move.y = HIWORD( lp );
        sgui_internal_window_fire_event( base, SGUI_MOUSE_MOVE_EVENT, &e );
        break;
    case WM_MOUSEWHEEL:
        e.mouse_wheel.direction = GET_WHEEL_DELTA_WPARAM( wp )/120;
        sgui_internal_window_fire_event( base, SGUI_MOUSE_WHEEL_EVENT, &e );
        break;
    case WM_LBUTTONDOWN:
        e.mouse_press.button = SGUI_MOUSE_BUTTON_LEFT;
        e.mouse_press.x = LOWORD( lp );
        e.mouse_press.y = HIWORD( lp );
        sgui_internal_window_fire_event( base, SGUI_MOUSE_PRESS_EVENT, &e );
        break;
    case WM_LBUTTONUP:
        e.mouse_press.button = SGUI_MOUSE_BUTTON_LEFT;
        e.mouse_press.x = LOWORD( lp );
        e.mouse_press.y = HIWORD( lp );
        sgui_internal_window_fire_event( base, SGUI_MOUSE_RELEASE_EVENT, &e );
        break;
    case WM_MBUTTONDOWN:
        e.mouse_press.button = SGUI_MOUSE_BUTTON_MIDDLE;
        e.mouse_press.x = LOWORD( lp );
        e.mouse_press.y = HIWORD( lp );
        sgui_internal_window_fire_event( base, SGUI_MOUSE_PRESS_EVENT, &e );
        break;
    case WM_MBUTTONUP:
        e.mouse_press.button = SGUI_MOUSE_BUTTON_MIDDLE;
        e.mouse_press.x = LOWORD( lp );
        e.mouse_press.y = HIWORD( lp );
        sgui_internal_window_fire_event( base, SGUI_MOUSE_RELEASE_EVENT, &e );
        break;
    case WM_RBUTTONDOWN:
        e.mouse_press.button = SGUI_MOUSE_BUTTON_RIGHT;
        e.mouse_press.x = LOWORD( lp );
        e.mouse_press.y = HIWORD( lp );
        sgui_internal_window_fire_event( base, SGUI_MOUSE_PRESS_EVENT, &e );
        break;
    case WM_RBUTTONUP:
        e.mouse_press.button = SGUI_MOUSE_BUTTON_RIGHT;
        e.mouse_press.x = LOWORD( lp );
        e.mouse_press.y = HIWORD( lp );
        sgui_internal_window_fire_event( base, SGUI_MOUSE_RELEASE_EVENT, &e );
        break;
    case WM_CHAR:
        c[0] = (WCHAR)wp;
        c[1] = '\0';

        WideCharToMultiByte( CP_UTF8, 0, c, 2,
                             (LPSTR)e.char_event.as_utf8_str, 8, NULL, NULL );

        if( (e.char_event.as_utf8_str[0] & 0x80) ||
            !iscntrl( e.char_event.as_utf8_str[0] ) )
        {
            sgui_internal_window_fire_event( base, SGUI_CHAR_EVENT, &e );
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
            sgui_internal_window_fire_event(base, SGUI_KEY_PRESSED_EVENT, &e);
        else
            sgui_internal_window_fire_event(base, SGUI_KEY_RELEASED_EVENT,&e);

        /* let DefWindowProc handle system keys, except ALT */
        if( (msg==WM_SYSKEYUP || msg==WM_SYSKEYDOWN) &&
            !(key==VK_MENU || key==VK_LMENU || key==VK_RMENU) )
        {
            return -1;
        }
        break;
    case WM_SIZE:
        base->w = LOWORD( lp );
        base->h = HIWORD( lp );

        /* send size change event */
        e.size.new_width  = base->w;
        e.size.new_height = base->h;

        sgui_internal_window_fire_event( base, SGUI_SIZE_CHANGE_EVENT, &e );

        /* resize canvas and redraw everything */
        sgui_canvas_resize( base->back_buffer, base->w, base->h );
        sgui_canvas_clear( base->back_buffer, NULL );
        sgui_widget_manager_draw_all( base->mgr, base->back_buffer );
        break;
    case WM_MOVE:
        base->x = LOWORD( lp );
        base->y = HIWORD( lp );
        break;
    case WM_PAINT:
        if( base->back_buffer )
        {
            hDC = BeginPaint( wnd->hWnd, &ps );

            display_canvas( hDC, base->back_buffer, 0, 0, base->w, base->h );

            EndPaint( wnd->hWnd, &ps );
            break;
        }
    default:
        return -1;
    }

    /* invalidate all dirty rects of the widget manager */
    num = sgui_widget_manager_num_dirty_rects( base->mgr );

    for( i=0; i<num; ++i )
    {
        sgui_widget_manager_get_dirty_rect( wnd->base.mgr, &sr, i );

        SetRect( &r, sr.left, sr.top, sr.right+1, sr.bottom+1 );
        InvalidateRect( wnd->hWnd, &r, TRUE );
    }

    /* redraw dirty rects */
    sgui_widget_manager_draw( wnd->base.mgr, wnd->base.back_buffer );
    sgui_widget_manager_clear_dirty_rects( wnd->base.mgr );

    return 0;
}

/****************************************************************************/

sgui_window* sgui_window_create( unsigned int width, unsigned int height,
                                 int resizeable, int backend )
{
    sgui_window_w32* wnd;
    DWORD style;
    RECT r;
    unsigned char rgb[3];
#ifndef SGUI_NO_OPENGL
    HGLRC temp, oldctx;
    HDC olddc;
#endif

    if( !width || !height )
        return NULL;

#ifdef SGUI_NO_OPENGL
    if( backend==SGUI_OPENGL_CORE || backend==SGUI_OPENGL_COMPAT )
        return NULL;
#endif

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
    if( backend==SGUI_OPENGL_CORE || backend==SGUI_OPENGL_COMPAT )
    {
#ifndef SGUI_NO_OPENGL
        /* get a device context and set a pixel format */
        wnd->hDC = GetDC( wnd->hWnd );

        if( !wnd->hDC || !set_pixel_format( wnd->hDC ) )
        {
            sgui_window_destroy( (sgui_window*)wnd );
            return NULL;
        }

        /* create an old fashioned OpenGL temporary context */
        temp = wglCreateContext( wnd->hDC );

        if( !temp )
        {
            sgui_window_destroy( (sgui_window*)wnd );
            return NULL;
        }

        /* try to make it current */
        oldctx = wglGetCurrentContext( );
        olddc = wglGetCurrentDC( );

        if( !wglMakeCurrent( wnd->hDC, temp ) )
        {
            sgui_window_destroy( (sgui_window*)wnd );
            return NULL;
        }

        /* now try to create a new context */
        wnd->hRC = create_context( wnd->hDC, backend==SGUI_OPENGL_COMPAT );

        /* restore the privous context */
        wglMakeCurrent( olddc, oldctx );

        /* delete the temporary context on success, keep it on error */
        if( wnd->hRC )
            wglDeleteContext( temp );
        else
            wnd->hRC = temp;
#endif
    }
    else
    {
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
    }

    return (sgui_window*)wnd;
}

void sgui_window_swap_buffers( sgui_window* wnd )
{
#ifdef SGUI_NO_OPENGL
    (void)wnd;
#else
    if( wnd )
        SwapBuffers( TO_W32(wnd)->hDC );
#endif
}

void sgui_window_make_current( sgui_window* wnd )
{
#ifdef SGUI_NO_OPENGL
    (void)wnd;
#else
    if( wnd )
        wglMakeCurrent( TO_W32(wnd)->hDC, TO_W32(wnd)->hRC );
    else
        wglMakeCurrent( NULL, NULL );
#endif
}

void sgui_window_destroy( sgui_window* wnd )
{
    MSG msg;

    if( wnd )
    {
        sgui_internal_window_fire_event( wnd, SGUI_API_DESTROY_EVENT, NULL );

#ifndef SGUI_NO_OPENGL
        if( TO_W32(wnd)->hRC )
            wglDeleteContext( TO_W32(wnd)->hRC );

        if( TO_W32(wnd)->hDC )
            ReleaseDC( TO_W32(wnd)->hWnd, TO_W32(wnd)->hDC );
#endif

        if( TO_W32(wnd)->hWnd )
        {
            DestroyWindow( TO_W32(wnd)->hWnd );
            PeekMessage(&msg, TO_W32(wnd)->hWnd, WM_QUIT, WM_QUIT, PM_REMOVE);
        }

        if( wnd->back_buffer )
            sgui_canvas_destroy( wnd->back_buffer );

        remove_window( (sgui_window_w32*)wnd );
    }
}

