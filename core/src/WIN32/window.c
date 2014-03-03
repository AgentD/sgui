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



static void resize_pixmap( sgui_window_w32* this )
{
    sgui_window* super = (sgui_window*)this;

    /* adjust size in the header */
    this->info.bmiHeader.biWidth  = super->w;
    this->info.bmiHeader.biHeight = -((int)super->h);

    /* unbind the the dib section and delete it */
    SelectObject( this->hDC, 0 );
    DeleteObject( this->bitmap );

    /* create a new dib section */
    this->bitmap = CreateDIBSection( this->hDC, &this->info, DIB_RGB_COLORS,
                                     &this->data, 0, 0 );

    /* bind it */
    SelectObject( this->hDC, this->bitmap );

    /* tell the memory canvas about the new data pointer */
    sgui_memory_canvas_set_buffer( super->canvas, TO_W32(this)->data );
}

/****************************************************************************/

static void w32_window_get_mouse_position( sgui_window* this, int* x, int* y )
{
    POINT pos = { 0, 0 };

    sgui_internal_lock_mutex( );
    GetCursorPos( &pos );
    ScreenToClient( TO_W32(this)->hWnd, &pos );
    sgui_internal_unlock_mutex( );

    *x = pos.x;
    *y = pos.y;
}

static void w32_window_set_mouse_position( sgui_window* this, int x, int y )
{
    POINT pos;

    pos.x = x;
    pos.y = y;

    sgui_internal_lock_mutex( );
    ClientToScreen( TO_W32(this)->hWnd, &pos );
    SetCursorPos( pos.x, pos.y );
    sgui_internal_unlock_mutex( );
}

static void w32_window_set_visible( sgui_window* this, int visible )
{
    sgui_internal_lock_mutex( );
    ShowWindow( TO_W32(this)->hWnd, visible ? SW_SHOWNORMAL : SW_HIDE );
    sgui_internal_unlock_mutex( );
}

static void w32_window_set_title( sgui_window* this, const char* title )
{
    sgui_internal_lock_mutex( );
    SetWindowTextA( TO_W32(this)->hWnd, title );
    sgui_internal_unlock_mutex( );
}

static void w32_window_set_size( sgui_window* this,
                                 unsigned int width, unsigned int height )
{
    RECT rcClient, rcWindow;
    POINT ptDiff;

    sgui_internal_lock_mutex( );

    /* Determine the actual window size for the given client size */
    GetClientRect( TO_W32(this)->hWnd, &rcClient );
    GetWindowRect( TO_W32(this)->hWnd, &rcWindow );

    ptDiff.x = (rcWindow.right  - rcWindow.left) - rcClient.right;
    ptDiff.y = (rcWindow.bottom - rcWindow.top ) - rcClient.bottom;

    MoveWindow( TO_W32(this)->hWnd, rcWindow.left, rcWindow.top,
                (int)width + ptDiff.x, (int)height + ptDiff.y, TRUE );

    this->w = width;
    this->h = height;

    /* resize the canvas pixmap */
    if( this->backend == SGUI_NATIVE )
        resize_pixmap( TO_W32(this) );

    sgui_internal_unlock_mutex( );
}

static void w32_window_move_center( sgui_window* this )
{
    RECT desktop, window;
    int w, h, dw, dh;

    sgui_internal_lock_mutex( );

    GetClientRect( GetDesktopWindow( ), &desktop );
    GetWindowRect( TO_W32(this)->hWnd,   &window  );

    w = window.right  - window.left;
    h = window.bottom - window.top;

    dw = desktop.right  - desktop.left;
    dh = desktop.bottom - desktop.top;

    MoveWindow( TO_W32(this)->hWnd, (dw>>1)-(w>>1),
                (dh>>1)-(h>>1), w, h, TRUE );

    sgui_internal_unlock_mutex( );
}

static void w32_window_move( sgui_window* this, int x, int y )
{
    RECT r;
    int w, h;

    sgui_internal_lock_mutex( );

    GetWindowRect( TO_W32(this)->hWnd, &r );

    w = r.right  - r.left;
    h = r.bottom - r.top;

    MoveWindow( TO_W32(this)->hWnd, x, y, w, h, TRUE );

    sgui_internal_unlock_mutex( );
}

static void w32_window_force_redraw( sgui_window* this, sgui_rect* r )
{
    RECT r0;

    sgui_internal_lock_mutex( );
    SetRect( &r0, r->left, r->top, r->right+1, r->bottom+1 );
    InvalidateRect( TO_W32(this)->hWnd, &r0, TRUE );
    sgui_internal_unlock_mutex( );
}

static void w32_window_destroy( sgui_window* this )
{
    MSG msg;

    sgui_internal_lock_mutex( );

    if( this->canvas )
    {
        sgui_canvas_destroy( this->canvas );
        this->canvas = NULL;    /* HACK: DestroyWindow calls message proc */
    }

    if( this->backend==SGUI_OPENGL_COMPAT || this->backend==SGUI_OPENGL_CORE )
    {
        destroy_gl_context( TO_W32(this) );
    }
    else if( TO_W32(this)->hDC )
    {
        SelectObject( TO_W32(this)->hDC, 0 );

        if( TO_W32(this)->bitmap )
            DeleteObject( TO_W32(this)->bitmap );

        DeleteDC( TO_W32(this)->hDC );
    }

    if( TO_W32(this)->hWnd )
    {
        DestroyWindow( TO_W32(this)->hWnd );
        PeekMessage( &msg, TO_W32(this)->hWnd, WM_QUIT, WM_QUIT, PM_REMOVE );
    }

    DeleteObject( TO_W32(this)->bgbrush );

    remove_window( (sgui_window_w32*)this );
    sgui_internal_unlock_mutex( );

    free( this );
}

/****************************************************************************/

void update_window( sgui_window_w32* this )
{
    sgui_window* super = (sgui_window*)this;
    unsigned int i, num;
    sgui_rect sr;
    RECT r;

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
}

int handle_window_events( sgui_window_w32* this, UINT msg, WPARAM wp,
                          LPARAM lp )
{
    sgui_window* super = (sgui_window*)this;
    BLENDFUNCTION ftn;
    RECT r;
    sgui_event e;
    PAINTSTRUCT ps;
    HDC hDC;
    WCHAR c[2];
    UINT key;

    e.window = super;

    switch( msg )
    {
    case WM_DESTROY:
        super->visible = 0;
        e.type = SGUI_USER_CLOSED_EVENT;
        sgui_internal_window_fire_event( super, &e );
        break;
    case WM_MOUSEMOVE:
        e.arg.i2.x = LOWORD( lp );
        e.arg.i2.y = HIWORD( lp );
        e.type = SGUI_MOUSE_MOVE_EVENT;
        sgui_internal_window_fire_event( super, &e );
        break;
    case WM_MOUSEWHEEL:
        e.arg.i = GET_WHEEL_DELTA_WPARAM( wp )/120;
        e.type = SGUI_MOUSE_WHEEL_EVENT;
        sgui_internal_window_fire_event( super, &e );
        break;
    case WM_LBUTTONDOWN:
        e.arg.i3.z = SGUI_MOUSE_BUTTON_LEFT;
        e.arg.i3.x = LOWORD( lp );
        e.arg.i3.y = HIWORD( lp );
        e.type = SGUI_MOUSE_PRESS_EVENT;
        sgui_internal_window_fire_event( super, &e );
        break;
    case WM_LBUTTONUP:
        e.arg.i3.z = SGUI_MOUSE_BUTTON_LEFT;
        e.arg.i3.x = LOWORD( lp );
        e.arg.i3.y = HIWORD( lp );
        e.type = SGUI_MOUSE_RELEASE_EVENT;
        sgui_internal_window_fire_event( super, &e );
        break;
    case WM_MBUTTONDOWN:
        e.arg.i3.z = SGUI_MOUSE_BUTTON_MIDDLE;
        e.arg.i3.x = LOWORD( lp );
        e.arg.i3.y = HIWORD( lp );
        e.type = SGUI_MOUSE_PRESS_EVENT;
        sgui_internal_window_fire_event( super, &e );
        break;
    case WM_MBUTTONUP:
        e.arg.i3.z = SGUI_MOUSE_BUTTON_MIDDLE;
        e.arg.i3.x = LOWORD( lp );
        e.arg.i3.y = HIWORD( lp );
        e.type = SGUI_MOUSE_RELEASE_EVENT;
        sgui_internal_window_fire_event( super, &e );
        break;
    case WM_RBUTTONDOWN:
        e.arg.i3.z = SGUI_MOUSE_BUTTON_RIGHT;
        e.arg.i3.x = LOWORD( lp );
        e.arg.i3.y = HIWORD( lp );
        e.type = SGUI_MOUSE_PRESS_EVENT;
        sgui_internal_window_fire_event( super, &e );
        break;
    case WM_RBUTTONUP:
        e.arg.i3.z = SGUI_MOUSE_BUTTON_RIGHT;
        e.arg.i3.x = LOWORD( lp );
        e.arg.i3.y = HIWORD( lp );
        e.type = SGUI_MOUSE_RELEASE_EVENT;
        sgui_internal_window_fire_event( super, &e );
        break;
    case WM_CHAR:
        c[0] = (WCHAR)wp;
        c[1] = '\0';

        WideCharToMultiByte( CP_UTF8, 0, c, 2, (LPSTR)e.arg.utf8, 8,
                             NULL, NULL );

        if( (e.arg.utf8[0] & 0x80) || !iscntrl( e.arg.utf8[0] ) )
        {
            e.type = SGUI_CHAR_EVENT;
            sgui_internal_window_fire_event( super, &e );
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
        if( msg==WM_KEYDOWN || msg==WM_SYSKEYDOWN )
            e.type = SGUI_KEY_PRESSED_EVENT;
        else
            e.type = SGUI_KEY_RELEASED_EVENT;

        e.arg.i = key;
        sgui_internal_window_fire_event( super, &e );

        /* let DefWindowProc handle system keys, except ALT */
        if( (msg==WM_SYSKEYUP || msg==WM_SYSKEYDOWN) &&
            !(key==VK_MENU || key==VK_LMENU || key==VK_RMENU) )
        {
            return -1;
        }
        break;
    case WM_SIZE:
        /* send size change event */
        e.arg.ui2.x = super->w = LOWORD( lp );
        e.arg.ui2.y = super->h = HIWORD( lp );
        e.type = SGUI_SIZE_CHANGE_EVENT;

        /* resize canvas and redraw everything */
        if( super->backend==SGUI_NATIVE )
        {
            resize_pixmap( this );
            sgui_canvas_resize( super->canvas, super->w, super->h );
        }

        /* fire a resize event */
        sgui_internal_window_fire_event( super, &e );

        /* redraw the widgets */
        if( super->canvas )
            sgui_canvas_draw_widgets( super->canvas, 1 );
        break;
    case WM_MOVE:
        super->x = LOWORD( lp );
        super->y = HIWORD( lp );
        break;
    case WM_PAINT:
        if( super->canvas )
        {
            e.type = SGUI_EXPOSE_EVENT;
            sgui_internal_window_fire_event( super, &e );

            ftn.BlendOp = AC_SRC_OVER;
            ftn.BlendFlags = 0;
            ftn.SourceConstantAlpha = 0xFF;
            ftn.AlphaFormat = AC_SRC_ALPHA;

            r.left = r.top = 0;
            r.right = super->w;
            r.bottom = super->h;

            hDC = BeginPaint( this->hWnd, &ps );
            FillRect( hDC, &r, this->bgbrush );
            AlphaBlend( hDC, 0, 0, super->w, super->h, this->hDC,
                        0, 0, super->w, super->h, ftn );
            EndPaint( this->hWnd, &ps );
        }

        if( super->backend==SGUI_OPENGL_CORE ||
            super->backend==SGUI_OPENGL_COMPAT )
        {
            e.type = SGUI_EXPOSE_EVENT;
            sgui_rect_set_size( &e.arg.rect, 0, 0, super->w, super->h );
            sgui_internal_window_fire_event( super, &e );
        }
    default:
        return -1;
    }

    return 0;
}

/****************************************************************************/

sgui_window* sgui_window_create_desc( const sgui_window_description* desc )
{
    sgui_window_w32* this;
    sgui_window* super;
    unsigned char color[4];
    HWND parent_hnd = 0;
    DWORD style;
    RECT r;

    if( !desc || !desc->width || !desc->height )
        return NULL;

    /*************** allocate space for the window structure ***************/
    this = malloc( sizeof(sgui_window_w32) );
    super = (sgui_window*)this;

    if( !this )
        return NULL;

    sgui_internal_lock_mutex( );
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
        goto failure;

    SET_USER_PTR( this->hWnd, this );

    /**************************** create canvas ****************************/
    if( desc->backend==SGUI_OPENGL_CORE || desc->backend==SGUI_OPENGL_COMPAT )
    {
        if( !create_gl_context( this, desc ) )
            goto failure;

        super->canvas = NULL;
        super->swap_buffers = gl_swap_buffers;
    }
    else
    {
        /* create an offscreen Device Context */
        if( !(this->hDC = CreateCompatibleDC( NULL )) )
            goto failure;

        /*fill the bitmap header */
        this->info.bmiHeader.biSize        = sizeof(this->info.bmiHeader);
        this->info.bmiHeader.biBitCount    = 32;
        this->info.bmiHeader.biCompression = BI_RGB;
        this->info.bmiHeader.biPlanes      = 1;
        this->info.bmiHeader.biWidth       = desc->width;
        this->info.bmiHeader.biHeight      = -((int)desc->height);

        /* create a DIB section = bitmap with accessable data pointer */
        this->bitmap = CreateDIBSection( this->hDC, &this->info,
                                         DIB_RGB_COLORS, &this->data, 0, 0 );

        if( !this->bitmap )
            goto failure;

        /* bind the dib section to the offscreen context */
        SelectObject( this->hDC, this->bitmap );

        super->canvas = sgui_memory_canvas_create( this->data,
                                                   desc->width, desc->height,
                                                   SGUI_RGBA8, 1 );

        if( !super->canvas )
            goto failure;
    }

    sgui_internal_window_post_init( (sgui_window*)this,
                                     desc->width, desc->height,
                                     desc->backend );

    sgui_skin_get_window_background_color( color );
    this->bgbrush = CreateSolidBrush( RGB(color[0],color[1],color[2]) );

    /* store entry points */
    super->get_mouse_position = w32_window_get_mouse_position;
    super->set_mouse_position = w32_window_set_mouse_position;
    super->set_visible        = w32_window_set_visible;
    super->set_title          = w32_window_set_title;
    super->set_size           = w32_window_set_size;
    super->move_center        = w32_window_move_center;
    super->move               = w32_window_move;
    super->force_redraw       = w32_window_force_redraw;
    super->destroy            = w32_window_destroy;
    super->write_clipboard    = w32_window_write_clipboard;
    super->read_clipboard     = w32_window_read_clipboard;

    sgui_internal_unlock_mutex( );
    return (sgui_window*)this;
failure:
    sgui_internal_unlock_mutex( );
    w32_window_destroy( (sgui_window*)this );
    return NULL;
}

void sgui_window_make_current( sgui_window* this )
{
    sgui_internal_lock_mutex( );

    if( this && (this->backend==SGUI_OPENGL_COMPAT ||
                 this->backend==SGUI_OPENGL_CORE) )
    {
        gl_make_current( TO_W32(this) );
    }
    else
        gl_make_current( NULL );

    sgui_internal_unlock_mutex( );
}

void sgui_window_set_vsync( sgui_window* this, int vsync_on )
{
    sgui_internal_lock_mutex( );

    if( this && (this->backend==SGUI_OPENGL_COMPAT ||
                 this->backend==SGUI_OPENGL_CORE) )
    {
        gl_set_vsync( TO_W32(this), vsync_on );
    }
    else
        gl_make_current( NULL );

    sgui_internal_unlock_mutex( );
}

void sgui_window_get_platform_data( const sgui_window* this,
                                    void* window, void* context )
{
    HWND* phWnd;
#ifndef SGUI_NO_OPENGL
    HGLRC* phRC;
#endif

    if( this )
    {
        if( window )
        {
            phWnd = window;
            *phWnd = TO_W32(this)->hWnd;
        }

    #ifndef SGUI_NO_OPENGL
        if( context )
        {
            if( this->backend==SGUI_OPENGL_COMPAT ||
                this->backend==SGUI_OPENGL_CORE )
            {
                phRC = context;
                *phRC = TO_W32(this)->hRC;
            }
        }
    #else
        (void)context;
    #endif
    }
}

