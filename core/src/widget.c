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

/****************************************************************************/

void sgui_widget_destroy( sgui_widget* widget )
{
    if( widget )
        widget->destroy( widget );
}

void sgui_widget_set_position( sgui_widget* w, int x, int y )
{
    int dx, dy;
    sgui_rect r;

    if( w )
    {
        /* flag the old area dirty */
        sgui_widget_get_absolute_rect( w, &r );
        sgui_canvas_add_dirty_rect( w->canvas, &r );

        /* move the widget area */
        dx = x - w->area.left;
        dy = y - w->area.top;

        w->area.left += dx; w->area.right  += dx;
        w->area.top  += dy; w->area.bottom += dy;

        /* flag the old area dirty */
        sgui_widget_get_absolute_rect( w, &r );
        sgui_canvas_add_dirty_rect( w->canvas, &r );

        /* call the state change callback if there is one */
        if( w->state_change_callback )
            w->state_change_callback( w );
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

void sgui_widget_set_visible( sgui_widget* w, int visible )
{
    if( w )
    {
        w->visible = visible;

        if( w->state_change_callback )
            w->state_change_callback( w );
    }
}

void sgui_widget_get_rect( sgui_widget* w, sgui_rect*r )
{
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

void sgui_widget_send_window_event( sgui_widget* widget, int type,
                                    sgui_event* event )
{
    if( widget && widget->window_event_callback )
        widget->window_event_callback( widget, type, event );
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

    if( widget )
    {
        i = widget->parent->children;

        if( i==widget )
        {
            widget->parent->children = widget->parent->children->next;
        }
        else
        {
            for( ; i!=NULL; i=i->next )
            {
                if( i->next == widget )
                {
                    i->next = i->next->next;
                    break;
                }
            }
        }

        sgui_widget_get_absolute_rect( widget, &r );
        sgui_canvas_add_dirty_rect( widget->canvas, &r );

        widget->parent = NULL;
        widget->next = NULL;
        widget->canvas = NULL;

        propagate_canvas( widget->children );
    }
}

void sgui_widget_add_child( sgui_widget* parent, sgui_widget* child )
{
    sgui_rect r;

    if( !child || !parent )
        return;

    /* add widget */
    child->parent = parent;
    child->canvas = parent->canvas;
    child->next = parent->children;
    parent->children = child;

    propagate_canvas( child->children );

    /* flag coresponding area as dirty */
    sgui_widget_get_absolute_rect( child, &r );
    sgui_canvas_add_dirty_rect( child->canvas, &r );
}

