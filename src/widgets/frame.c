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
#include "sgui_scroll_bar.h"

#include "widget_internal.h"

#include <stdlib.h>
#include <string.h>


typedef struct
{
    sgui_widget widget;

    sgui_widget_manager* mgr;   /* nested widget manager */
    sgui_widget* v_bar;         /* vertical scroll bar */

    /*
        border width, distance to vertical scroll bar,
        and whether to draw the vertical scroll bar
     */
    int border, v_bar_dist, draw_v_bar;

    /* "virtual" mouse position within the frame */
    int mouse_x, mouse_y;
}
sgui_frame;



void frame_on_event( sgui_widget* widget, int type, sgui_event* event )
{
    sgui_frame* f = (sgui_frame*)widget;
    int send_v_bar = 0;
    int offset = sgui_scroll_bar_get_offset( f->v_bar );
    sgui_event e;

    /* "manually" scroll the frame on mouse wheel */
    if( type==SGUI_MOUSE_WHEEL_EVENT && event )
    {
        f->mouse_y -= offset;

        if( event->mouse_wheel.direction > 0 )
            offset -= widget->height / 4;
        else
            offset += widget->height / 4;

        if( offset < 0 )
            offset = 0;

        sgui_scroll_bar_set_offset( f->v_bar, offset );

        /* update "virtual" mouse position */
        f->mouse_y += sgui_scroll_bar_get_offset( f->v_bar );

        /* generate a mouse move event with the new mouse position */
        e.mouse_move.x = f->mouse_x;
        e.mouse_move.y = f->mouse_y;

        sgui_widget_manager_send_window_event( f->mgr, SGUI_MOUSE_MOVE_EVENT,
                                               &e );

        widget->need_redraw = 1;
        return;
    }

    /* adjust the mouse position for mouse events */
    if( type==SGUI_MOUSE_MOVE_EVENT && event )
    {
        if( event->mouse_move.x >= f->v_bar_dist && f->draw_v_bar )
        {
            /* redirect to scrollbar */
            send_v_bar = 1;
            event->mouse_move.x -= f->v_bar_dist;
        }
        else
        {
            event->mouse_move.y += offset;
            f->mouse_x = event->mouse_move.x;
            f->mouse_y = event->mouse_move.y;
        }
    }
    else if( (type==SGUI_MOUSE_PRESS_EVENT || type==SGUI_MOUSE_RELEASE_EVENT)
             && event )
    {
        if( event->mouse_press.x >= f->v_bar_dist && f->draw_v_bar )
        {
            /* redirect to scrollbar */
            send_v_bar = 1;
            event->mouse_move.x -= f->v_bar_dist;
        }
        else
        {
            event->mouse_press.y += offset;
            f->mouse_x = event->mouse_move.x;
            f->mouse_y = event->mouse_move.y;
        }
    }
    else if( type==SGUI_MOUSE_LEAVE_EVENT ||
             type==SGUI_FOCUS_LOSE_EVENT )
    {
        sgui_widget_send_window_event( f->v_bar, type, event );
    }

    /* send the event to the frames widgets */
    if( send_v_bar )
        sgui_widget_send_window_event( f->v_bar, type, event );
    else
        sgui_widget_manager_send_window_event( f->mgr, type, event );
}

void frame_update( sgui_widget* widget )
{
    sgui_frame* f = (sgui_frame*)widget;

    sgui_widget_manager_update( f->mgr );

    widget->need_redraw = sgui_widget_manager_num_dirty_rects( f->mgr );

    widget->need_redraw |= sgui_widget_need_redraw( f->v_bar, 0 );

    sgui_widget_manager_clear_dirty_rects( f->mgr );
}

void frame_draw( sgui_widget* widget, sgui_canvas* cv )
{
    sgui_frame* f = (sgui_frame*)widget;
    int offset, clear;

    offset = sgui_scroll_bar_get_offset( f->v_bar );
    clear  = sgui_canvas_is_clear_allowed( cv );

    /* draw background */
    sgui_skin_draw_frame( cv, widget->x, widget->y,
                              widget->width, widget->height );

    sgui_canvas_allow_clear( cv, 0 );

    /* adjust offset and scissor rect to frame area */
    sgui_canvas_set_offset( cv, widget->x, widget->y );
    sgui_canvas_set_scissor_rect(cv, f->border, f->border,
                                     widget->width-2*f->border,
                                     widget->height-2*f->border );

    /* apply scrolling offset */
    sgui_canvas_set_offset( cv, 0, -offset );

    /* draw the widgets */
    sgui_widget_manager_force_draw( f->mgr, cv, 0, offset,
                                    widget->width, widget->height );

    /* draw the scroll bar "manually" at fixed position */
    sgui_canvas_restore_offset( cv );

    if( f->draw_v_bar )
        sgui_widget_draw( f->v_bar, cv );

    /* restore canvas state */
    sgui_canvas_set_scissor_rect( cv, 0, 0, 0, 0 );
    sgui_canvas_restore_offset( cv );
    sgui_canvas_allow_clear( cv, clear );
}



sgui_widget* sgui_frame_create( int x, int y, unsigned int width,
                                unsigned int height )
{
    unsigned int w, h, bw, bh;
    sgui_frame* f = malloc( sizeof(sgui_frame) );

    if( !f )
        return NULL;

    memset( f, 0, sizeof(sgui_frame) );

    f->mgr = sgui_widget_manager_create( );

    if( !f->mgr )
    {
        free( f );
        return NULL;
    }

    sgui_skin_get_scroll_bar_extents( 0, height, &w, &h, &bw, &bh );

    sgui_internal_widget_init( (sgui_widget*)f, x, y, width, height, 0 );

    f->widget.draw_callback         = frame_draw;
    f->widget.update_callback       = frame_update;
    f->widget.window_event_callback = frame_on_event;
    f->border                       = sgui_skin_get_frame_border_width( );
    f->v_bar_dist                   = width - w - f->border;

    f->v_bar = sgui_scroll_bar_create( f->v_bar_dist, f->border, 0,
                                       height-2*f->border, height, height );

    if( !f->v_bar )
    {
        sgui_internal_widget_deinit( (sgui_widget*)f );
        sgui_widget_manager_destroy( f->mgr );
        free( f );
        return NULL;
    }

    return (sgui_widget*)f;
}

void sgui_frame_destroy( sgui_widget* frame )
{
    sgui_frame* f = (sgui_frame*)frame;

    if( f )
    {
        sgui_scroll_bar_destroy( f->v_bar );
        sgui_internal_widget_deinit( frame );
        sgui_widget_manager_destroy( f->mgr );
        free( f );
    }
}

void sgui_frame_add_widget( sgui_widget* frame, sgui_widget* w )
{
    sgui_frame* f = (sgui_frame*)frame;
    unsigned int ww, wh;
    int wx, wy;

    if( f && w )
    {
        sgui_widget_manager_add_widget( f->mgr, w );

        /* check if the widget lies outside the displayed frame area */
        sgui_widget_get_position( w, &wx, &wy );
        sgui_widget_get_size( w, &ww, &wh );

        /* if so, draw the vertical scroll bar */
        if( ((unsigned int)wy+wh) > (frame->height-2*f->border) )
        {
            f->draw_v_bar = 1;
            sgui_scroll_bar_set_area( f->v_bar, wy+wh+10, frame->height );
        }
    }
}

void sgui_frame_remove_widget( sgui_widget* frame, sgui_widget* w )
{
    sgui_frame* f = (sgui_frame*)frame;

    if( f )
        sgui_widget_manager_remove_widget( f->mgr, w );
}

