/*
 * scroll_bar.c
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
#include "sgui_scroll_bar.h"
#include "sgui_skin.h"
#include "sgui_event.h"
#include "sgui_internal.h"
#include "sgui_widget.h"

#include <stdlib.h>
#include <string.h>


typedef struct
{
    sgui_widget widget;
    unsigned int bw, bh, length, p_length, v_length, p_offset, v_offset,
                 v_max;
    int horizontal, inc_button_state, dec_button_state;

    sgui_scrollbar_callback scroll_fun;
    void* userptr;
}
sgui_scroll_bar;



static void scroll_bar_on_event_h( sgui_widget* widget, int type,
                                   sgui_event* event )
{
    sgui_scroll_bar* b = (sgui_scroll_bar*)widget;
    unsigned int l = b->length - 2*b->bw, old;
    sgui_rect r;

    if( type==SGUI_MOUSE_WHEEL_EVENT )
    {
        old = b->v_offset;

        if( event->mouse_wheel.direction < 0 )
        {
            if( ((b->v_offset + b->v_length + b->v_length/4) < b->v_max) &&
                ((b->p_offset + b->p_length + b->p_length/4) < l) )
            {
                b->p_offset += b->p_length / 4;
                b->v_offset += b->v_length / 4;
            }
            else
            {
                b->p_offset = l - b->p_length;
                b->v_offset = b->v_max - b->v_length;
            }
        }
        else
        {
            if( (b->p_offset > (b->p_length/4)) &&
                (b->v_offset > (b->v_length/4)) )
            {
                b->p_offset -= b->p_length / 4;
                b->v_offset -= b->v_length / 4;
            }
            else
            {
                b->p_offset = 0;
                b->v_offset = 0;            
            }
        }

        if( b->scroll_fun && (b->v_offset!=old) )
            b->scroll_fun( b->userptr, b->v_offset, b->v_offset-old );

        sgui_widget_get_absolute_rect( widget, &r );
        sgui_canvas_add_dirty_rect( widget->canvas, &r );
    }
    else if( type==SGUI_MOUSE_RELEASE_EVENT || type==SGUI_MOUSE_LEAVE_EVENT )
    {
        /* buttons return to out state, need redraw */
        if( b->dec_button_state || b->inc_button_state )
        {
            sgui_widget_get_absolute_rect( widget, &r );
            sgui_canvas_add_dirty_rect( widget->canvas, &r );
        }

        b->dec_button_state = b->inc_button_state = 0;
    }
    else if( type==SGUI_MOUSE_PRESS_EVENT )
    {
        /* update button state, request redraw */
        b->dec_button_state = event->mouse_press.x < (int)b->bw;
        b->inc_button_state = event->mouse_press.x >
                              ((int)b->length-(int)b->bw);

        sgui_widget_get_absolute_rect( widget, &r );
        sgui_canvas_add_dirty_rect( widget->canvas, &r );

        /* modify offset accordingly if a button was pressed */
        old = b->v_offset;

        if( b->inc_button_state )
        {
            if( ((b->v_offset + b->v_length + b->v_length/4) < b->v_max) &&
                ((b->p_offset + b->p_length + b->p_length/4) < l) )
            {
                b->p_offset += b->p_length / 4;
                b->v_offset += b->v_length / 4;
            }
            else
            {
                b->p_offset = l - b->p_length;
                b->v_offset = b->v_max - b->v_length;
            }
        }
        else if( b->dec_button_state )
        {
            if( (b->p_offset > (b->p_length/4)) &&
                (b->v_offset > (b->v_length/4)) )
            {
                b->p_offset -= b->p_length / 4;
                b->v_offset -= b->v_length / 4;
            }
            else
            {
                b->p_offset = 0;
                b->v_offset = 0;            
            }
        }

        if( b->scroll_fun && (b->v_offset==old) )
            b->scroll_fun( b->userptr, b->v_offset, b->v_offset-old );
    }
}

static void scroll_bar_on_event_v( sgui_widget* widget, int type,
                                   sgui_event* event )
{
    sgui_scroll_bar* b = (sgui_scroll_bar*)widget;
    unsigned int l = b->length - 2*b->bh, old;
    sgui_rect r;

    if( type==SGUI_MOUSE_WHEEL_EVENT )
    {
        old = b->v_offset;

        if( event->mouse_wheel.direction < 0 )
        {
            if( ((b->v_offset + b->v_length + b->v_length/4) < b->v_max) &&
                ((b->p_offset + b->p_length + b->p_length/4) < l) )
            {
                b->p_offset += b->p_length / 4;
                b->v_offset += b->v_length / 4;
            }
            else
            {
                b->p_offset = l - b->p_length;
                b->v_offset = b->v_max - b->v_length;
            }
        }
        else
        {
            if( (b->p_offset > (b->p_length/4)) &&
                (b->v_offset > (b->v_length/4)) )
            {
                b->p_offset -= b->p_length / 4;
                b->v_offset -= b->v_length / 4;
            }
            else
            {
                b->p_offset = 0;
                b->v_offset = 0;
            }
        }

        if( b->scroll_fun && (b->v_offset!=old) )
            b->scroll_fun( b->userptr, b->v_offset, b->v_offset-old );

        sgui_widget_get_absolute_rect( widget, &r );
        sgui_canvas_add_dirty_rect( widget->canvas, &r );
    }
    else if( type==SGUI_MOUSE_RELEASE_EVENT || type==SGUI_MOUSE_LEAVE_EVENT )
    {
        /* buttons return to out state, need redraw */
        if( b->dec_button_state || b->inc_button_state )
        {
            sgui_widget_get_absolute_rect( widget, &r );
            sgui_canvas_add_dirty_rect( widget->canvas, &r );
        }

        b->dec_button_state = b->inc_button_state = 0;
    }
    else if( type==SGUI_MOUSE_PRESS_EVENT )
    {
        /* update button state, request redraw */
        b->dec_button_state = event->mouse_press.y < (int)b->bh;
        b->inc_button_state = event->mouse_press.y >
                              ((int)b->length-(int)b->bh);

        sgui_widget_get_absolute_rect( widget, &r );
        sgui_canvas_add_dirty_rect( widget->canvas, &r );

        /* modify offset accordingly if a button was pressed */
        old = b->v_offset;

        if( b->inc_button_state )
        {
            if( ((b->v_offset + b->v_length + b->v_length/4) < b->v_max) &&
                ((b->p_offset + b->p_length + b->p_length/4) < l) )
            {
                b->p_offset += b->p_length / 4;
                b->v_offset += b->v_length / 4;
            }
            else
            {
                b->p_offset = l - b->p_length;
                b->v_offset = b->v_max - b->v_length;
            }
        }
        else if( b->dec_button_state )
        {
            if( (b->p_offset > (b->p_length/4)) &&
                (b->v_offset > (b->v_length/4)) )
            {
                b->p_offset -= b->p_length / 4;
                b->v_offset -= b->v_length / 4;
            }
            else
            {
                b->p_offset = 0;
                b->v_offset = 0;
            }
        }

        if( b->scroll_fun && (b->v_offset!=old) )
            b->scroll_fun( b->userptr, b->v_offset, b->v_offset-old );
    }
}

static void scroll_bar_draw_h( sgui_widget* widget )
{
    sgui_scroll_bar* b = (sgui_scroll_bar*)widget;
    sgui_canvas* cv = widget->canvas;
    int x = widget->area.left, y = widget->area.top;
    unsigned int length = b->length, p_length = b->p_length;
    sgui_pixmap* skin_pixmap = sgui_canvas_get_skin_pixmap( cv );
    sgui_rect r, dst;

    /* right button */
    sgui_skin_get_element( b->inc_button_state ? 
                           SGUI_SCROLL_BAR_BUTTON_RIGHT_IN :
                           SGUI_SCROLL_BAR_BUTTON_RIGHT, &r );
    sgui_canvas_blend( cv, x+length-SGUI_RECT_WIDTH(r), y,
                       skin_pixmap, &r );
    length -= SGUI_RECT_WIDTH(r);

    /* left button */
    sgui_skin_get_element( b->dec_button_state ? 
                           SGUI_SCROLL_BAR_BUTTON_LEFT_IN :
                           SGUI_SCROLL_BAR_BUTTON_LEFT, &r );
    sgui_canvas_blend( cv, x, y, skin_pixmap, &r );
    x += SGUI_RECT_WIDTH(r);
    length -= SGUI_RECT_WIDTH(r);

    /* pane background */
    sgui_skin_get_element( SGUI_SCROLL_BAR_H_BACKGROUND, &r );
    sgui_rect_set_size( &dst, x, y, length, SGUI_RECT_HEIGHT(r) );
    sgui_canvas_stretch_blend( cv, skin_pixmap, &r, &dst, 0 );

    /* pane */
    x += b->p_offset;
    sgui_skin_get_element( SGUI_SCROLL_BAR_H_PANE_LEFT, &r );
    sgui_canvas_blend( cv, x, y, skin_pixmap, &r );
    p_length -= SGUI_RECT_WIDTH(r);
    x += SGUI_RECT_WIDTH(r);

    sgui_skin_get_element( SGUI_SCROLL_BAR_H_PANE_RIGHT, &r );
    p_length -= SGUI_RECT_WIDTH(r);
    sgui_canvas_blend( cv, x+p_length, y, skin_pixmap, &r );

    sgui_rect_set_size( &dst, x, y, p_length+1, SGUI_RECT_HEIGHT(r) );
    sgui_skin_get_element( SGUI_SCROLL_BAR_H_PANE_CENTER, &r );
    sgui_canvas_stretch_blend( cv, skin_pixmap, &r, &dst, 0 );
}

static void scroll_bar_draw_v( sgui_widget* widget )
{
    sgui_scroll_bar* b = (sgui_scroll_bar*)widget;
    sgui_canvas* cv = widget->canvas;
    int x = widget->area.left, y = widget->area.top;
    unsigned int length = b->length, p_length = b->p_length;
    sgui_pixmap* skin_pixmap = sgui_canvas_get_skin_pixmap( cv );
    sgui_rect r, dst;

    /* lower button */
    sgui_skin_get_element( b->inc_button_state ? 
                           SGUI_SCROLL_BAR_BUTTON_DOWN_IN :
                           SGUI_SCROLL_BAR_BUTTON_DOWN, &r );
    sgui_canvas_blend( cv, x, y+length-SGUI_RECT_HEIGHT(r),
                       skin_pixmap, &r );
    length -= SGUI_RECT_HEIGHT(r);

    /* upper button */
    sgui_skin_get_element( b->dec_button_state ? 
                           SGUI_SCROLL_BAR_BUTTON_UP_IN :
                           SGUI_SCROLL_BAR_BUTTON_UP, &r );
    sgui_canvas_blend( cv, x, y, skin_pixmap, &r );
    y += SGUI_RECT_HEIGHT(r);
    length -= SGUI_RECT_HEIGHT(r);

    /* pane background */
    sgui_skin_get_element( SGUI_SCROLL_BAR_V_BACKGROUND, &r );
    sgui_rect_set_size( &dst, x, y, SGUI_RECT_HEIGHT(r), length );
    sgui_canvas_stretch_blend( cv, skin_pixmap, &r, &dst, 0 );

    /* pane */
    y += b->p_offset;
    sgui_skin_get_element( SGUI_SCROLL_BAR_V_PANE_TOP, &r );
    sgui_canvas_blend( cv, x, y, skin_pixmap, &r );
    p_length -= SGUI_RECT_HEIGHT(r);
    y += SGUI_RECT_HEIGHT(r);

    sgui_skin_get_element( SGUI_SCROLL_BAR_V_PANE_BOTTOM, &r );
    p_length -= SGUI_RECT_HEIGHT(r);
    sgui_canvas_blend( cv, x, y+p_length, skin_pixmap, &r );

    sgui_rect_set_size( &dst, x, y, SGUI_RECT_WIDTH(r), p_length+1 );
    sgui_skin_get_element( SGUI_SCROLL_BAR_V_PANE_CENTER, &r );
    sgui_canvas_stretch_blend( cv, skin_pixmap, &r, &dst, 0 );
}

static void scroll_bar_destroy( sgui_widget* bar )
{
    free( bar );
}



sgui_widget* sgui_scroll_bar_create( int x, int y, int horizontal,
                                     unsigned int length,
                                     unsigned int scroll_area_length,
                                     unsigned int disp_area_length )
{
    sgui_scroll_bar* b = malloc( sizeof(sgui_scroll_bar) );
    unsigned int w=0, h=0;
    sgui_rect r;

    if( !b )
        return NULL;

    memset( b, 0, sizeof(sgui_scroll_bar) );

    if( horizontal )
    {
        sgui_skin_get_element( SGUI_SCROLL_BAR_H_PANE_CENTER, &r );
        h = MAX(h, (unsigned int)SGUI_RECT_HEIGHT(r));
        sgui_skin_get_element( SGUI_SCROLL_BAR_H_BACKGROUND, &r );
        h = MAX(h, (unsigned int)SGUI_RECT_HEIGHT(r));
        sgui_skin_get_element( SGUI_SCROLL_BAR_BUTTON_LEFT, &r );
        h = MAX(h, (unsigned int)SGUI_RECT_HEIGHT(r));
        sgui_skin_get_element( SGUI_SCROLL_BAR_BUTTON_RIGHT, &r );
        h = MAX(h, (unsigned int)SGUI_RECT_HEIGHT(r));
        b->bw = SGUI_RECT_WIDTH( r );
        b->bh = SGUI_RECT_HEIGHT( r );

        w = length;
    }
    else
    {
        sgui_skin_get_element( SGUI_SCROLL_BAR_V_PANE_CENTER, &r );
        w = MAX(w, (unsigned int)SGUI_RECT_WIDTH(r));
        sgui_skin_get_element( SGUI_SCROLL_BAR_V_BACKGROUND, &r );
        w = MAX(w, (unsigned int)SGUI_RECT_WIDTH(r));
        sgui_skin_get_element( SGUI_SCROLL_BAR_BUTTON_UP, &r );
        w = MAX(w, (unsigned int)SGUI_RECT_WIDTH(r));
        sgui_skin_get_element( SGUI_SCROLL_BAR_BUTTON_DOWN, &r );
        w = MAX(w, (unsigned int)SGUI_RECT_WIDTH(r));
        b->bw = SGUI_RECT_WIDTH( r );
        b->bh = SGUI_RECT_HEIGHT( r );

        h = length;
    }

    sgui_internal_widget_init( (sgui_widget*)b, x, y, w, h );

    if( horizontal )
    {
        b->widget.window_event_callback = scroll_bar_on_event_h;
        b->widget.draw_callback = scroll_bar_draw_h;
    }
    else
    {
        b->widget.window_event_callback = scroll_bar_on_event_v;
        b->widget.draw_callback = scroll_bar_draw_v;
    }

    b->widget.destroy = scroll_bar_destroy;
    b->horizontal     = horizontal;
    b->length         = length;
    b->v_length       = disp_area_length;
    b->v_max          = scroll_area_length;
    b->p_length       = ((b->v_length<<8) / b->v_max) *
                        (length - 2*(horizontal ? b->bw : b->bh));

    b->p_length >>= 8;

    return (sgui_widget*)b;
}

void sgui_scroll_bar_on_scroll( sgui_widget* bar, sgui_scrollbar_callback fun,
                                void* userptr )
{
    sgui_scroll_bar* b = (sgui_scroll_bar*)bar;

    if( b )
    {
        b->scroll_fun = fun;
        b->userptr = userptr;
    }
}

void sgui_scroll_bar_set_offset( sgui_widget* bar, unsigned int offset )
{
    sgui_scroll_bar* b = (sgui_scroll_bar*)bar;
    sgui_rect r;

    if( b )
    {
        unsigned int l = b->length - 2*(b->horizontal ? b->bh : b->bw);

        if( (offset + b->v_length) < b->v_max )
        {
            b->v_offset = offset;
            b->p_offset = (((offset<<8)/b->v_max) * l) >> 8;
        }
        else
        {
            b->v_offset = b->v_max - b->v_length;
            b->p_offset = l - b->p_length;
        }

        sgui_widget_get_absolute_rect( bar, &r );
        sgui_canvas_add_dirty_rect( bar->canvas, &r );
    }
}

unsigned int sgui_scroll_bar_get_offset( sgui_widget* bar )
{
    return bar ? (((sgui_scroll_bar*)bar)->v_offset) : 0;
}

void sgui_scroll_bar_set_area( sgui_widget* bar,
                               unsigned int scroll_area_length,
                               unsigned int disp_area_length )
{
    sgui_scroll_bar* b = (sgui_scroll_bar*)bar;
    sgui_rect r;

    if( b )
    {
        b->v_length = disp_area_length;
        b->v_max    = scroll_area_length;
        b->p_length = ((b->v_length<<8) / b->v_max) *
                       (b->length - 2*(b->horizontal ? b->bw : b->bh));

        b->p_length >>= 8;

        sgui_widget_get_absolute_rect( bar, &r );
        sgui_canvas_add_dirty_rect( bar->canvas, &r );
    }
}

void sgui_scroll_bar_set_length( sgui_widget* bar, unsigned int length )
{
    sgui_scroll_bar* b = (sgui_scroll_bar*)bar;
    sgui_rect r;

    if( b && length!=b->length )
    {
        /* if the bar is shrinked, add old area as dirty rect */
        if( b->length < length )
        {
            sgui_widget_get_absolute_rect( bar, &r );
            sgui_canvas_add_dirty_rect( bar->canvas, &r );
        }

        /* update length and pane dimension */
        b->length = length;
        b->p_length = ((b->v_length<<8) / b->v_max) *
                       (b->length - 2*(b->horizontal ? b->bw : b->bh));

        b->p_length >>= 8;

        /* update widget area */
        if( b->horizontal )
        {
            bar->area.right = bar->area.left + length;
        }
        else
        {
            bar->area.bottom = bar->area.top + length;
        }

        /* if the bar is enlarged, add new area as dirty rect */
        if( b->length > length )
        {
            sgui_widget_get_absolute_rect( bar, &r );
            sgui_canvas_add_dirty_rect( bar->canvas, &r );
        }
    }
}

