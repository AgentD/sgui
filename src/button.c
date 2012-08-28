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
#include "sgui_button.h"
#include "sgui_skin.h"

#include "widget_internal.h"

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



/**************************** radio button code ****************************/
void sgui_radio_draw( sgui_widget* w, sgui_canvas* cv )
{
    unsigned char color[3];
    sgui_button* b = (sgui_button*)w;
    sgui_font* f   = sgui_skin_get_default_font( 0, 0 );
    unsigned int h = sgui_skin_get_default_font_height( );
    int oy         = h > b->cy ? (h/2 - b->cy/2) : 0;

    sgui_skin_get_default_font_color( color );
    sgui_skin_draw_radio_button( cv, w->x, w->y + oy, b->state );
    sgui_canvas_draw_text_plain( cv, w->x + b->cx, w->y, f, h, color,
                                 SCF_RGB8, b->text, (unsigned int)-1 );
}

void sgui_radio_on_event( sgui_widget* widget, int type, sgui_event* event )
{
    sgui_button* b = (sgui_button*)widget;
    sgui_button* i;
    (void)event;

    if( type == SGUI_MOUSE_RELEASE_EVENT )
    {
        b->state = 1;
        b->widget.need_redraw = 1;

        sgui_internal_widget_fire_event( widget,
                                         SGUI_RADIO_BUTTON_SELECT_EVENT );

        /* uncheck all preceeding and following radio buttons */
        for( i=b->prev; i!=NULL; i=i->prev )
        {
            i->state = 0;
            i->widget.need_redraw = 1;
        }

        for( i=b->next; i!=NULL; i=i->next )
        {
            i->state = 0;
            i->widget.need_redraw = 1;
        }
    }
}

sgui_widget* sgui_radio_button_create( int x, int y, const char* text )
{
    unsigned int len = strlen( text );
    unsigned int width, height, h = sgui_skin_get_default_font_height( );
    sgui_button* b = malloc( sizeof(sgui_button) );

    b->text       = malloc( len + 1 );
    b->state      = 0;
    b->type       = BUTTON_RADIO;
    b->text_width = sgui_skin_default_font_extents( text, len, 0, 0 );
    b->prev       = NULL;
    b->next       = NULL;

    memcpy( b->text, text, len + 1 );

    sgui_skin_get_radio_button_extents( &b->cx, &b->cy );
    width  = b->cx + b->text_width;
    height = b->cy < (h+h/2) ? (h+h/2) : b->cy;

    sgui_internal_widget_init( (sgui_widget*)b, x, y, width, height, 1 );

    b->widget.window_event_callback = sgui_radio_on_event;
    b->widget.draw_callback         = sgui_radio_draw;

    return (sgui_widget*)b;
}

void sgui_radio_button_connect( sgui_widget* radio, sgui_widget* previous,
                                sgui_widget* next )
{
    sgui_button* b = (sgui_button*)radio;

    if( b->type == BUTTON_RADIO )
    {
        if( b->prev ) b->prev->next = b->next;
        if( b->next ) b->next->prev = b->prev;

        b->prev = (sgui_button*)previous;
        b->next = (sgui_button*)next;

        if( b->prev ) b->prev->next = b;
        if( b->next ) b->next->prev = b;
    }
}

/****************************** checkbox code ******************************/
void sgui_checkbox_draw( sgui_widget* w, sgui_canvas* cv )
{
    unsigned char color[3];
    sgui_button* b = (sgui_button*)w;
    sgui_font* f   = sgui_skin_get_default_font( 0, 0 );
    unsigned int h = sgui_skin_get_default_font_height( );
    int oy         = h > b->cy ? (h/2 - b->cy/2) : 0;

    sgui_skin_get_default_font_color( color );
    sgui_skin_draw_checkbox( cv, w->x, w->y + oy, b->state );
    sgui_canvas_draw_text_plain( cv, w->x + b->cx, w->y, f, h, color,
                                 SCF_RGB8, b->text, (unsigned int)-1 );
}

void sgui_checkbox_on_event(sgui_widget* widget, int type, sgui_event* event)
{
    sgui_button* b = (sgui_button*)widget;
    (void)event;

    if( type == SGUI_MOUSE_RELEASE_EVENT )
    {
        b->state = !b->state;
        b->widget.need_redraw = 1;

        sgui_internal_widget_fire_event( widget, b->state ?
                                         SGUI_CHECKBOX_CHECK_EVENT :
                                         SGUI_CHECKBOX_UNCHECK_EVENT );
    }
}

sgui_widget* sgui_checkbox_create( int x, int y, const char* text )
{
    unsigned int len = strlen( text );
    unsigned int width, height, h = sgui_skin_get_default_font_height( );
    sgui_button* b = malloc( sizeof(sgui_button) );

    b->text       = malloc( len + 1 );
    b->state      = 0;
    b->type       = BUTTON_CHECKBOX;
    b->text_width = sgui_skin_default_font_extents( text, len, 0, 0 );

    memcpy( b->text, text, len + 1 );

    sgui_skin_get_checkbox_extents( &b->cx, &b->cy );
    width  = b->cx + b->text_width;
    height = b->cy < (h+h/2) ? (h+h/2) : b->cy;

    sgui_internal_widget_init( (sgui_widget*)b, x, y, width, height, 1 );

    b->widget.window_event_callback = sgui_checkbox_on_event;
    b->widget.draw_callback         = sgui_checkbox_draw;

    return (sgui_widget*)b;
}

/******************************* button code *******************************/
void sgui_button_draw( sgui_widget* w, sgui_canvas* cv )
{
    unsigned char color[3];
    sgui_button* b = (sgui_button*)w;
    sgui_font*   f = sgui_skin_get_default_font( 0, 0 );
    unsigned int h = sgui_skin_get_default_font_height( );

    sgui_skin_get_default_font_color( color );
    sgui_skin_draw_button( cv, w->x, w->y, w->width, w->height, b->state );
    sgui_canvas_draw_text_plain( cv, b->cx - b->state, b->cy - b->state, f, h,
                                 color, SCF_RGB8, b->text, (unsigned int)-1 );
}

void sgui_button_on_event( sgui_widget* widget, int type, sgui_event* event )
{
    sgui_button* b = (sgui_button*)widget;
    (void)event;

    if( type == SGUI_MOUSE_LEAVE_EVENT )
    {
        b->state = 0;
        b->widget.need_redraw = 1;
    }
    else if( type == SGUI_MOUSE_PRESS_EVENT )
    {
        b->state = 1;
        b->widget.need_redraw = 1;
    }
    else if( type == SGUI_MOUSE_RELEASE_EVENT )
    {
        if( b->state )
        {
            sgui_internal_widget_fire_event( widget,
                                             SGUI_BUTTON_CLICK_EVENT );
        }

        b->state = 0;
        b->widget.need_redraw = 1;
    }
}

sgui_widget* sgui_button_create( int x, int y,
                                 unsigned int width, unsigned int height,
                                 const char* text )
{
    unsigned int len = strlen( text );
    unsigned int h = sgui_skin_get_default_font_height( );
    sgui_button* b = malloc( sizeof(sgui_button) );

    b->text       = malloc( len + 1 );
    b->state      = 0;
    b->type       = BUTTON_NORMAL;
    b->text_width = sgui_skin_default_font_extents( text, len, 0, 0 );
    b->cx         = x + width /2 - b->text_width/2;
    b->cy         = y + height/2 -             h/2 - h/8;

    memcpy( b->text, text, len + 1 );

    width  = width  < b->text_width ? b->text_width : width;
    height = height < (h + h/2)     ? (h + h/2)     : height;

    sgui_internal_widget_init( (sgui_widget*)b, x, y, width, height, 1 );

    b->widget.window_event_callback = sgui_button_on_event;
    b->widget.draw_callback         = sgui_button_draw;

    return (sgui_widget*)b;
}

/******************************* common code *******************************/
void sgui_button_destroy( sgui_widget* button )
{
    sgui_button* b = (sgui_button*)button;

    if( button )
    {
        if( b->type == BUTTON_RADIO )
        {
            if( b->prev ) b->prev->next = b->next;
            if( b->next ) b->next->prev = b->prev;
        }

        sgui_internal_widget_deinit( button );

        free( b->text );
        free( button );
    }
}

void sgui_button_set_text( sgui_widget* button, const char* text )
{
    sgui_button* b = (sgui_button*)button;
    unsigned int len, h;

    if( button )
    {
        len = strlen( text );

        b->text_width = sgui_skin_default_font_extents( text, len, 0, 0 );
        h = sgui_skin_get_default_font_height( );

        if( b->type == BUTTON_CHECKBOX )
        {
            sgui_skin_get_checkbox_extents( &b->cx, &b->cy );
            button->width  = b->cx + b->text_width;
            button->height = b->cy < (h+h/2) ? (h+h/2) : b->cy;
        }
        else if( b->type == BUTTON_RADIO )
        {
            sgui_skin_get_radio_button_extents( &b->cx, &b->cy );
            button->width  = b->cx + b->text_width;
            button->height = b->cy < (h+h/2) ? (h+h/2) : b->cy;
        }
        else
        {
            button->width  = button->width<b->text_width ?
                             b->text_width : button->width;
            button->height = button->height<(h + h/2) ? (h + h/2) :
                             button->height;

            b->cx = button->x + button->width /2 - b->text_width/2;
            b->cy = button->y + button->height/2 -             h/2 - h/8;
        }

        free( b->text );
        b->text = malloc( len + 1 );
        memcpy( b->text, text, len + 1 );
    }
}

void sgui_button_set_state( sgui_widget* button, int state )
{
    sgui_button* b = (sgui_button*)button;
    sgui_button* i;

    if( b && (b->type==BUTTON_CHECKBOX || b->type==BUTTON_RADIO) )
    {
        b->state = state;

        /* uncheck all preceeding and following radio buttons */
        if( state && b->type==BUTTON_RADIO )
        {
            for( i=b->prev; i!=NULL; i=i->prev )
            {
                i->state = 0;
                i->widget.need_redraw = 1;
            }

            for( i=b->next; i!=NULL; i=i->next )
            {
                i->state = 0;
                i->widget.need_redraw = 1;
            }
        }
    }
}

int sgui_button_get_state( sgui_widget* button )
{
    sgui_button* b = (sgui_button*)button;

    return b ? b->state : 0;
}

