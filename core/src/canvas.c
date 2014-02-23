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
        dst.left = src->left + canvas->ox;\
        dst.right = src->right + canvas->ox;\
        dst.top = src->top + canvas->oy;\
        dst.bottom = src->bottom + canvas->oy;



void sgui_internal_canvas_init( sgui_canvas* cv, unsigned int width,
                                unsigned int height )
{
    memset( cv, 0, sizeof(sgui_canvas) );

    cv->width = width;
    cv->height = height;
    cv->draw_focus = 0;

    cv->root.area.right = width-1;
    cv->root.area.bottom = height-1;
    cv->root.visible = 1;
    cv->root.canvas = cv;
    cv->root.focus_policy = 0;
}

/****************************************************************************/

static void draw_children( sgui_canvas* canvas, sgui_widget* widget,
                           sgui_rect* r )
{
    int old_ox, old_oy, fbw;
    sgui_widget* i;
    sgui_rect wr, old_sc;

    old_ox = widget->canvas->ox;
    old_oy = widget->canvas->oy;

    widget->canvas->ox += widget->area.left;
    widget->canvas->oy += widget->area.top;

    if( r )
    {
        /* redraw all widgets that lie inside the rect */
        for( i=widget->children; i!=NULL; i=i->next )
        {
            if( !sgui_widget_is_visible( i ) )
                continue;

            sgui_widget_get_absolute_rect( i, &wr );

            if( wr.left>=wr.right || wr.top>=wr.bottom )
                continue;

            if( !sgui_rect_get_intersection( &wr, r, &wr ) )
                continue;

            old_sc = widget->canvas->sc;
            sgui_rect_get_intersection( &widget->canvas->sc,
                                        &widget->canvas->sc, &wr );

            sgui_widget_draw( i );
            draw_children( canvas, i, &wr );

            /* draw focus box */
            if( i==canvas->focus && (i->focus_policy & SGUI_FOCUS_DRAW) &&
                canvas->draw_focus )
            {
                fbw = sgui_skin_get_focus_box_width( );
                sgui_widget_get_absolute_rect( i, &wr );
                wr.left -= fbw; wr.top -= fbw;
                wr.right += fbw; wr.bottom += fbw;
                sgui_rect_get_intersection( &widget->canvas->sc,
                                            &old_sc, &wr );
                wr.left -= widget->canvas->ox;
                wr.right -= widget->canvas->ox;
                wr.top -= widget->canvas->oy;
                wr.bottom -= widget->canvas->oy;
                sgui_skin_draw_focus_box( canvas, &wr );
            }

            widget->canvas->sc = old_sc;
        }
    }
    else
    {
        /* draw all widgets */
        for( i=widget->children; i!=NULL; i=i->next )
        {
            if( !sgui_widget_is_visible( i ) )
                continue;

            sgui_widget_get_absolute_rect( i, &wr );

            if( wr.left>=wr.right || wr.top>=wr.bottom )
                continue;

            old_sc = widget->canvas->sc;
            sgui_rect_get_intersection( &widget->canvas->sc,
                                        &widget->canvas->sc, &wr );

            sgui_widget_draw( i );
            draw_children( canvas, i, NULL );

            /* draw focus box */
            if( i==canvas->focus && (i->focus_policy & SGUI_FOCUS_DRAW) &&
                canvas->draw_focus )
            {
                fbw = sgui_skin_get_focus_box_width( );
                sgui_widget_get_absolute_rect( i, &wr );
                wr.left -= fbw; wr.top -= fbw;
                wr.right += fbw; wr.bottom += fbw;
                sgui_rect_get_intersection( &widget->canvas->sc,
                                            &old_sc, &wr );

                wr.left -= widget->canvas->ox;
                wr.right -= widget->canvas->ox;
                wr.top -= widget->canvas->oy;
                wr.bottom -= widget->canvas->oy;
                sgui_skin_draw_focus_box( canvas, &wr );
            }

            widget->canvas->sc = old_sc;
        }
    }

    widget->canvas->ox = old_ox;
    widget->canvas->oy = old_oy;
}

static sgui_widget* find_child_focus( sgui_widget* widget )
{
    sgui_widget* candidate;
    sgui_widget* w;

    for( w=widget->children; w!=NULL; w=w->next )
    {
        if( !w->visible )
            continue;

        if( (w->focus_policy & SGUI_FOCUS_ACCEPT) )
            return w;
    }

    for( w=widget->children; w!=NULL; w=w->next )
    {
        if( !w->visible )
            continue;

        if( (candidate = find_child_focus( w )) )
            return candidate;
    }

    return NULL;
}

static sgui_widget* find_next_focus( sgui_widget* widget )
{
    sgui_widget* w;
    sgui_widget* v;

    while( widget )
    {
        /* try to find a child of the current widget that accepts focus */
        if( (w = find_child_focus( widget )) )
            return w;

        /*
            try to find a right neightbour that accepts focus or has a child
            that accepts focus.
         */
        for( w=widget->next; w!=NULL; w=w->next )
        {
            if( !w->visible )
                continue;

            if( w && (w->focus_policy & SGUI_FOCUS_ACCEPT) )
                return w;

            if( (v = find_child_focus( w )) )
                return v;
        }

        /* go to the right uncle, check if it accepts focus, reiterate */
        widget = widget->parent ? widget->parent->next : NULL;

        while( widget && !widget->visible )
            widget = widget->next;

        if( widget && (widget->focus_policy & SGUI_FOCUS_ACCEPT) )
            return widget;
    }

    return NULL;
}

/****************************************************************************/

sgui_widget* sgui_canvas_get_root( sgui_canvas* canvas )
{
    return canvas ? &(canvas->root) : NULL;
}

void sgui_canvas_set_focus( sgui_canvas* canvas, sgui_widget* widget )
{
    unsigned int fbw;
    sgui_widget* i;
    sgui_event ev;
    sgui_rect r;

    if( !canvas )
        return;

    sgui_internal_lock_mutex( );

    /* test if the widget actually belongs to the canvas */
    if( widget )
    {
        for( i=widget; i!=NULL && i!=&(canvas->root); i=i->parent );

        if( !i )
        {
            sgui_internal_unlock_mutex( );
            return;
        }
    }

    /* make sure the focus box gets redrawn */
    fbw = sgui_skin_get_focus_box_width( );

    if( canvas->focus && (canvas->focus->focus_policy & SGUI_FOCUS_DRAW) &&
        canvas->draw_focus )
    {
        sgui_widget_get_absolute_rect( canvas->focus, &r );
        r.left -= fbw;
        r.top -= fbw;
        r.right += fbw;
        r.bottom += fbw;
        sgui_canvas_add_dirty_rect( canvas, &r );
    }

    if( widget && (widget->focus_policy & SGUI_FOCUS_DRAW) &&
        (widget->focus_policy & SGUI_FOCUS_ACCEPT) )
    {
        sgui_widget_get_absolute_rect( widget, &r );
        r.left -= fbw;
        r.top -= fbw;
        r.right += fbw;
        r.bottom += fbw;
        sgui_canvas_add_dirty_rect( canvas, &r );
    }

    /* send events */
    ev.widget = widget;
    ev.type = SGUI_FOCUS_LOSE_EVENT;
    sgui_widget_send_event( canvas->focus, &ev, 0 );

    if( widget && (widget->focus_policy & SGUI_FOCUS_ACCEPT) )
    {
        ev.type = SGUI_FOCUS_EVENT;
        sgui_widget_send_event( widget, &ev, 0 );

        canvas->focus = widget;
    }
    else
    {
        canvas->focus = NULL;
    }
    sgui_internal_unlock_mutex( );
}

void sgui_canvas_add_dirty_rect( sgui_canvas* canvas, sgui_rect* r )
{
    unsigned int i;

    if( !canvas || !r )
        return;

    sgui_internal_lock_mutex( );

    /* try to find an existing diry rect it touches */
    for( i=0; i<canvas->num_dirty; ++i )
    {
        if( sgui_rect_join( canvas->dirty + i, r, 1 ) )
        {
            sgui_internal_unlock_mutex( );
            return;
        }
    }

    /* add a new one if posible, join all existing if not */
    if( canvas->num_dirty < CANVAS_MAX_DIRTY )
    {
        sgui_rect_copy( canvas->dirty + (canvas->num_dirty++), r );
    }
    else
    {
        for( i=1; i<canvas->num_dirty; ++i )
            sgui_rect_join( canvas->dirty, canvas->dirty + i, 0 );

        sgui_rect_copy( canvas->dirty + 1, r );
        canvas->num_dirty = 2;
    }

    sgui_internal_unlock_mutex( );
}

unsigned int sgui_canvas_num_dirty_rects( sgui_canvas* canvas )
{
    return canvas ? canvas->num_dirty : 0;
}

void sgui_canvas_get_dirty_rect( sgui_canvas* canvas, sgui_rect* rect,
                                 unsigned int i )
{
    sgui_internal_lock_mutex( );

    if( canvas && (i < canvas->num_dirty) )
        sgui_rect_copy( rect, canvas->dirty + i );

    sgui_internal_unlock_mutex( );
}

void sgui_canvas_clear_dirty_rects( sgui_canvas* canvas )
{
    if( canvas )
    {
        sgui_internal_lock_mutex( );
        canvas->num_dirty = 0;
        sgui_internal_unlock_mutex( );
    }
}

void sgui_canvas_redraw_widgets( sgui_canvas* canvas, int clear )
{
    int need_end = 0;
    unsigned int i;

    if( canvas )
    {
        sgui_internal_lock_mutex( );

        if( !canvas->began )
        {
            sgui_canvas_begin( canvas, NULL );
            need_end = 1;
        }

        for( i=0; i<canvas->num_dirty; ++i )
        {
            if( clear )
                canvas->clear( canvas, canvas->dirty + i );

            if( canvas->root.children )
                draw_children( canvas, &canvas->root, canvas->dirty + i );
        }

        if( need_end )
            sgui_canvas_end( canvas );

        canvas->num_dirty = 0;

        sgui_internal_unlock_mutex( );
    }
}

void sgui_canvas_draw_widgets( sgui_canvas* canvas, int clear )
{
    sgui_rect r1;
    int need_end = 0;

    if( canvas )
    {
        sgui_internal_lock_mutex( );

        sgui_rect_set_size( &r1, 0, 0, canvas->width, canvas->height );
        canvas->num_dirty = 0;

        if( !canvas->began )
        {
            sgui_canvas_begin( canvas, NULL );
            need_end = 1;
        }

        if( clear )
            canvas->clear( canvas, &r1 );

        if( canvas->root.children )
            draw_children( canvas, &canvas->root, NULL );

        if( need_end )
            sgui_canvas_end( canvas );

        sgui_internal_unlock_mutex( );
    }
}

void sgui_canvas_send_window_event( sgui_canvas* canvas, sgui_event* e )
{
    unsigned int fbw;
    sgui_widget* i;
    sgui_event ev;
    sgui_rect r;
    int x, y;

    if( !canvas || !canvas->root.children )
        return;

    /* don't handle events that the canvas generates */
    if( e->type==SGUI_FOCUS_EVENT || e->type==SGUI_MOUSE_ENTER_EVENT ||
        e->type==SGUI_MOUSE_ENTER_EVENT || e->type==SGUI_MOUSE_LEAVE_EVENT )
        return;

    sgui_internal_lock_mutex( );

    if( e->type == SGUI_MOUSE_MOVE_EVENT )
    {
        /* find the widget under the mouse cursor */
        i = sgui_widget_get_child_from_point( &canvas->root,
                                              e->arg.i2.x, e->arg.i2.y );

        /* generate mouse enter/leave events */
        if( canvas->mouse_over != i )
        {
            ev.window = e->window;
            ev.type = SGUI_MOUSE_ENTER_EVENT;
            sgui_widget_send_event( i, &ev, 0 );

            ev.type = SGUI_MOUSE_LEAVE_EVENT;
            sgui_widget_send_event( canvas->mouse_over, &ev, 0 );

            canvas->mouse_over = i;
        }
    }

    switch( e->type )
    {
    case SGUI_MOUSE_PRESS_EVENT:
    case SGUI_MOUSE_RELEASE_EVENT:
        /* transform to widget local coordinates */
        sgui_widget_get_absolute_position( canvas->mouse_over, &x, &y );

        e->arg.i3.x -= x;
        e->arg.i3.y -= y;

        /* inject event */
        sgui_widget_send_event( canvas->mouse_over, e, 0 );

        /* give clicked widget focus */
        if( canvas->focus != canvas->mouse_over )
        {
            /* make sure the focus box gets redrawn */
            if( canvas->focus &&
                (canvas->focus->focus_policy & SGUI_FOCUS_DRAW) )
            {
                fbw = sgui_skin_get_focus_box_width( );
                sgui_widget_get_absolute_rect( canvas->focus, &r );
                r.left -= fbw;
                r.top -= fbw;
                r.right += fbw;
                r.bottom += fbw;
                sgui_canvas_add_dirty_rect( canvas, &r );
            }

            /* send events */
            ev.widget = canvas->mouse_over;
            ev.type = SGUI_FOCUS_LOSE_EVENT;
            sgui_widget_send_event( canvas->focus, &ev, 0 );

            if( canvas->mouse_over &&
                (canvas->mouse_over->focus_policy & SGUI_FOCUS_ACCEPT) )
            {
                ev.type = SGUI_FOCUS_EVENT;
                sgui_widget_send_event( canvas->mouse_over, &ev, 0 );
                canvas->focus = canvas->mouse_over;
            }
            else
            {
                canvas->focus = NULL;
            }

            canvas->draw_focus = 0;
        }
        break;
    case SGUI_MOUSE_MOVE_EVENT:
        /* transform to widget local coordinates */
        sgui_widget_get_absolute_position( canvas->mouse_over, &x, &y );

        e->arg.i2.x -= x;
        e->arg.i2.y -= y;

        /* inject event */
        sgui_widget_send_event( canvas->mouse_over, e, 0 );
        break;

    /* only send to mouse over widget */
    case SGUI_MOUSE_WHEEL_EVENT:
        sgui_widget_send_event( canvas->mouse_over, e, 0 );
        break;

    /* only send keyboard events to widget that has focus */
    case SGUI_KEY_RELEASED_EVENT:
        if( e->arg.i==SGUI_KC_TAB )
        {
            /* tab key pressed -> lose focus if policy says so */
            if( canvas->focus &&
                (canvas->focus->focus_policy & SGUI_FOCUS_DROP_TAB) )
            {
                i = find_next_focus( canvas->focus );

                if( i )
                {
                    canvas->draw_focus = 1;

                    /* make the focus widget loose focus */
                    if( canvas->focus->focus_policy & SGUI_FOCUS_DRAW )
                    {
                        fbw = sgui_skin_get_focus_box_width( );
                        sgui_widget_get_absolute_rect( canvas->focus, &r );
                        r.left -= fbw;
                        r.top -= fbw;
                        r.right += fbw;
                        r.bottom += fbw;
                        sgui_canvas_add_dirty_rect( canvas, &r );
                    }

                    ev.widget = NULL;
                    ev.type = SGUI_FOCUS_LOSE_EVENT;
                    sgui_widget_send_event( canvas->focus, &ev, 0 );

                    /* give the new widget focus */
                    if( i->focus_policy & SGUI_FOCUS_DRAW )
                    {
                        fbw = sgui_skin_get_focus_box_width( );
                        sgui_widget_get_absolute_rect( i, &r );
                        r.left -= fbw;
                        r.top -= fbw;
                        r.right += fbw;
                        r.bottom += fbw;
                        sgui_canvas_add_dirty_rect( canvas, &r );
                    }

                    ev.widget = NULL;
                    ev.type = SGUI_FOCUS_EVENT;
                    sgui_widget_send_event( i, &ev, 0 );
                    canvas->focus = i;
                    break;
                }
            }
            else if( !canvas->focus )
            {
                canvas->focus = find_next_focus( &canvas->root );

                if( canvas->focus )
                {
                    canvas->draw_focus = 1;

                    if( canvas->focus->focus_policy & SGUI_FOCUS_DRAW )
                    {
                        fbw = sgui_skin_get_focus_box_width( );
                        sgui_widget_get_absolute_rect( canvas->focus, &r );
                        r.left -= fbw;
                        r.top -= fbw;
                        r.right += fbw;
                        r.bottom += fbw;
                        sgui_canvas_add_dirty_rect( canvas, &r );
                    }

                    ev.widget = NULL;
                    ev.type = SGUI_FOCUS_EVENT;
                    sgui_widget_send_event( canvas->focus, &ev, 0 );
                    break;
                }
            }
        }
    case SGUI_KEY_PRESSED_EVENT:
    case SGUI_CHAR_EVENT:
        /* escape key pressed -> lose focus if policy says so */
        if( canvas->focus && e->arg.i==SGUI_KC_ESCAPE &&
            (canvas->focus->focus_policy & SGUI_FOCUS_DROP_ESC) )
        {
            if( (canvas->focus->focus_policy & SGUI_FOCUS_DRAW) &&
                canvas->draw_focus )
            {
                fbw = sgui_skin_get_focus_box_width( );
                sgui_widget_get_absolute_rect( canvas->focus, &r );
                r.left -= fbw;
                r.top -= fbw;
                r.right += fbw;
                r.bottom += fbw;
                sgui_canvas_add_dirty_rect( canvas, &r );
            }

            ev.widget = NULL;
            ev.type = SGUI_FOCUS_LOSE_EVENT;
            sgui_widget_send_event( canvas->focus, &ev, 0 );
            canvas->focus = NULL;
            break;
        }

        /* send the event */
        sgui_widget_send_event( canvas->focus, e, 0 );

        /* make sure the focus box gets drawn */
        if( canvas->focus && !canvas->draw_focus )
        {
            canvas->draw_focus = 1;
            sgui_widget_get_absolute_rect( canvas->focus, &r );
            fbw = sgui_skin_get_focus_box_width( );
            r.left -= fbw;
            r.top -= fbw;
            r.right += fbw;
            r.bottom += fbw;
            sgui_canvas_add_dirty_rect( canvas, &r );
        }
        break;

    /* propagate all other events */
    default:
        sgui_widget_send_event( &canvas->root, e, 1 );
        break;
    }

    sgui_internal_unlock_mutex( );
}

/****************************************************************************/

void sgui_canvas_destroy( sgui_canvas* canvas )
{
    sgui_widget* i;

    if( canvas )
    {
        for( i=canvas->root.children; i!=NULL; i=i->next )
            sgui_widget_remove_from_parent( i );

        canvas->destroy( canvas );
    }
}

void sgui_canvas_resize( sgui_canvas* canvas, unsigned int width,
                         unsigned int height )
{
    if( canvas && (canvas->width!=width || canvas->height!=height) )
    {
        sgui_internal_lock_mutex( );

        if( canvas->resize )
            canvas->resize( canvas, width, height );

        canvas->width = width;
        canvas->height = height;

        canvas->root.area.right  = width-1;
        canvas->root.area.bottom = height-1;

        sgui_internal_unlock_mutex( );
    }
}

void sgui_canvas_get_size( sgui_canvas* canvas, unsigned int* width,
                           unsigned int* height )
{
    if( width  ) *width  = canvas ? canvas->width  : 0;
    if( height ) *height = canvas ? canvas->height : 0;
}

sgui_pixmap* sgui_canvas_create_pixmap( sgui_canvas* canvas,
                                        unsigned int width,
                                        unsigned int height, int format )
{
    if( !canvas || !width || !height )
        return NULL;

    return canvas->create_pixmap( canvas, width, height, format );
}

void sgui_canvas_begin( sgui_canvas* canvas, sgui_rect* r )
{
    sgui_rect r0;

    if( canvas && !canvas->began )
    {
        if( r )
        {
            sgui_rect_copy( &r0, r );

            /* clip region to canvas size */
            if( r0.left < 0 )
                r0.left = 0;

            if( r0.top < 0 )
                r0.top = 0;

            if( r0.right >= (int)canvas->width )
                r0.right = canvas->width - 1;

            if( r0.bottom >= (int)canvas->height )
                r0.bottom = canvas->height - 1;
        }
        else
        {
            sgui_rect_set_size( &r0, 0, 0, canvas->width, canvas->height );
        }

        /* tell the implementation to begin drawing */
        if( canvas->begin )
            canvas->begin( canvas, &r0 );

        canvas->sc = r0;
        canvas->began = 1;
    }
}

void sgui_canvas_end( sgui_canvas* canvas )
{
    if( canvas && canvas->began )
    {
        canvas->began = 0;

        if( canvas->end )
            canvas->end( canvas );
    }
}

void sgui_canvas_clear( sgui_canvas* canvas, sgui_rect* r )
{
    sgui_rect r1;

    if( !canvas || !canvas->began )
        return;

    /* if no rect is given, set to the full canvas area */
    if( r )
    {
        COPY_RECT_OFFSET( r1, r );
    }
    else
    {
        sgui_rect_set_size( &r1, 0, 0, canvas->width, canvas->height );
    }

    /* clear if we have an intersection */
    if( sgui_rect_get_intersection( &r1, &canvas->sc, &r1 ) )
        canvas->clear( canvas, &r1 );
}

/**************************** drawing functions ****************************/

void sgui_canvas_draw_box( sgui_canvas* canvas, sgui_rect* r,
                           unsigned char* color, int format )
{
    sgui_rect r1;

    /* sanity check */
    if( !canvas || !color || !canvas->began || !r )
        return;

    if( format==SGUI_RGBA8 && color[3]==0xFF )
        format = SGUI_RGB8;

    /* offset and clip the given rectangle */
    COPY_RECT_OFFSET( r1, r );

    if( !sgui_rect_get_intersection( &r1, &canvas->sc, &r1 ) )
        return;

    canvas->draw_box( canvas, &r1, color, format );
}

void sgui_canvas_draw_line( sgui_canvas* canvas, int x, int y,
                            unsigned int length, int horizontal,
                            unsigned char* color, int format )
{
    sgui_rect r;

    /* santiy check */
    if( !canvas || !canvas->began || !color )
        return;

    if( format==SGUI_RGBA8 && color[3]==0xFF )
        format = SGUI_RGB8;

    if( horizontal )
        sgui_rect_set_size( &r, x, y, length, 1 );
    else
        sgui_rect_set_size( &r, x, y, 1, length );

    /* offset and clip the given rectangle */
    r.left   += canvas->ox;
    r.right  += canvas->ox;
    r.top    += canvas->oy;
    r.bottom += canvas->oy;

    if( !sgui_rect_get_intersection( &r, &canvas->sc, &r ) )
        return;

    canvas->draw_box( canvas, &r, color, format );
}

void sgui_canvas_blit( sgui_canvas* canvas, int x, int y,
                       sgui_pixmap* pixmap, sgui_rect* srcrect )
{
    unsigned int w, h;
    sgui_rect r, clip;

    /* sanity check */
    if( !canvas || !pixmap || !canvas->began )
        return;

    x += canvas->ox;
    y += canvas->oy;

    sgui_pixmap_get_size( pixmap, &w, &h );
    sgui_rect_set_size( &r, 0, 0, w, h );

    if( srcrect )
        sgui_rect_get_intersection( &r, &r, srcrect );

    /* clip the against the scissor rectangle */
    clip.left   = x;
    clip.top    = y;
    clip.right  = x + r.right  - r.left;
    clip.bottom = y + r.bottom - r.top;

    sgui_rect_get_intersection( &clip, &clip, &canvas->sc );

    r.left  += clip.left - x;
    r.top   += clip.top  - y;
    r.right  = r.left + clip.right-clip.left;
    r.bottom = r.top  + clip.bottom-clip.top;

    x = clip.left;
    y = clip.top;

    /* do the blitting */
    canvas->blit( canvas, x, y, pixmap, &r );
}

void sgui_canvas_blend( sgui_canvas* canvas, int x, int y,
                        sgui_pixmap* pixmap, sgui_rect* srcrect )
{
    unsigned int w, h;
    sgui_rect r, clip;

    /* sanity check */
    if( !canvas || !pixmap || !canvas->began )
        return;

    x += canvas->ox;
    y += canvas->oy;

    sgui_pixmap_get_size( pixmap, &w, &h );
    sgui_rect_set_size( &r, 0, 0, w, h );

    if( srcrect )
        sgui_rect_get_intersection( &r, &r, srcrect );

    /* clip the against the scissor rectangle */
    clip.left   = x;
    clip.top    = y;
    clip.right  = x + r.right  - r.left;
    clip.bottom = y + r.bottom - r.top;

    sgui_rect_get_intersection( &clip, &clip, &canvas->sc );

    r.left  += clip.left - x;
    r.top   += clip.top  - y;
    r.right  = r.left + clip.right-clip.left;
    r.bottom = r.top  + clip.bottom-clip.top;

    x = clip.left;
    y = clip.top;

    /* do the blending */
    canvas->blend( canvas, x, y, pixmap, &r );
}

int sgui_canvas_draw_text_plain( sgui_canvas* canvas, int x, int y,
                                 int bold, int italic,
                                 unsigned char* color,
                                 const char* text, unsigned int length )
{
    sgui_font* font = sgui_skin_get_default_font( bold, italic );

    /* sanity check */
    if( !canvas || !font || !color || !text || !canvas->began || !length )
        return 0;

    x += canvas->ox;
    y += canvas->oy;

    if( x>=canvas->sc.right || y>= canvas->sc.bottom )
        return 0;

    return canvas->draw_string( canvas, x, y, font, color, text, length );
}

void sgui_canvas_draw_text( sgui_canvas* canvas, int x, int y,
                            const char* text )
{
    int i = 0, X = 0, font_stack_index = 0, font_height;
    unsigned char col[3], font_stack[10], f = 0;
    long c;

    /* sanity check */
    if( !canvas || !text )
        return;

    sgui_skin_get_default_font_color( col );

    font_height = sgui_skin_get_default_font_height( );

    for( ; text && text[ i ]; ++i )
    {
        if( text[ i ] == '<' )  /* we encountered a tag */
        {
            /* draw what we got so far with the current settings */
            X += sgui_canvas_draw_text_plain( canvas, x+X, y, f & 0x02, f & 0x01,
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
            sgui_canvas_draw_text_plain( canvas, x+X, y, f&0x02, f&0x01,
                                         col, text, i );

            text += i + 1;    /* skip to next line */
            i = -1;           /* reset i to 0 at next iteration */
            X = 0;            /* adjust move cursor */
            y += font_height;
        }
    }

    /* draw what is still left */
    sgui_canvas_draw_text_plain(canvas, x+X, y, f&0x02, f&0x01, col, text, i);
}

