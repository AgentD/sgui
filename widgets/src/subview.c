/*
 * subview.c
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
#include "sgui_subview.h"
#include "sgui_event.h"
#include "sgui_widget.h"
#include "sgui_internal.h"

#include <stdlib.h>



typedef struct
{
    sgui_widget widget;

    sgui_window* subwnd;

    sgui_subview_window_fun window_fun;
    sgui_subview_draw_fun draw_fun;
}
sgui_subview;



/* subview event handler for events from the parent canvas */
static void subview_on_parent_event( sgui_widget* w, int type,
                                     sgui_event* event )
{
    sgui_subview* view = (sgui_subview*)w;

    /* handle internal events not to be redirected */
    switch( type )
    {
    case SGUI_TAB_DESELECTED:
        sgui_widget_set_visible( w, SGUI_INVISIBLE );
        return;
    case SGUI_TAB_SELECTED:
        sgui_widget_set_visible( w, SGUI_VISIBLE );
        return;
    }

    /* send events accordingly */
    if( view->window_fun )
        view->window_fun( w, type, event );
}

/* subview event handler for events from the managed subwindow */
static void subview_on_subwindow_event( sgui_window* wnd, int type,
                                        sgui_event* event )
{
    sgui_subview* view;
    (void)type; (void)event;

    view = sgui_window_get_userptr( wnd );

    /* perform a redraw if required */
    if( type==SGUI_SIZE_CHANGE_EVENT || type==SGUI_EXPOSE_EVENT )
    {
        if( view->draw_fun )
        {
            view->draw_fun( (sgui_widget*)view );
        }
    }
}

/* subview event handler for widget state change events from base class */
static void subview_on_state_change( sgui_widget* w, int change )
{
    sgui_subview* view = (sgui_subview*)w;
    unsigned int ww, wh;
    int x, y, visible;

    /* adjust size and position of the subwindow */
    if( change & (WIDGET_POSITION_CHANGED|WIDGET_PARENT_CHANGED) )
    {
        sgui_widget_get_absolute_position( w, &x, &y );
        sgui_widget_get_size( w, &ww, &wh );

        sgui_window_move( view->subwnd, x, y );
        sgui_window_set_size( view->subwnd, ww, wh );
    }

    /* "adjust" visibillity of the window */
    if( change & WIDGET_VISIBILLITY_CHANGED )
    {
        visible = sgui_widget_is_absolute_visible( w );

        sgui_window_set_visible( view->subwnd, visible );
    }
}

static void subview_destroy( sgui_widget* widget )
{
    sgui_window_destroy( ((sgui_subview*)widget)->subwnd );
    free( widget );
}

/****************************************************************************/

sgui_widget* sgui_subview_create( sgui_window* parent, int x, int y,
                                  unsigned int width, unsigned int height,
                                  int backend )
{
    sgui_subview* view;

    /* sanity check */
    if( !parent )
        return NULL;

    /* allocate storage for the view */
    view = malloc( sizeof(sgui_subview) );

    if( !view )
        return NULL;

    /* initialise the window */
    sgui_internal_widget_init( (sgui_widget*)view, x, y, width, height );
    view->widget.window_event_callback = subview_on_parent_event;
    view->widget.state_change_callback = subview_on_state_change;
    view->widget.destroy               = subview_destroy;
    view->window_fun = NULL;
    view->draw_fun = NULL;

    /* create the OpenGL subwindow */
    view->subwnd = sgui_window_create( parent, width, height, 0, backend );

    if( !view->subwnd )
    {
        free( view );
        return NULL;
    }

    sgui_window_set_visible( view->subwnd, SGUI_VISIBLE );
    sgui_window_on_event( view->subwnd, subview_on_subwindow_event );
    sgui_window_set_userptr( view->subwnd, view );

    return (sgui_widget*)view;
}

void sgui_subview_set_background_color( sgui_widget* subview,
                                        unsigned char* color )
{
    sgui_subview* view = (sgui_subview*)subview;
    sgui_canvas* cv;

    if( view && color )
    {
        cv = sgui_window_get_canvas( view->subwnd );
        sgui_canvas_set_background_color( cv, color );
    }
}

sgui_window* sgui_subview_get_window( sgui_widget* subview )
{
    return subview ? ((sgui_subview*)subview)->subwnd : NULL;
}

void sgui_subview_set_draw_callback( sgui_widget* subview,
                                     sgui_subview_draw_fun drawcb )
{
    if( subview )
    {
        ((sgui_subview*)subview)->draw_fun = drawcb;
    }
}

void sgui_subview_on_window_event( sgui_widget* subview,
                                   sgui_subview_window_fun windowcb )
{
    if( subview )
    {
        ((sgui_subview*)subview)->window_fun = windowcb;
    }
}

