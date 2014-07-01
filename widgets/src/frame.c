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


#ifndef SGUI_NO_FRAME
typedef struct
{
    sgui_widget super;

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
    sgui_widget* this = userptr;
    sgui_widget* i;
    sgui_rect r;
    int x, y;
    (void)new_offset;

    sgui_internal_lock_mutex( );
    sgui_widget_get_absolute_rect( this, &r );
    sgui_canvas_add_dirty_rect( this->canvas, &r );

    for( i=this->children; i!=NULL; i=i->next )
    {
        if( i!=((sgui_frame*)this)->v_bar && i!=((sgui_frame*)this)->h_bar )
        {
            sgui_widget_get_position( i, &x, &y );
            y -= delta;
            sgui_widget_set_position( i, x, y );
        }
    }

    sgui_internal_unlock_mutex( );
}

static void frame_on_scroll_h( void* userptr, int new_offset, int delta )
{
    sgui_widget* this = userptr;
    sgui_widget* i;
    sgui_rect r;
    int x, y;
    (void)new_offset;

    sgui_internal_lock_mutex( );
    sgui_widget_get_absolute_rect( this, &r );
    sgui_canvas_add_dirty_rect( this->canvas, &r );

    for( i=this->children; i!=NULL; i=i->next )
    {
        if( i!=((sgui_frame*)this)->v_bar && i!=((sgui_frame*)this)->h_bar )
        {
            sgui_widget_get_position( i, &x, &y );
            x -= delta;
            sgui_widget_set_position( i, x, y );
        }
    }

    sgui_internal_unlock_mutex( );
}

static void frame_draw( sgui_widget* this )
{
    sgui_skin_draw_frame( this->canvas, &(this->area) );
}

static void frame_destroy( sgui_widget* super )
{
    sgui_frame* this = (sgui_frame*)super;
    sgui_widget* i;

    /* recursive destroy might have already destroyed the scrollbars */
    if( super->children )
    {
        for( i=super->children; i!=NULL; i=i->next )
        {
            if( i->next == this->v_bar )
            {
                i->next = i->next->next;
                sgui_widget_destroy( this->v_bar );
                break;
            }
        }

        for( i=super->children; i!=NULL; i=i->next )
        {
            if( i->next == this->h_bar )
            {
                i->next = i->next->next;
                sgui_widget_destroy( this->h_bar );
                break;
            }
        }
    }

    free( this );
}

static void frame_on_state_change( sgui_widget* super, int change )
{
    sgui_frame* this = (sgui_frame*)super;
    unsigned int w, ww, wh, width, height, new_height, new_width;
    sgui_widget* i;
    sgui_rect r;
    int wx, wy;

    sgui_internal_lock_mutex( );

    if( change & (SGUI_WIDGET_CHILD_ADDED|SGUI_WIDGET_CHILD_REMOVED) )
    {
        width = SGUI_RECT_WIDTH( super->area );
        height = SGUI_RECT_HEIGHT( super->area );
        new_height = 0;
        new_width = 0;

        /* determine the required frame size */
        for( i=super->children; i!=NULL; i=i->next )
        {
            if( i!=this->v_bar && i!=this->h_bar )
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
                sgui_skin_get_scroll_bar_button_extents( &r );
                new_height += SGUI_RECT_HEIGHT( r );
            }

            sgui_scroll_bar_set_area( this->v_bar, new_height+10, height );

            if( !this->override_scrollbars )
                sgui_widget_set_visible( this->v_bar, 1 );
        }
        else if( !this->override_scrollbars )
        {
            sgui_widget_set_visible( this->v_bar, 0 );
        }

        /* draw the horizontal scroll bar if the required width is larger */
        if( new_width > width )
        {
            /* if the vertical scroll bar is drawn, take it into account */
            if( new_height > height )
            {
                sgui_skin_get_scroll_bar_button_extents( &r );
                new_width += SGUI_RECT_WIDTH( r );
                ww = SGUI_RECT_WIDTH( r );

                w = SGUI_RECT_WIDTH(super->area);
                sgui_scroll_bar_set_length( this->h_bar,
                                            w-2*this->v_border-ww );
            }
            else
            {
                w = SGUI_RECT_WIDTH(super->area);
                sgui_scroll_bar_set_length( this->h_bar, w-2*this->h_border );
            }

            sgui_scroll_bar_set_area( this->h_bar, new_width+10, width );

            if( !this->override_scrollbars )
                sgui_widget_set_visible( this->h_bar, 1 );
        }
        else if( !this->override_scrollbars )
        {
            sgui_widget_set_visible( this->h_bar, 0 );
        }
    }

    sgui_internal_unlock_mutex( );
}

static void frame_on_event( sgui_widget* super, const sgui_event* event )
{
    sgui_frame* this = (sgui_frame*)super;
    int page, new_offset, old_offset;

    if( event->type == SGUI_MOUSE_WHEEL_EVENT )
    {
        sgui_internal_lock_mutex( );

        /* determine scroll page size and current offset */
        page = SGUI_RECT_HEIGHT( super->area );
        old_offset = sgui_scroll_bar_get_offset( this->v_bar );

        /* compute new offset */
        new_offset = event->arg.i<0 ? (old_offset + (page >> 2)) :
                                      (old_offset - (page >> 2));
        new_offset = new_offset<0 ? 0 : new_offset;

        /* udpate offset and call scroll handler */
        sgui_scroll_bar_set_offset( this->v_bar, new_offset );
        new_offset = sgui_scroll_bar_get_offset( this->v_bar );
        frame_on_scroll_v( super, new_offset, new_offset-old_offset );

        sgui_internal_unlock_mutex( );
    }
    else if( event->type == SGUI_KEY_RELEASED_EVENT )
    {
        if( event->arg.i==SGUI_KC_LEFT || event->arg.i==SGUI_KC_RIGHT )
        {
            sgui_internal_lock_mutex( );

            /* determine scroll page size and current offset */
            page = SGUI_RECT_WIDTH( super->area );
            old_offset = sgui_scroll_bar_get_offset( this->h_bar );

            /* compute new offset */
            new_offset = event->arg.i==SGUI_KC_RIGHT ?
                         (old_offset + (page >> 2)) :
                         (old_offset - (page >> 2));
            new_offset = new_offset<0 ? 0 : new_offset;

            /* udpate offset and call scroll handler */
            sgui_scroll_bar_set_offset( this->h_bar, new_offset );
            new_offset = sgui_scroll_bar_get_offset( this->h_bar );
            frame_on_scroll_h( super, new_offset, new_offset-old_offset );

            sgui_internal_unlock_mutex( );
        }
        else if( event->arg.i==SGUI_KC_UP || event->arg.i==SGUI_KC_DOWN )
        {
            sgui_internal_lock_mutex( );

            /* determine scroll page size and current offset */
            page = SGUI_RECT_HEIGHT( super->area );
            old_offset = sgui_scroll_bar_get_offset( this->v_bar );

            /* compute new offset */
            new_offset = event->arg.i==SGUI_KC_DOWN ?
                         (old_offset + (page >> 2)) :
                         (old_offset - (page >> 2));
            new_offset = new_offset<0 ? 0 : new_offset;

            /* udpate offset and call scroll handler */
            sgui_scroll_bar_set_offset( this->v_bar, new_offset );
            new_offset = sgui_scroll_bar_get_offset( this->v_bar );
            frame_on_scroll_v( super, new_offset, new_offset-old_offset );

            sgui_internal_unlock_mutex( );
        }
    }
}

/****************************************************************************/

sgui_widget* sgui_frame_create( int x, int y, unsigned int width,
                                unsigned int height )
{
    sgui_frame* this = malloc( sizeof(sgui_frame) );
    sgui_widget* super = (sgui_widget*)this;
    unsigned int w, h;
    sgui_rect r;

    if( !this )
        return NULL;

    sgui_widget_init( (sgui_widget*)this, x, y, width, height );

    /* try to create a vertical scroll bar */
    sgui_skin_get_scroll_bar_button_extents( &r );
    w = SGUI_RECT_WIDTH( r );
    h = SGUI_RECT_HEIGHT( r ) + height;

    this->h_border = sgui_skin_get_frame_border_width( );
    this->v_border = sgui_skin_get_frame_border_width( );
    this->v_bar_dist = width - w - this->v_border;
    this->v_bar = sgui_scroll_bar_create( this->v_bar_dist, this->v_border, 0,
                                          height-2*this->v_border,
                                          height-2*this->v_border,
                                          height-2*this->v_border );

    if( !this->v_bar )
    {
        free( this );
        return NULL;
    }

    /* try to create a horizontal scroll bar */
    sgui_skin_get_scroll_bar_button_extents( &r );
    w = SGUI_RECT_WIDTH( r ) + width;
    h = SGUI_RECT_HEIGHT( r );

    this->h_bar_dist = height - h - this->v_border;
    this->h_bar = sgui_scroll_bar_create( this->h_border, this->h_bar_dist, 1,
                                          width-2*this->h_border,
                                          width-2*this->h_border,
                                          width-2*this->h_border );

    if( !this->h_bar )
    {
        free( this );
        return NULL;
    }

    /* add scroll bars to frame */
    sgui_scroll_bar_on_scroll( this->v_bar, frame_on_scroll_v, this );
    sgui_scroll_bar_on_scroll( this->h_bar, frame_on_scroll_h, this );
    sgui_widget_set_visible( this->v_bar, 0 );
    sgui_widget_set_visible( this->h_bar, 0 );
    sgui_widget_add_child( super, this->v_bar );
    sgui_widget_add_child( super, this->h_bar );

    /* finish initialisation */
    super->window_event_callback = frame_on_event;
    super->draw_callback         = frame_draw;
    super->state_change_callback = frame_on_state_change;
    super->destroy               = frame_destroy;
    this->override_scrollbars    = 0;
    super->focus_policy          = SGUI_FOCUS_ACCEPT|SGUI_FOCUS_DROP_ESC|
                                   SGUI_FOCUS_DROP_TAB|SGUI_FOCUS_DRAW;

    return super;
}

void sgui_frame_override_scrollbars( sgui_widget* super, int always_draw )
{
    sgui_frame* this = (sgui_frame*)super;

    if( this )
    {
        this->override_scrollbars = always_draw;

        sgui_widget_set_visible( this->v_bar, 1 );
        sgui_widget_set_visible( this->h_bar, 1 );
    }
}
#elif defined(SGUI_NOP_IMPLEMENTATIONS)
sgui_widget* sgui_frame_create( int x, int y, unsigned int width,
                                unsigned int height )
{
    (void)x; (void)y; (void)width; (void)height;
    return NULL;
}
void sgui_frame_override_scrollbars( sgui_widget* super, int always_draw )
{
    (void)super;
    (void)always_draw;
}
#endif /* !SGUI_NO_FRAME */

