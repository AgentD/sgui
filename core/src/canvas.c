/*
 * canvas.c
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
#include "sgui_canvas.h"
#include "sgui_internal.h"
#include "sgui_utf8.h"
#include "sgui_font.h"
#include "sgui_skin.h"
#include "sgui_pixmap.h"
#include "sgui_widget.h"
#include "sgui_event.h"

#include <string.h>
#include <stdlib.h>



int sgui_canvas_init( sgui_canvas* this, unsigned int width,
                      unsigned int height )
{
    this->dirty = calloc( SGUI_CANVAS_MAX_DIRTY, sizeof(sgui_rect) );

    if( !this->dirty )
        return 0;

    this->width = width;
    this->height = height;

    sgui_rect_set_size( &this->root.area, 0, 0, width, height );
    this->root.flags = SGUI_WIDGET_VISIBLE;
    this->root.canvas = this;
    return 1;
}

void sgui_canvas_set_focus( sgui_canvas* this, sgui_widget* widget )
{
    unsigned int fbw;
    sgui_skin* skin;
    sgui_widget* i;
    sgui_event ev;
    sgui_rect r;

    if( widget==this->focus )
        return;

    sgui_internal_lock_mutex( );

    /* test if the widget actually belongs to the canvas */
    if( widget )
    {
        for( i=widget; i!=NULL && i!=&(this->root); i=i->parent );

        if( !i || widget->canvas!=this )
        {
            sgui_internal_unlock_mutex( );
            return;
        }
    }

    /* make sure the focus box gets redrawn */
    skin = sgui_skin_get( );
    fbw = skin->get_focus_box_width( skin );

    if( this->focus && (this->focus->flags & SGUI_FOCUS_DRAW) &&
        (this->flags & SGUI_CANVAS_DRAW_FOCUS) )
    {
        sgui_widget_get_absolute_rect( this->focus, &r );
        sgui_rect_extend( &r, fbw, fbw );
        sgui_canvas_add_dirty_rect( this, &r );
    }

    if( widget && (widget->flags & SGUI_FOCUS_DRAW) &&
        (widget->flags & SGUI_FOCUS_ACCEPT) )
    {
        sgui_widget_get_absolute_rect( widget, &r );
        sgui_rect_extend( &r, fbw, fbw );
        sgui_canvas_add_dirty_rect( this, &r );
    }

    /* send events */
    if( this->focus )
    {
        ev.src.widget = widget;
        ev.type = SGUI_FOCUS_LOSE_EVENT;
        sgui_widget_send_event( this->focus, &ev, 0 );
    }

    if( widget && (widget->flags & SGUI_FOCUS_ACCEPT) )
    {
        ev.type = SGUI_FOCUS_EVENT;
        sgui_widget_send_event( widget, &ev, 0 );

        this->focus = widget;
    }
    else
    {
        this->focus = NULL;
    }

    sgui_internal_unlock_mutex( );
}

void sgui_canvas_add_dirty_rect( sgui_canvas* this, sgui_rect* r )
{
    unsigned int i;
    sgui_rect r0;

    sgui_internal_lock_mutex( );

    /* make sure dirty rect is inside canvase area */
    sgui_rect_set_size( &r0, 0, 0, this->width, this->height );
    if( !sgui_rect_get_intersection( &r0, &r0, r ) )
    {
        sgui_internal_unlock_mutex( );
        return;
    }

    /* try to find an existing diry rect it touches */
    for( i=0; i<this->num_dirty; ++i )
    {
        if( sgui_rect_join( this->dirty + i, &r0, 1 ) )
        {
            sgui_internal_unlock_mutex( );
            return;
        }
    }

    /* add a new one if posible, join all existing if not */
    if( this->num_dirty >= SGUI_CANVAS_MAX_DIRTY )
    {
        for( i=1; i<this->num_dirty; ++i )
            sgui_rect_join( this->dirty, this->dirty + i, 0 );

        this->num_dirty = 1;
    }

    this->dirty[ this->num_dirty++ ] = r0;
    sgui_internal_unlock_mutex( );
}

unsigned int sgui_canvas_num_dirty_rects( const sgui_canvas* this )
{
    return this->num_dirty;
}

void sgui_canvas_get_dirty_rect( const sgui_canvas* this, sgui_rect* rect,
                                 unsigned int i )
{
    sgui_internal_lock_mutex( );

    if( i < this->num_dirty )
        *rect = this->dirty[ i ];

    sgui_internal_unlock_mutex( );
}

void sgui_canvas_clear_dirty_rects( sgui_canvas* this )
{
    sgui_internal_lock_mutex( );
    this->num_dirty = 0;
    sgui_internal_unlock_mutex( );
}

void sgui_canvas_redraw_widgets( sgui_canvas* this, int clear )
{
    int need_end = 0;
    unsigned int i;

    sgui_internal_lock_mutex( );

    if( !(this->flags & SGUI_CANVAS_BEGAN) )
    {
        sgui_canvas_begin( this, NULL );
        need_end = 1;
    }

    for( i=0; i<this->num_dirty; ++i )
    {
        if( clear )
            this->clear( this, this->dirty + i );

        if( this->root.children )
        {
            sgui_widget_draw( &this->root, this->dirty + i,
                              (this->flags & SGUI_CANVAS_DRAW_FOCUS) ?
                              this->focus : NULL );
        }
    }

    if( need_end )
        sgui_canvas_end( this );

    this->num_dirty = 0;

    sgui_internal_unlock_mutex( );
}

void sgui_canvas_draw_widgets( sgui_canvas* this, int clear )
{
    sgui_rect r1;
    int need_end = 0;

    sgui_internal_lock_mutex( );

    sgui_rect_set_size( &r1, 0, 0, this->width, this->height );
    this->num_dirty = 0;

    if( !(this->flags & SGUI_CANVAS_BEGAN) )
    {
        sgui_canvas_begin( this, NULL );
        need_end = 1;
    }

    if( clear )
        this->clear( this, &r1 );

    if( this->root.children )
    {
        sgui_widget_draw( &this->root, NULL,
                          (this->flags & SGUI_CANVAS_DRAW_FOCUS) ?
                          this->focus : NULL );
    }

    if( need_end )
        sgui_canvas_end( this );

    sgui_internal_unlock_mutex( );
}

void sgui_canvas_send_window_event( sgui_canvas* this, const sgui_event* e )
{
    unsigned int fbw;
    sgui_skin* skin;
    sgui_widget* i;
    sgui_event ev;
    sgui_rect r;
    int x, y;

    if( !this->root.children )
        return;

    /* don't handle events that the canvas generates */
    if( e->type==SGUI_MOUSE_ENTER_EVENT || e->type==SGUI_MOUSE_LEAVE_EVENT ||
        e->type==SGUI_FOCUS_EVENT )
        return;

    sgui_internal_lock_mutex( );

    if( e->type==SGUI_MOUSE_MOVE_EVENT || e->type==SGUI_DOUBLE_CLICK_EVENT )
    {
        /* find the widget under the mouse cursor */
        i = sgui_widget_get_child_from_point( &this->root,
                                              e->arg.i2.x, e->arg.i2.y );

        /* generate mouse enter/leave events */
        if( this->mouse_over != i )
        {
            ev.src.window = e->src.window;
            if( i )
            {
                ev.type = SGUI_MOUSE_ENTER_EVENT;
                sgui_widget_send_event( i, &ev, 0 );
            }
            if( this->mouse_over )
            {
                ev.type = SGUI_MOUSE_LEAVE_EVENT;
                sgui_widget_send_event( this->mouse_over, &ev, 0 );
            }
            this->mouse_over = i;
        }
    }

    switch( e->type )
    {
    case SGUI_FOCUS_LOSE_EVENT:
        if( this->focus )
            sgui_canvas_set_focus( this, NULL );
        break;
    case SGUI_MOUSE_PRESS_EVENT:
    case SGUI_MOUSE_RELEASE_EVENT:
        if( this->mouse_over )
        {
            /* transform to widget local coordinates and redirect event */
            sgui_widget_get_absolute_position( this->mouse_over, &x, &y );

            ev = *e;
            ev.arg.i3.x -= x;
            ev.arg.i3.y -= y;

            sgui_widget_send_event( this->mouse_over, &ev, 0 );
        }

        /* give clicked widget focus */
        if( this->focus != this->mouse_over )
        {
            sgui_canvas_set_focus( this, this->mouse_over );
            this->flags &= ~SGUI_CANVAS_DRAW_FOCUS;
        }
        break;
    case SGUI_MOUSE_MOVE_EVENT:
    case SGUI_DOUBLE_CLICK_EVENT:
        if( this->mouse_over )
        {
            /* transform to widget local coordinates and redirect event */
            sgui_widget_get_absolute_position( this->mouse_over, &x, &y );
            ev = *e;
            ev.arg.i2.x -= x;
            ev.arg.i2.y -= y;

            sgui_widget_send_event( this->mouse_over, &ev, 0 );
        }
        break;
    case SGUI_MOUSE_WHEEL_EVENT:
        if( this->mouse_over )
            sgui_widget_send_event( this->mouse_over, e, 0 );
        break;
    case SGUI_KEY_RELEASED_EVENT:
        if( e->arg.i==SGUI_KC_TAB && !this->focus )
        {
            if( (i = sgui_widget_find_next_focus( &this->root )) )
            {
                sgui_canvas_set_focus( this, i );
                this->flags |= SGUI_CANVAS_DRAW_FOCUS;
                break;
            }
        }
    case SGUI_KEY_PRESSED_EVENT:
    case SGUI_CHAR_EVENT:
        if( this->focus )
        {
            sgui_widget_send_event( this->focus, e, 0 );

            /* make sure the focus box gets drawn */
            if( !(this->flags & SGUI_CANVAS_DRAW_FOCUS) )
            {
                this->flags |= SGUI_CANVAS_DRAW_FOCUS;
                sgui_widget_get_absolute_rect( this->focus, &r );
                skin = sgui_skin_get( );
                fbw = skin->get_focus_box_width( skin );
                sgui_rect_extend( &r, fbw, fbw );
                sgui_canvas_add_dirty_rect( this, &r );
            }
        }
        break;
    default:
        /* propagate all other events */
        sgui_widget_send_event( &this->root, e, 1 );
        break;
    }

    sgui_internal_unlock_mutex( );
}

/****************************************************************************/

void sgui_canvas_destroy( sgui_canvas* this )
{
    sgui_widget* i;

    for( i=this->root.children; i!=NULL; i=i->next )
        sgui_widget_remove_from_parent( i );

    free( this->dirty );

    this->destroy( this );
}

void sgui_canvas_resize( sgui_canvas* this, unsigned int width,
                         unsigned int height )
{
    if( (this->width!=width || this->height!=height) && this->resize )
    {
        sgui_internal_lock_mutex( );

        this->resize( this, width, height );

        this->width = width;
        this->height = height;

        sgui_rect_set_size( &this->root.area, 0, 0, width, height );

        sgui_internal_unlock_mutex( );
    }
}

sgui_pixmap* sgui_canvas_create_pixmap( sgui_canvas* this,
                                        unsigned int width,
                                        unsigned int height, int format )
{
    if( !width || !height )
        return NULL;

    return this->create_pixmap( this, width, height, format );
}

void sgui_canvas_get_scissor_rect( const sgui_canvas* this, sgui_rect* r )
{
    *r = this->sc;
}

void sgui_canvas_get_offset( const sgui_canvas* this, int* x, int* y )
{
    *x = this->ox;
    *y = this->oy;
}

void sgui_canvas_set_offset( sgui_canvas* this, int x, int y )
{
    this->ox = x;
    this->oy = y;
}

void sgui_canvas_set_scissor_rect( sgui_canvas* this, const sgui_rect* r )
{
    if( r )
    {
        this->sc.left   = MAX(0,                   r->left  );
        this->sc.top    = MAX(0,                   r->top   );
        this->sc.right  = MIN((int)this->width-1,  r->right );
        this->sc.bottom = MIN((int)this->height-1, r->bottom);
    }
    else
    {
        sgui_rect_set_size( &this->sc, 0, 0, this->width, this->height );
    }
}

void sgui_canvas_begin( sgui_canvas* this, const sgui_rect* r )
{
    sgui_rect r0;

    if( !(this->flags & SGUI_CANVAS_BEGAN) )
    {
        sgui_rect_set_size( &r0, 0, 0, this->width, this->height );

        if( r && !sgui_rect_get_intersection( &r0, &r0, r ) )
            return;

        /* tell the implementation to begin drawing */
        if( this->begin && !this->begin( this, &r0 ) )
            return;

        this->sc = r0;
        this->flags |= SGUI_CANVAS_BEGAN;
    }
}

void sgui_canvas_end( sgui_canvas* this )
{
    if( this->flags & SGUI_CANVAS_BEGAN )
    {
        this->flags &= ~SGUI_CANVAS_BEGAN;

        if( this->end )
            this->end( this );
    }
}

void sgui_canvas_clear( sgui_canvas* this, sgui_rect* r )
{
    sgui_rect r1;

    if( !(this->flags & SGUI_CANVAS_BEGAN) )
        return;

    /* if no rect is given, set to the full canvas area */
    if( r )
    {
        r1 = *r;
        sgui_rect_add_offset( &r1, this->ox, this->oy );
    }
    else
    {
        sgui_rect_set_size( &r1, 0, 0, this->width, this->height );
    }

    /* clear if we have an intersection */
    if( sgui_rect_get_intersection( &r1, &this->sc, &r1 ) )
        this->clear( this, &r1 );
}

/**************************** drawing functions ****************************/

void sgui_canvas_draw_box( sgui_canvas* this, sgui_rect* r,
                           const unsigned char* color, int format )
{
    sgui_rect r1;

    if( !(this->flags & SGUI_CANVAS_BEGAN) )
        return;

    if( format==SGUI_RGBA8 && color[3]==0xFF )
        format = SGUI_RGB8;

    r1 = *r;
    sgui_rect_add_offset( &r1, this->ox, this->oy );

    if( sgui_rect_get_intersection( &r1, &this->sc, &r1 ) )
        this->draw_box( this, &r1, color, format );
}

void sgui_canvas_draw_line( sgui_canvas* this, int x, int y,
                            unsigned int length, int horizontal,
                            const unsigned char* color, int format )
{
    sgui_rect r;

    if( !(this->flags & SGUI_CANVAS_BEGAN) )
        return;

    if( format==SGUI_RGBA8 && color[3]==0xFF )
        format = SGUI_RGB8;

    if( horizontal )
        sgui_rect_set_size( &r, x+this->ox, y+this->oy, length, 1 );
    else
        sgui_rect_set_size( &r, x+this->ox, y+this->oy, 1, length );

    if( sgui_rect_get_intersection( &r, &this->sc, &r ) )
        this->draw_box( this, &r, color, format );
}

void sgui_canvas_draw_pixmap( sgui_canvas* this, int x, int y,
                              sgui_pixmap* pixmap, sgui_rect* srcrect,
                              int blend )
{
    unsigned int w, h;
    sgui_rect src, clip;

    if( !(this->flags & SGUI_CANVAS_BEGAN) )
        return;

    sgui_pixmap_get_size( pixmap, &w, &h );
    sgui_rect_set_size( &src, 0, 0, w, h );

    if( srcrect && !sgui_rect_get_intersection( &src, &src, srcrect ) )
        return;

    x -= src.left;
    y -= src.top;
    clip = this->sc;
    sgui_rect_add_offset( &clip, -this->ox-x, -this->oy-y );

    if( sgui_rect_get_intersection( &src, &src, &clip ) )
    {
        x += src.left + this->ox;
        y += src.top  + this->oy;

        /* do the drawing */
        if( blend )
            this->blend( this, x, y, pixmap, &src );
        else
            this->blit( this, x, y, pixmap, &src );
    }
}

int sgui_canvas_draw_text_plain( sgui_canvas* this, int x, int y,
                                 int bold, int italic,
                                 const unsigned char* color,
                                 const char* text, unsigned int length )
{
    sgui_font* font = sgui_skin_get_default_font( bold, italic );

    /* sanity check */
    if( !(this->flags & SGUI_CANVAS_BEGAN) || !length )
        return 0;

    x += this->ox;
    y += this->oy;

    if( x>=this->sc.right || y>= this->sc.bottom )
        return 0;

    return this->draw_string( this, x, y, font, color, text, length );
}

