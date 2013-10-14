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



static void w32_window_get_mouse_position( sgui_window* this, int* x, int* y )
{
    POINT pos = { 0, 0 };

    GetCursorPos( &pos );
    ScreenToClient( TO_W32(this)->hWnd, &pos );

    *x = pos.x;
    *y = pos.y;
}

static void w32_window_set_mouse_position( sgui_window* this, int x, int y )
{
    POINT pos;

    pos.x = x;
    pos.y = y;
    ClientToScreen( TO_W32(this)->hWnd, &pos );
    SetCursorPos( pos.x, pos.y );
}

static void w32_window_set_visible( sgui_window* this, int visible )
{
    ShowWindow( TO_W32(this)->hWnd, visible ? SW_SHOWNORMAL : SW_HIDE );
}

static void w32_window_set_title( sgui_window* this, const char* title )
{
    SetWindowTextA( TO_W32(this)->hWnd, title );
}

static void w32_window_set_size( sgui_window* this,
                                 unsigned int width, unsigned int height )
{
    RECT rcClient, rcWindow;
    POINT ptDiff;

    /* Determine the actual window size for the given client size */
    GetClientRect( TO_W32(this)->hWnd, &rcClient );
    GetWindowRect( TO_W32(this)->hWnd, &rcWindow );

    ptDiff.x = (rcWindow.right  - rcWindow.left) - rcClient.right;
    ptDiff.y = (rcWindow.bottom - rcWindow.top ) - rcClient.bottom;

    MoveWindow( TO_W32(this)->hWnd, rcWindow.left, rcWindow.top,
                (int)width + ptDiff.x, (int)height + ptDiff.y, TRUE );

    this->w = width;
    this->h = height;
}

static void w32_window_move_center( sgui_window* this )
{
    RECT desktop, window;
    int w, h, dw, dh;

    GetClientRect( GetDesktopWindow( ), &desktop );
    GetWindowRect( TO_W32(this)->hWnd,   &window  );

    w = window.right  - window.left;
    h = window.bottom - window.top;

    dw = desktop.right  - desktop.left;
    dh = desktop.bottom - desktop.top;

    MoveWindow( TO_W32(this)->hWnd, (dw>>1)-(w>>1),
                (dh>>1)-(h>>1), w, h, TRUE );
}

static void w32_window_move( sgui_window* this, int x, int y )
{
    RECT r;
    int w, h;

    GetWindowRect( TO_W32(this)->hWnd, &r );

    w = r.right  - r.left;
    h = r.bottom - r.top;

    MoveWindow( TO_W32(this)->hWnd, x, y, w, h, TRUE );
}

static void w32_window_destroy( sgui_window* this )
{
    MSG msg;

    if( this->canvas )
        sgui_canvas_destroy( this->canvas );

    if( this->backend==SGUI_OPENGL_COMPAT || this->backend==SGUI_OPENGL_CORE )
        destroy_gl_context( TO_W32(this) );

    if( TO_W32(this)->hWnd )
    {
        DestroyWindow( TO_W32(this)->hWnd );
        PeekMessage( &msg, TO_W32(this)->hWnd, WM_QUIT, WM_QUIT, PM_REMOVE );
    }

    remove_window( (sgui_window_w32*)this );
    free( this );
}

/****************************************************************************/

int handle_window_events( sgui_window_w32* this, UINT msg, WPARAM wp,
                          LPARAM lp )
{
    sgui_window* super = (sgui_window*)this;
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
        super->visible = 0;
        sgui_internal_window_fire_event(super, SGUI_USER_CLOSED_EVENT, NULL);
        break;
    case WM_MOUSEMOVE:
        e.mouse_move.x = LOWORD( lp );
        e.mouse_move.y = HIWORD( lp );
        sgui_internal_window_fire_event( super, SGUI_MOUSE_MOVE_EVENT, &e );
        break;
    case WM_MOUSEWHEEL:
        e.mouse_wheel.direction = GET_WHEEL_DELTA_WPARAM( wp )/120;
        sgui_internal_window_fire_event( super, SGUI_MOUSE_WHEEL_EVENT, &e );
        break;
    case WM_LBUTTONDOWN:
        e.mouse_press.button = SGUI_MOUSE_BUTTON_LEFT;
        e.mouse_press.x = LOWORD( lp );
        e.mouse_press.y = HIWORD( lp );
        sgui_internal_window_fire_event( super, SGUI_MOUSE_PRESS_EVENT, &e );
        break;
    case WM_LBUTTONUP:
        e.mouse_press.button = SGUI_MOUSE_BUTTON_LEFT;
        e.mouse_press.x = LOWORD( lp );
        e.mouse_press.y = HIWORD( lp );
        sgui_internal_window_fire_event( super, SGUI_MOUSE_RELEASE_EVENT, &e );
        break;
    case WM_MBUTTONDOWN:
        e.mouse_press.button = SGUI_MOUSE_BUTTON_MIDDLE;
        e.mouse_press.x = LOWORD( lp );
        e.mouse_press.y = HIWORD( lp );
        sgui_internal_window_fire_event( super, SGUI_MOUSE_PRESS_EVENT, &e );
        break;
    case WM_MBUTTONUP:
        e.mouse_press.button = SGUI_MOUSE_BUTTON_MIDDLE;
        e.mouse_press.x = LOWORD( lp );
        e.mouse_press.y = HIWORD( lp );
        sgui_internal_window_fire_event( super, SGUI_MOUSE_RELEASE_EVENT, &e );
        break;
    case WM_RBUTTONDOWN:
        e.mouse_press.button = SGUI_MOUSE_BUTTON_RIGHT;
        e.mouse_press.x = LOWORD( lp );
        e.mouse_press.y = HIWORD( lp );
        sgui_internal_window_fire_event( super, SGUI_MOUSE_PRESS_EVENT, &e );
        break;
    case WM_RBUTTONUP:
        e.mouse_press.button = SGUI_MOUSE_BUTTON_RIGHT;
        e.mouse_press.x = LOWORD( lp );
        e.mouse_press.y = HIWORD( lp );
        sgui_internal_window_fire_event(super, SGUI_MOUSE_RELEASE_EVENT, &e);
        break;
    case WM_CHAR:
        c[0] = (WCHAR)wp;
        c[1] = '\0';

        WideCharToMultiByte( CP_UTF8, 0, c, 2,
                             (LPSTR)e.char_event.as_utf8_str, 8, NULL, NULL );

        if( (e.char_event.as_utf8_str[0] & 0x80) ||
            !iscntrl( e.char_event.as_utf8_str[0] ) )
        {
            sgui_internal_window_fire_event( super, SGUI_CHAR_EVENT, &e );
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
            sgui_internal_window_fire_event(super,SGUI_KEY_PRESSED_EVENT, &e);
        else
            sgui_internal_window_fire_event(super,SGUI_KEY_RELEASED_EVENT,&e);

        /* let DefWindowProc handle system keys, except ALT */
        if( (msg==WM_SYSKEYUP || msg==WM_SYSKEYDOWN) &&
            !(key==VK_MENU || key==VK_LMENU || key==VK_RMENU) )
        {
            return -1;
        }
        break;
    case WM_SIZE:
        super->w = LOWORD( lp );
        super->h = HIWORD( lp );

        /* send size change event */
        e.size.new_width  = super->w;
        e.size.new_height = super->h;

        sgui_internal_window_fire_event( super, SGUI_SIZE_CHANGE_EVENT, &e );

        /* resize canvas and redraw everything */
        if( super->canvas )
        {
            sgui_canvas_resize( super->canvas, super->w, super->h );
            sgui_canvas_draw_widgets( super->canvas, 1 );
        }
        break;
    case WM_MOVE:
        super->x = LOWORD( lp );
        super->y = HIWORD( lp );
        break;
    case WM_PAINT:
        if( super->canvas )
        {
            num = sgui_canvas_num_dirty_rects( super->canvas );

            if( num )
                sgui_canvas_redraw_widgets( super->canvas, 1 );

            sgui_internal_window_fire_event( super, SGUI_EXPOSE_EVENT, &e );
            hDC = BeginPaint( this->hWnd, &ps );
            canvas_gdi_display( hDC,super->canvas, 0, 0, super->w, super->h );
            EndPaint( this->hWnd, &ps );
        }

        if( super->backend==SGUI_OPENGL_CORE ||
            super->backend==SGUI_OPENGL_COMPAT )
        {
            sgui_rect_set_size( &e.expose_event, 0, 0, super->w, super->h );
            sgui_internal_window_fire_event( super, SGUI_EXPOSE_EVENT, &e );
        }
    default:
        return -1;
    }

    /* invalidate all dirty rects of the canvas */
    if( super->canvas )
    {
        num = sgui_canvas_num_dirty_rects( super->canvas );

        for( i=0; i<num; ++i )
        {
            sgui_canvas_get_dirty_rect( super->canvas, &sr, i );

            SetRect( &r, sr.left, sr.top, sr.right+1, sr.bottom+1 );
            InvalidateRect( this->hWnd, &r, TRUE );
        }

        sgui_canvas_redraw_widgets( super->canvas, 1 );
    }

    return 0;
}

/****************************************************************************/

sgui_window* sgui_window_create_desc( sgui_window_description* desc )
{
    sgui_window_w32* this;
    sgui_window* super;
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
    this = malloc( sizeof(sgui_window_w32) );
    super = (sgui_window*)this;

    if( !this )
        return NULL;

    memset( this, 0, sizeof(sgui_window_w32) );

    add_window( this );

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

    this->hWnd = CreateWindowEx( 0, wndclass, "", style, 0, 0, r.right-r.left,
                                 r.bottom-r.top, parent_hnd, 0, hInstance, 0);

    if( !this->hWnd )
    {
        w32_window_destroy( (sgui_window*)this );
        return NULL;
    }

    SET_USER_PTR( this->hWnd, this );

    /**************************** create canvas ****************************/
    if( desc->backend==SGUI_OPENGL_CORE || desc->backend==SGUI_OPENGL_COMPAT )
    {
        if( !create_gl_context( this, desc ) )
        {
            w32_window_destroy( (sgui_window*)this );
            return NULL;
        }

        super->canvas = NULL;
        super->swap_buffers = gl_swap_buffers;
    }
    else
    {
        super->canvas = canvas_gdi_create( desc->width, desc->height );

        if( !super->canvas )
        {
            w32_window_destroy( (sgui_window*)this );
            return NULL;
        }
    }

    sgui_internal_window_post_init( (sgui_window*)this,
                                     desc->width, desc->height,
                                     desc->backend );

    /* store entry points */
    super->get_mouse_position = w32_window_get_mouse_position;
    super->set_mouse_position = w32_window_set_mouse_position;
    super->set_visible        = w32_window_set_visible;
    super->set_title          = w32_window_set_title;
    super->set_size           = w32_window_set_size;
    super->move_center        = w32_window_move_center;
    super->move               = w32_window_move;
    super->destroy            = w32_window_destroy;

    return (sgui_window*)this;
}

void sgui_window_make_current( sgui_window* this )
{
    if( this && (this->backend==SGUI_OPENGL_COMPAT ||
                 this->backend==SGUI_OPENGL_CORE) )
    {
        gl_make_current( TO_W32(this) );
    }
    else
        gl_make_current( NULL );
}

void sgui_window_set_vsync( sgui_window* this, int vsync_on )
{
    if( this && (this->backend==SGUI_OPENGL_COMPAT ||
                 this->backend==SGUI_OPENGL_CORE) )
    {
        gl_set_vsync( TO_W32(this), vsync_on );
    }
    else
        gl_make_current( NULL );
}

void sgui_window_get_platform_data( sgui_window* this,
                                    void* window, void* context )
{
    HWND* phWnd;
    HGLRC* phRC;

    if( this )
    {
        if( window )
        {
            phWnd = window;
            *phWnd = TO_W32(this)->hWnd;
        }

        if( context )
        {
        #ifndef SGUI_NO_OPENGL
            if( this->backend==SGUI_OPENGL_COMPAT ||
                this->backend==SGUI_OPENGL_CORE )
            {
                phRC = context;
                *phRC = TO_W32(this)->hRC;
            }
        #endif
        }
    }
}

