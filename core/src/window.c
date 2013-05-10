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
#include "sgui_internal.h"
#include "sgui_window.h"
#include "sgui_event.h"
#include "sgui_canvas.h"
#include "sgui_skin.h"
#include "sgui_widget.h"

#include <stddef.h>



int sgui_internal_window_init( sgui_window* window )
{
    (void)window;
    return 1;
}

void sgui_internal_window_post_init( sgui_window* window, unsigned int width,
                                     unsigned int height, int backend )
{
    unsigned char rgb[3];

    if( window )
    {
        window->w               = width;
        window->h               = height;
        window->backend         = backend;
        window->override_canvas = 0;

        sgui_skin_get_window_background_color( rgb );
        sgui_canvas_set_background_color( window->back_buffer, rgb );
        sgui_window_make_current( window );
        sgui_canvas_begin( window->back_buffer, NULL );
        sgui_canvas_clear( window->back_buffer, NULL );
        sgui_canvas_end( window->back_buffer );
        sgui_window_make_current( NULL );
    }
}

void sgui_internal_window_deinit( sgui_window* window )
{
    if( window )
    {
        sgui_canvas_destroy( window->back_buffer );
        window->back_buffer = NULL;
    }
}

void sgui_internal_window_fire_event( sgui_window* wnd, int event,
                                      sgui_event* e )
{
    if( wnd )
    {
        if( wnd->event_fun )
            wnd->event_fun( wnd, event, e );

        sgui_canvas_send_window_event( wnd->back_buffer, event, e );
    }
}

/****************************************************************************/

sgui_window* sgui_window_create( sgui_window* parent, unsigned int width,
                                 unsigned int height, int resizeable )
{
    sgui_window_description desc;

    desc.parent         = parent;
    desc.width          = width;
    desc.height         = height;
    desc.resizeable     = resizeable;
    desc.backend        = SGUI_NATIVE;
    desc.doublebuffer   = SGUI_DOUBLEBUFFERED;
    desc.bits_per_pixel = 32;
    desc.depth_bits     = 24;
    desc.stencil_bits   = 8;

    return sgui_window_create_desc( &desc );
}

void sgui_window_get_mouse_position( sgui_window* wnd, int* x, int* y )
{
    int mx = 0, my = 0;

    if( wnd )
    {
        wnd->get_mouse_position( wnd, &mx, &my );

        mx = mx<0 ? 0 : (mx>=(int)wnd->w ? ((int)wnd->w-1) : mx);
        my = my<0 ? 0 : (my>=(int)wnd->h ? ((int)wnd->h-1) : my);
    }

    if( x ) *x = mx;
    if( y ) *y = my;
}

void sgui_window_set_mouse_position( sgui_window* wnd, int x, int y,
                                     int send_event )
{
    sgui_event e;

    if( wnd && wnd->visible )
    {
        x = x<0 ? 0 : (x>=(int)wnd->w ? ((int)wnd->w-1) : x);
        y = y<0 ? 0 : (y>=(int)wnd->h ? ((int)wnd->h-1) : y);

        wnd->set_mouse_position( wnd, x, y );

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
    if( !wnd || (wnd->visible==visible) )
        return;

    wnd->set_visible( wnd, visible );
    wnd->visible = visible;

    if( !visible )
        sgui_internal_window_fire_event(wnd, SGUI_API_INVISIBLE_EVENT, NULL);
}

void sgui_window_set_title( sgui_window* wnd, const char* title )
{
    if( wnd && title )
        wnd->set_title( wnd, title );
}

void sgui_window_set_size( sgui_window* wnd,
                           unsigned int width, unsigned int height )
{
    if( wnd && width && height && (width!=wnd->w || height!=wnd->h) )
    {
        wnd->set_size( wnd, width, height );

        /* resize the canvas */
        sgui_canvas_resize( wnd->back_buffer, wnd->w, wnd->h );

        if( wnd->backend==SGUI_NATIVE && !wnd->override_canvas )
            sgui_canvas_draw_widgets( wnd->back_buffer, 1 );
    }
}

void sgui_window_move_center( sgui_window* wnd )
{
    if( wnd )
        wnd->move_center( wnd );
}

void sgui_window_move( sgui_window* wnd, int x, int y )
{
    if( wnd )
    {
        wnd->move( wnd, x, y );
        wnd->x = x;
        wnd->y = y;
    }
}

void sgui_window_swap_buffers( sgui_window* wnd )
{
    if( wnd && wnd->swap_buffers )
        wnd->swap_buffers( wnd );
}

void sgui_window_destroy( sgui_window* wnd )
{
    if( wnd )
    {
        sgui_internal_window_fire_event( wnd, SGUI_API_DESTROY_EVENT, NULL );

        wnd->destroy( wnd );
    }
}

/****************************************************************************/

int sgui_window_is_visible( sgui_window* wnd )
{
    return wnd ? wnd->visible : 0;
}

void sgui_window_get_position( sgui_window* wnd, int* x, int* y )
{
    if( wnd )
    {
        if( x ) *x = wnd->x;
        if( y ) *y = wnd->y;
    }
    else
    {
        if( x ) *x = 0;
        if( y ) *y = 0;
    }
}

void sgui_window_get_size( sgui_window* wnd, unsigned int* width,
                           unsigned int* height )
{
    if( wnd )
    {
        if( width  ) *width  = wnd->w;
        if( height ) *height = wnd->h;
    }
    else
    {
        if( width  ) *width  = 0;
        if( height ) *height = 0;
    }
}

void sgui_window_add_widget( sgui_window* wnd, sgui_widget* widget )
{
    if( wnd )
        sgui_widget_add_child(sgui_canvas_get_root(wnd->back_buffer), widget);
}

void sgui_window_on_widget_event( sgui_window* wnd,
                                  sgui_widget_callback fun, void* user )
{
    if( wnd )
        sgui_canvas_on_event( wnd->back_buffer, fun, user );
}

void sgui_window_on_event( sgui_window* wnd, sgui_window_callback fun )
{
    if( wnd )
        wnd->event_fun = fun;
}

sgui_canvas* sgui_window_get_canvas( sgui_window* wnd )
{
    return wnd ? wnd->back_buffer : NULL;
}

void sgui_window_set_userptr( sgui_window* wnd, void* ptr )
{
    if( wnd )
        wnd->userptr = ptr;
}

void* sgui_window_get_userptr( sgui_window* wnd )
{
    return wnd ? wnd->userptr : NULL;
}

void sgui_window_override_drawing( sgui_window* wnd, int override )
{
    if( wnd )
        wnd->override_canvas = override;
}

