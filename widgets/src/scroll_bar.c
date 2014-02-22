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



static void scroll_bar_on_event_h( sgui_widget* widget, sgui_event* e )
{
    sgui_scroll_bar* b = (sgui_scroll_bar*)widget;
    unsigned int l = b->length - 2*b->bw, old;
    sgui_rect r;

    sgui_internal_lock_mutex( );

    if( e->type==SGUI_MOUSE_WHEEL_EVENT )
    {
        old = b->v_offset;

        if( e->arg.i < 0 )
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

        if( b->v_offset!=old )
        {
            if( b->scroll_fun )
                b->scroll_fun( b->userptr, b->v_offset, b->v_offset-old );

            sgui_widget_get_absolute_rect( widget, &r );
            sgui_canvas_add_dirty_rect( widget->canvas, &r );
        }
    }
    else if( e->type==SGUI_MOUSE_RELEASE_EVENT ||
             e->type==SGUI_MOUSE_LEAVE_EVENT )
    {
        /* buttons return to out state, need redraw */
        if( b->dec_button_state || b->inc_button_state )
        {
            sgui_widget_get_absolute_rect( widget, &r );
            sgui_canvas_add_dirty_rect( widget->canvas, &r );
        }

        b->dec_button_state = b->inc_button_state = 0;
    }
    else if( e->type==SGUI_MOUSE_PRESS_EVENT )
    {
        /* update button state, request redraw */
        b->dec_button_state = e->arg.i3.x < (int)b->bw;
        b->inc_button_state = e->arg.i3.x > ((int)b->length-(int)b->bw);

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

    sgui_internal_unlock_mutex( );
}

static void scroll_bar_on_event_v( sgui_widget* widget, sgui_event* e )
{
    sgui_scroll_bar* b = (sgui_scroll_bar*)widget;
    unsigned int l = b->length - 2*b->bh, old;
    sgui_rect r;

    sgui_internal_lock_mutex( );

    if( e->type==SGUI_MOUSE_WHEEL_EVENT )
    {
        old = b->v_offset;

        if( e->arg.i < 0 )
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

        if( b->v_offset!=old )
        {
            if( b->scroll_fun )
                b->scroll_fun( b->userptr, b->v_offset, b->v_offset-old );

            sgui_widget_get_absolute_rect( widget, &r );
            sgui_canvas_add_dirty_rect( widget->canvas, &r );
        }
    }
    else if( e->type==SGUI_MOUSE_RELEASE_EVENT ||
             e->type==SGUI_MOUSE_LEAVE_EVENT )
    {
        /* buttons return to out state, need redraw */
        if( b->dec_button_state || b->inc_button_state )
        {
            sgui_widget_get_absolute_rect( widget, &r );
            sgui_canvas_add_dirty_rect( widget->canvas, &r );
        }

        b->dec_button_state = b->inc_button_state = 0;
    }
    else if( e->type==SGUI_MOUSE_PRESS_EVENT )
    {
        /* update button state, request redraw */
        b->dec_button_state = e->arg.i3.y < (int)b->bh;
        b->inc_button_state = e->arg.i3.y > ((int)b->length-(int)b->bh);

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

    sgui_internal_unlock_mutex( );
}

static void scroll_bar_draw( sgui_widget* super )
{
    sgui_scroll_bar* this = (sgui_scroll_bar*)super;

    sgui_skin_draw_scroll_bar( super->canvas,
                               super->area.left, super->area.top,
                               this->length, !this->horizontal,
                               this->p_offset, this->p_length,
                               this->inc_button_state,
                               this->dec_button_state );
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
    sgui_skin_get_scroll_bar_button_extents( &r );

    b->bw = SGUI_RECT_WIDTH( r );
    b->bh = SGUI_RECT_HEIGHT( r );

    if( horizontal )
    {
        w = length;
        h = sgui_skin_get_scroll_bar_width( );
    }
    else
    {
        w = sgui_skin_get_scroll_bar_width( );
        h = length;
    }

    sgui_internal_widget_init( (sgui_widget*)b, x, y, w, h );

    b->widget.window_event_callback = horizontal ? scroll_bar_on_event_h :
                                                   scroll_bar_on_event_v;

    b->widget.draw_callback = scroll_bar_draw;
    b->widget.destroy       = scroll_bar_destroy;
    b->horizontal           = horizontal;
    b->length               = length;
    b->v_length             = disp_area_length;
    b->v_max                = scroll_area_length;
    b->p_length             = ((b->v_length<<8) / b->v_max) *
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
    unsigned int l;
    sgui_rect r;

    if( b )
    {
        sgui_internal_lock_mutex( );

        l = b->length - 2*(b->horizontal ? b->bh : b->bw);

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

        sgui_internal_unlock_mutex( );
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
        sgui_internal_lock_mutex( );

        b->v_length = disp_area_length;
        b->v_max    = scroll_area_length;
        b->p_length = ((b->v_length<<8) / b->v_max) *
                       (b->length - 2*(b->horizontal ? b->bw : b->bh));

        b->p_length >>= 8;

        sgui_widget_get_absolute_rect( bar, &r );
        sgui_canvas_add_dirty_rect( bar->canvas, &r );

        sgui_internal_unlock_mutex( );
    }
}

void sgui_scroll_bar_set_length( sgui_widget* bar, unsigned int length )
{
    sgui_scroll_bar* b = (sgui_scroll_bar*)bar;
    sgui_rect r;

    if( b && length!=b->length )
    {
        sgui_internal_lock_mutex( );

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

        sgui_internal_unlock_mutex( );
    }
}

