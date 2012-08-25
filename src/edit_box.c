/*
 * edit_box.c
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
#include "sgui_edit_box.h"
#include "sgui_skin.h"

#include "widget_internal.h"

#include <stdlib.h>
#include <string.h>



typedef struct
{
    sgui_widget widget;

    unsigned int max_chars, num_entered;
    unsigned int end;

    unsigned char* buffer;
}
sgui_edit_box;



void sgui_edit_box_on_event( sgui_widget* widget, sgui_window* wnd,
                             int type, sgui_event* event )
{
    sgui_edit_box* b = (sgui_edit_box*)widget;
    unsigned int i;

    if( type == SGUI_DRAW_EVENT )
    {
        sgui_skin_draw_edit_box( wnd, widget->x, widget->y, b->buffer,
                                 widget->width, b->end );
    }
    else if( type == SGUI_KEY_PRESSED_EVENT )
    {
        if( (event->keyboard_event.code==SGUI_KC_BACK) && b->num_entered )
        {
            /* roll back an entire UTF8 character */
            --(b->end);

            while( ((b->buffer[ b->end ]) & 0xC0) == 0x80 )
                --(b->end);

            /* overwrite with null terminator */
            b->buffer[ b->end ] = '\0';

            /* update state */
            b->num_entered -= 1;
            widget->need_redraw = 1;
        }
    }
    else if( type == SGUI_CHAR_EVENT )
    {
        /* check if we still have space for more characters */
        if( b->num_entered == b->max_chars )
            return;

        /* append entire UTF8 string */
        for( i=0; event->char_event.as_utf8_str[i]!='\0'; ++i )
            b->buffer[ b->end++ ] = event->char_event.as_utf8_str[i];

        /* add null terminator */
        b->buffer[ b->end ] = '\0';

        /* update state */
        b->num_entered += 1;
        widget->need_redraw = 1;
    }
}



sgui_widget* sgui_edit_box_create( int x, int y, unsigned int width,
                                   unsigned int max_chars )
{
    sgui_edit_box* b;

    b = malloc( sizeof(sgui_edit_box) );

    sgui_internal_widget_init( (sgui_widget*)b, x, y, width,
                               sgui_skin_get_edit_box_height( ), 0 );

    b->widget.window_event_callback = sgui_edit_box_on_event;
    b->max_chars                    = max_chars;
    b->num_entered                  = 0;
    b->end                          = 0;
    b->buffer                       = malloc( max_chars * 6 + 1 );
    b->buffer[0]                    = '\0';

    return (sgui_widget*)b;
}

void sgui_edit_box_destroy( sgui_widget* box )
{
    if( box )
    {
        sgui_internal_widget_deinit( box );

        free( ((sgui_edit_box*)box)->buffer );
        free( box );
    }
}

