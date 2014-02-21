/*
 * widget.c
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
#include "sgui_widget.h"
#include "sgui_internal.h"

#include <stddef.h>



void sgui_internal_widget_init( sgui_widget* widget, int x, int y,
                                unsigned int width, unsigned int height )
{
    sgui_rect_set_size( &widget->area, x, y, width, height );

    widget->visible               = 1;
    widget->next                  = NULL;
    widget->children              = NULL;
    widget->parent                = NULL;
    widget->canvas                = NULL;
    widget->draw_callback         = NULL;
    widget->window_event_callback = NULL;
    widget->state_change_callback = NULL;
}

/****************************************************************************/

static void propagate_canvas( sgui_widget* i )
{
    for( ; i!=NULL; i=i->next )
    {
        i->canvas = i->parent->canvas;
        propagate_canvas( i->children );
    }
}

static void propagat_state_change( sgui_widget* i, int change )
{
    for( ; i!=NULL; i=i->next )
    {
        if( i->state_change_callback )
            i->state_change_callback( i, change );

        propagat_state_change( i->children, change );
    }
}

/****************************************************************************/

void sgui_widget_destroy( sgui_widget* widget )
{
    if( widget )
        widget->destroy( widget );
}

void sgui_widget_set_position( sgui_widget* w, int x, int y )
{
    sgui_rect r;
    int visible;

    if( w )
    {
        visible = sgui_widget_is_absolute_visible( w );

        /* flag the old area dirty */
        if( visible )
        {
            sgui_widget_get_absolute_rect( w, &r );
            sgui_canvas_add_dirty_rect( w->canvas, &r );
        }

        /* move the widget area */
        sgui_rect_set_position( &w->area, x, y );

        /* flag the new area dirty */
        if( visible )
        {
            sgui_widget_get_absolute_rect( w, &r );
            sgui_canvas_add_dirty_rect( w->canvas, &r );
        }

        /* call the state change callback if there is one */
        if( w->state_change_callback )
            w->state_change_callback( w, WIDGET_POSITION_CHANGED );
    }
}

void sgui_widget_get_position( sgui_widget* w, int* x, int* y )
{
    if( w )
    {
        if( x ) *x = w->area.left;
        if( y ) *y = w->area.top;
    }
    else
    {
        if( x ) *x = 0;
        if( y ) *y = 0;
    }
}

void sgui_widget_get_absolute_position( sgui_widget* w, int* x, int* y )
{
    sgui_widget* i;
    int X, Y;

    for( X=0, Y=0, i=w; i!=NULL; i=i->parent )
    {
        X += i->area.left;
        Y += i->area.top;
    }

    if( x ) *x = X;
    if( y ) *y = Y;
}

void sgui_widget_get_size( sgui_widget* w,
                           unsigned int* width, unsigned int* height )
{
    if( w )
    {
        if( width  ) *width  = SGUI_RECT_WIDTH( w->area );
        if( height ) *height = SGUI_RECT_HEIGHT( w->area );
    }
    else
    {
        if( width  ) *width  = 0;
        if( height ) *height = 0;
    }
}

int sgui_widget_is_visible( sgui_widget* w )
{
    return w ? w->visible : 0;
}

int sgui_widget_is_absolute_visible( sgui_widget* w )
{
    for( ; w!=NULL; w=w->parent )
    {
        if( !w->visible )
            return 0;
    }

    return 1;
}

void sgui_widget_set_visible( sgui_widget* w, int visible )
{
    sgui_rect r;

    if( w && w->visible!=visible )
    {
        w->visible = visible;

        if( w->state_change_callback )
            w->state_change_callback( w, WIDGET_VISIBILLITY_CHANGED );

        propagat_state_change( w->children, WIDGET_VISIBILLITY_CHANGED );

        /* flag area as dirty */ 
        sgui_widget_get_absolute_rect( w, &r );
        sgui_canvas_add_dirty_rect( w->canvas, &r );
    }
}

void sgui_widget_get_rect( sgui_widget* w, sgui_rect*r )
{
    if( w && r )
        sgui_rect_copy( r, &w->area );
}

void sgui_widget_get_absolute_rect( sgui_widget* w, sgui_rect* r )
{
    sgui_widget* i;

    if( w && r )
    {
        sgui_rect_copy( r, &w->area );

        for( i=w->parent; i!=NULL; i=i->parent )
        {
            /* transform to parent local */
            r->left   += i->area.left;
            r->right  += i->area.left;
            r->top    += i->area.top;
            r->bottom += i->area.top;

            /* clip against the parent rectangle */
            if( !sgui_rect_get_intersection( r, r, &i->area ) )
                return;
        }
    }
}

void sgui_widget_send_event( sgui_widget* widget, sgui_event* event,
                             int propagate )
{
    sgui_widget* i;

    if( widget )
    {
        if( widget->window_event_callback )
            widget->window_event_callback( widget, event );

        if( propagate )
        {
            for( i=widget->children; i!=NULL; i=i->next )
            {
                sgui_widget_send_event( i, event, 1 );
            }
        }
    }
}

void sgui_widget_draw( sgui_widget* widget )
{
    if( widget && widget->draw_callback )
        widget->draw_callback( widget );
}

void sgui_widget_remove_from_parent( sgui_widget* widget )
{
    sgui_rect r;
    sgui_widget* i = NULL;
    int change = WIDGET_PARENT_CHANGED;

    if( widget && widget->parent )
    {
        i = widget->parent->children;

        SGUI_REMOVE_FROM_LIST( widget->parent->children, i, widget );

        if( sgui_widget_is_absolute_visible( widget ) )
        {
            sgui_widget_get_absolute_rect( widget, &r );
            sgui_canvas_add_dirty_rect( widget->canvas, &r );
        }

        /* add canvas change flag if the widget had a canvas before */
        if( widget->canvas )
            change |= WIDGET_CANVAS_CHANGED;

        /* store a pointer to the old parent */
        i = widget->parent;

        /* update links and canvas */
        widget->parent = NULL;
        widget->next = NULL;
        widget->canvas = NULL;

        propagate_canvas( widget->children );

        /* call state change callbacks */
        if( i && i->state_change_callback )
            i->state_change_callback( i, WIDGET_CHILD_REMOVED );

        if( widget->state_change_callback )
            widget->state_change_callback( widget, change );

        if( change & WIDGET_CANVAS_CHANGED )
            propagat_state_change( widget->children, WIDGET_CANVAS_CHANGED );
    }
}

void sgui_widget_add_child( sgui_widget* parent, sgui_widget* child )
{
    sgui_rect r;
    int change = WIDGET_PARENT_CHANGED;

    if( !child || !parent )
        return;

    /* add canvas change flag if the widget had a different canvas before */
    if( child->canvas != parent->canvas )
        change |= WIDGET_CANVAS_CHANGED;

    /* add widget */
    child->parent = parent;
    child->canvas = parent->canvas;

    SGUI_ADD_TO_LIST( parent->children, child );

    propagate_canvas( child->children );

    /* flag coresponding area as dirty */
    if( sgui_widget_is_absolute_visible( child ) )
    {
        sgui_widget_get_absolute_rect( child, &r );
        sgui_canvas_add_dirty_rect( child->canvas, &r );
    }

    /* call state change callbacks */
    if( parent->state_change_callback )
        parent->state_change_callback( parent, WIDGET_CHILD_ADDED );

    if( child->state_change_callback )
        child->state_change_callback( child, change );

    if( change & WIDGET_CANVAS_CHANGED )
        propagat_state_change( child->children, WIDGET_CANVAS_CHANGED );
}

sgui_widget* sgui_widget_get_child_from_point( sgui_widget* widget,
                                               int x, int y )
{
    sgui_widget* candidate = widget;
    sgui_widget* it;

    /* check if the given widget exists and the point is inside */
    if( !widget || !sgui_rect_is_point_inside( &widget->area, x, y ) )
        return NULL;

    do
    {
        /* continue searching last candidate, clear candidate */
        widget = candidate;
        candidate = NULL;

        /* transform to widget local coordinates */
        x -= widget->area.left;
        y -= widget->area.top;

        /* find last child at position */
        for( it=widget->children; it!=NULL; it=it->next )
        {
            if( it->visible && sgui_rect_is_point_inside( &it->area, x, y ) )
                candidate = it;
        }
    }
    while( candidate );

    return widget;
}

