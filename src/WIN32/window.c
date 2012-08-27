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




LRESULT CALLBACK WindowProcFun( HWND hWnd, UINT msg, WPARAM wp, LPARAM lp )
{
    sgui_window* wnd;
    sgui_event e;
    PAINTSTRUCT ps;
    HDC hDC;
    WCHAR c[2];
    UINT key;
    void* data;
    unsigned char rgb[3];

    wnd = (sgui_window*)GET_USER_PTR( hWnd );

    if( !wnd )
        return DefWindowProc( hWnd, msg, wp, lp );

    switch( msg )
    {
    case WM_DESTROY:
        wnd->visible = 0;
        SEND_EVENT( wnd, SGUI_USER_CLOSED_EVENT, NULL );
        break;
    case WM_MOUSEMOVE:
        e.mouse_move.x = LOWORD( lp );
        e.mouse_move.y = HIWORD( lp );
        SEND_EVENT( wnd, SGUI_MOUSE_MOVE_EVENT, &e );
        break;
    case WM_MOUSEWHEEL:
        e.mouse_wheel.direction = GET_WHEEL_DELTA_WPARAM( wp )/120;
        SEND_EVENT( wnd, SGUI_MOUSE_WHEEL_EVENT, &e );
        break;
    case WM_LBUTTONDOWN:
        e.mouse_press.button = SGUI_MOUSE_BUTTON_LEFT;
        e.mouse_press.x = LOWORD( lp );
        e.mouse_press.y = HIWORD( lp );
        SEND_EVENT( wnd, SGUI_MOUSE_PRESS_EVENT, &e );
        break;
    case WM_LBUTTONUP:
        e.mouse_press.button = SGUI_MOUSE_BUTTON_LEFT;
        e.mouse_press.x = LOWORD( lp );
        e.mouse_press.y = HIWORD( lp );
        SEND_EVENT( wnd, SGUI_MOUSE_RELEASE_EVENT, &e );
        break;
    case WM_MBUTTONDOWN:
        e.mouse_press.button = SGUI_MOUSE_BUTTON_MIDDLE;
        e.mouse_press.x = LOWORD( lp );
        e.mouse_press.y = HIWORD( lp );
        SEND_EVENT( wnd, SGUI_MOUSE_PRESS_EVENT, &e );
        break;
    case WM_MBUTTONUP:
        e.mouse_press.button = SGUI_MOUSE_BUTTON_MIDDLE;
        e.mouse_press.x = LOWORD( lp );
        e.mouse_press.y = HIWORD( lp );
        SEND_EVENT( wnd, SGUI_MOUSE_RELEASE_EVENT, &e );
        break;
    case WM_RBUTTONDOWN:
        e.mouse_press.button = SGUI_MOUSE_BUTTON_RIGHT;
        e.mouse_press.x = LOWORD( lp );
        e.mouse_press.y = HIWORD( lp );
        SEND_EVENT( wnd, SGUI_MOUSE_PRESS_EVENT, &e );
        break;
    case WM_RBUTTONUP:
        e.mouse_press.button = SGUI_MOUSE_BUTTON_RIGHT;
        e.mouse_press.x = LOWORD( lp );
        e.mouse_press.y = HIWORD( lp );
        SEND_EVENT( wnd, SGUI_MOUSE_RELEASE_EVENT, &e );
        break;
    case WM_CHAR:
        c[0] = (WCHAR)wp;
        c[1] = '\0';

        WideCharToMultiByte( CP_UTF8, 0, c, 2,
                             (LPSTR)e.char_event.as_utf8_str, 8, NULL, NULL );

        if( (e.char_event.as_utf8_str[0] & 0x80) ||
            !iscntrl( e.char_event.as_utf8_str[0] ) )
        {
            SEND_EVENT( wnd, SGUI_CHAR_EVENT, &e );
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
        {
            SEND_EVENT( wnd, SGUI_KEY_PRESSED_EVENT, &e );
        }
        else
        {
            SEND_EVENT( wnd, SGUI_KEY_RELEASED_EVENT, &e );
        }

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

        /* resize the double buffering context/bitmap */
        wnd->info.bmiHeader.biWidth  = wnd->w;
        wnd->info.bmiHeader.biHeight = -((int)wnd->h);

        SelectObject( wnd->dc, wnd->old_bitmap );
        DeleteObject( wnd->bitmap );
        DeleteDC( wnd->dc );

        wnd->dc = CreateCompatibleDC( NULL );
        wnd->bitmap = CreateDIBSection( wnd->dc, &wnd->info, DIB_RGB_COLORS,
                                        (void**)&data, 0, 0 );
        wnd->old_bitmap = (HBITMAP)SelectObject( wnd->dc, wnd->bitmap );

        sgui_canvas_set_raw_data( wnd->back_buffer, SCF_BGRA8, wnd->w, wnd->h,
                                  data );

        sgui_skin_get_window_background_color( rgb );

        sgui_canvas_draw_box( wnd->back_buffer, 0, 0, wnd->w, wnd->h,
                              rgb, SCF_RGB8 );

        /* send size change event */
        e.size.new_width  = wnd->w;
        e.size.new_height = wnd->h;

        SEND_EVENT( wnd, SGUI_SIZE_CHANGE_EVENT, &e );

        /* redraw everything */
        SEND_EVENT( wnd, SGUI_DRAW_EVENT, NULL );
        break;
    case WM_PAINT:
        hDC = BeginPaint( hWnd, &ps );

        BitBlt( hDC, 0, 0, wnd->w+1, wnd->h, wnd->dc, 0, 0, SRCCOPY );

        EndPaint( hWnd, &ps );
        break;
    default:
        return DefWindowProc( hWnd, msg, wp, lp );
    }

    return 0;
}





sgui_window* sgui_window_create( unsigned int width, unsigned int height,
                                 int resizeable )
{
    sgui_window* wnd;
    WNDCLASSEX wc;
    DWORD style;
    RECT r;
    unsigned char rgb[3];
    void* data;

    if( !width || !height )
        return NULL;

    /* allocate space for the window structure */
    wnd = malloc( sizeof(sgui_window) );

    if( !wnd )
        return NULL;

    memset( wnd, 0, sizeof(sgui_window) );

    /* create a widget manager */
    wnd->mgr = sgui_widget_manager_create( );

    if( !wnd->mgr )
    {
        free( wnd );
        return NULL;
    }

    /* get HINSTANCE, window style and actual window size */
    wnd->hInstance = GetModuleHandle( NULL );

    style = resizeable ? WS_OVERLAPPEDWINDOW : (WS_CAPTION | WS_SYSMENU);

    SetRect( &r, 0, 0, width, height );
    AdjustWindowRect( &r, style, FALSE );

    /* setup and register a window class if not done yet */
    if( !GetClassInfoEx( wnd->hInstance, "sgui_wnd_class", &wc ) )
    {
        memset( &wc, 0, sizeof(WNDCLASSEX) );

        wc.cbSize        = sizeof(WNDCLASSEX);
        wc.style         = CS_HREDRAW | CS_VREDRAW;
        wc.lpfnWndProc   = WindowProcFun;
        wc.hInstance     = wnd->hInstance;
        wc.lpszClassName = "sgui_wnd_class";
        wc.hCursor       = LoadCursor( NULL, IDC_ARROW );

        if( RegisterClassEx( &wc ) == 0 )
        {
            sgui_window_destroy( wnd );
            return NULL;
        }
    }

    /* create a window */
    wnd->hWnd = CreateWindowEx( 0, "sgui_wnd_class", "", style, 0, 0,
                                r.right-r.left, r.bottom-r.top,
                                0, 0, wnd->hInstance, NULL );

    if( !wnd->hWnd )
    {
        sgui_window_destroy( wnd );
        return NULL;
    }

    SET_USER_PTR( wnd->hWnd, wnd );

    wnd->w = width;
    wnd->h = height;

    /* create double buffering context/bitmap */
    wnd->info.bmiHeader.biSize        = sizeof(wnd->info.bmiHeader);
    wnd->info.bmiHeader.biBitCount    = 32;
    wnd->info.bmiHeader.biCompression = BI_RGB;
    wnd->info.bmiHeader.biPlanes      = 1;
    wnd->info.bmiHeader.biWidth       = wnd->w;
    wnd->info.bmiHeader.biHeight      = -((int)wnd->h);

    wnd->dc = CreateCompatibleDC( NULL );
    wnd->bitmap = CreateDIBSection( wnd->dc, &wnd->info, DIB_RGB_COLORS,
                                    (void**)&data, 0, 0 );
    wnd->old_bitmap = (HBITMAP)SelectObject( wnd->dc, wnd->bitmap );


    wnd->back_buffer = sgui_canvas_create_use_buffer( data, wnd->w, wnd->h,
                                                      SCF_BGRA8 );

    if( !wnd->back_buffer )
    {
        sgui_window_destroy( wnd );
        return NULL;
    }

    sgui_skin_get_window_background_color( rgb );

    sgui_canvas_draw_box( wnd->back_buffer, 0, 0, wnd->w, wnd->h,
                          rgb, SCF_RGB8 );

    return wnd;
}

void sgui_window_destroy( sgui_window* wnd )
{
    MSG msg;

    if( wnd )
    {
        SEND_EVENT( wnd, SGUI_API_DESTROY_EVENT, NULL );

        if( wnd->dc )
        {
            SelectObject( wnd->dc, wnd->old_bitmap );
            DeleteObject( wnd->bitmap );
            DeleteDC( wnd->dc );
        }

        if( wnd->hWnd )
        {
            DestroyWindow( wnd->hWnd );
            PeekMessage( &msg, wnd->hWnd, WM_QUIT, WM_QUIT, PM_REMOVE );
        }

        if( wnd->back_buffer )
            sgui_canvas_destroy( wnd->back_buffer );

        sgui_widget_manager_destroy( wnd->mgr );

        free( wnd );
    }
}

void sgui_window_get_mouse_position( sgui_window* wnd, int* x, int* y )
{
    POINT pos;

    if( wnd )
    {
        GetCursorPos( &pos );
        ScreenToClient( wnd->hWnd, &pos );

        if( x )
            *x = pos.x<0 ? 0 : (pos.x>=(int)wnd->w ? ((int)wnd->w-1) : pos.x);

        if( y )
            *y = pos.y<0 ? 0 : (pos.y>=(int)wnd->h ? ((int)wnd->h-1) : pos.y);
    }
    else
    {
        if( x ) *x = 0;
        if( y ) *y = 0;
    }
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
        ClientToScreen( wnd->hWnd, &pos );
        SetCursorPos( pos.x, pos.y );

        if( send_event )
        {
            e.mouse_move.x = x;
            e.mouse_move.y = y;
            SEND_EVENT( wnd, SGUI_MOUSE_MOVE_EVENT, &e );
        }
    }
}

void sgui_window_set_visible( sgui_window* wnd, int visible )
{
    if( wnd )
    {
        ShowWindow( wnd->hWnd, visible ? SW_SHOWNORMAL : SW_HIDE );
        wnd->visible = visible;

        if( !visible )
            SEND_EVENT( wnd, SGUI_API_INVISIBLE_EVENT, NULL );
    }
}

int sgui_window_is_visible( sgui_window* wnd )
{
    return wnd ? wnd->visible : 0;
}

void sgui_window_set_title( sgui_window* wnd, const char* title )
{
    if( wnd )
        SetWindowTextA( wnd->hWnd, title );
}

void sgui_window_set_size( sgui_window* wnd,
                           unsigned int width, unsigned int height )
{
    RECT rcClient, rcWindow;
    POINT ptDiff;
    unsigned char rgb[3];
    void* data;

    if( wnd )
    {
        /* Resize the client rect of the window */
        GetClientRect( wnd->hWnd, &rcClient );
        GetWindowRect( wnd->hWnd, &rcWindow );

        ptDiff.x = (rcWindow.right  - rcWindow.left) - rcClient.right;
        ptDiff.y = (rcWindow.bottom - rcWindow.top ) - rcClient.bottom;

        MoveWindow( wnd->hWnd, rcWindow.left, rcWindow.top,
                    (int)width + ptDiff.x, (int)height + ptDiff.y, TRUE );

        wnd->w = width;
        wnd->h = height;

        /* resize the double buffering context/bitmap */
        wnd->info.bmiHeader.biWidth  = wnd->w;
        wnd->info.bmiHeader.biHeight = -((int)wnd->h);

        SelectObject( wnd->dc, wnd->old_bitmap );
        DeleteObject( wnd->bitmap );
        DeleteDC( wnd->dc );

        wnd->dc = CreateCompatibleDC( NULL );
        wnd->bitmap = CreateDIBSection( wnd->dc, &wnd->info, DIB_RGB_COLORS,
                                        (void**)&data, 0, 0 );
        wnd->old_bitmap = (HBITMAP)SelectObject( wnd->dc, wnd->bitmap );

        sgui_canvas_set_raw_data( wnd->back_buffer, SCF_BGRA8, wnd->w, wnd->h,
                                  data );

        sgui_skin_get_window_background_color( rgb );

        sgui_canvas_draw_box( wnd->back_buffer, 0, 0, wnd->w, wnd->h,
                              rgb, SCF_RGB8 );

        /* redraw everything */
        SEND_EVENT( wnd, SGUI_DRAW_EVENT, NULL );
    }
}

void sgui_window_get_size( sgui_window* wnd,
                           unsigned int* width, unsigned int* height )
{
    if( wnd )
    {
        if( width  ) *width  = wnd->w;
        if( height ) *height = wnd->h;
    }
}

void sgui_window_move_center( sgui_window* wnd )
{
    RECT desktop, window;
    int w, h, dw, dh;

    if( wnd )
    {
        GetClientRect( GetDesktopWindow( ), &desktop );
        GetWindowRect( wnd->hWnd,           &window );

        w = window.right  - window.left;
        h = window.bottom - window.top;

        dw = desktop.right  - desktop.left;
        dh = desktop.bottom - desktop.top;

        MoveWindow( wnd->hWnd, (dw>>1)-(w>>1), (dh>>1)-(h>>1), w, h, TRUE );
    }
}

void sgui_window_move( sgui_window* wnd, int x, int y )
{
    RECT outer, inner;
    int w, h, dx, dy;

    if( wnd )
    {
        GetClientRect( wnd->hWnd, &inner );
        GetWindowRect( wnd->hWnd, &outer );

        w = outer.right  - outer.left;
        h = outer.bottom - outer.top;

        dx = inner.left - outer.left;
        dy = inner.top  - outer.top;

        MoveWindow( wnd->hWnd, x+dx, y+dy, w, h, TRUE );
    }
}

void sgui_window_get_position( sgui_window* wnd, int* x, int* y )
{
    RECT r;

    if( wnd )
    {
        GetClientRect( wnd->hWnd, &r );

        if( x ) *x = r.left;
        if( y ) *y = r.top;
    }
}

int sgui_window_update( sgui_window* wnd )
{
    MSG msg;
    RECT r;

    if( !wnd || !wnd->visible )
        return 0;

    /* update the widgets, redraw window if there was any change */
    if( sgui_widget_manager_update( wnd->mgr, wnd ) )
    {
        SetRect( &r, 0, 0, wnd->w-1, wnd->h-1 );
        InvalidateRect( wnd->hWnd, &r, TRUE );
    }

    /* message loop */
    while( PeekMessage( &msg, wnd->hWnd, 0, 0, PM_REMOVE ) )
    {
        TranslateMessage( &msg );
        DispatchMessage( &msg );
    }

    return 1;
}

void sgui_window_on_event( sgui_window* wnd, sgui_window_callback fun )
{
    if( wnd )
        wnd->event_fun = fun;
}


void sgui_window_add_widget( sgui_window* wnd, sgui_widget* widget )
{
    if( wnd )
        sgui_widget_manager_add_widget( wnd->mgr, widget );
}

void sgui_window_remove_widget( sgui_window* wnd, sgui_widget* widget )
{
    if( wnd )
        sgui_widget_manager_remove_widget( wnd->mgr, widget );
}



sgui_canvas* sgui_window_get_canvas( sgui_window* wnd )
{
    return wnd ? wnd->back_buffer : NULL;
}

void sgui_window_blend_image( sgui_window* wnd, int x, int y,
                              unsigned int width, unsigned int height,
                              unsigned char* image )
{
    if( wnd )
        sgui_canvas_blend( wnd->back_buffer, x, y, width, height,
                           SCF_RGBA8, image );
}

