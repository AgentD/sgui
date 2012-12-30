/*
 * widget_manager.c
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
#include "sgui_widget_manager.h"
#include "sgui_widget.h"
#include "sgui_internal.h"
#include "sgui_canvas.h"
#include "sgui_event.h"

#include <stdlib.h>
#include <string.h>


#define WIDGET_MANAGER_MAX_DIRTY 10



struct sgui_widget_manager
{
    sgui_widget* mouse_over;
    sgui_widget* focus;
    sgui_widget* widgets;

    sgui_rect dirty[ WIDGET_MANAGER_MAX_DIRTY ];
    unsigned int num_dirty;

    sgui_widget_callback fun;
    void* fun_user;
};



sgui_widget_manager* sgui_widget_manager_create( void )
{
    sgui_widget_manager* mgr = malloc( sizeof(sgui_widget_manager) );

    if( mgr )
        memset( mgr, 0, sizeof(sgui_widget_manager) );

    return mgr;
}

void sgui_widget_manager_destroy( sgui_widget_manager* mgr )
{
    if( mgr )
        free( mgr );
}

void sgui_widget_manager_add_widget( sgui_widget_manager* mgr,
                                     sgui_widget* widget )
{
    unsigned int w, h;
    int x, y;
    sgui_rect r;

    if( !mgr || !widget )
        return;

    /* add widget */
    widget->mgr = mgr;
    widget->next = mgr->widgets;
    mgr->widgets = widget;

    /* flag coresponding area as dirty */
    sgui_widget_get_position( widget, &x, &y );
    sgui_widget_get_size( widget, &w, &h );

    sgui_rect_set_size( &r, x, y, w, h );

    sgui_widget_manager_add_dirty_rect( mgr, &r );
}

void sgui_widget_manager_remove_widget( sgui_widget_manager* mgr,
                                        sgui_widget* widget )
{
    unsigned int w, h;
    int x, y;
    sgui_rect r;
    sgui_widget* i;

    if( !mgr || !widget )
        return;

    for( i=mgr->widgets; i->next; i=i->next )
    {
        if( i->next == widget )
        {
            i->next = i->next->next;
            widget->mgr = NULL;

            sgui_widget_get_position( widget, &x, &y );
            sgui_widget_get_size( widget, &w, &h );

            sgui_rect_set_size( &r, x, y, w, h );

            sgui_widget_manager_add_dirty_rect( mgr, &r );
            break;
        }
    }
}

void sgui_widget_manager_add_dirty_rect( sgui_widget_manager* mgr,
                                         sgui_rect* r )
{
    unsigned int i;

    if( !mgr || !r )
        return;

    /* try to find an existing diry rect it touches */
    for( i=0; i<mgr->num_dirty; ++i )
    {
        if( sgui_rect_join( mgr->dirty + i, r, 1 ) )
            return;
    }

    /* add a new one if posible, join all existing if not */
    if( mgr->num_dirty < WIDGET_MANAGER_MAX_DIRTY )
    {
        sgui_rect_copy( mgr->dirty + (mgr->num_dirty++), r );
    }
    else
    {
        for( i=1; i<mgr->num_dirty; ++i )
            sgui_rect_join( mgr->dirty, mgr->dirty + i, 0 );

        sgui_rect_copy( mgr->dirty + 1, r );
        mgr->num_dirty = 2;
    }
}

unsigned int sgui_widget_manager_num_dirty_rects( sgui_widget_manager* mgr )
{
    return mgr ? mgr->num_dirty : 0;
}

void sgui_widget_manager_get_dirty_rect( sgui_widget_manager* mgr,
                                         sgui_rect* rect, unsigned int i )
{
    if( mgr && (i < mgr->num_dirty) )
        sgui_rect_copy( rect, mgr->dirty + i );
}

void sgui_widget_manager_clear_dirty_rects( sgui_widget_manager* mgr )
{
    if( mgr )
        mgr->num_dirty = 0;
}

void sgui_widget_manager_draw( sgui_widget_manager* mgr, sgui_canvas* cv )
{
    unsigned int j;
    sgui_rect* r;
    sgui_rect wr;
    sgui_widget* i;

    if( !mgr || !cv )
        return;

    for( j=0; j<mgr->num_dirty; ++j )
    {
        r = mgr->dirty + j;

        sgui_canvas_begin( cv, r );

        /* redraw all widgets that lie inside the current rect */
        for( i=mgr->widgets; i!=NULL; i=i->next )
        {
            sgui_widget_get_rect( i, &wr );

            if( sgui_rect_get_intersection( NULL, r, &wr ) &&
                sgui_widget_is_visible( i ) )
            {
                sgui_widget_draw( i, cv );
            }
        }

        sgui_canvas_end( cv );
    }
}

void sgui_widget_manager_draw_all( sgui_widget_manager* mgr,
                                   sgui_canvas* cv )
{
    sgui_widget* i;
    sgui_rect acc, r;

    if( mgr && cv && mgr->widgets )
    {
        i = mgr->widgets;

        /* accumulate all widget rects */
        sgui_widget_get_rect( i, &acc );

        for( i=i->next; i!=NULL; i=i->next )
        {
            if( sgui_widget_is_visible( i ) )
            {
                sgui_widget_get_rect( i, &r );
                sgui_rect_join( &acc, &r, 0 );
            }
        }

        /* draw all widgets into the accumulated rect */
        sgui_canvas_begin( cv, &acc );

        for( i=mgr->widgets; i!=NULL; i=i->next )
        {
            if( sgui_widget_is_visible( i ) )
            {
                sgui_widget_draw( i, cv );
            }
        }

        sgui_canvas_end( cv );

        /* there can't by any dirty rects anymore */
        mgr->num_dirty = 0;
    }
}

void sgui_widget_manager_send_window_event( sgui_widget_manager* mgr,
                                            int event, sgui_event* e )
{
    sgui_widget* i;
    int x, y;

    if( !mgr || !mgr->widgets )
        return;

    /* generated by the widget manager itself, propagating them through a
       nested widget manager would generate strange bahaviour */
    if( event==SGUI_FOCUS_EVENT || event==SGUI_MOUSE_ENTER_EVENT )
        return;

    if( event == SGUI_MOUSE_MOVE_EVENT )
    {
        /* find the widget under the mouse cursor */
        for( i=mgr->widgets; i!=NULL; i=i->next )
        {
            if( sgui_widget_is_point_inside( i, e->mouse_move.x,
                                                e->mouse_move.y ) )
            {
                break;
            }
        }

        /* generate mouse enter/leave events */
        if( mgr->mouse_over != i )
        {
            sgui_widget_send_window_event( i, SGUI_MOUSE_ENTER_EVENT, NULL );

            sgui_widget_send_window_event( mgr->mouse_over,
                                           SGUI_MOUSE_LEAVE_EVENT, NULL );

            mgr->mouse_over = i;
        }
    }

    switch( event )
    {
    case SGUI_MOUSE_PRESS_EVENT:
    case SGUI_MOUSE_RELEASE_EVENT:
        /* transform to widget local coordinates */
        sgui_widget_get_position( mgr->mouse_over, &x, &y );

        e->mouse_press.x -= x;
        e->mouse_press.y -= y;

        /* inject event */
        sgui_widget_send_window_event( mgr->mouse_over, event, e );

        /* give clicked widget focus */
        if( mgr->focus != mgr->mouse_over )
        {
            sgui_widget_send_window_event( mgr->focus,
                                           SGUI_FOCUS_LOSE_EVENT, NULL );

            sgui_widget_send_window_event( mgr->mouse_over,
                                           SGUI_FOCUS_EVENT, NULL );

            mgr->focus = mgr->mouse_over;
        }
        break;
    case SGUI_MOUSE_MOVE_EVENT:
        /* transform to widget local coordinates */
        sgui_widget_get_position( mgr->mouse_over, &x, &y );

        e->mouse_move.x -= x;
        e->mouse_move.y -= y;

        /* inject event */
        sgui_widget_send_window_event( mgr->mouse_over, event, e );
        break;

    /* only send to mouse over widget */
    case SGUI_MOUSE_WHEEL_EVENT:
        sgui_widget_send_window_event( mgr->mouse_over, event, e );
        break;

    /* only send keyboard events to widget that has focus */
    case SGUI_KEY_PRESSED_EVENT:
    case SGUI_KEY_RELEASED_EVENT:
    case SGUI_CHAR_EVENT:
        sgui_widget_send_window_event( mgr->focus, event, e );
        break;

    /* propagate all other events */
    default:
        for( i=mgr->widgets; i!=NULL; i=i->next )
            sgui_widget_send_window_event( i, event, e );
        break;
    }
}

void sgui_widget_manager_on_event( sgui_widget_manager* mgr,
                                   sgui_widget_callback fun, void* user )
{
    if( mgr )
    {
        mgr->fun = fun;
        mgr->fun_user = user;
    }
}

void sgui_widget_manager_fire_widget_event( sgui_widget_manager* mgr,
                                            sgui_widget* widget, int event )
{
    if( mgr && mgr->fun && widget )
        mgr->fun( widget, event, mgr->fun_user );
}

