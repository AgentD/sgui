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



typedef struct
{
    sgui_widget widget;

    unsigned char* text;
    unsigned int text_width;
    int state;
    int type;

    unsigned int cx, cy;
}
sgui_button;



/**************************** checkbox callbacks ****************************/
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

void sgui_checkbox_on_event( sgui_widget* widget, int type, sgui_event* event )
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

/***************************** button callbacks *****************************/
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

/******************************* common code ********************************/
sgui_widget* sgui_button_create( int x, int y,
                                 unsigned int width, unsigned int height,
                                 const unsigned char* text, int type )
{
    sgui_button* b;
    unsigned int len = strlen( (const char*)text );
    unsigned int h = sgui_skin_get_default_font_height( );

    b = malloc( sizeof(sgui_button) );

    b->text       = malloc( len + 1 );
    b->state      = 0;
    b->type       = type;
    b->text_width = sgui_skin_default_font_extents( text, len, 0, 0 );

    memcpy( b->text, text, len + 1 );

    if( type == SGUI_BUTTON_CHECKBOX )
    {
        sgui_skin_get_checkbox_extents( &b->cx, &b->cy );
        width  = b->cx + b->text_width;
        height = b->cy < (h+h/2) ? (h+h/2) : b->cy;

        sgui_internal_widget_init( (sgui_widget*)b, x, y, width, height, 1 );

        b->widget.window_event_callback = sgui_checkbox_on_event;
        b->widget.draw_callback         = sgui_checkbox_draw;
    }
    else
    {
        width  = width<b->text_width ? b->text_width : width;
        height = height<(h + h/2) ? (h + h/2) : height;

        sgui_internal_widget_init( (sgui_widget*)b, x, y, width, height, 1 );

        b->widget.window_event_callback = sgui_button_on_event;
        b->widget.draw_callback         = sgui_button_draw;

        b->cx = x + width /2 - b->text_width/2;
        b->cy = y + height/2 -             h/2 - h/8;
    }

    return (sgui_widget*)b;
}

void sgui_button_destroy( sgui_widget* button )
{
    if( button )
    {
        sgui_internal_widget_deinit( button );

        free( ((sgui_button*)button)->text );
        free( button );
    }
}

void sgui_button_set_text( sgui_widget* button, const unsigned char* text )
{
    sgui_button* b = (sgui_button*)button;
    unsigned int len, h;

    if( button )
    {
        len = strlen( (const char*)text );

        b->text_width = sgui_skin_default_font_extents( text, len, 0, 0 );
        h = sgui_skin_get_default_font_height( );

        if( b->type == SGUI_BUTTON_CHECKBOX )
        {
            sgui_skin_get_checkbox_extents( &b->cx, &b->cy );
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

    if( b && (b->type == SGUI_BUTTON_CHECKBOX) )
        b->state = state;
}

int sgui_button_get_state( sgui_widget* button )
{
    sgui_button* b = (sgui_button*)button;

    return b ? b->state : 0;
}

