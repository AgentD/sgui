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

#include <stdlib.h>



struct sgui_widget_manager
{
    sgui_widget*  mouse_over;
    sgui_widget*  focus;
    sgui_widget** widgets;
    unsigned int num_widgets;
    unsigned int widgets_avail;
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
}

void sgui_widget_manager_remove_widget( sgui_widget_manager* mgr,
                                        sgui_widget* widget )
{
    unsigned int i;

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
}

int sgui_widget_manager_update( sgui_widget_manager* mgr )
{
    unsigned int i, redraw = 0;

    if( !mgr )
        return 0;

    for( i=0; i<mgr->num_widgets; ++i )
    {
        sgui_widget_update( mgr->widgets[i] );

        if( sgui_widget_need_redraw( mgr->widgets[i], 1 ) )
            redraw = 1;
    }

    return redraw;
}

void sgui_widget_manager_draw( sgui_widget_manager* mgr, sgui_canvas* cv )
{
    unsigned int i;

    if( !mgr || !cv )
        return;

    for( i=0; i<mgr->num_widgets; ++i )
    {
        if( sgui_widget_need_redraw( mgr->widgets[i], 1 ) &&
            sgui_widget_is_visible( mgr->widgets[i] ) )
        {
            /* redraw widget */
            sgui_widget_draw( mgr->widgets[i], cv );

            /* clear redraw flag */
            sgui_widget_need_redraw( mgr->widgets[i], 0 );
        }
    }

    return;
}

void sgui_widget_manager_force_draw( sgui_widget_manager* mgr,
                                     sgui_canvas* cv, int x, int y,
                                     unsigned int w, unsigned int h )
{
    unsigned int i;

    if( !mgr || !cv )
        return;

    sgui_canvas_set_scissor_rect( cv, x, y, w, h );

    for( i=0; i<mgr->num_widgets; ++i )
    {
        if( sgui_widget_intersects_area( mgr->widgets[i], x, y, w, h ) &&
            sgui_widget_is_visible( mgr->widgets[i] ) )
        {
            /* make sure the need redraw flag is cleared */
            sgui_widget_need_redraw( mgr->widgets[i], 0 );

            /* draw the widget */
            sgui_widget_draw( mgr->widgets[i], cv );
        }
    }

    sgui_canvas_set_scissor_rect( cv, 0, 0, 0, 0 );
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

