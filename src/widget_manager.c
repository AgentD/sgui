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
#include "sgui_canvas.h"

#include <stdlib.h>



struct sgui_widget_manager
{
    sgui_widget*  mouse_over;
    sgui_widget*  focus;
    sgui_widget** widgets;
    unsigned int num_widgets;
    unsigned int widgets_avail;
    int clear_bg;
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
    mgr->clear_bg      = 1;

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

void sgui_widget_manager_enable_clear( sgui_widget_manager* mgr, int enable )
{
    if( mgr )
        mgr->clear_bg = enable;
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
    unsigned int i, w, h;
    int x, y;

    if( !mgr || !cv )
        return;

    if( mgr->clear_bg )
    {
        for( i=0; i<mgr->num_widgets; ++i )
        {
            if( sgui_widget_need_redraw( mgr->widgets[i], 0 ) )
            {
                /* clear the background */
                sgui_widget_get_position( mgr->widgets[i], &x, &y );
                sgui_widget_get_size( mgr->widgets[i], &w, &h );

                sgui_canvas_clear( cv, x, y, w, h );

                /* draw the widget */
                sgui_widget_draw( mgr->widgets[i], cv );
            }
        }
    }
    else
    {
        for( i=0; i<mgr->num_widgets; ++i )
        {
            if( sgui_widget_need_redraw( mgr->widgets[i], 0 ) )
                sgui_widget_draw( mgr->widgets[i], cv );
        }
    }

    return;
}

void sgui_widget_manager_force_draw( sgui_widget_manager* mgr,
                                     sgui_canvas* cv, int x, int y,
                                     unsigned int w, unsigned int h )
{
    unsigned int i, ww, wh;
    int wx, wy;

    if( !mgr || !cv )
        return;

    sgui_canvas_set_scissor_rect( cv, x, y, w, h );

    if( mgr->clear_bg )
    {
        for( i=0; i<mgr->num_widgets; ++i )
        {
            if( sgui_widget_intersects_area( mgr->widgets[i], x, y, w, h ) )
            {
                /* make sure the need redraw flag is cleared */
                sgui_widget_need_redraw( mgr->widgets[i], 0 );

                /* clear the background */
                sgui_widget_get_position( mgr->widgets[i], &wx, &wy );
                sgui_widget_get_size( mgr->widgets[i], &ww, &wh );

                sgui_canvas_clear( cv, wx, wy, ww, wh );

                /* draw the widget */
                sgui_widget_draw( mgr->widgets[i], cv );
            }
        }
    }
    else
    {
        for( i=0; i<mgr->num_widgets; ++i )
        {
            if( sgui_widget_intersects_area( mgr->widgets[i], x, y, w, h ) )
            {
                /* make sure the need redraw flag is cleared */
                sgui_widget_need_redraw( mgr->widgets[i], 0 );

                /* draw the widget */
                sgui_widget_draw( mgr->widgets[i], cv );
            }
        }
    }

    sgui_canvas_set_scissor_rect( cv, 0, 0, 0, 0 );
}

void sgui_widget_manager_send_window_event( sgui_widget_manager* mgr,
                                            int event, sgui_event* e )
{
    unsigned int i;
    sgui_widget* new_mouse_over = NULL;

    if( !mgr )
        return;

    if( event == SGUI_MOUSE_MOVE_EVENT )
    {
        /* find the widget under the mouse cursor */
        for( i=0; i<mgr->num_widgets; ++i )
        {
            if( sgui_widget_is_point_inside( mgr->widgets[i],
                                             e->mouse_move.x,
                                             e->mouse_move.y ) )
            {
                new_mouse_over = mgr->widgets[i];
                break;
            }
        }

        /* old widget under cursor != new widget under cursor */
        if( mgr->mouse_over != new_mouse_over )
        {
            /* send mouse enter event to new one */
            if( new_mouse_over )
            {
                sgui_widget_send_window_event( new_mouse_over,
                                               SGUI_MOUSE_ENTER_EVENT,
                                               NULL );
            }

            /* send mouse leave event to old one */
            if( mgr->mouse_over )
            {
                sgui_widget_send_window_event( mgr->mouse_over,
                                               SGUI_MOUSE_LEAVE_EVENT,
                                               NULL );
            }

            /* store the new one */
            mgr->mouse_over = new_mouse_over;
        }
    }

    if( event==SGUI_MOUSE_PRESS_EVENT || event==SGUI_MOUSE_RELEASE_EVENT )
    {
        /*
            only send mouse press/release events to the mouse_over widget
            and give it focus if it got clicked
         */
        if( mgr->mouse_over )
        {
            sgui_widget_send_window_event( mgr->mouse_over, event, e );

            if( mgr->focus != mgr->mouse_over )
            {
                if( mgr->focus )
                    sgui_widget_send_window_event( mgr->focus,
                                                   SGUI_FOCUS_LOSE_EVENT,
                                                   NULL );

                sgui_widget_send_window_event( mgr->mouse_over,
                                               SGUI_FOCUS_EVENT, NULL );

                mgr->focus = mgr->mouse_over;
            }
        }
        else
        {
            if( mgr->focus )
                sgui_widget_send_window_event( mgr->focus,
                                               SGUI_FOCUS_LOSE_EVENT, NULL );

            mgr->focus = NULL;
        }
    }
    else if( event==SGUI_MOUSE_WHEEL_EVENT )
    {
        /* only send mouse wheel events to mouse over widget */
        if( mgr->mouse_over )
            sgui_widget_send_window_event( mgr->mouse_over, event, e );
    }
    else if( (event==SGUI_KEY_PRESSED_EVENT) ||
             (event==SGUI_KEY_RELEASED_EVENT) || (event==SGUI_CHAR_EVENT) )
    {
        /* only send keyboard events to the widget that has focus */
        if( mgr->focus )
            sgui_widget_send_window_event( mgr->focus, event, e );
    }
    else
    {
        /* propagate all other events */
        for( i=0; i<mgr->num_widgets; ++i )
            sgui_widget_send_window_event( mgr->widgets[i], event, e );
    }
}

