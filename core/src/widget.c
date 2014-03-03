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



void sgui_internal_widget_init( sgui_widget* this, int x, int y,
                                unsigned int width, unsigned int height )
{
    sgui_rect_set_size( &this->area, x, y, width, height );

    this->visible               = 1;
    this->next                  = NULL;
    this->children              = NULL;
    this->parent                = NULL;
    this->canvas                = NULL;
    this->draw_callback         = NULL;
    this->window_event_callback = NULL;
    this->state_change_callback = NULL;
    this->focus_policy          = SGUI_FOCUS_ACCEPT|SGUI_FOCUS_DRAW|
                                  SGUI_FOCUS_DROP_ESC|SGUI_FOCUS_DROP_TAB;
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

void sgui_widget_destroy( sgui_widget* this )
{
    if( this )
        this->destroy( this );
}

void sgui_widget_set_position( sgui_widget* this, int x, int y )
{
    sgui_rect r;
    int visible;

    if( this )
    {
        sgui_internal_lock_mutex( );

        visible = sgui_widget_is_absolute_visible( this );

        /* flag the old area dirty */
        if( visible )
        {
            sgui_widget_get_absolute_rect( this, &r );
            sgui_canvas_add_dirty_rect( this->canvas, &r );
        }

        /* move the widget area */
        sgui_rect_set_position( &this->area, x, y );

        /* flag the new area dirty */
        if( visible )
        {
            sgui_widget_get_absolute_rect( this, &r );
            sgui_canvas_add_dirty_rect( this->canvas, &r );
        }

        /* call the state change callback if there is one */
        if( this->state_change_callback )
            this->state_change_callback( this, WIDGET_POSITION_CHANGED );

        sgui_internal_unlock_mutex( );
    }
}

void sgui_widget_get_position( const sgui_widget* this, int* x, int* y )
{
    if( this )
    {
        if( x ) *x = this->area.left;
        if( y ) *y = this->area.top;
    }
    else
    {
        if( x ) *x = 0;
        if( y ) *y = 0;
    }
}

void sgui_widget_get_absolute_position( const sgui_widget* this,
                                        int* x, int* y )
{
    const sgui_widget* i;
    int X, Y;

    sgui_internal_lock_mutex( );

    for( X=0, Y=0, i=this; i!=NULL; i=i->parent )
    {
        X += i->area.left;
        Y += i->area.top;
    }

    sgui_internal_unlock_mutex( );

    if( x ) *x = X;
    if( y ) *y = Y;
}

void sgui_widget_get_size( const sgui_widget* this,
                           unsigned int* width, unsigned int* height )
{
    if( this )
    {
        if( width  ) *width  = SGUI_RECT_WIDTH( this->area );
        if( height ) *height = SGUI_RECT_HEIGHT( this->area );
    }
    else
    {
        if( width  ) *width  = 0;
        if( height ) *height = 0;
    }
}

int sgui_widget_is_visible( const sgui_widget* this )
{
    return this ? this->visible : 0;
}

int sgui_widget_is_absolute_visible( const sgui_widget* this )
{
    sgui_internal_lock_mutex( );

    for( ; this!=NULL; this=this->parent )
    {
        if( !this->visible )
        {
            sgui_internal_unlock_mutex( );
            return 0;
        }
    }

    sgui_internal_unlock_mutex( );
    return 1;
}

void sgui_widget_set_visible( sgui_widget* this, int visible )
{
    sgui_rect r;

    if( this && this->visible!=visible )
    {
        sgui_internal_lock_mutex( );

        this->visible = visible;

        if( this->state_change_callback )
            this->state_change_callback( this, WIDGET_VISIBILLITY_CHANGED );

        propagat_state_change( this->children, WIDGET_VISIBILLITY_CHANGED );

        /* flag area as dirty */ 
        sgui_widget_get_absolute_rect( this, &r );
        sgui_canvas_add_dirty_rect( this->canvas, &r );

        sgui_internal_unlock_mutex( );
    }
}

void sgui_widget_get_rect( const sgui_widget* this, sgui_rect*r )
{
    if( this && r )
        sgui_rect_copy( r, &this->area );
}

void sgui_widget_get_absolute_rect( const sgui_widget* this, sgui_rect* r )
{
    sgui_widget* i;

    if( this && r )
    {
        sgui_internal_lock_mutex( );
        sgui_rect_copy( r, &this->area );

        for( i=this->parent; i!=NULL; i=i->parent )
        {
            /* transform to parent local */
            r->left   += i->area.left;
            r->right  += i->area.left;
            r->top    += i->area.top;
            r->bottom += i->area.top;

            /* clip against the parent rectangle */
            if( !sgui_rect_get_intersection( r, r, &i->area ) )
                break;
        }

        sgui_internal_unlock_mutex( );
    }
}

void sgui_widget_send_event( sgui_widget* this, const sgui_event* event,
                             int propagate )
{
    sgui_widget* i;

    if( this )
    {
        if( this->window_event_callback )
            this->window_event_callback( this, event );

        if( propagate )
        {
            sgui_internal_lock_mutex( );

            for( i=this->children; i!=NULL; i=i->next )
            {
                sgui_widget_send_event( i, event, 1 );
            }

            sgui_internal_unlock_mutex( );
        }
    }
}

void sgui_widget_draw( sgui_widget* this )
{
    if( this && this->draw_callback )
        this->draw_callback( this );
}

void sgui_widget_remove_from_parent( sgui_widget* this )
{
    sgui_rect r;
    sgui_widget* i = NULL;
    int change = WIDGET_PARENT_CHANGED;

    if( this && this->parent )
    {
        sgui_internal_lock_mutex( );

        i = this->parent->children;

        SGUI_REMOVE_FROM_LIST( this->parent->children, i, this );

        if( sgui_widget_is_absolute_visible( this ) )
        {
            sgui_widget_get_absolute_rect( this, &r );
            sgui_canvas_add_dirty_rect( this->canvas, &r );
        }

        /* add canvas change flag if the widget had a canvas before */
        if( this->canvas )
            change |= WIDGET_CANVAS_CHANGED;

        /* store a pointer to the old parent */
        i = this->parent;

        /* update links and canvas */
        this->parent = NULL;
        this->next = NULL;
        this->canvas = NULL;

        propagate_canvas( this->children );

        /* call state change callbacks */
        if( i && i->state_change_callback )
            i->state_change_callback( i, WIDGET_CHILD_REMOVED );

        if( this->state_change_callback )
            this->state_change_callback( this, change );

        if( change & WIDGET_CANVAS_CHANGED )
            propagat_state_change( this->children, WIDGET_CANVAS_CHANGED );

        sgui_internal_unlock_mutex( );
    }
}

void sgui_widget_add_child( sgui_widget* this, sgui_widget* child )
{
    sgui_rect r;
    sgui_widget* i;
    int change = WIDGET_PARENT_CHANGED;

    if( !child || !this )
        return;

    /* add canvas change flag if the widget had a different canvas before */
    if( child->canvas != this->canvas )
        change |= WIDGET_CANVAS_CHANGED;

    sgui_internal_lock_mutex( );

    /* add widget */
    child->parent = this;
    child->canvas = this->canvas;

    if( this->children )
    {
        for( i=this->children; i->next; i=i->next );
        i->next = child;
        child->next = NULL;
    }
    else
    {
        this->children = child;
    }

    propagate_canvas( child->children );

    /* flag coresponding area as dirty */
    if( sgui_widget_is_absolute_visible( child ) )
    {
        sgui_widget_get_absolute_rect( child, &r );
        sgui_canvas_add_dirty_rect( child->canvas, &r );
    }

    /* call state change callbacks */
    if( this->state_change_callback )
        this->state_change_callback( this, WIDGET_CHILD_ADDED );

    if( child->state_change_callback )
        child->state_change_callback( child, change );

    if( change & WIDGET_CANVAS_CHANGED )
        propagat_state_change( child->children, WIDGET_CANVAS_CHANGED );

    sgui_internal_unlock_mutex( );
}

sgui_widget* sgui_widget_get_child_from_point( const sgui_widget* this,
                                               int x, int y )
{
    const sgui_widget* candidate = this;
    sgui_widget* it;

    /* check if the given widget exists and the point is inside */
    if( !this || !sgui_rect_is_point_inside( &this->area, x, y ) )
        return NULL;

    sgui_internal_lock_mutex( );

    do
    {
        /* continue searching last candidate, clear candidate */
        this = candidate;
        candidate = NULL;

        /* transform to widget local coordinates */
        x -= this->area.left;
        y -= this->area.top;

        /* find last child at position */
        for( it=this->children; it!=NULL; it=it->next )
        {
            if( it->visible && sgui_rect_is_point_inside( &it->area, x, y ) )
                candidate = it;
        }
    }
    while( candidate );

    sgui_internal_unlock_mutex( );

    return (sgui_widget*)this;
}

