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



#define COPY_RECT_OFFSET( dst, src )\
        dst.left = src->left + this->ox;\
        dst.right = src->right + this->ox;\
        dst.top = src->top + this->oy;\
        dst.bottom = src->bottom + this->oy;



void sgui_internal_canvas_init( sgui_canvas* this, unsigned int width,
                                unsigned int height )
{
    memset( this, 0, sizeof(sgui_canvas) );

    this->width = width;
    this->height = height;
    this->draw_focus = 0;

    this->root.area.right = width-1;
    this->root.area.bottom = height-1;
    this->root.visible = 1;
    this->root.canvas = this;
    this->root.focus_policy = 0;
}

/****************************************************************************/

static void draw_children( sgui_canvas* this, sgui_widget* widget,
                           sgui_rect* r )
{
    int old_ox, old_oy, fbw;
    sgui_widget* i;
    sgui_rect wr, old_sc;

    old_ox = widget->canvas->ox;
    old_oy = widget->canvas->oy;

    widget->canvas->ox += widget->area.left;
    widget->canvas->oy += widget->area.top;

    for( i=widget->children; i!=NULL; i=i->next )
    {
        if( !sgui_widget_is_visible( i ) )
            continue;

        sgui_widget_get_absolute_rect( i, &wr );

        /* test validity and intersection */
        if( wr.left>=wr.right || wr.top>=wr.bottom )
            continue;

        if( r && !sgui_rect_get_intersection( &wr, r, &wr ) )
            continue;

        /* set scisor rect and draw widget & children */
        old_sc = widget->canvas->sc;

        if( !sgui_rect_get_intersection( &widget->canvas->sc,
                                         &widget->canvas->sc, &wr ) )
        {
            continue;
        }

        sgui_widget_draw( i );
        draw_children( this, i, r ? &wr : NULL );

        /* draw focus box */
        if( i==this->focus && (i->focus_policy & SGUI_FOCUS_DRAW) &&
            this->draw_focus )
        {
            fbw = sgui_skin_get_focus_box_width( );
            sgui_widget_get_absolute_rect( i, &wr );
            wr.left -= fbw; wr.top -= fbw;
            wr.right += fbw; wr.bottom += fbw;

            if( sgui_rect_get_intersection( &widget->canvas->sc,
                                            &old_sc, &wr ) )
            {
                wr.left -= widget->canvas->ox;
                wr.right -= widget->canvas->ox;
                wr.top -= widget->canvas->oy;
                wr.bottom -= widget->canvas->oy;
                sgui_skin_draw_focus_box( this, &wr );
            }
        }

        widget->canvas->sc = old_sc;
    }

    widget->canvas->ox = old_ox;
    widget->canvas->oy = old_oy;
}

/****************************************************************************/

sgui_widget* sgui_canvas_get_root( const sgui_canvas* this )
{
    return this ? (sgui_widget*)&(this->root) : NULL;
}

void sgui_canvas_set_focus( sgui_canvas* this, sgui_widget* widget )
{
    unsigned int fbw;
    sgui_widget* i;
    sgui_event ev;
    sgui_rect r;

    if( !this )
        return;

    sgui_internal_lock_mutex( );

    /* test if the widget actually belongs to the canvas */
    if( widget )
    {
        for( i=widget; i!=NULL && i!=&(this->root); i=i->parent );

        if( !i )
        {
            sgui_internal_unlock_mutex( );
            return;
        }
    }

    /* make sure the focus box gets redrawn */
    fbw = sgui_skin_get_focus_box_width( );

    if( this->focus && (this->focus->focus_policy & SGUI_FOCUS_DRAW) &&
        this->draw_focus )
    {
        sgui_widget_get_absolute_rect( this->focus, &r );
        r.left -= fbw;
        r.top -= fbw;
        r.right += fbw;
        r.bottom += fbw;
        sgui_canvas_add_dirty_rect( this, &r );
    }

    if( widget && (widget->focus_policy & SGUI_FOCUS_DRAW) &&
        (widget->focus_policy & SGUI_FOCUS_ACCEPT) )
    {
        sgui_widget_get_absolute_rect( widget, &r );
        r.left -= fbw;
        r.top -= fbw;
        r.right += fbw;
        r.bottom += fbw;
        sgui_canvas_add_dirty_rect( this, &r );
    }

    /* send events */
    ev.src.widget = widget;
    ev.type = SGUI_FOCUS_LOSE_EVENT;
    sgui_widget_send_event( this->focus, &ev, 0 );

    if( widget && (widget->focus_policy & SGUI_FOCUS_ACCEPT) )
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

    if( !this || !r )
        return;

    sgui_internal_lock_mutex( );

    /* try to find an existing diry rect it touches */
    for( i=0; i<this->num_dirty; ++i )
    {
        if( sgui_rect_join( this->dirty + i, r, 1 ) )
        {
            sgui_internal_unlock_mutex( );
            return;
        }
    }

    /* add a new one if posible, join all existing if not */
    if( this->num_dirty < CANVAS_MAX_DIRTY )
    {
        sgui_rect_copy( this->dirty + (this->num_dirty++), r );
    }
    else
    {
        for( i=1; i<this->num_dirty; ++i )
            sgui_rect_join( this->dirty, this->dirty + i, 0 );

        sgui_rect_copy( this->dirty + 1, r );
        this->num_dirty = 2;
    }

    sgui_internal_unlock_mutex( );
}

unsigned int sgui_canvas_num_dirty_rects( const sgui_canvas* this )
{
    return this ? this->num_dirty : 0;
}

void sgui_canvas_get_dirty_rect( const sgui_canvas* this, sgui_rect* rect,
                                 unsigned int i )
{
    sgui_internal_lock_mutex( );

    if( this && (i < this->num_dirty) )
        sgui_rect_copy( rect, this->dirty + i );

    sgui_internal_unlock_mutex( );
}

void sgui_canvas_clear_dirty_rects( sgui_canvas* this )
{
    if( this )
    {
        sgui_internal_lock_mutex( );
        this->num_dirty = 0;
        sgui_internal_unlock_mutex( );
    }
}

void sgui_canvas_redraw_widgets( sgui_canvas* this, int clear )
{
    int need_end = 0;
    unsigned int i;

    if( this )
    {
        sgui_internal_lock_mutex( );

        if( !this->began )
        {
            sgui_canvas_begin( this, NULL );
            need_end = 1;
        }

        for( i=0; i<this->num_dirty; ++i )
        {
            if( clear )
                this->clear( this, this->dirty + i );

            if( this->root.children )
                draw_children( this, &this->root, this->dirty + i );
        }

        if( need_end )
            sgui_canvas_end( this );

        this->num_dirty = 0;

        sgui_internal_unlock_mutex( );
    }
}

void sgui_canvas_draw_widgets( sgui_canvas* this, int clear )
{
    sgui_rect r1;
    int need_end = 0;

    if( this )
    {
        sgui_internal_lock_mutex( );

        sgui_rect_set_size( &r1, 0, 0, this->width, this->height );
        this->num_dirty = 0;

        if( !this->began )
        {
            sgui_canvas_begin( this, NULL );
            need_end = 1;
        }

        if( clear )
            this->clear( this, &r1 );

        if( this->root.children )
            draw_children( this, &this->root, NULL );

        if( need_end )
            sgui_canvas_end( this );

        sgui_internal_unlock_mutex( );
    }
}

void sgui_canvas_send_window_event( sgui_canvas* this, const sgui_event* e )
{
    unsigned int fbw;
    sgui_widget* i;
    sgui_event ev;
    sgui_rect r;
    int x, y;

    if( !this || !this->root.children )
        return;

    /* don't handle events that the canvas generates */
    if( e->type==SGUI_FOCUS_EVENT || e->type==SGUI_MOUSE_ENTER_EVENT ||
        e->type==SGUI_MOUSE_ENTER_EVENT || e->type==SGUI_MOUSE_LEAVE_EVENT )
        return;

    sgui_internal_lock_mutex( );

    if( e->type == SGUI_MOUSE_MOVE_EVENT )
    {
        /* find the widget under the mouse cursor */
        i = sgui_widget_get_child_from_point( &this->root,
                                              e->arg.i2.x, e->arg.i2.y );

        /* generate mouse enter/leave events */
        if( this->mouse_over != i )
        {
            ev.src.window = e->src.window;
            ev.type = SGUI_MOUSE_ENTER_EVENT;
            sgui_widget_send_event( i, &ev, 0 );

            ev.type = SGUI_MOUSE_LEAVE_EVENT;
            sgui_widget_send_event( this->mouse_over, &ev, 0 );

            this->mouse_over = i;
        }
    }

    switch( e->type )
    {
    case SGUI_MOUSE_PRESS_EVENT:
    case SGUI_MOUSE_RELEASE_EVENT:
        /* transform to widget local coordinates and redirect event */
        sgui_widget_get_absolute_position( this->mouse_over, &x, &y );

        ev = *e;
        ev.arg.i3.x -= x;
        ev.arg.i3.y -= y;

        sgui_widget_send_event( this->mouse_over, &ev, 0 );

        /* give clicked widget focus */
        if( this->focus != this->mouse_over )
        {
            sgui_canvas_set_focus( this, this->mouse_over );
            this->draw_focus = 0;
        }
        break;
    case SGUI_MOUSE_MOVE_EVENT:
        /* transform to widget local coordinates and redirect event */
        sgui_widget_get_absolute_position( this->mouse_over, &x, &y );
        ev = *e;
        ev.arg.i2.x -= x;
        ev.arg.i2.y -= y;

        sgui_widget_send_event( this->mouse_over, &ev, 0 );
        break;
    case SGUI_MOUSE_WHEEL_EVENT:
        sgui_widget_send_event( this->mouse_over, e, 0 );
        break;
    case SGUI_KEY_RELEASED_EVENT:
        /* tab key pressed -> lose focus if policy says so */
        if( e->arg.i==SGUI_KC_TAB )
        {
            if( !this->focus ||
                (this->focus->focus_policy & SGUI_FOCUS_DROP_TAB) )
            {
                i = sgui_widget_find_next_focus( this->focus );
                i = i ? i : sgui_widget_find_next_focus( &this->root );

                if( i )
                {
                    sgui_canvas_set_focus( this, i );
                    this->draw_focus = 1;
                    break;
                }
            }
        }
    case SGUI_KEY_PRESSED_EVENT:
    case SGUI_CHAR_EVENT:
        /* escape key pressed -> lose focus if policy says so */
        if( e->arg.i==SGUI_KC_ESCAPE && this->focus &&
            (this->focus->focus_policy & SGUI_FOCUS_DROP_ESC) )
        {
            sgui_canvas_set_focus( this, NULL );
            break;
        }

        /* send the event */
        sgui_widget_send_event( this->focus, e, 0 );

        /* make sure the focus box gets drawn */
        if( this->focus && !this->draw_focus )
        {
            this->draw_focus = 1;
            sgui_widget_get_absolute_rect( this->focus, &r );
            fbw = sgui_skin_get_focus_box_width( );
            r.left -= fbw;
            r.top -= fbw;
            r.right += fbw;
            r.bottom += fbw;
            sgui_canvas_add_dirty_rect( this, &r );
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

    if( this )
    {
        for( i=this->root.children; i!=NULL; i=i->next )
            sgui_widget_remove_from_parent( i );

        this->destroy( this );
    }
}

void sgui_canvas_resize( sgui_canvas* this, unsigned int width,
                         unsigned int height )
{
    if( this && (this->width!=width || this->height!=height) )
    {
        sgui_internal_lock_mutex( );

        if( this->resize )
            this->resize( this, width, height );

        this->width = width;
        this->height = height;

        this->root.area.right  = width-1;
        this->root.area.bottom = height-1;

        sgui_internal_unlock_mutex( );
    }
}

void sgui_canvas_get_size( const sgui_canvas* this, unsigned int* width,
                           unsigned int* height )
{
    if( width  ) *width  = this ? this->width  : 0;
    if( height ) *height = this ? this->height : 0;
}

sgui_pixmap* sgui_canvas_create_pixmap( sgui_canvas* this,
                                        unsigned int width,
                                        unsigned int height, int format )
{
    if( !this || !width || !height )
        return NULL;

    return this->create_pixmap( this, width, height, format );
}

void sgui_canvas_begin( sgui_canvas* this, const sgui_rect* r )
{
    sgui_rect r0;

    if( this && !this->began )
    {
        sgui_rect_set_size( &r0, 0, 0, this->width, this->height );

        if( r )
            sgui_rect_get_intersection( &r0, &r0, r );

        /* tell the implementation to begin drawing */
        if( this->begin )
            this->begin( this, &r0 );

        this->sc = r0;
        this->began = 1;
    }
}

void sgui_canvas_end( sgui_canvas* this )
{
    if( this && this->began )
    {
        this->began = 0;

        if( this->end )
            this->end( this );
    }
}

void sgui_canvas_clear( sgui_canvas* this, sgui_rect* r )
{
    sgui_rect r1;

    if( !this || !this->began )
        return;

    /* if no rect is given, set to the full canvas area */
    if( r )
    {
        COPY_RECT_OFFSET( r1, r );
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
                           unsigned char* color, int format )
{
    sgui_rect r1;

    /* sanity check */
    if( !this || !color || !this->began || !r )
        return;

    if( format==SGUI_RGBA8 && color[3]==0xFF )
        format = SGUI_RGB8;

    /* offset and clip the given rectangle */
    COPY_RECT_OFFSET( r1, r );

    if( sgui_rect_get_intersection( &r1, &this->sc, &r1 ) )
        this->draw_box( this, &r1, color, format );
}

void sgui_canvas_draw_line( sgui_canvas* this, int x, int y,
                            unsigned int length, int horizontal,
                            unsigned char* color, int format )
{
    sgui_rect r;

    /* santiy check */
    if( !this || !this->began || !color )
        return;

    if( format==SGUI_RGBA8 && color[3]==0xFF )
        format = SGUI_RGB8;

    if( horizontal )
        sgui_rect_set_size( &r, x, y, length, 1 );
    else
        sgui_rect_set_size( &r, x, y, 1, length );

    /* offset and clip the given rectangle */
    r.left   += this->ox;
    r.right  += this->ox;
    r.top    += this->oy;
    r.bottom += this->oy;

    if( sgui_rect_get_intersection( &r, &this->sc, &r ) )
        this->draw_box( this, &r, color, format );
}

void sgui_canvas_blit( sgui_canvas* this, int x, int y,
                       sgui_pixmap* pixmap, sgui_rect* srcrect )
{
    unsigned int w, h;
    sgui_rect r, clip;

    /* sanity check */
    if( !this || !pixmap || !this->began )
        return;

    x += this->ox;
    y += this->oy;

    sgui_pixmap_get_size( pixmap, &w, &h );
    sgui_rect_set_size( &r, 0, 0, w, h );

    if( srcrect )
        sgui_rect_get_intersection( &r, &r, srcrect );

    /* clip the against the scissor rectangle */
    clip.left   = x;
    clip.top    = y;
    clip.right  = x + r.right  - r.left;
    clip.bottom = y + r.bottom - r.top;

    sgui_rect_get_intersection( &clip, &clip, &this->sc );

    r.left  += clip.left - x;
    r.top   += clip.top  - y;
    r.right  = r.left + clip.right-clip.left;
    r.bottom = r.top  + clip.bottom-clip.top;

    x = clip.left;
    y = clip.top;

    /* do the blitting */
    this->blit( this, x, y, pixmap, &r );
}

void sgui_canvas_blend( sgui_canvas* this, int x, int y,
                        sgui_pixmap* pixmap, sgui_rect* srcrect )
{
    unsigned int w, h;
    sgui_rect r, clip;

    /* sanity check */
    if( !this || !pixmap || !this->began )
        return;

    x += this->ox;
    y += this->oy;

    sgui_pixmap_get_size( pixmap, &w, &h );
    sgui_rect_set_size( &r, 0, 0, w, h );

    if( srcrect )
        sgui_rect_get_intersection( &r, &r, srcrect );

    /* clip the against the scissor rectangle */
    clip.left   = x;
    clip.top    = y;
    clip.right  = x + r.right  - r.left;
    clip.bottom = y + r.bottom - r.top;

    sgui_rect_get_intersection( &clip, &clip, &this->sc );

    r.left  += clip.left - x;
    r.top   += clip.top  - y;
    r.right  = r.left + clip.right-clip.left;
    r.bottom = r.top  + clip.bottom-clip.top;

    x = clip.left;
    y = clip.top;

    /* do the blending */
    this->blend( this, x, y, pixmap, &r );
}

int sgui_canvas_draw_text_plain( sgui_canvas* this, int x, int y,
                                 int bold, int italic,
                                 unsigned char* color,
                                 const char* text, unsigned int length )
{
    sgui_font* font = sgui_skin_get_default_font( bold, italic );

    /* sanity check */
    if( !this || !font || !color || !text || !this->began || !length )
        return 0;

    x += this->ox;
    y += this->oy;

    if( x>=this->sc.right || y>= this->sc.bottom )
        return 0;

    return this->draw_string( this, x, y, font, color, text, length );
}

void sgui_canvas_draw_text( sgui_canvas* this, int x, int y,
                            const char* text )
{
    int i = 0, X = 0, font_stack_index = 0, font_height;
    unsigned char col[3], font_stack[10], f = 0;
    long c;

    /* sanity check */
    if( !this || !text )
        return;

    sgui_skin_get_default_font_color( col );

    font_height = sgui_skin_get_default_font_height( );

    for( ; text && text[ i ]; ++i )
    {
        if( text[ i ] == '<' )  /* we encountered a tag */
        {
            /* draw what we got so far with the current settings */
            X += sgui_canvas_draw_text_plain( this, x+X, y, f&0x02, f&0x01,
                                              col, text, i );

            if( !strncmp( text+i+1, "color=", 6 ) ) /* it's a color tag */
            {
                if( !strncmp( text+i+9, "default", 7 ) )
                {
                    sgui_skin_get_default_font_color( col );
                }
                else
                {
                    c = strtol( text+i+9, NULL, 16 );

                    col[0] = (c>>16) & 0xFF;
                    col[1] = (c>>8 ) & 0xFF;
                    col[2] =  c      & 0xFF;
                }
            }
            else if( text[ i+1 ] == 'b' )   /* it's a <b> tag */
            {
                font_stack[ font_stack_index++ ] = f;
                f |= 0x02;
            }
            else if( text[ i+1 ] == 'i' )   /* it's an <i> tag */
            {
                font_stack[ font_stack_index++ ] = f;
                f |= 0x01;
            }
            else if( text[ i+1 ] == '/' && font_stack_index )   /* end tag */
            {
                f = font_stack[ --font_stack_index ];   /* pop from stack */
            }

            /* skip to the end of the tag */
            if( (text = strchr( text+i, '>' )) )
                ++text;

            i = -1; /* reset i to 0 at next iteration */
        }
        else if( text[ i ] == '\n' )
        {
            /* draw what we got so far */
            sgui_canvas_draw_text_plain( this, x+X, y, f&0x02, f&0x01,
                                         col, text, i );

            text += i + 1;    /* skip to next line */
            i = -1;           /* reset i to 0 at next iteration */
            X = 0;            /* adjust move cursor */
            y += font_height;
        }
    }

    /* draw what is still left */
    sgui_canvas_draw_text_plain( this, x+X, y, f&0x02, f&0x01, col, text, i );
}

