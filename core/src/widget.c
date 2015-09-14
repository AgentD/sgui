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
#include "sgui_event.h"
#include "sgui_widget.h"
#include "sgui_internal.h"

#include <stddef.h>



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
        if( i->state_change_event )
            i->state_change_event( i, change );

        propagat_state_change( i->children, change );
    }
}

static sgui_widget* find_child_focus( const sgui_widget* this )
{
    sgui_widget* candidate = NULL;
    sgui_widget* w;

    for( w=this->children; w!=NULL; w=w->next )
    {
        if( w->flags & SGUI_WIDGET_VISIBLE )
        {
            if( w->flags & SGUI_FOCUS_ACCEPT )
                return w;

            candidate = candidate ? candidate : find_child_focus( w );
        }
    }

    return candidate;
}

/****************************************************************************/

void sgui_widget_init( sgui_widget* this, int x, int y,
                       unsigned int width, unsigned int height )
{
    sgui_rect_set_size( &this->area, x, y, width, height );

    this->next               = NULL;
    this->children           = NULL;
    this->parent             = NULL;
    this->canvas             = NULL;
    this->draw               = NULL;
    this->window_event       = NULL;
    this->state_change_event = NULL;
    this->flags              = SGUI_FOCUS_ACCEPT|SGUI_FOCUS_DRAW|
                               SGUI_FOCUS_DROP_ESC|SGUI_FOCUS_DROP_TAB|
                               SGUI_WIDGET_VISIBLE;
}

void sgui_widget_destroy_children( sgui_widget* this )
{
    sgui_widget *i, *old;

    sgui_internal_lock_mutex( );

    i = this->children;
    while( i!=NULL )
    {
        old = i;
        i = i->next;
        old->destroy( old );
    }

    this->children = NULL;

    sgui_internal_unlock_mutex( );
}

void sgui_widget_destroy_all_children( sgui_widget* this )
{
    sgui_widget* i;

    sgui_internal_lock_mutex( );
    for( i=this->children; i!=NULL; i=i->next )
        sgui_widget_destroy_all_children( i );

    sgui_widget_destroy_children( this );
    sgui_internal_unlock_mutex( );
}

void sgui_widget_set_position( sgui_widget* this, int x, int y )
{
    sgui_rect r;
    int visible;

    sgui_internal_lock_mutex( );

    visible = sgui_widget_is_absolute_visible( this );

    /* flag the old area dirty */
    if( visible && this->canvas )
    {
        sgui_widget_get_absolute_rect( this, &r );
        sgui_canvas_add_dirty_rect( this->canvas, &r );
    }

    /* move the widget area */
    sgui_rect_set_position( &this->area, x, y );

    /* flag the new area dirty */
    if( visible && this->canvas )
    {
        sgui_widget_get_absolute_rect( this, &r );
        sgui_canvas_add_dirty_rect( this->canvas, &r );
    }

    /* call the state change callback if there is one */
    if( this->state_change_event )
        this->state_change_event( this, SGUI_WIDGET_POSITION_CHANGED );

    sgui_internal_unlock_mutex( );
}

void sgui_widget_get_absolute_position( const sgui_widget* this,
                                        int* x, int* y )
{
    const sgui_widget* i;

    sgui_internal_lock_mutex( );

    for( *x=0, *y=0, i=this; i!=NULL; i=i->parent )
    {
        *x += i->area.left;
        *y += i->area.top;
    }

    sgui_internal_unlock_mutex( );
}

void sgui_widget_get_size( const sgui_widget* this,
                           unsigned int* width, unsigned int* height )
{
    *width  = SGUI_RECT_WIDTH( this->area );
    *height = SGUI_RECT_HEIGHT( this->area );
}

int sgui_widget_is_absolute_visible( const sgui_widget* this )
{
    sgui_internal_lock_mutex( );

    for( ; this!=NULL; this=this->parent )
    {
        if( !(this->flags & SGUI_WIDGET_VISIBLE) )
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

    if( ((this->flags & SGUI_WIDGET_VISIBLE)!=0) ^ (visible!=0) )
    {
        sgui_internal_lock_mutex( );

        if( visible )
            this->flags |= SGUI_WIDGET_VISIBLE;
        else
            this->flags &= ~SGUI_WIDGET_VISIBLE;

        if( this->state_change_event )
            this->state_change_event(this,SGUI_WIDGET_VISIBILLITY_CHANGED);

        propagat_state_change(this->children,SGUI_WIDGET_VISIBILLITY_CHANGED);

        /* flag area as dirty */ 
        if( this->canvas )
        {
            sgui_widget_get_absolute_rect( this, &r );
            sgui_canvas_add_dirty_rect( this->canvas, &r );
        }
        sgui_internal_unlock_mutex( );
    }
}

void sgui_widget_get_absolute_rect( const sgui_widget* this, sgui_rect* r )
{
    sgui_widget* i;

    sgui_internal_lock_mutex( );
    *r = this->area;

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

void sgui_widget_send_event( sgui_widget* this, const sgui_event* event,
                             int propagate )
{
    sgui_widget* i;

    /* XXX: Keyboard events are only sent to widget that has focus */
    if( event->type==SGUI_KEY_PRESSED_EVENT ||
        event->type==SGUI_KEY_RELEASED_EVENT )
    {
        /* escape key pressed -> lose focus if policy says so. */
        if( event->arg.i==SGUI_KC_ESCAPE && 
            (this->flags & SGUI_FOCUS_DROP_ESC) )
        {
            sgui_canvas_set_focus( this->canvas, NULL );
            return;
        }

        /* tab key pressed -> advance focus if policy says so. */
        if( event->type==SGUI_KEY_RELEASED_EVENT &&
            event->arg.i==SGUI_KC_TAB &&
            (this->flags & SGUI_FOCUS_DROP_TAB) )
        {
            i = sgui_widget_find_next_focus( this );

            /* no successor? try to restart at root widget */
            if( !i )
            {
                for( i=this; i->parent!=NULL; i=i->parent ) { }
                i = sgui_widget_find_next_focus( i );
            }

            if( i )
                sgui_canvas_set_focus( this->canvas, i );
            return;
        }
    }

    if( this->window_event )
        this->window_event( this, event );

    if( propagate )
    {
        sgui_internal_lock_mutex( );

        for( i=this->children; i!=NULL; i=i->next )
            sgui_widget_send_event( i, event, 1 );

        sgui_internal_unlock_mutex( );
    }
}

void sgui_widget_remove_from_parent( sgui_widget* this )
{
    sgui_rect r;
    sgui_widget* i = NULL;
    int change = SGUI_WIDGET_PARENT_CHANGED;

    if( this->parent )
    {
        sgui_internal_lock_mutex( );

        i = this->parent->children;

        SGUI_REMOVE_FROM_LIST( this->parent->children, i, this );

        if( this->canvas && sgui_widget_is_absolute_visible( this ) )
        {
            sgui_widget_get_absolute_rect( this, &r );
            sgui_canvas_add_dirty_rect( this->canvas, &r );
        }

        /* add canvas change flag if the widget had a canvas before */
        if( this->canvas )
            change |= SGUI_WIDGET_CANVAS_CHANGED;

        /* store a pointer to the old parent */
        i = this->parent;

        /* update links and canvas */
        this->parent = NULL;
        this->next = NULL;
        this->canvas = NULL;

        propagate_canvas( this->children );

        /* call state change callbacks */
        if( i && i->state_change_event )
            i->state_change_event( i, SGUI_WIDGET_CHILD_REMOVED );

        if( this->state_change_event )
            this->state_change_event( this, change );

        if( change & SGUI_WIDGET_CANVAS_CHANGED )
            propagat_state_change(this->children, SGUI_WIDGET_CANVAS_CHANGED);

        sgui_internal_unlock_mutex( );
    }
}

void sgui_widget_add_child( sgui_widget* this, sgui_widget* child )
{
    sgui_rect r;
    sgui_widget* i;
    int change = SGUI_WIDGET_PARENT_CHANGED;

    /* add canvas change flag if the widget had a different canvas before */
    if( child->canvas != this->canvas )
        change |= SGUI_WIDGET_CANVAS_CHANGED;

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
    if( this->canvas && sgui_widget_is_absolute_visible( child ) )
    {
        sgui_widget_get_absolute_rect( child, &r );
        sgui_canvas_add_dirty_rect( child->canvas, &r );
    }

    /* call state change callbacks */
    if( this->state_change_event )
        this->state_change_event( this, SGUI_WIDGET_CHILD_ADDED );

    if( child->state_change_event )
        child->state_change_event( child, change );

    if( change & SGUI_WIDGET_CANVAS_CHANGED )
        propagat_state_change( child->children, SGUI_WIDGET_CANVAS_CHANGED );

    sgui_internal_unlock_mutex( );
}

sgui_widget* sgui_widget_get_child_from_point( const sgui_widget* this,
                                               int x, int y )
{
    const sgui_widget* candidate = this;
    sgui_widget* it;

    /* check if the given widget exists and the point is inside */
    if( !sgui_rect_is_point_inside( &this->area, x, y ) )
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
            if( (it->flags & SGUI_WIDGET_VISIBLE) &&
                sgui_rect_is_point_inside( &it->area, x, y ) )
            {
                candidate = it;
            }
        }
    }
    while( candidate );

    sgui_internal_unlock_mutex( );

    return (sgui_widget*)this;
}

sgui_widget* sgui_widget_find_next_focus( const sgui_widget* this )
{
    sgui_widget* w;
    sgui_widget* v;

    sgui_internal_lock_mutex( );

    while( this )
    {
        /* try to find a child of the current widget that accepts focus */
        if( (w = find_child_focus( this )) )
        {
            sgui_internal_unlock_mutex( );
            return w;
        }

        /*
            try to find a right neightbour that accepts focus or has a child
            that accepts focus.
         */
        for( w=this->next; w!=NULL; w=w->next )
        {
            if( !(w->flags & SGUI_WIDGET_VISIBLE) )
                continue;

            if( w && (w->flags & SGUI_FOCUS_ACCEPT) )
            {
                sgui_internal_unlock_mutex( );
                return w;
            }

            if( (v = find_child_focus( w )) )
            {
                sgui_internal_unlock_mutex( );
                return v;
            }
        }

        /* go to the right uncle, check if it accepts focus, reiterate */
        this = this->parent ? this->parent->next : NULL;

        while( this && !(this->flags & SGUI_WIDGET_VISIBLE) )
            this = this->next;

        if( this && (this->flags & SGUI_FOCUS_ACCEPT) )
        {
            sgui_internal_unlock_mutex( );
            return (sgui_widget*)this;
        }
    }

    sgui_internal_unlock_mutex( );

    return NULL;
}

void sgui_widget_draw( sgui_widget* this, sgui_rect* bounds,
                       sgui_widget* focus )
{
    sgui_rect wr, old_sc;
    sgui_skin* skin;
    sgui_widget* i;
    int fbw;

    sgui_internal_lock_mutex( );

    if( this->flags & SGUI_WIDGET_VISIBLE )
    {
        sgui_widget_get_absolute_rect( this, &wr );

        if( wr.left>=wr.right || wr.top>=wr.bottom )
            goto out;

        if( bounds && !sgui_rect_get_intersection( &wr, bounds, &wr ) )
            goto out;

        old_sc = this->canvas->sc;

        if( !sgui_rect_get_intersection( &this->canvas->sc, &old_sc, &wr ) )
            goto out;

        if( this->draw )
            this->draw( this );

        this->canvas->ox += this->area.left;
        this->canvas->oy += this->area.top;

        for( i=this->children; i!=NULL; i=i->next )
            sgui_widget_draw( i, bounds ? &wr : NULL, focus );

        this->canvas->ox -= this->area.left;
        this->canvas->oy -= this->area.top;

        if( this==focus && (this->flags & SGUI_FOCUS_DRAW) )
        {
            skin = sgui_skin_get( );
            fbw = skin->get_focus_box_width( skin );

            sgui_widget_get_absolute_rect( this, &wr );
            sgui_rect_extend( &wr, fbw, fbw );

            if( sgui_rect_get_intersection(&this->canvas->sc, &old_sc, &wr) )
            {
                sgui_rect_add_offset( &wr, -this->canvas->ox,
                                           -this->canvas->oy );
                skin->draw_focus_box( skin, this->canvas, &wr );
            }
        }

        this->canvas->sc = old_sc;
    }
out:
    sgui_internal_unlock_mutex( );
}

