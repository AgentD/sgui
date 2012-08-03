/*
 * radio_menu.c
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
#include "sgui_radio_menu.h"
#include "sgui_skin.h"

#include "widget_internal.h"

#include <stdlib.h>
#include <string.h>



typedef struct
{
    sgui_widget widget;

    unsigned char** options;
    unsigned int num_options, selected;

    int mouse_y;
}
sgui_radio_menu;



void sgui_radio_menu_on_event( sgui_widget* widget, sgui_window* wnd,
                               int type, sgui_event* event )
{
    sgui_radio_menu* m = (sgui_radio_menu*)widget;
    int y;

    if( type == SGUI_DRAW_EVENT )
    {
        sgui_skin_draw_radio_menu( wnd, widget->x, widget->y,
                                   (const unsigned char**)m->options,
                                   m->num_options, m->selected,
                                   widget->width, widget->height );
    }
    else if( type == SGUI_MOUSE_MOVE_EVENT )
    {
        m->mouse_y = event->mouse_move.y;
    }
    else if( type == SGUI_MOUSE_PRESS_EVENT )
    {
        if( event->mouse_press.button == SGUI_MOUSE_BUTTON_LEFT &&
            !event->mouse_press.pressed )
        {
            y = m->mouse_y - widget->y;

            m->selected = sgui_skin_get_radio_menu_option_from_point( y );
            widget->need_redraw = 1;
        }
    }
}



sgui_widget* sgui_radio_menu_create( int x, int y, unsigned int num_options,
                                     const unsigned char** options,
                                     unsigned int initial_option )
{
    sgui_radio_menu* m;
    unsigned int w, h;
    unsigned int i, len;

    m = malloc( sizeof(sgui_radio_menu) );

    sgui_skin_get_radio_menu_extents( options, num_options, &w, &h );

    sgui_internal_widget_init( (sgui_widget*)m, x, y, w, h, 1 );

    m->widget.window_event_callback = sgui_radio_menu_on_event;

    m->options     = malloc( sizeof(unsigned char*) * num_options );
    m->num_options = num_options;
    m->selected    = initial_option;
    m->mouse_y     = 0;

    for( i=0; i<num_options; ++i )
    {
        len = strlen( (const char*)options[i] );

        m->options[ i ] = malloc( len + 1 );

        memcpy( m->options[ i ], options[ i ], len + 1 );
    }

    return (sgui_widget*)m;
}

void sgui_radio_menu_destroy( sgui_widget* menu )
{
    unsigned int i;
    sgui_radio_menu* m = (sgui_radio_menu*)menu;

    if( menu )
    {
        for( i=0; i<m->num_options; ++i )
            free( m->options[ i ] );

        free( m->options );

        sgui_internal_widget_deinit( menu );
        free( menu );
    }
}

unsigned int sgui_radio_menu_get_selection( sgui_widget* menu )
{
    sgui_radio_menu* m = (sgui_radio_menu*)menu;

    return m ? m->selected : 0;
}

void sgui_radio_menu_set_selection( sgui_widget* menu, unsigned int index )
{
    sgui_radio_menu* m = (sgui_radio_menu*)menu;

    if( m )
        m->selected = index;
}

