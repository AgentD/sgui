/*
 * button.c
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
#include "sgui_button.h"
#include "sgui_skin.h"
#include "sgui_event.h"
#include "sgui_canvas.h"
#include "sgui_font.h"
#include "sgui_internal.h"
#include "sgui_widget.h"
#include "sgui_icon_cache.h"

#include <stdlib.h>
#include <string.h>



#define BUTTON        0x00
#define TOGGLE_BUTTON 0x01
#define CHECKBOX      0x02
#define RADIO_BUTTON  0x03
#define SELECTED      0x10
#define HAVE_ICON     0x20




typedef struct sgui_button
{
    sgui_widget super;

    union
    {
        char* text;

#ifndef SGUI_NO_ICON_CACHE
        struct
        {
            sgui_icon_cache* cache;
            unsigned int id;
        }
        icon;
#endif
    }
    dpy;

    int flags;
    unsigned int cx, cy;      /* button: text position,
                                 checkbox/radio button: text/object offset */

    struct sgui_button* prev; /* radio button menu: next in menu */
    struct sgui_button* next; /* radio button menu: previous in menu */
}
sgui_button;



static void button_select( sgui_button* this, int select, int postevent )
{
    sgui_button* i;
    sgui_event ev;
    sgui_rect r;

    if( !this )
        return;

    sgui_internal_lock_mutex( );

    this->flags = select ? (this->flags | SELECTED) : 
                           (this->flags & (~SELECTED));

    sgui_widget_get_absolute_rect( &(this->super), &r );
    sgui_canvas_add_dirty_rect( this->super.canvas, &r );

    if( postevent )
    {
        ev.src.widget = (sgui_widget*)this;
        ev.type = select ? SGUI_BUTTON_IN_EVENT : SGUI_BUTTON_OUT_EVENT;
        sgui_event_post( &ev );
    }

    /* if not in a group, we are done here */
    if( !this->next && !this->prev )
        goto done;

    /* deselect all preceeding buttons in a group */
    for( i=this->prev; i!=NULL; i=i->prev )
    {
        if( i->flags & SELECTED )
        {
            i->flags &= ~SELECTED;
            sgui_widget_get_absolute_rect( &(i->super), &r );
            sgui_canvas_add_dirty_rect( i->super.canvas, &r );
            goto done;
        }
    }

    /* deselect all following buttons in a group */
    for( i=this->next; i!=NULL; i=i->next )
    {
        if( i->flags & SELECTED )
        {
            i->flags &= ~SELECTED;
            sgui_widget_get_absolute_rect( &(i->super), &r );
            sgui_canvas_add_dirty_rect( i->super.canvas, &r );
            goto done;
        }
    }

done:
    sgui_internal_unlock_mutex( );
}

static void button_draw( sgui_widget* super )
{
    sgui_button* this = (sgui_button*)super;
    int in = (this->flags & SELECTED)!=0;
    int type = (this->flags & 0x03);

    if( type==BUTTON || type==TOGGLE_BUTTON )
    {
        sgui_skin_draw_button( super->canvas, &super->area, in );

        if( this->flags & HAVE_ICON )
        {
#ifndef SGUI_NO_ICON_CACHE
            sgui_icon_cache_draw_icon( this->dpy.icon.cache,
                                       this->dpy.icon.id,
                                       super->area.left+ this->cx - in,
                                       super->area.top + this->cy - in );
#endif
        }
        else
        {
            sgui_canvas_draw_text( super->canvas,
                                   super->area.left+this->cx-in,
                                   super->area.top+this->cy-in,
                                   this->dpy.text );
        }
    }
    else
    {
        if( type==CHECKBOX )
        {
            sgui_skin_draw_checkbox( super->canvas, super->area.left,
                                     super->area.top+this->cy, in );
        }
        else
        {
            sgui_skin_draw_radio_button( super->canvas, super->area.left,
                                         super->area.top+this->cy, in );
        }

        if( this->flags & HAVE_ICON )
        {
#ifndef SGUI_NO_ICON_CACHE
            sgui_icon_cache_draw_icon( this->dpy.icon.cache,
                                       this->dpy.icon.id,
                                       super->area.left+this->cx,
                                       super->area.top );
#endif
        }
        else
        {
            sgui_canvas_draw_text( super->canvas, super->area.left+this->cx,
                                   super->area.top, this->dpy.text );
        }
    }
}

static void toggle_button_on_event( sgui_widget* super, const sgui_event* e )
{
    sgui_button* this = (sgui_button*)super;
    int prev=0, next=0;

    if( this->prev || this->next )
    {
        if( e->type==SGUI_KEY_RELEASED_EVENT )
        {
            if( this->flags & SELECTED )
            {
                prev = e->arg.i==SGUI_KC_UP || e->arg.i==SGUI_KC_LEFT;
                next = e->arg.i==SGUI_KC_DOWN || e->arg.i==SGUI_KC_RIGHT;

                if( prev && this->prev )
                {
                    sgui_canvas_set_focus( this->super.canvas,
                                           (sgui_widget*)this->prev );
                    button_select( this->prev, 1, 1 );
                }
                else if( next && this->next )
                {
                    sgui_canvas_set_focus( this->super.canvas,
                                           (sgui_widget*)this->next );
                    button_select( this->next, 1, 1 );
                }
            }
            else if( e->arg.i==SGUI_KC_RETURN || e->arg.i==SGUI_KC_SPACE )
            {
                button_select( this, 1, 1 );
            }
        }
        else if( e->type==SGUI_MOUSE_RELEASE_EVENT )
        {
            if( !(this->flags & SELECTED) )
                button_select( this, 1, 1 );
        }
    }
    else
    {
        if( (e->type == SGUI_MOUSE_RELEASE_EVENT) ||
            (e->type==SGUI_KEY_RELEASED_EVENT &&
             (e->arg.i==SGUI_KC_RETURN || e->arg.i==SGUI_KC_SPACE)) )
        {
            button_select( this, !(this->flags & SELECTED), 1 );
        }
    }
}

static void button_on_event( sgui_widget* super, const sgui_event* e )
{
    sgui_button* this = (sgui_button*)super;

    if( this->flags & SELECTED )
    {
        if( e->type==SGUI_MOUSE_LEAVE_EVENT )
        {
            button_select( this, 0, 0 );
        }
        else if( e->type==SGUI_MOUSE_RELEASE_EVENT ||
                 (e->type==SGUI_KEY_RELEASED_EVENT &&
                  (e->arg.i==SGUI_KC_RETURN || e->arg.i==SGUI_KC_SPACE)) )
        {
            button_select( this, 0, 1 );
        }
    }
    else if( e->type==SGUI_MOUSE_PRESS_EVENT ||
             (e->type==SGUI_KEY_PRESSED_EVENT &&
              (e->arg.i==SGUI_KC_RETURN || e->arg.i==SGUI_KC_SPACE)) )
    {
        button_select( this, 1, 0 );
    }
}

static void button_destroy( sgui_widget* super )
{
    sgui_button* this = (sgui_button*)super;

    /* detatch from linked list */
    sgui_internal_lock_mutex( );
    if( this->prev ) this->prev->next = this->next;
    if( this->next ) this->next->prev = this->prev;
    sgui_internal_unlock_mutex( );

    /* free memory of text buffer and button */
    if( !(this->flags & HAVE_ICON) )
        free( this->dpy.text );

    free( this );
}

static sgui_widget* button_create_common( int x, int y, unsigned int width,
                                          unsigned int height,
                                          unsigned int iconid,
                                          sgui_icon_cache* cache,
                                          const char* text, int flags )
{
    unsigned int len, text_width, text_height;
    sgui_widget* super;
    sgui_button* this;
    sgui_rect r;

    /* sanity check */
    if( flags & HAVE_ICON )
    {
        if( !cache || !sgui_icon_cache_get_icon_area( cache, iconid, &r ) )
            return NULL;
    }
    else
    {
        if( !text )
            return NULL;
    }

    /* allocate button */
    this = malloc( sizeof(sgui_button) );
    super = (sgui_widget*)this;

    if( !this )
        return NULL;

    /* allocate space for the text */
    if( flags & HAVE_ICON )
    {
#ifndef SGUI_NO_ICON_CACHE
        this->dpy.icon.cache = cache;
        this->dpy.icon.id = iconid;
#else
        this->dpy.text = 0;
        r.left = r.right = r.top = r.bottom = 0;
#endif
    }
    else
    {
        len = strlen( text );
        this->dpy.text = malloc( len + 1 );

        if( !this->dpy.text )
        {
            free( this );
            return NULL;
        }

        sgui_skin_get_text_extents( text, &r );
        memcpy( this->dpy.text, text, len + 1 );
    }

    text_width = SGUI_RECT_WIDTH( r );
    text_height = SGUI_RECT_HEIGHT( r );

    /* compute size */
    if( (flags & 0x03)==BUTTON || (flags & 0x03)==TOGGLE_BUTTON )
    {
        this->cx = width /2 - text_width/2;
        this->cy = height/2 - text_height/2;
    }
    else
    {
        if( (flags & 0x03)==CHECKBOX )
            sgui_skin_get_checkbox_extents( &r );
        else if( (flags & 0x03)==RADIO_BUTTON )
            sgui_skin_get_radio_button_extents( &r );

        this->cx = SGUI_RECT_WIDTH( r );
        this->cy = SGUI_RECT_HEIGHT( r );

        width  = this->cx + text_width;
        height = MAX(this->cy, text_height);

        this->cy = (height - this->cy) / 2;
        this->cy = MAX( this->cy, 0 );
    }

    /* initialise remaining fields */
    sgui_internal_widget_init( super, x, y, width, height );

    this->prev = NULL;
    this->next = NULL;
    this->flags = flags;

    if( (flags & 0x03)==BUTTON )
        this->super.window_event_callback = button_on_event;
    else
        this->super.window_event_callback = toggle_button_on_event;

    super->draw_callback = button_draw;
    super->destroy = button_destroy;

    return (sgui_widget*)this;
}

/***************************************************************************/
sgui_widget* sgui_icon_button_create( int x, int y, unsigned int width,
                                      unsigned int height,
                                      sgui_icon_cache* cache,
                                      unsigned int icon, int toggleable )
{
    return button_create_common( x, y, width, height, icon, cache, NULL,
                                 (toggleable ? TOGGLE_BUTTON : BUTTON) |
                                 HAVE_ICON );
}

sgui_widget* sgui_radio_button_create( int x, int y, const char* text )
{
    return button_create_common( x, y, 0, 0, 0, NULL, text, RADIO_BUTTON );
}

sgui_widget* sgui_checkbox_create( int x, int y, const char* text )
{
    return button_create_common( x, y, 0, 0, 0, NULL, text, CHECKBOX );
}

sgui_widget* sgui_button_create( int x, int y,
                                 unsigned int width, unsigned int height,
                                 const char* text, int toggleable )
{
    return button_create_common( x, y, width, height, 0, NULL, text,
                                 toggleable ? TOGGLE_BUTTON : BUTTON );
}

void sgui_button_group_connect( sgui_widget* super, sgui_widget* previous,
                                sgui_widget* next )
{
    sgui_button* this = (sgui_button*)super;

    if( this )
    {
        sgui_internal_lock_mutex( );

        /* disconnect from existing linked list */
        if( this->prev ) this->prev->next = this->next;
        if( this->next ) this->next->prev = this->prev;

        /* connect to new list */
        this->prev = (sgui_button*)previous;
        this->next = (sgui_button*)next;

        if( this->prev ) this->prev->next = this;
        if( this->next ) this->next->prev = this;

        sgui_internal_unlock_mutex( );
    }
}

void sgui_button_set_text( sgui_widget* super, const char* text )
{
    unsigned int len, text_width, text_height;
    sgui_button* this = (sgui_button*)super;
    sgui_rect r;

    /* sanity check */
    if( !this || !text )
        return;

    len = strlen( text );

    sgui_skin_get_text_extents( text, &r );
    text_width = SGUI_RECT_WIDTH( r );
    text_height = SGUI_RECT_HEIGHT( r );

    sgui_internal_lock_mutex( );

    /* adjust buffer size and copy */
    if( !(this->flags & HAVE_ICON) )
        free( this->dpy.text );

    this->flags &= ~HAVE_ICON;
    this->dpy.text = malloc( len + 1 );

    if( this->dpy.text )
        memcpy( this->dpy.text, text, len + 1 );

    /* determine text position */
    if( (this->flags & 0x03)==BUTTON || (this->flags & 0x03)==TOGGLE_BUTTON )
    {
        this->cx = (super->area.left + super->area.right - text_width)>>1;
        this->cy = (super->area.top + super->area.bottom - text_height)>>1;
    }
    else
    {
        super->area.right = super->area.left + this->cx + text_width;
    }

    sgui_internal_unlock_mutex( );
}

void sgui_button_set_icon( sgui_widget* super, sgui_icon_cache* cache,
                           unsigned int icon )
{
#ifndef SGUI_NO_ICON_CACHE
    unsigned int img_width, img_height, width, height;
    sgui_button* this = (sgui_button*)super;
    sgui_rect r;

    /* sanity check */
    if( !this || !cache )
        return;

    if( !sgui_icon_cache_get_icon_area( cache, icon, &r ) )
        return;

    sgui_internal_lock_mutex( );

    /* copy display data */
    if( !(this->flags & HAVE_ICON) )
        free( this->dpy.text );

    this->flags |= HAVE_ICON;
    this->dpy.icon.cache = cache;
    this->dpy.icon.id = icon;

    img_width = SGUI_RECT_WIDTH( r );
    img_height = SGUI_RECT_HEIGHT( r );

    /* adjust position and size */
    if( (this->flags & 0x03)==BUTTON || (this->flags & 0x03)==TOGGLE_BUTTON )
    {
        this->cx = (super->area.left + super->area.right - img_width)>>1;
        this->cy = (super->area.top + super->area.bottom - img_height)>>1;
    }
    else
    {
        if( (this->flags & 0x03)==CHECKBOX )
            sgui_skin_get_checkbox_extents( &r );
        else if( (this->flags & 0x03)==RADIO_BUTTON )
            sgui_skin_get_radio_button_extents( &r );

        this->cx = SGUI_RECT_WIDTH( r );
        this->cy = SGUI_RECT_HEIGHT( r );

        width  = this->cx + img_width;
        height = MAX(this->cy, img_height);

        this->cy = (height - this->cy) / 2;
        this->cy = MAX( this->cy, 0 );

        sgui_rect_set_size( &(super->area),
                            super->area.left, super->area.right,
                            width, height );
    }

    sgui_internal_unlock_mutex( );
#else
    (void)super; (void)cache; (void)icon;
#endif
}

void sgui_button_set_state( sgui_widget* super, int state )
{
    sgui_button* this = (sgui_button*)super;

    if( this && (this->flags & 0x03)!=BUTTON )
    {
        button_select( this, state, 0 );
    }
}

int sgui_button_get_state( sgui_widget* this )
{
    return this ? (((sgui_button*)this)->flags & SELECTED)!=0 : 0;
}

