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
#include "sgui_widget_manager.h"
#include "sgui_widget.h"
#include "sgui_internal.h"

#include <stdlib.h>



#define WIDGET_MANAGER_MAX_DIRTY 10



struct sgui_widget_manager
{
    sgui_widget*  mouse_over;
    sgui_widget*  focus;
    sgui_widget** widgets;
    unsigned int num_widgets;
    unsigned int widgets_avail;

    sgui_rect dirty[ WIDGET_MANAGER_MAX_DIRTY ];
    unsigned int num_dirty;

    sgui_widget_callback fun;
    void* fun_user;
};



sgui_widget_manager* sgui_widget_manager_create( void )
{
    sgui_widget_manager* mgr = malloc( sizeof(sgui_widget_manager) );

    if( !mgr )
        return NULL;

    mgr->widgets       = malloc( sizeof(sgui_widget*)*10 );
    mgr->mouse_over    = NULL;
    mgr->focus         = NULL;
    mgr->num_widgets   = 0;
    mgr->widgets_avail = 10;
    mgr->num_dirty     = 0;
    mgr->fun           = NULL;
    mgr->fun_user      = NULL;

    if( !mgr->widgets )
    {
        free( mgr );
        return NULL;
    }

    return mgr;
}

void sgui_widget_manager_destroy( sgui_widget_manager* mgr )
{
    if( mgr )
    {
        free( mgr->widgets );
        free( mgr );
    }
}

void sgui_widget_manager_add_widget( sgui_widget_manager* mgr,
                                     sgui_widget* widget )
{
    sgui_widget** nw;
    unsigned int w, h;
    int x, y;
    sgui_rect r;

    if( !mgr || !widget )
        return;

    /* try to resize widget array if required */
    if( mgr->num_widgets == mgr->widgets_avail )
    {
        mgr->widgets_avail += 10;

        nw = realloc( mgr->widgets, mgr->widgets_avail*sizeof(sgui_widget*) );

        if( !nw )
        {
            mgr->widgets_avail -= 10;
            return;
        }

        mgr->widgets = nw;
    }

    /* add widget */
    mgr->widgets[ mgr->num_widgets++ ] = widget;

    widget->mgr = mgr;

    sgui_widget_get_position( widget, &x, &y );
    sgui_widget_get_size( widget, &w, &h );

    sgui_rect_set_size( &r, x, y, w, h );

    sgui_widget_manager_add_dirty_rect( mgr, &r );
}

void sgui_widget_manager_remove_widget( sgui_widget_manager* mgr,
                                        sgui_widget* widget )
{
    unsigned int i, w, h;
    int x, y;
    sgui_rect r;

    if( !mgr || !widget )
        return;

    for( i=0; i<mgr->num_widgets; ++i )
    {
        if( mgr->widgets[ i ] == widget )
        {
            for( ; i<(mgr->num_widgets-1); ++i )
                mgr->widgets[ i ] = mgr->widgets[ i+1 ];

            --mgr->num_widgets;
            break;
        }
    }

    sgui_widget_get_position( widget, &x, &y );
    sgui_widget_get_size( widget, &w, &h );

    sgui_rect_set_size( &r, x, y, w, h );

    sgui_widget_manager_add_dirty_rect( mgr, &r );

    widget->mgr = NULL;
}

void sgui_widget_manager_update( sgui_widget_manager* mgr )
{
    unsigned int i;

    if( mgr )
    {
        for( i=0; i<mgr->num_widgets; ++i )
            sgui_widget_update( mgr->widgets[i] );
    }
}

void sgui_widget_manager_add_dirty_rect( sgui_widget_manager* mgr,
                                         sgui_rect* r )
{
    unsigned int i;

    if( !mgr || !r )
        return;

    for( i=0; i<mgr->num_dirty; ++i )
    {
        if( sgui_rect_join( mgr->dirty + i, r, 1 ) )
            return;
    }

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
    unsigned int i, j;
    sgui_rect* r;
    sgui_rect wr;

    if( !mgr || !cv )
        return;

    for( j=0; j<mgr->num_dirty; ++j )
    {
        r = mgr->dirty + j;

        sgui_canvas_begin( cv, r );

        for( i=0; i<mgr->num_widgets; ++i )
        {
            sgui_widget_get_rect( mgr->widgets[i], &wr );

            if( sgui_rect_get_intersection( NULL, r, &wr ) &&
                sgui_widget_is_visible( mgr->widgets[i] ) )
            {
                sgui_widget_draw( mgr->widgets[i], cv );
            }
        }

        sgui_canvas_end( cv );
    }
}

void sgui_widget_manager_draw_all( sgui_widget_manager* mgr,
                                   sgui_canvas* cv )
{
    unsigned int i;
    sgui_rect acc, r;

    if( mgr && cv && mgr->num_widgets )
    {
        sgui_widget_get_rect( mgr->widgets[0], &acc );

        for( i=1; i<mgr->num_widgets; ++i )
        {
            if( sgui_widget_is_visible( mgr->widgets[i] ) )
            {
                sgui_widget_get_rect( mgr->widgets[i], &r );
                sgui_rect_join( &acc, &r, 0 );
            }
        }

        sgui_canvas_begin( cv, &acc );

        for( i=0; i<mgr->num_widgets; ++i )
        {
            if( sgui_widget_is_visible( mgr->widgets[i] ) )
            {
                sgui_widget_draw( mgr->widgets[i], cv );
            }
        }

        sgui_canvas_end( cv );

        mgr->num_dirty = 0;
    }
}

void sgui_widget_manager_send_window_event( sgui_widget_manager* mgr,
                                            int event, sgui_event* e )
{
    unsigned int i;
    int x, y;
    sgui_widget* new_mouse_over = NULL;

    if( !mgr )
        return;

    /* generated by the widget manager itself, propagating them through a
       nested widget manager would generate strange bahaviour             */
    if( event==SGUI_FOCUS_EVENT || event==SGUI_MOUSE_ENTER_EVENT )
        return;

    if( event == SGUI_MOUSE_MOVE_EVENT )
    {
        /* find the widget under the mouse cursor */
        for( i=0; i<mgr->num_widgets; ++i )
        {
            if( sgui_widget_is_point_inside( mgr->widgets[i], e->mouse_move.x,
                                             e->mouse_move.y ) )
            {
                new_mouse_over = mgr->widgets[i];
                break;
            }
        }

        if( mgr->mouse_over != new_mouse_over )
        {
            sgui_widget_send_window_event( new_mouse_over,
                                           SGUI_MOUSE_ENTER_EVENT, NULL );

            sgui_widget_send_window_event( mgr->mouse_over,
                                           SGUI_MOUSE_LEAVE_EVENT, NULL );

            mgr->mouse_over = new_mouse_over;
        }
    }

    switch( event )
    {
    case SGUI_MOUSE_PRESS_EVENT:
    case SGUI_MOUSE_RELEASE_EVENT:
        sgui_widget_get_position( mgr->mouse_over, &x, &y );

        e->mouse_press.x -= x;
        e->mouse_press.y -= y;

        sgui_widget_send_window_event( mgr->mouse_over, event, e );

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
        sgui_widget_get_position( mgr->mouse_over, &x, &y );

        e->mouse_move.x -= x;
        e->mouse_move.y -= y;

        sgui_widget_send_window_event( mgr->mouse_over, event, e );
        break;
    case SGUI_MOUSE_WHEEL_EVENT:
        sgui_widget_send_window_event( mgr->mouse_over, event, e );
        break;
    case SGUI_KEY_PRESSED_EVENT:
    case SGUI_KEY_RELEASED_EVENT:
    case SGUI_CHAR_EVENT:
        sgui_widget_send_window_event( mgr->focus, event, e );
        break;
    default:
        for( i=0; i<mgr->num_widgets; ++i )
            sgui_widget_send_window_event( mgr->widgets[i], event, e );
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

