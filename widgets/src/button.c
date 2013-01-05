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

#include <stdlib.h>
#include <string.h>



#define BUTTON_NORMAL   0
#define BUTTON_CHECKBOX 1
#define BUTTON_RADIO    2



typedef struct s_sgui_button
{
    sgui_widget widget;

    char* text;
    unsigned int text_width;
    int state;                  /* pressed/checked/selected */
    int type;                   /* BUTTON_* type id */

    unsigned int cx, cy;        /* button: text position,
                                   checkbox/radio button: object size */

    struct s_sgui_button* prev; /* radio button menu: next in menu */
    struct s_sgui_button* next; /* radio button menu: previous in menu */
}
sgui_button;



void sgui_button_draw( sgui_widget* w, sgui_canvas* cv );
sgui_button* sgui_button_create_common( const char* text, int type );



/**************************** radio button code ****************************/
void sgui_radio_button_select( sgui_button* b )
{
    sgui_button* i;

    b->state = 1;
    sgui_widget_manager_add_dirty_rect( b->widget.mgr,
                                        &b->widget.area );

    /* deselect all preceeding radio buttons */
    for( i=b->prev; i!=NULL; i=i->prev )
    {
        if( i->state )
            sgui_widget_manager_add_dirty_rect( i->widget.mgr,
                                                &i->widget.area );
        i->state = 0;
    }

    /* deselect all following radio buttons */
    for( i=b->next; i!=NULL; i=i->next )
    {
        if( i->state )
            sgui_widget_manager_add_dirty_rect( i->widget.mgr,
                                                &i->widget.area );
        i->state = 0;
    }
}

void sgui_radio_on_event( sgui_widget* widget, int type, sgui_event* event )
{
    sgui_button* b = (sgui_button*)widget;
    (void)event;

    /* the radio button got clicked and is not selected */
    if( type == SGUI_MOUSE_RELEASE_EVENT && !b->state )
    {
        /* select the current radio button */
        sgui_radio_button_select( b );

        /* fire an appropriate event */
        sgui_widget_manager_fire_widget_event(widget->mgr, widget,
                                              SGUI_RADIO_BUTTON_SELECT_EVENT);
    }
}

sgui_widget* sgui_radio_button_create( int x, int y, const char* text )
{
    unsigned int width, height, h = sgui_skin_get_default_font_height( );
    sgui_button* b = sgui_button_create_common( text, BUTTON_RADIO );

    if( b )
    {
        b->prev = NULL;
        b->next = NULL;

        /* compute size */
        sgui_skin_get_radio_button_extents( &b->cx, &b->cy );
        width  = b->cx + b->text_width;
        height = b->cy < (h+h/2) ? (h+h/2) : b->cy;

        /* complete init and store state */
        sgui_internal_widget_init( (sgui_widget*)b, x, y, width, height );

        b->widget.window_event_callback = sgui_radio_on_event;
        b->widget.draw_callback         = sgui_button_draw;
    }

    return (sgui_widget*)b;
}

void sgui_radio_button_connect( sgui_widget* radio, sgui_widget* previous,
                                sgui_widget* next )
{
    sgui_button* b = (sgui_button*)radio;

    if( b->type == BUTTON_RADIO )
    {
        /* disconnect from existing linked list */
        if( b->prev ) b->prev->next = b->next;
        if( b->next ) b->next->prev = b->prev;

        /* store next and previous pointers */
        b->prev = (sgui_button*)previous;
        b->next = (sgui_button*)next;

        /* connect previous and next to the given pointer */
        if( b->prev ) b->prev->next = b;
        if( b->next ) b->next->prev = b;
    }
}

/****************************** checkbox code ******************************/
void sgui_checkbox_on_event(sgui_widget* widget, int type, sgui_event* event)
{
    sgui_button* b = (sgui_button*)widget;
    int e;
    (void)event;

    if( type == SGUI_MOUSE_RELEASE_EVENT )  /* the check box got clicked */
    {
        b->state = !b->state;   /* invert state */

        /* flag dirty */
        sgui_widget_manager_add_dirty_rect( widget->mgr, &widget->area );

        /* fire appropriate event */
        e = b->state ? SGUI_CHECKBOX_CHECK_EVENT : 
                       SGUI_CHECKBOX_UNCHECK_EVENT;

        sgui_widget_manager_fire_widget_event( widget->mgr, widget, e );
    }
}

sgui_widget* sgui_checkbox_create( int x, int y, const char* text )
{
    unsigned int width, height, h = sgui_skin_get_default_font_height( );
    sgui_button* b = sgui_button_create_common( text, BUTTON_CHECKBOX );

    if( b )
    {
        /* compute widget size */
        sgui_skin_get_checkbox_extents( &b->cx, &b->cy );
        width  = b->cx + b->text_width;
        height = b->cy < (h+h/2) ? (h+h/2) : b->cy;

        /* complete init */
        sgui_internal_widget_init( (sgui_widget*)b, x, y, width, height );

        b->widget.window_event_callback = sgui_checkbox_on_event;
        b->widget.draw_callback         = sgui_button_draw;
    }

    return (sgui_widget*)b;
}

/******************************* button code *******************************/
void sgui_button_on_event( sgui_widget* widget, int type, sgui_event* event )
{
    sgui_button* b = (sgui_button*)widget;
    (void)event;

    /* the mouse left a pressed button */
    if( type == SGUI_MOUSE_LEAVE_EVENT && b->state!=0 )
    {
        sgui_widget_manager_add_dirty_rect( widget->mgr, &widget->area );
        b->state = 0;
    }
    else if( type == SGUI_MOUSE_PRESS_EVENT )   /* a button got pressed */
    {
        b->state = 1;
        sgui_widget_manager_add_dirty_rect( widget->mgr, &widget->area );
    }
    else if( type == SGUI_MOUSE_RELEASE_EVENT && b->state )
    {
        /* a pressed button got released */
        sgui_widget_manager_fire_widget_event( widget->mgr, widget,
                                               SGUI_BUTTON_CLICK_EVENT );

        b->state = 0;
        sgui_widget_manager_add_dirty_rect( widget->mgr, &widget->area );
    }
}

sgui_widget* sgui_button_create( int x, int y,
                                 unsigned int width, unsigned int height,
                                 const char* text )
{
    unsigned int h = sgui_skin_get_default_font_height( );
    sgui_button* b = sgui_button_create_common( text, BUTTON_NORMAL );

    if( b )
    {
        /* adjust button size to text size */
        width  = width  < b->text_width ? b->text_width : width;
        height = height < (h + h/2)     ? (h + h/2)     : height;

        /* get the text position */
        b->cx  = x + width /2 - b->text_width/2;
        b->cy  = y + height/2 -             h/2 - h/8;

        /* complete init */
        sgui_internal_widget_init( (sgui_widget*)b, x, y, width, height );

        b->widget.window_event_callback = sgui_button_on_event;
        b->widget.draw_callback         = sgui_button_draw;
    }

    return (sgui_widget*)b;
}

/******************************* common code *******************************/
void sgui_button_draw( sgui_widget* w, sgui_canvas* cv )
{
    unsigned char color[3];
    sgui_button* b = (sgui_button*)w;
    sgui_font* f = sgui_skin_get_default_font( 0, 0 );
    unsigned int h = sgui_skin_get_default_font_height( );
    int oy = h > b->cy ? (h/2-b->cy/4) : 0;
    int x = w->area.left, y = w->area.top;
    sgui_rect r;

    sgui_skin_get_default_font_color( color );

    /* draw widget */
    if( b->type == BUTTON_RADIO )
        sgui_skin_draw_radio_button( cv, x, y + oy, b->state );
    else if( b->type == BUTTON_CHECKBOX )
        sgui_skin_draw_checkbox( cv, x, y + oy, b->state );
    else
        sgui_skin_draw_button( cv, &w->area, b->state );

    /* draw text */
    if( b->type == BUTTON_NORMAL )
    {
        sgui_rect_set_size( &r, b->cx - b->state-1, b->cy - b->state-1,
                                b->text_width+2, h+2 );
        sgui_canvas_clear( cv, &r );
        sgui_font_draw_text_plain( cv, b->cx - b->state, b->cy - b->state,
                                   f, h, color, b->text, (unsigned int)-1 );
    }
    else
    {
        r.left = w->area.left + b->cx; r.right  = w->area.right;
        r.top  = w->area.top;          r.bottom = w->area.bottom;

        sgui_canvas_clear( cv, &r );
        sgui_font_draw_text_plain( cv, x+b->cx, y, f, h, color,
                                   b->text, (unsigned int)-1 );
    }
}

sgui_button* sgui_button_create_common( const char* text, int type )
{
    unsigned int len;
    sgui_button* b = malloc( sizeof(sgui_button) );

    if( b )
    {
        /* allocate space for the text */
        len = strlen( text );

        b->text = malloc( len + 1 );

        if( b->text )
        {
            /* copy the text and set internal state stub */
            memcpy( b->text, text, len + 1 );

            b->state      = 0;
            b->type       = type;
            b->text_width = sgui_skin_default_font_extents( text, len, 0, 0 );
        }
        else
        {
            free( b );
            b = NULL;
        }

        b->prev = NULL;
        b->next = NULL;
    }

    return b;
}

void sgui_button_destroy( sgui_widget* button )
{
    sgui_button* b = (sgui_button*)button;

    if( button )
    {
        /* detatch from linked list */
        if( b->prev ) b->prev->next = b->next;
        if( b->next ) b->next->prev = b->prev;

        /* free memory of text buffer and button */
        free( b->text );
        free( button );
    }
}

void sgui_button_set_text( sgui_widget* button, const char* text )
{
    sgui_button* b = (sgui_button*)button;
    unsigned int len, h;

    /* sanity check */
    if( !button || !text )
        return;

    len = strlen( text );

    b->text_width = sgui_skin_default_font_extents( text, len, 0, 0 );
    h = sgui_skin_get_default_font_height( );

    /* determine text position */
    if( b->type == BUTTON_NORMAL )
    {
        b->cx = button->area.left +
                SGUI_RECT_WIDTH(button->area)/2 - b->text_width/2;

        b->cy = button->area.right +
                SGUI_RECT_HEIGHT(button->area)/2 - h/2 - h/8;
    }
    else
    {
        if( b->type == BUTTON_CHECKBOX )
            sgui_skin_get_checkbox_extents( &b->cx, &b->cy );
        else
            sgui_skin_get_radio_button_extents( &b->cx, &b->cy );

        button->area.right = button->area.left + b->cx + b->text_width;
    }

    /* adjust buffer size and copy */
    free( b->text );
    b->text = malloc( len + 1 );
    memcpy( b->text, text, len + 1 );
}

void sgui_button_set_state( sgui_widget* button, int state )
{
    sgui_button* b = (sgui_button*)button;

    /* does not work on normal buttons */
    if( !b || b->type==BUTTON_NORMAL )
        return;

    /* if it's a radio button, we can only select, not deselect */
    if( b->type==BUTTON_RADIO && (!state || b->state) )
        return;

    /* perform appropriate action */
    if( b->type==BUTTON_RADIO )
        sgui_radio_button_select( b );
    else
        sgui_widget_manager_add_dirty_rect( button->mgr, &button->area );

    b->state = state;
}

int sgui_button_get_state( sgui_widget* button )
{
    return button ? ((sgui_button*)button)->state : 0;
}

