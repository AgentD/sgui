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
#include "sgui_opengl.h"



static void w32_window_get_mouse_position( sgui_window* wnd, int* x, int* y )
{
    POINT pos = { 0, 0 };

    GetCursorPos( &pos );
    ScreenToClient( TO_W32(wnd)->hWnd, &pos );

    *x = pos.x;
    *y = pos.y;
}

static void w32_window_set_mouse_position( sgui_window* wnd, int x, int y )
{
    POINT pos;

    pos.x = x;
    pos.y = y;
    ClientToScreen( TO_W32(wnd)->hWnd, &pos );
    SetCursorPos( pos.x, pos.y );
}

static void w32_window_set_visible( sgui_window* wnd, int visible )
{
    ShowWindow( TO_W32(wnd)->hWnd, visible ? SW_SHOWNORMAL : SW_HIDE );
}

static void w32_window_set_title( sgui_window* wnd, const char* title )
{
    SetWindowTextA( TO_W32(wnd)->hWnd, title );
}

static void w32_window_set_size( sgui_window* wnd,
                                 unsigned int width, unsigned int height )
{
    RECT rcClient, rcWindow;
    POINT ptDiff;

    /* Determine the actual window size for the given client size */
    GetClientRect( TO_W32(wnd)->hWnd, &rcClient );
    GetWindowRect( TO_W32(wnd)->hWnd, &rcWindow );

    ptDiff.x = (rcWindow.right  - rcWindow.left) - rcClient.right;
    ptDiff.y = (rcWindow.bottom - rcWindow.top ) - rcClient.bottom;

    MoveWindow( TO_W32(wnd)->hWnd, rcWindow.left, rcWindow.top,
                (int)width + ptDiff.x, (int)height + ptDiff.y, TRUE );

    wnd->w = width;
    wnd->h = height;
}

static void w32_window_move_center( sgui_window* wnd )
{
    RECT desktop, window;
    int w, h, dw, dh;

    GetClientRect( GetDesktopWindow( ), &desktop );
    GetWindowRect( TO_W32(wnd)->hWnd,   &window  );

    w = window.right  - window.left;
    h = window.bottom - window.top;

    dw = desktop.right  - desktop.left;
    dh = desktop.bottom - desktop.top;

    MoveWindow( TO_W32(wnd)->hWnd, (dw>>1)-(w>>1),
                (dh>>1)-(h>>1), w, h, TRUE );
}

static void w32_window_move( sgui_window* wnd, int x, int y )
{
    RECT r;
    int w, h;

    GetWindowRect( TO_W32(wnd)->hWnd, &r );

    w = r.right  - r.left;
    h = r.bottom - r.top;

    MoveWindow( TO_W32(wnd)->hWnd, x, y, w, h, TRUE );
}

static void w32_window_destroy( sgui_window* wnd )
{
    MSG msg;

    sgui_internal_window_deinit( (sgui_window*)wnd );

    if(wnd->backend==SGUI_OPENGL_COMPAT || wnd->backend==SGUI_OPENGL_CORE)
        destroy_gl_context( TO_W32(wnd) );

    if( TO_W32(wnd)->hWnd )
    {
        DestroyWindow( TO_W32(wnd)->hWnd );
        PeekMessage(&msg, TO_W32(wnd)->hWnd, WM_QUIT, WM_QUIT, PM_REMOVE);
    }

    remove_window( (sgui_window_w32*)wnd );
    free( wnd );
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
        e.keyboard_event.code = key;

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

        if( wnd->base.backend==SGUI_NATIVE )
            sgui_canvas_draw_widgets( base->back_buffer, 1 );
        break;
    case WM_MOVE:
        base->x = LOWORD( lp );
        base->y = HIWORD( lp );
        break;
    case WM_PAINT:
        sgui_rect_set_size( &e.expose_event, 0, 0, base->w, base->h );

        if( wnd->base.backend==SGUI_NATIVE )
        {
            sgui_internal_window_fire_event( base, SGUI_EXPOSE_EVENT, &e );
            hDC = BeginPaint( wnd->hWnd, &ps );
            canvas_gdi_display(hDC,base->back_buffer,0,0,base->w,base->h);
            EndPaint( wnd->hWnd, &ps );
        }
        else if( wnd->base.backend==SGUI_OPENGL_CORE ||
                 wnd->base.backend==SGUI_OPENGL_COMPAT )
        {
            sgui_window_make_current( (sgui_window*)wnd );

            sgui_canvas_begin( wnd->base.back_buffer, NULL );
            sgui_canvas_clear( wnd->base.back_buffer, NULL );
            sgui_canvas_end( wnd->base.back_buffer );

            sgui_internal_window_fire_event( base, SGUI_EXPOSE_EVENT, &e );

            sgui_canvas_draw_widgets( wnd->base.back_buffer, 0 );
            sgui_window_swap_buffers( (sgui_window*)wnd );
            sgui_window_make_current( NULL );
        }
    default:
        return -1;
    }

    /* invalidate all dirty rects of the canvas */
    num = sgui_canvas_num_dirty_rects( base->back_buffer );

    if( wnd->base.backend==SGUI_NATIVE )
    {
        for( i=0; i<num; ++i )
        {
            sgui_canvas_get_dirty_rect( wnd->base.back_buffer, &sr, i );

            SetRect( &r, sr.left, sr.top, sr.right+1, sr.bottom+1 );
            InvalidateRect( wnd->hWnd, &r, TRUE );
        }

        sgui_canvas_redraw_widgets( wnd->base.back_buffer, 1 );
    }
    else if( wnd->base.backend==SGUI_OPENGL_CORE ||
             wnd->base.backend==SGUI_OPENGL_COMPAT )
    {
        if( num )
        {
            SetRect( &r, 0, 0, wnd->base.w-1, wnd->base.h-1 );
            InvalidateRect( wnd->hWnd, &r, TRUE );
            sgui_canvas_clear_dirty_rects( wnd->base.back_buffer );
        }
    }

    return 0;
}

/****************************************************************************/

sgui_window* sgui_window_create_desc( sgui_window_description* desc )
{
    sgui_window_w32* wnd;
    HWND parent_hnd = 0;
    DWORD style;
    RECT r;

    if( !desc || !desc->width || !desc->height )
        return NULL;

#ifdef SGUI_NO_OPENGL
    if( desc->backend==SGUI_OPENGL_CORE || desc->backend==SGUI_OPENGL_COMPAT )
        return NULL;
#endif

    /*************** allocate space for the window structure ***************/
    wnd = malloc( sizeof(sgui_window_w32) );

    if( !wnd )
        return NULL;

    memset( wnd, 0, sizeof(sgui_window_w32) );

    if( !sgui_internal_window_init( (sgui_window*)wnd ) )
    {
        free( wnd );
        return NULL;
    }

    add_window( wnd );

    /*************************** create a window ***************************/
    SetRect( &r, 0, 0, desc->width, desc->height );

    if( desc->parent )
    {
        parent_hnd = TO_W32(desc->parent)->hWnd;
        style = WS_CHILD;
    }
    else
    {
        style = desc->resizeable ? WS_OVERLAPPEDWINDOW :
                                   (WS_CAPTION | WS_SYSMENU);
        AdjustWindowRect( &r, style, FALSE );
    }

    style |= WS_CLIPCHILDREN | WS_CLIPSIBLINGS;

    wnd->hWnd = CreateWindowEx( 0, wndclass, "", style, 0, 0, r.right-r.left,
                                r.bottom-r.top, parent_hnd, 0, hInstance, 0 );

    if( !wnd->hWnd )
    {
        w32_window_destroy( (sgui_window*)wnd );
        return NULL;
    }

    SET_USER_PTR( wnd->hWnd, wnd );

    /**************************** create canvas ****************************/
    if( desc->backend==SGUI_OPENGL_CORE || desc->backend==SGUI_OPENGL_COMPAT )
    {
        if( !create_gl_context( wnd, desc->backend==SGUI_OPENGL_COMPAT ) )
        {
            w32_window_destroy( (sgui_window*)wnd );
            return NULL;
        }

        wnd->base.back_buffer = sgui_opengl_canvas_create( desc->width,
                                                           desc->height );
        wnd->base.swap_buffers = gl_swap_buffers;
    }
    else
    {
        wnd->base.back_buffer = canvas_gdi_create( desc->width,
                                                   desc->height );
    }

    if( !wnd->base.back_buffer )
    {
        w32_window_destroy( (sgui_window*)wnd );
        return NULL;
    }

    sgui_internal_window_post_init( (sgui_window*)wnd,
                                     desc->width, desc->height,
                                     desc->backend );

    /* store entry points */
    wnd->base.get_mouse_position = w32_window_get_mouse_position;
    wnd->base.set_mouse_position = w32_window_set_mouse_position;
    wnd->base.set_visible        = w32_window_set_visible;
    wnd->base.set_title          = w32_window_set_title;
    wnd->base.set_size           = w32_window_set_size;
    wnd->base.move_center        = w32_window_move_center;
    wnd->base.move               = w32_window_move;
    wnd->base.destroy            = w32_window_destroy;

    return (sgui_window*)wnd;
}

void sgui_window_make_current( sgui_window* wnd )
{
    if( wnd && (wnd->backend==SGUI_OPENGL_COMPAT ||
                wnd->backend==SGUI_OPENGL_CORE) )
    {
        gl_make_current( TO_W32(wnd) );
    }
    else
        gl_make_current( NULL );
}

