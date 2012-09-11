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



void sgui_button_draw( sgui_widget* w, sgui_canvas* cv );
sgui_button* sgui_button_create_common( const char* text, int type );



/**************************** radio button code ****************************/
void sgui_radio_on_event( sgui_widget* widget, int type, sgui_event* event )
{
    sgui_button* b = (sgui_button*)widget;
    sgui_button* i;
    (void)event;

    if( type == SGUI_MOUSE_RELEASE_EVENT && !b->state )
    {
        b->widget.need_redraw = 1;
        b->state              = 1;

        sgui_internal_widget_fire_event( widget,
                                         SGUI_RADIO_BUTTON_SELECT_EVENT );

        /* uncheck all preceeding and following radio buttons */
        for( i=b->prev; i!=NULL; i=i->prev )
        {
            i->widget.need_redraw = i->state;
            i->state = 0;
        }

        for( i=b->next; i!=NULL; i=i->next )
        {
            i->widget.need_redraw = i->state;
            i->state = 0;
        }
    }
}

sgui_widget* sgui_radio_button_create( int x, int y, const char* text )
{
    unsigned int width, height, h = sgui_skin_get_default_font_height( );
    sgui_button* b = sgui_button_create_common( text, BUTTON_RADIO );

    if( !b )
        return NULL;

    b->prev = NULL;
    b->next = NULL;

    sgui_skin_get_radio_button_extents( &b->cx, &b->cy );
    width  = b->cx + b->text_width;
    height = b->cy < (h+h/2) ? (h+h/2) : b->cy;

    sgui_internal_widget_init( (sgui_widget*)b, x, y, width, height, 1 );

    b->widget.window_event_callback = sgui_radio_on_event;
    b->widget.draw_callback         = sgui_button_draw;

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
    unsigned int width, height, h = sgui_skin_get_default_font_height( );
    sgui_button* b = sgui_button_create_common( text, BUTTON_CHECKBOX );

    if( !b )
        return NULL;

    sgui_skin_get_checkbox_extents( &b->cx, &b->cy );
    width  = b->cx + b->text_width;
    height = b->cy < (h+h/2) ? (h+h/2) : b->cy;

    sgui_internal_widget_init( (sgui_widget*)b, x, y, width, height, 1 );

    b->widget.window_event_callback = sgui_checkbox_on_event;
    b->widget.draw_callback         = sgui_button_draw;

    return (sgui_widget*)b;
}

/******************************* button code *******************************/
void sgui_button_on_event( sgui_widget* widget, int type, sgui_event* event )
{
    sgui_button* b = (sgui_button*)widget;
    (void)event;

    if( type == SGUI_MOUSE_LEAVE_EVENT )
    {
        b->widget.need_redraw = (b->state!=0);
        b->state = 0;
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
    unsigned int h = sgui_skin_get_default_font_height( );
    sgui_button* b = sgui_button_create_common( text, BUTTON_NORMAL );

    if( !b )
        return NULL;

    b->cx  = x + width /2 - b->text_width/2;
    b->cy  = y + height/2 -             h/2 - h/8;

    width  = width  < b->text_width ? b->text_width : width;
    height = height < (h + h/2)     ? (h + h/2)     : height;

    sgui_internal_widget_init( (sgui_widget*)b, x, y, width, height, 1 );

    b->widget.window_event_callback = sgui_button_on_event;
    b->widget.draw_callback         = sgui_button_draw;

    return (sgui_widget*)b;
}

/******************************* common code *******************************/
void sgui_button_draw( sgui_widget* w, sgui_canvas* cv )
{
    unsigned char color[3];
    sgui_button* b = (sgui_button*)w;
    sgui_font* f   = sgui_skin_get_default_font( 0, 0 );
    unsigned int h = sgui_skin_get_default_font_height( );
    int oy         = h > b->cy ? (h/2-b->cy/4) : 0;

    sgui_skin_get_default_font_color( color );

    if( b->type == BUTTON_RADIO )
        sgui_skin_draw_radio_button( cv, w->x, w->y + oy, b->state );
    else if( b->type == BUTTON_CHECKBOX )
        sgui_skin_draw_checkbox( cv, w->x, w->y + oy, b->state );
    else
        sgui_skin_draw_button( cv, w->x, w->y,
                               w->width, w->height, b->state );

    if( b->type == BUTTON_NORMAL )
    {
        sgui_canvas_clear( cv, b->cx - b->state-1, b->cy - b->state-1,
                               b->text_width+2, h+2 );
        sgui_canvas_draw_text_plain( cv, b->cx - b->state, b->cy - b->state,
                                     f, h, color, SCF_RGB8,
                                     b->text, (unsigned int)-1 );
    }
    else
    {
        sgui_canvas_clear( cv, w->x + b->cx, w->y, b->text_width, w->height );
        sgui_canvas_draw_text_plain( cv, w->x + b->cx, w->y, f, h, color,
                                     SCF_RGB8, b->text, (unsigned int)-1 );
    }
}

sgui_button* sgui_button_create_common( const char* text, int type )
{
    unsigned int len;
    sgui_button* b = malloc( sizeof(sgui_button) );

    if( !b )
        return NULL;

    len = strlen( text );

    b->text = malloc( len + 1 );

    if( !b->text )
    {
        free( b );
        return NULL;
    }

    memcpy( b->text, text, len + 1 );

    b->state      = 0;
    b->type       = type;
    b->text_width = sgui_skin_default_font_extents( text, len, 0, 0 );

    return b;
}

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

    if( !button || !text )
        return;

    len = strlen( text );

    b->text_width = sgui_skin_default_font_extents( text, len, 0, 0 );
    h = sgui_skin_get_default_font_height( );

    if( b->type == BUTTON_NORMAL )
    {
        button->width  = button->width<b->text_width ?
                         b->text_width : button->width;
        button->height = button->height<(h + h/2) ? (h + h/2) :
                         button->height;

        b->cx = button->x + button->width /2 - b->text_width/2;
        b->cy = button->y + button->height/2 -             h/2 - h/8;
    }
    else
    {
        if( b->type == BUTTON_CHECKBOX )
            sgui_skin_get_checkbox_extents( &b->cx, &b->cy );
        else
            sgui_skin_get_radio_button_extents( &b->cx, &b->cy );

        button->width  = b->cx + b->text_width;
        button->height = b->cy < (h+h/2) ? (h+h/2) : b->cy;
    }

    free( b->text );
    b->text = malloc( len + 1 );
    memcpy( b->text, text, len + 1 );
}

void sgui_button_set_state( sgui_widget* button, int state )
{
    sgui_button* b = (sgui_button*)button;
    sgui_button* i;

    if( !b || b->type==BUTTON_NORMAL || (b->type==BUTTON_RADIO && !state) )
        return;

    b->state = state;

    /* uncheck all preceeding and following radio buttons */
    if( b->type==BUTTON_RADIO )
    {
        for( i=b->prev; i!=NULL; i=i->prev )
        {
            i->widget.need_redraw = i->state;
            i->state = 0;
        }

        for( i=b->next; i!=NULL; i=i->next )
        {
            i->widget.need_redraw = i->state;
            i->state = 0;
        }
    }
}

int sgui_button_get_state( sgui_widget* button )
{
    return button ? ((sgui_button*)button)->state : 0;
}

