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
#define SGUI_BUILDING_DLL
#include "sgui_frame.h"
#include "sgui_skin.h"
#include "sgui_scroll_bar.h"
#include "sgui_event.h"
#include "sgui_canvas.h"
#include "sgui_widget.h"
#include "sgui_internal.h"
#include "sgui_widget.h"

#include <stdlib.h>
#include <string.h>


typedef struct
{
    sgui_widget widget;

    sgui_widget* v_bar;         /* vertical scroll bar */

    /*
        border width, distance to vertical scroll bar and whether to draw
        the vertical scroll bar
     */
    int border, v_bar_dist;
}
sgui_frame;



static void frame_on_scroll( void* userptr, int new_offset, int delta )
{
    sgui_widget* frame = userptr;
    sgui_widget* i;
    int x, y;
    (void)new_offset;

    for( i=frame->children; i!=NULL; i=i->next )
    {
        if( i!=((sgui_frame*)frame)->v_bar )
        {
            sgui_widget_get_position( i, &x, &y );
            y -= delta;
            sgui_widget_set_position( i, x, y );
        }
    }
}

static void frame_on_event( sgui_widget* widget, int type, sgui_event* event )
{
    sgui_frame* f = (sgui_frame*)widget;
    int offset, old_offset, x, y, delta;
    sgui_rect r;
    sgui_widget* i;

    /* "manually" scroll the frame on mouse wheel */
    if( type==SGUI_MOUSE_WHEEL_EVENT )
    {
        offset = sgui_scroll_bar_get_offset( f->v_bar );
        old_offset = offset;

        /* apply a scroll of a quarter frame height */
        if( event->mouse_wheel.direction > 0 )
            delta = -SGUI_RECT_HEIGHT(widget->area) / 4;
        else
            delta = SGUI_RECT_HEIGHT(widget->area) / 4;

        offset += delta;

        if( offset < 0 )
            offset = 0;

        sgui_scroll_bar_set_offset( f->v_bar, offset );
        offset = sgui_scroll_bar_get_offset( f->v_bar );

        if( offset==old_offset )
            return;

        for( i=widget->children; i!=NULL; i=i->next )
        {
            if( i!=f->v_bar )
            {
                sgui_widget_get_position( i, &x, &y );
                y -= delta;
                sgui_widget_set_position( i, x, y );
            }
        }

        sgui_widget_get_absolute_rect( f->v_bar, &r );
        sgui_canvas_add_dirty_rect( widget->canvas, &r );
    }
}

static void frame_draw( sgui_widget* widget )
{
    sgui_skin_draw_frame( widget->canvas, widget->area.left, widget->area.top,
                          SGUI_RECT_WIDTH(widget->area),
                          SGUI_RECT_HEIGHT(widget->area) );
}

static void frame_destroy( sgui_widget* frame )
{
    sgui_frame* f = (sgui_frame*)frame;

    sgui_widget_destroy( f->v_bar );
    free( f );
}

static void frame_on_state_change( sgui_widget* frame, int change )
{
    sgui_frame* f = (sgui_frame*)frame;
    unsigned int ww, wh, height, new_height;
    sgui_widget* i;
    int wx, wy;

    if( change & (WIDGET_CHILD_ADDED|WIDGET_CHILD_REMOVED) )
    {
        height = SGUI_RECT_HEIGHT( frame->area );
        new_height = 0;

        /* determine the required frame height */
        for( i=frame->children; i!=NULL; i=i->next )
        {
            if( i==f->v_bar )
                continue;

            sgui_widget_get_position( i, &wx, &wy );
            sgui_widget_get_size( i, &ww, &wh );

            new_height = MAX( new_height, wy+wh );
        }

        /* draw the vertical scroll bar if the required hight is larger */
        if( new_height > height )
        {
            sgui_widget_set_visible( f->v_bar, 1 );
            sgui_scroll_bar_set_area( f->v_bar, new_height+10, height );
        }
        else
        {
            sgui_widget_set_visible( f->v_bar, 0 );
        }
    }
}

/****************************************************************************/

sgui_widget* sgui_frame_create( int x, int y, unsigned int width,
                                unsigned int height )
{
    unsigned int w, h, bw, bh;
    sgui_frame* f = malloc( sizeof(sgui_frame) );

    if( !f )
        return NULL;

    sgui_internal_widget_init( (sgui_widget*)f, x, y, width, height );

    /* try to create a scroll bar */
    sgui_skin_get_scroll_bar_extents( 0, height, &w, &h, &bw, &bh );

    f->border = sgui_skin_get_frame_border_width( );
    f->v_bar_dist = width - w - f->border;
    f->v_bar = sgui_scroll_bar_create( f->v_bar_dist, f->border, 0,
                                       height-2*f->border, height-2*f->border,
                                       height-2*f->border );

    if( !f->v_bar )
    {
        free( f );
        return NULL;
    }

    sgui_scroll_bar_on_scroll( f->v_bar, frame_on_scroll, f );
    sgui_widget_set_visible( f->v_bar, 0 );
    sgui_widget_add_child( (sgui_widget*)f, f->v_bar );

    /* finish initialisation */
    f->widget.draw_callback         = frame_draw;
    f->widget.window_event_callback = frame_on_event;
    f->widget.state_change_callback = frame_on_state_change;
    f->widget.destroy               = frame_destroy;

    return (sgui_widget*)f;
}

