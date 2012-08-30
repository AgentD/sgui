/*
 * frame.c
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
#include "sgui_frame.h"
#include "sgui_widget_manager.h"
#include "sgui_skin.h"
#include "widget_internal.h"

#include <stdlib.h>



typedef struct
{
    sgui_widget widget;

    sgui_widget_manager* mgr;
}
sgui_frame;



void frame_on_event( sgui_widget* widget, int type, sgui_event* event )
{
    sgui_frame* f = (sgui_frame*)widget;

    if( type==SGUI_FOCUS_EVENT || type==SGUI_FOCUS_LOSE_EVENT ||
        type==SGUI_MOUSE_ENTER_EVENT || type==SGUI_MOUSE_LEAVE_EVENT )
        return;

    if( type==SGUI_MOUSE_MOVE_EVENT && event )
    {
        event->mouse_move.x -= widget->x;
        event->mouse_move.y -= widget->y;
    }
    else if( (type==SGUI_MOUSE_PRESS_EVENT || type==SGUI_MOUSE_RELEASE_EVENT)
             && event )
    {
        event->mouse_press.x -= widget->x;
        event->mouse_press.y -= widget->y;
    }

    sgui_widget_manager_send_window_event( f->mgr, type, event );
}

void frame_update( sgui_widget* widget )
{
    sgui_frame* f = (sgui_frame*)widget;

    widget->need_redraw = sgui_widget_manager_update( f->mgr );
}

void frame_draw( sgui_widget* widget, sgui_canvas* cv )
{
    sgui_frame* f = (sgui_frame*)widget;

    sgui_skin_draw_frame( cv, widget->x, widget->y,
                              widget->width, widget->height );

    sgui_canvas_set_offset( cv, widget->x, widget->y );
    sgui_canvas_set_scissor_rect(cv, 1, 1, widget->width-2, widget->height-2);

    sgui_widget_manager_force_draw( f->mgr, cv, 0, 0,
                                    widget->width, widget->height );

    sgui_canvas_set_scissor_rect( cv, 0, 0, 0, 0 );
    sgui_canvas_restore_offset( cv );
}



sgui_widget* sgui_frame_create( int x, int y, unsigned int width,
                                unsigned int height )
{
    sgui_frame* f = malloc( sizeof(sgui_frame) );

    sgui_internal_widget_init( (sgui_widget*)f, x, y, width, height, 0 );

    f->widget.draw_callback         = frame_draw;
    f->widget.update_callback       = frame_update;
    f->widget.window_event_callback = frame_on_event;
    f->mgr                          = sgui_widget_manager_create( );

    sgui_widget_manager_enable_clear( f->mgr, 0 );

    return (sgui_widget*)f;
}

void sgui_frame_destroy( sgui_widget* frame )
{
    sgui_frame* f = (sgui_frame*)frame;

    if( f )
    {
        sgui_internal_widget_deinit( frame );
        sgui_widget_manager_destroy( f->mgr );
        free( f );
    }
}

void sgui_frame_add_widget( sgui_widget* frame, sgui_widget* w )
{
    sgui_frame* f = (sgui_frame*)frame;

    if( f )
        sgui_widget_manager_add_widget( f->mgr, w );
}

void sgui_frame_remove_widget( sgui_widget* frame, sgui_widget* w )
{
    sgui_frame* f = (sgui_frame*)frame;

    if( f )
        sgui_widget_manager_remove_widget( f->mgr, w );
}

