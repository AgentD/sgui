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

#include <stdlib.h>
#include <string.h>



#define SGUI_BUTTON                 1
#define SGUI_BUTTON_SELECTED        2
#define SGUI_CHECKBOX               3
#define SGUI_CHECKBOX_SELECTED      4
#define SGUI_RADIO_BUTTON           5
#define SGUI_RADIO_BUTTON_SELECTED  6




typedef struct sgui_button
{
    sgui_widget super;

    char* text;
    int type;                 /* button type */
    unsigned int cx, cy;      /* button: text position,
                                 checkbox/radio button: text/object offset */

    struct sgui_button* prev; /* radio button menu: next in menu */
    struct sgui_button* next; /* radio button menu: previous in menu */
}
sgui_button;



static void checkbox_draw( sgui_widget* super )
{
    sgui_button* this = (sgui_button*)super;

    sgui_skin_draw_checkbox( super->canvas, super->area.left,
                             super->area.top+this->cy,
                             this->type==SGUI_CHECKBOX_SELECTED );

    sgui_canvas_draw_text( super->canvas, super->area.left+this->cx,
                           super->area.top, this->text );
}

static void radiobutton_draw( sgui_widget* super )
{
    sgui_button* this = (sgui_button*)super;

    sgui_skin_draw_radio_button( super->canvas,
                                 super->area.left, super->area.top+this->cy,
                                 this->type==SGUI_RADIO_BUTTON_SELECTED );

    sgui_canvas_draw_text( super->canvas, super->area.left+this->cx,
                           super->area.top, this->text );
}

static void button_draw( sgui_widget* super )
{
    sgui_button* this = (sgui_button*)super;
    int in = this->type==SGUI_BUTTON_SELECTED;

    sgui_skin_draw_button( super->canvas, &super->area, in );

    sgui_canvas_draw_text( super->canvas, super->area.left+this->cx-in,
                           super->area.top+this->cy-in, this->text );
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
    free( this->text );
    free( this );
}

static sgui_button* button_create_common( int x, int y, unsigned int width,
                                          unsigned int height,
                                          const char* text, int type )
{
    sgui_button* this = malloc( sizeof(sgui_button) );
    sgui_widget* super = (sgui_widget*)this;
    unsigned int len, text_width, text_height;
    sgui_rect r;

    if( !this )
        return NULL;

    /* allocate space for the text */
    len = strlen( text );
    this->text = malloc( len + 1 );

    if( !this->text )
    {
        free( this );
        return NULL;
    }

    /* compute text size */
    sgui_skin_get_text_extents( text, &r );
    text_width = SGUI_RECT_WIDTH( r );
    text_height = SGUI_RECT_HEIGHT( r );

    /* compute size */
    if( type==SGUI_CHECKBOX )
    {
        sgui_skin_get_checkbox_extents( &r );

        this->cx = SGUI_RECT_WIDTH( r );
        this->cy = SGUI_RECT_HEIGHT( r );

        width  = this->cx + text_width;
        height = MAX(this->cy, text_height);

        this->cy = (height - this->cy) / 2;
        this->cy = MAX( this->cy, 0 );
    }
    else if( type==SGUI_RADIO_BUTTON )
    {
        sgui_skin_get_radio_button_extents( &r );

        this->cx = SGUI_RECT_WIDTH( r );
        this->cy = SGUI_RECT_HEIGHT( r );

        width  = this->cx + text_width;
        height = MAX(this->cy, text_height);

        this->cy = (height - this->cy) / 2;
        this->cy = MAX( this->cy, 0 );
    }
    else
    {
        this->cx  = width /2 - text_width/2;
        this->cy  = height/2 - text_height/2;
    }

    sgui_internal_widget_init( super, x, y, width, height );

    /* initialise remaining fields */
    memcpy( this->text, text, len + 1 );

    this->prev = NULL;
    this->next = NULL;
    this->type = type;

    super->destroy = button_destroy;
    super->draw_callback = type==SGUI_BUTTON ? button_draw :
                           type==SGUI_CHECKBOX ? checkbox_draw :
                           radiobutton_draw;

    return this;
}

/**************************** radio button code ****************************/
static void radio_button_select( sgui_button* this )
{
    sgui_widget* super = &this->super;
    sgui_button* i;
    sgui_rect r;

    sgui_internal_lock_mutex( );

    this->type = SGUI_RADIO_BUTTON_SELECTED;
    sgui_widget_get_absolute_rect( super, &r );
    sgui_canvas_add_dirty_rect( super->canvas, &r );

    /* deselect all preceeding radio buttons */
    for( i=this->prev; i!=NULL; i=i->prev )
    {
        if( i->type == SGUI_RADIO_BUTTON_SELECTED )
        {
            sgui_widget_get_absolute_rect( &i->super, &r );
            sgui_canvas_add_dirty_rect( i->super.canvas, &r );
            i->type = SGUI_RADIO_BUTTON;
            goto done;
        }
    }

    /* deselect all following radio buttons */
    for( i=this->next; i!=NULL; i=i->next )
    {
        if( i->type == SGUI_RADIO_BUTTON_SELECTED )
        {
            sgui_widget_get_absolute_rect( &i->super, &r );
            sgui_canvas_add_dirty_rect( i->super.canvas, &r );
            i->type = SGUI_RADIO_BUTTON;
            goto done;
        }
    }

done:
    sgui_internal_unlock_mutex( );
}

static void radio_on_event( sgui_widget* super, const sgui_event* e )
{
    sgui_button* this = (sgui_button*)super;
    sgui_event ev;

    sgui_internal_lock_mutex( );

    if( (e->type==SGUI_MOUSE_RELEASE_EVENT ||
         (e->type==SGUI_KEY_RELEASED_EVENT &&
          (e->arg.i==SGUI_KC_RETURN || e->arg.i==SGUI_KC_SPACE))) &&
        this->type==SGUI_RADIO_BUTTON )
    {
        radio_button_select( this );

        ev.widget = super;
        ev.type = SGUI_RADIO_BUTTON_SELECT_EVENT;
        sgui_event_post( &ev );
    }
    else if( e->type==SGUI_KEY_RELEASED_EVENT &&
             this->type==SGUI_RADIO_BUTTON_SELECTED )
    {
        if( (e->arg.i==SGUI_KC_UP || e->arg.i==SGUI_KC_LEFT) && this->prev )
        {
            sgui_canvas_set_focus( super->canvas, (sgui_widget*)this->prev );
            radio_button_select( this->prev );
            ev.widget = (sgui_widget*)this->prev;
            ev.type = SGUI_RADIO_BUTTON_SELECT_EVENT;
            sgui_event_post( &ev );
        }
        else if( (e->arg.i==SGUI_KC_DOWN || e->arg.i==SGUI_KC_RIGHT) &&
                 this->next )
        {
            sgui_canvas_set_focus( super->canvas, (sgui_widget*)this->next );
            radio_button_select( this->next );
            ev.widget = (sgui_widget*)this->next;
            ev.type = SGUI_RADIO_BUTTON_SELECT_EVENT;
            sgui_event_post( &ev );
        }
    }

    sgui_internal_unlock_mutex( );
}

sgui_widget* sgui_radio_button_create( int x, int y, const char* text )
{
    sgui_button* this = button_create_common(x,y,0,0,text,SGUI_RADIO_BUTTON);

    if( this )
        this->super.window_event_callback = radio_on_event;

    return (sgui_widget*)this;
}

void sgui_radio_button_connect( sgui_widget* super, sgui_widget* previous,
                                sgui_widget* next )
{
    sgui_button* this = (sgui_button*)super;

    if( this && (this->type==SGUI_RADIO_BUTTON ||
                 this->type==SGUI_RADIO_BUTTON_SELECTED) )
    {
        sgui_internal_lock_mutex( );

        /* disconnect from existing linked list */
        if( this->prev ) this->prev->next = this->next;
        if( this->next ) this->next->prev = this->prev;

        /* store next and previous pointers */
        this->prev = (sgui_button*)previous;
        this->next = (sgui_button*)next;

        /* connect previous and next to the given pointer */
        if( this->prev ) this->prev->next = this;
        if( this->next ) this->next->prev = this;

        sgui_internal_unlock_mutex( );
    }
}

/****************************** checkbox code ******************************/
static void checkbox_on_event( sgui_widget* super, const sgui_event* e )
{
    sgui_button* this = (sgui_button*)super;
    sgui_event ev;
    sgui_rect r;

    if( (e->type == SGUI_MOUSE_RELEASE_EVENT) ||
        (e->type==SGUI_KEY_RELEASED_EVENT &&
         (e->arg.i==SGUI_KC_RETURN || e->arg.i==SGUI_KC_SPACE)) )
    {
        ev.widget = super;

        /* invert state, set event type */
        if( this->type==SGUI_CHECKBOX )
        {
            this->type = SGUI_CHECKBOX_SELECTED;
            ev.type = SGUI_CHECKBOX_CHECK_EVENT;
        }
        else
        {
            this->type = SGUI_CHECKBOX;
            ev.type = SGUI_CHECKBOX_UNCHECK_EVENT;
        }

        /* flag dirty */
        sgui_widget_get_absolute_rect( super, &r );
        sgui_canvas_add_dirty_rect( super->canvas, &r );

        /* fire event */
        sgui_event_post( &ev );
    }
}

sgui_widget* sgui_checkbox_create( int x, int y, const char* text )
{
    sgui_button* this = button_create_common(x, y, 0, 0, text, SGUI_CHECKBOX);

    if( this )
        this->super.window_event_callback = checkbox_on_event;

    return (sgui_widget*)this;
}

/******************************* button code *******************************/
static void button_on_event( sgui_widget* super, const sgui_event* e )
{
    sgui_button* this = (sgui_button*)super;
    sgui_event ev;
    sgui_rect r;

    /* the mouse left a pressed button */
    if( e->type == SGUI_MOUSE_LEAVE_EVENT && this->type != SGUI_BUTTON )
    {
        sgui_widget_get_absolute_rect( super, &r );
        sgui_canvas_add_dirty_rect( super->canvas, &r );
        this->type = SGUI_BUTTON;
    }
    else if( (e->type == SGUI_MOUSE_PRESS_EVENT) ||
             (e->type==SGUI_KEY_PRESSED_EVENT &&
              (e->arg.i==SGUI_KC_RETURN || e->arg.i==SGUI_KC_SPACE)) )
    {
        /* the button got pressed */
        this->type = SGUI_BUTTON_SELECTED;
        sgui_widget_get_absolute_rect( super, &r );
        sgui_canvas_add_dirty_rect( super->canvas, &r );
    }
    else if( ((e->type == SGUI_MOUSE_RELEASE_EVENT) ||
              (e->type==SGUI_KEY_RELEASED_EVENT &&
              (e->arg.i==SGUI_KC_RETURN || e->arg.i==SGUI_KC_SPACE))) &&
              this->type != SGUI_BUTTON )
    {
        /* a pressed button got released */
        ev.widget = super;
        ev.type = SGUI_BUTTON_CLICK_EVENT;
        sgui_event_post( &ev );

        this->type = SGUI_BUTTON;
        sgui_widget_get_absolute_rect( super, &r );
        sgui_canvas_add_dirty_rect( super->canvas, &r );
    }
}

sgui_widget* sgui_button_create( int x, int y,
                                 unsigned int width, unsigned int height,
                                 const char* text )
{
    sgui_button* this=button_create_common(x,y,width,height,text,SGUI_BUTTON);

    if( this )
        this->super.window_event_callback = button_on_event;

    return (sgui_widget*)this;
}

/******************************* common code *******************************/
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

    /* determine text position */
    if( this->type==SGUI_BUTTON || this->type==SGUI_BUTTON_SELECTED )
    {
        this->cx = (super->area.left + super->area.right - text_width)>>1;
        this->cy = (super->area.top + super->area.bottom - text_height)>>1;
    }
    else
    {
        super->area.right = super->area.left + this->cx + text_width;
    }

    /* adjust buffer size and copy */
    free( this->text );
    this->text = malloc( len + 1 );

    if( this->text )
        memcpy( this->text, text, len + 1 );

    sgui_internal_unlock_mutex( );
}

void sgui_button_set_state( sgui_widget* super, int state )
{
    sgui_button* this = (sgui_button*)super;
    sgui_rect r;

    if( !this || this->type==SGUI_BUTTON || this->type==SGUI_BUTTON_SELECTED )
        return;

    sgui_internal_lock_mutex( );

    if( this->type==SGUI_RADIO_BUTTON )
    {
        radio_button_select( this );
    }
    else if( this->type==SGUI_CHECKBOX && state )
    {
        this->type = SGUI_CHECKBOX_SELECTED;
        sgui_widget_get_absolute_rect( super, &r );
        sgui_canvas_add_dirty_rect( super->canvas, &r );
    }
    else if( this->type==SGUI_CHECKBOX_SELECTED && !state )
    {
        this->type = SGUI_CHECKBOX;
        sgui_widget_get_absolute_rect( super, &r );
        sgui_canvas_add_dirty_rect( super->canvas, &r );
    }

    sgui_internal_unlock_mutex( );
}

int sgui_button_get_state( sgui_widget* super )
{
    sgui_button* this = (sgui_button*)super;

    if( this )
    {
        return this->type==SGUI_RADIO_BUTTON_SELECTED ||
               this->type==SGUI_CHECKBOX_SELECTED ||
               this->type==SGUI_BUTTON_SELECTED;
    }

    return 0;
}

