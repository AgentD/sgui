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
    sgui_widget* h_bar;         /* horizontal scroll bar */

    /*
        border width, distance to vertical scroll bar, distance to
        horizontal scrollbar and whether to override scroll bar drawing
     */
    int v_border, h_border, v_bar_dist, h_bar_dist, override_scrollbars;
}
sgui_frame;



static void frame_on_scroll_v( void* userptr, int new_offset, int delta )
{
    sgui_widget* frame = userptr;
    sgui_widget* i;
    sgui_rect r;
    int x, y;
    (void)new_offset;

    sgui_widget_get_absolute_rect( frame, &r );
    sgui_canvas_add_dirty_rect( frame->canvas, &r );

    for( i=frame->children; i!=NULL; i=i->next )
    {
        if( i!=((sgui_frame*)frame)->v_bar && i!=((sgui_frame*)frame)->h_bar )
        {
            sgui_widget_get_position( i, &x, &y );
            y -= delta;
            sgui_widget_set_position( i, x, y );
        }
    }
}

static void frame_on_scroll_h( void* userptr, int new_offset, int delta )
{
    sgui_widget* frame = userptr;
    sgui_widget* i;
    sgui_rect r;
    int x, y;
    (void)new_offset;

    sgui_widget_get_absolute_rect( frame, &r );
    sgui_canvas_add_dirty_rect( frame->canvas, &r );

    for( i=frame->children; i!=NULL; i=i->next )
    {
        if( i!=((sgui_frame*)frame)->v_bar && i!=((sgui_frame*)frame)->h_bar )
        {
            sgui_widget_get_position( i, &x, &y );
            x -= delta;
            sgui_widget_set_position( i, x, y );
        }
    }
}

static void frame_draw( sgui_widget* widget )
{
    sgui_rect lt, rt, lb, rb, l, r, t, b, f, dst;
    sgui_canvas* cv = widget->canvas;
    sgui_pixmap* skin_pixmap = sgui_canvas_get_skin_pixmap( cv );

    int x  = widget->area.left;
    int y  = widget->area.top;
    int x1 = widget->area.right;
    int y1 = widget->area.bottom;

    /* get required skin elements */
    sgui_skin_get_element( SGUI_FRAME_LEFT_TOP, &lt );
    sgui_skin_get_element( SGUI_FRAME_RIGHT_TOP, &rt );
    sgui_skin_get_element( SGUI_FRAME_LEFT_BOTTOM, &lb );
    sgui_skin_get_element( SGUI_FRAME_RIGHT_BOTTOM, &rb );
    sgui_skin_get_element( SGUI_FRAME_LEFT, &l );
    sgui_skin_get_element( SGUI_FRAME_RIGHT, &r );
    sgui_skin_get_element( SGUI_FRAME_TOP, &t );
    sgui_skin_get_element( SGUI_FRAME_BOTTOM, &b );
    sgui_skin_get_element( SGUI_FRAME_CENTER, &f );

    /* draw corners */
    sgui_canvas_blend( cv, x, y, skin_pixmap, &lt );
    sgui_canvas_blend( cv, x1-(rt.right-rt.left), y, skin_pixmap, &rt );
    sgui_canvas_blend( cv, x, y1-(lb.bottom-lb.top), skin_pixmap, &lb );
    sgui_canvas_blend( cv, x1-(rb.right-rb.left), y1-(rb.bottom-rb.top),
                       skin_pixmap, &rb );

    /* draw borders */
    dst.left   = x;
    dst.right  = x  + (l.right - l.left);
    dst.top    = y  + SGUI_RECT_HEIGHT(lt);
    dst.bottom = y1 - SGUI_RECT_HEIGHT(lb);
    sgui_canvas_stretch_blend( cv, skin_pixmap, &l, &dst, 0 );

    dst.left   = x1 - (r.right-r.left);
    dst.right  = x1;
    dst.top    = y  + SGUI_RECT_HEIGHT(rt);
    dst.bottom = y1 - SGUI_RECT_HEIGHT(rb);
    sgui_canvas_stretch_blend( cv, skin_pixmap, &r, &dst, 0 );

    dst.left   = x  + SGUI_RECT_WIDTH(lt);
    dst.right  = x1 - SGUI_RECT_WIDTH(rt);
    dst.top    = y;
    dst.bottom = y  + (t.bottom-t.top);
    sgui_canvas_stretch_blend( cv, skin_pixmap, &t, &dst, 0 );

    dst.left   = x  + SGUI_RECT_WIDTH(lb);
    dst.right  = x1 - SGUI_RECT_WIDTH(rb);
    dst.top    = y1 - (b.bottom-b.top);
    dst.bottom = y1;
    sgui_canvas_stretch_blend( cv, skin_pixmap, &b, &dst, 0 );

    /* draw background */
    dst.left   = x  + SGUI_RECT_WIDTH(l);
    dst.right  = x1 - SGUI_RECT_WIDTH(r);
    dst.top    = y  + SGUI_RECT_HEIGHT(t);
    dst.bottom = y1 - SGUI_RECT_HEIGHT(b);
    sgui_canvas_stretch_blend( cv, skin_pixmap, &f, &dst, 0 );
}

static void frame_destroy( sgui_widget* frame )
{
    sgui_frame* f = (sgui_frame*)frame;

    sgui_widget_destroy( f->v_bar );
    sgui_widget_destroy( f->h_bar );
    free( f );
}

static void frame_on_state_change( sgui_widget* frame, int change )
{
    sgui_frame* f = (sgui_frame*)frame;
    unsigned int w, ww, wh, width, height, new_height, new_width;
    sgui_widget* i;
    sgui_rect r;
    int wx, wy;

    if( change & (WIDGET_CHILD_ADDED|WIDGET_CHILD_REMOVED) )
    {
        width = SGUI_RECT_WIDTH( frame->area );
        height = SGUI_RECT_HEIGHT( frame->area );
        new_height = 0;
        new_width = 0;

        /* determine the required frame size */
        for( i=frame->children; i!=NULL; i=i->next )
        {
            if( i!=f->v_bar && i!=f->h_bar )
            {
                sgui_widget_get_position( i, &wx, &wy );
                sgui_widget_get_size( i, &ww, &wh );

                new_height = MAX( new_height, wy+wh );
                new_width = MAX( new_width, wx+ww );
            }
        }

        /* draw the vertical scroll bar if the required height is larger */
        if( new_height > height )
        {
            /* if the horizontal scroll bar is drawn, take it into account */
            if( new_width > width )
            {
                sgui_skin_get_element( SGUI_SCROLL_BAR_BUTTON_LEFT, &r );
                new_height += SGUI_RECT_HEIGHT( r );
            }

            sgui_scroll_bar_set_area( f->v_bar, new_height+10, height );

            if( !f->override_scrollbars )
                sgui_widget_set_visible( f->v_bar, 1 );
        }
        else if( !f->override_scrollbars )
        {
            sgui_widget_set_visible( f->v_bar, 0 );
        }

        /* draw the horizontal scroll bar if the required width is larger */
        if( new_width > width )
        {
            /* if the vertical scroll bar is drawn, take it into account */
            if( new_height > height )
            {
                sgui_skin_get_element( SGUI_SCROLL_BAR_BUTTON_UP, &r );
                new_width += SGUI_RECT_WIDTH( r );
                ww = SGUI_RECT_WIDTH( r );

                w = SGUI_RECT_WIDTH(frame->area);
                sgui_scroll_bar_set_length( f->h_bar, w-2*f->v_border-ww );
            }
            else
            {
                w = SGUI_RECT_WIDTH(frame->area);
                sgui_scroll_bar_set_length( f->h_bar, w-2*f->h_border );
            }

            sgui_scroll_bar_set_area( f->h_bar, new_width+10, width );

            if( !f->override_scrollbars )
                sgui_widget_set_visible( f->h_bar, 1 );
        }
        else if( !f->override_scrollbars )
        {
            sgui_widget_set_visible( f->h_bar, 0 );
        }
    }
}

/****************************************************************************/

sgui_widget* sgui_frame_create( int x, int y, unsigned int width,
                                unsigned int height )
{
    sgui_frame* f = malloc( sizeof(sgui_frame) );
    unsigned int w, h;
    sgui_rect r;

    if( !f )
        return NULL;

    sgui_internal_widget_init( (sgui_widget*)f, x, y, width, height );

    /* try to create a vertical scroll bar */
    sgui_skin_get_element( SGUI_SCROLL_BAR_V_BACKGROUND, &r );
    w = SGUI_RECT_WIDTH( r );
    h = SGUI_RECT_HEIGHT( r ) + height;

    sgui_skin_get_element( SGUI_FRAME_BORDER, &r );
    f->h_border = SGUI_RECT_WIDTH(r);
    f->v_border = SGUI_RECT_HEIGHT(r);
    f->v_bar_dist = width - w - f->v_border;
    f->v_bar = sgui_scroll_bar_create( f->v_bar_dist, f->v_border, 0,
                                       height-2*f->v_border,
                                       height-2*f->v_border,
                                       height-2*f->v_border );

    if( !f->v_bar )
    {
        free( f );
        return NULL;
    }

    /* try to create a horizontal scroll bar */
    sgui_skin_get_element( SGUI_SCROLL_BAR_H_BACKGROUND, &r );
    w = SGUI_RECT_WIDTH( r ) + width;
    h = SGUI_RECT_HEIGHT( r );

    f->h_bar_dist = height - h - f->v_border;
    f->h_bar = sgui_scroll_bar_create( f->h_border, f->h_bar_dist, 1,
                                       width-2*f->h_border,
                                       width-2*f->h_border,
                                       width-2*f->h_border );

    if( !f->h_bar )
    {
        free( f );
        return NULL;
    }

    /* add scroll bars to frame */
    sgui_scroll_bar_on_scroll( f->v_bar, frame_on_scroll_v, f );
    sgui_scroll_bar_on_scroll( f->h_bar, frame_on_scroll_h, f );
    sgui_widget_set_visible( f->v_bar, 0 );
    sgui_widget_set_visible( f->h_bar, 0 );
    sgui_widget_add_child( (sgui_widget*)f, f->v_bar );
    sgui_widget_add_child( (sgui_widget*)f, f->h_bar );

    /* finish initialisation */
    f->widget.draw_callback         = frame_draw;
    f->widget.state_change_callback = frame_on_state_change;
    f->widget.destroy               = frame_destroy;
    f->override_scrollbars          = 0;

    return (sgui_widget*)f;
}

void sgui_frame_override_scrollbars( sgui_widget* frame, int always_draw )
{
    sgui_frame* f = (sgui_frame*)frame;

    if( f )
    {
        f->override_scrollbars = always_draw;

        sgui_widget_set_visible( f->v_bar, 1 );
        sgui_widget_set_visible( f->h_bar, 1 );
    }
}

