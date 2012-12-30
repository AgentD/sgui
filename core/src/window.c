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

#include <stddef.h>



void sgui_internal_window_fire_event( sgui_window* wnd, int event,
                                      sgui_event* e )
{
    if( wnd )
    {
        if( wnd->event_fun )
            wnd->event_fun( wnd, event, e );

        sgui_widget_manager_send_window_event( wnd->mgr, event, e );
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
        sgui_widget_manager_add_widget( wnd->mgr, widget );
}

void sgui_window_remove_widget( sgui_window* wnd, sgui_widget* widget )
{
    if( wnd )
        sgui_widget_manager_remove_widget( wnd->mgr, widget );
}

void sgui_window_on_widget_event( sgui_window* wnd,
                                  sgui_widget_callback fun, void* user )
{
    if( wnd )
        sgui_widget_manager_on_event( wnd->mgr, fun, user );
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

