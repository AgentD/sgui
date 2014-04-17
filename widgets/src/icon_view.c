/*
 * icon_view.c
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
#include "sgui_icon_cache.h"
#include "sgui_icon_view.h"
#include "sgui_internal.h"
#include "sgui_widget.h"
#include "sgui_event.h"
#include "sgui_skin.h"

#include <stdlib.h>
#include <string.h>



typedef struct icon
{
    unsigned int id;        /* id of the icon */
    sgui_rect icon_area;    /* area inside the view */
    sgui_rect text_area;    /* area of the subtext inside the view */
    char* subtext;          /* the text to write underneath the icon */
    struct icon* next;      /* linked list pointer */
}
icon;

typedef struct
{
    sgui_widget super;
    icon* icons;            /* a linked list of icons */
    icon* selected;         /* the currently selected icon */
    sgui_icon_cache* cache; /* the underlying icon cache */
    int draw_background;    /* whether to draw the background */
    int grab_x, grab_y;     /* the selected icon last grabbing position */
    int draging;            /* whether we are currently draging an icon */
    int offset;             /* the offset from the border of the view */
}
icon_view;



static void get_icon_bounding_box( icon_view* this, icon* i, sgui_rect* r )
{
    int x, y;

    r->top    = i->icon_area.top;
    r->left   = MIN(i->icon_area.left,   i->text_area.left);
    r->right  = MAX(i->icon_area.right,  i->text_area.right);
    r->bottom = MAX(i->icon_area.bottom, i->text_area.bottom);

    sgui_widget_get_absolute_position( &(this->super), &x, &y );
    r->left   += x;
    r->right  += x;
    r->top    += y;
    r->bottom += y;
}

static void icon_move_to_front( icon_view* this, icon* front )
{
    sgui_rect r;
    icon* i;

    get_icon_bounding_box( this, front, &r );
    sgui_canvas_add_dirty_rect( this->super.canvas, &r );

    if( front == this->icons )
    {
        this->icons = this->icons->next;
        for( i=this->icons; i->next!=NULL; i=i->next ) { }
    }
    else
    {
        for( i=this->icons; i->next!=front; i=i->next ) { }
        i->next = i->next->next;
        for( ; i->next!=NULL; i=i->next ) { }
    }

    i->next = front;
    front->next = NULL;
}

static void icon_view_on_event( sgui_widget* super, const sgui_event* e )
{
    icon_view* this = (icon_view*)super;
    int x, y, dx, dy;
    sgui_rect r;
    icon* new;
    icon* i;

    sgui_internal_lock_mutex( );

    if( e->type==SGUI_MOUSE_PRESS_EVENT &&
        e->arg.i3.z==SGUI_MOUSE_BUTTON_LEFT )
    {
        this->grab_x = e->arg.i3.x;
        this->grab_y = e->arg.i3.y;

        /* try to find an icon that got clicked (last in list = top most) */
        for( new=NULL, i=this->icons; i!=NULL; i=i->next )
        {
            if( sgui_rect_is_point_inside( &(i->icon_area),
                                           e->arg.i3.x, e->arg.i3.y ) ||
                sgui_rect_is_point_inside( &(i->text_area),
                                           e->arg.i3.x, e->arg.i3.y ) )
            {
                new = i;
            }
        }

        if( this->selected!=new )
        {
            if( this->selected )
            {
                get_icon_bounding_box( this, this->selected, &r );
                sgui_canvas_add_dirty_rect( super->canvas, &r );
            }

            this->selected = new;

            if( this->selected )
            {
                icon_move_to_front( this, this->selected );
            }
        }

        this->draging = (this->selected!=NULL);
    }
    else if(e->type==SGUI_MOUSE_MOVE_EVENT && this->selected && this->draging)
    {
        /* get mouse movement difference vector */
        dx = e->arg.i2.x - this->grab_x;
        dy = e->arg.i2.y - this->grab_y;

        /* clamp movement vector to keep icon inside area */
        x = SGUI_RECT_WIDTH(super->area) - this->offset;
        y = SGUI_RECT_HEIGHT(super->area) - this->offset;

        if( (this->selected->icon_area.left + dx)<this->offset )
            dx = this->offset - this->selected->icon_area.left;

        if( (this->selected->icon_area.top + dy)<this->offset )
            dy = this->offset - this->selected->icon_area.top;

        if( (this->selected->icon_area.right + dx)>x )
            dx = x - this->selected->icon_area.right;

        if( (this->selected->icon_area.bottom + dy)>y )
            dy = y - this->selected->icon_area.bottom;

        if( (this->selected->text_area.left + dx)<this->offset )
            dx = this->offset - this->selected->text_area.left;

        if( (this->selected->text_area.top + dy)<this->offset )
            dy = this->offset - this->selected->text_area.top;

        if( (this->selected->text_area.right + dx)>x )
            dx = x - this->selected->text_area.right;

        if( (this->selected->text_area.bottom + dy)>y )
            dy = y - this->selected->text_area.bottom;

        /* flag affected area as dirty */
        get_icon_bounding_box( this, this->selected, &r );

        if( dx>0 ) r.right += dx;
        else       r.left  += dx;

        if( dy>0 ) r.bottom += dy;
        else       r.top    += dy;

        sgui_canvas_add_dirty_rect( super->canvas, &r );

        /* move selected icon */
        this->grab_x = e->arg.i2.x;
        this->grab_y = e->arg.i2.y;

        this->selected->icon_area.left += dx;
        this->selected->icon_area.top += dy;
        this->selected->icon_area.right += dx;
        this->selected->icon_area.bottom += dy;

        this->selected->text_area.left += dx;
        this->selected->text_area.top += dy;
        this->selected->text_area.right += dx;
        this->selected->text_area.bottom += dy;
    }
    else if( (e->type==SGUI_MOUSE_RELEASE_EVENT &&
              e->arg.i3.z==SGUI_MOUSE_BUTTON_LEFT) ||
             e->type==SGUI_MOUSE_LEAVE_EVENT )
    {
        this->draging = 0;
    }
    else if( e->type==SGUI_FOCUS_LOSE_EVENT && this->selected )
    {
        get_icon_bounding_box( this, this->selected, &r );
        sgui_canvas_add_dirty_rect( super->canvas, &r );
        this->selected = NULL;
        this->draging = 0;
    }

    sgui_internal_unlock_mutex( );
}

static void icon_view_draw( sgui_widget* super )
{
    icon_view* this = (icon_view*)super;
    sgui_rect r;
    icon* i;

    if( this->draw_background )
    {
        sgui_skin_draw_frame( super->canvas, &(super->area) );
    }

    for( i=this->icons; i!=NULL; i=i->next )
    {
        sgui_icon_cache_draw_icon( this->cache, i->id,
                                   super->area.left + i->icon_area.left,
                                   super->area.top  + i->icon_area.top );

        if( i==this->selected && i->subtext )
        {
            r = i->text_area;
            r.left += super->area.left;
            r.right += super->area.left;
            r.top += super->area.top;
            r.bottom += super->area.top;
            sgui_skin_draw_focus_box( super->canvas, &r );
        }

        sgui_canvas_draw_text( super->canvas,
                               super->area.left + i->text_area.left,
                               super->area.top  + i->text_area.top,
                               i->subtext );
    }
}

static void icon_view_destroy( sgui_widget* super )
{
    icon_view* this = (icon_view*)super;
    icon* old;
    icon* i;

    sgui_internal_lock_mutex( );

    for( i=this->icons; i!=NULL; )
    {
        old = i;
        i = i->next;
        free( old->subtext );
        free( old );
    }

    sgui_internal_unlock_mutex( );

    free( this );
}

/***************************************************************************/

sgui_widget* sgui_icon_view_create( int x, int y, unsigned width,
                                    unsigned int height,
                                    sgui_icon_cache* cache,
                                    int background )
{
    sgui_widget* super;
    icon_view* this;

    this = malloc( sizeof(icon_view) );
    super = (sgui_widget*)this;

    if( this )
    {
        memset( this, 0, sizeof(icon_view) );

        sgui_internal_widget_init( super, x, y, width, height );

        this->cache = cache;
        this->draw_background = background;
        this->offset = background ? sgui_skin_get_frame_border_width( ) : 0;

        super->window_event_callback = icon_view_on_event;
        super->draw_callback = icon_view_draw;
        super->destroy = icon_view_destroy;

        super->focus_policy = SGUI_FOCUS_ACCEPT|
                              SGUI_FOCUS_DROP_ESC|
                              SGUI_FOCUS_DROP_TAB;
    }

    return super;
}

void sgui_icon_view_add_icon( sgui_widget* super, int x, int y,
                              const char* subtext, unsigned int id )
{
    unsigned int txt_w, txt_h, img_w, img_h;
    icon_view* this = (icon_view*)super;
    sgui_rect r;
    icon* i;

    /* sanity check */
    if( !this )
        return;

    /* obtain icon size */
    if( !sgui_icon_cache_get_icon_area( this->cache, id, &r ) )
        return;

    img_w = SGUI_RECT_WIDTH(r);
    img_h = SGUI_RECT_HEIGHT(r);

    /* try to allocate icon object */
    i = malloc( sizeof(icon) );

    if( !i )
        return;

    if( subtext )
    {
        /* try to allocate text buffer */
        i->subtext = malloc( strlen(subtext)+1 );

        if( !i->subtext )
        {
            free( i );
            return;
        }

        /* copy text and get text size */
        strcpy( i->subtext, subtext );
        sgui_skin_get_text_extents( subtext, &r );

        txt_w = SGUI_RECT_WIDTH(r);
        txt_h = SGUI_RECT_HEIGHT(r);
    }
    else
    {
        i->subtext = NULL;
        txt_w = txt_h = 0;
    }

    /* initialize icon & text area */
    if( img_w>=txt_w )
    {
        sgui_rect_set_size( &(i->icon_area), x, y, img_w, img_h );
        sgui_rect_set_size( &(i->text_area),
                            x+((img_w-txt_w)/2),
                            i->icon_area.bottom, txt_w, txt_h );
    }
    else
    {
        sgui_rect_set_size( &(i->icon_area), (x + (txt_w-img_w)/2), y,
                                             img_w, img_h );

        sgui_rect_set_size( &(i->text_area), x, i->icon_area.bottom,
                                             txt_w, txt_h );
    }

    i->id = id;

    sgui_internal_lock_mutex( );
    i->next = this->icons;
    this->icons = i;
    sgui_internal_unlock_mutex( );
}

