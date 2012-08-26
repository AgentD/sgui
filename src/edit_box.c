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

    unsigned int cursor;
    unsigned int offset;

    int draw_cursor;

    unsigned char* buffer;
}
sgui_edit_box;




void sgui_edit_box_determine_offset( sgui_edit_box* b )
{
    unsigned int cx;

    if( !b->num_entered )
        return;

    if( b->offset && (b->cursor <= b->offset) )
    {
        --(b->offset);

        while( ((b->buffer[ b->offset ]) & 0xC0) == 0x80 )
            --(b->offset);
    }
    else
    {
        cx = sgui_skin_default_font_extents( b->buffer + b->offset,
                                             b->cursor - b->offset, 0, 0 );

        if( cx > b->widget.width )
        {
            ++(b->offset);

            while( ((b->buffer[ b->offset ]) & 0xC0) == 0x80 )
                ++(b->offset);
        }
    }
}






void sgui_edit_box_on_event( sgui_widget* widget, sgui_window* wnd,
                             int type, sgui_event* event )
{
    sgui_edit_box* b = (sgui_edit_box*)widget;

    if( type == SGUI_FOCUS_EVENT )
    {
        b->draw_cursor = 1;
        widget->need_redraw = 1;
    }
    else if( type == SGUI_FOCUS_LOSE_EVENT )
    {
        b->draw_cursor = 0;
        widget->need_redraw = 1;
    }
    else if( type == SGUI_DRAW_EVENT )
    {
        sgui_skin_draw_edit_box( wnd, widget->x, widget->y,
                                 b->buffer + b->offset, widget->width,
                                 b->draw_cursor ?
                                 (int)(b->cursor-b->offset) : -1 );
    }
    else if( type == SGUI_KEY_PRESSED_EVENT )
    {
        if( (event->keyboard_event.code==SGUI_KC_BACK) && b->num_entered &&
            b->cursor )
        {
            unsigned int old = b->cursor;

            /* roll back an entire UTF8 character */
            --(b->cursor);

            while( ((b->buffer[ b->cursor ]) & 0xC0) == 0x80 )
                --(b->cursor);

            /* move entire text block back by one character */
            memmove( b->buffer+b->cursor, b->buffer+old, b->end-old+1 );

            /* update state */
            b->num_entered -= 1;
            b->end -= (old - b->cursor);

            widget->need_redraw = 1;
            sgui_edit_box_determine_offset( b );
        }
        else if( (event->keyboard_event.code==SGUI_KC_DELETE) &&
                 (b->cursor < b->end) && b->num_entered )
        {
            unsigned int offset = b->cursor;

            /* skip an entire UTF8 character */
            ++(offset);

            while( ((b->buffer[ offset ]) & 0xC0) == 0x80 )
                ++(offset);

            /* move entire text block back by one character */
            memmove( b->buffer+b->cursor, b->buffer+offset, b->end-offset+1 );

            /* update state */
            b->num_entered -= 1;
            b->end -= (offset - b->cursor);

            widget->need_redraw = 1;
            sgui_edit_box_determine_offset( b );
        }
        else if( (event->keyboard_event.code==SGUI_KC_LEFT) && b->cursor )
        {
            /* move cursor left by one UTF8 character */
            --(b->cursor);

            while( ((b->buffer[ b->cursor ]) & 0xC0) == 0x80 )
                --(b->cursor);

            widget->need_redraw = 1;
            sgui_edit_box_determine_offset( b );
        }
        else if( (event->keyboard_event.code==SGUI_KC_RIGHT) &&
                 (b->cursor < b->end) )
        {
            /* move cursor right by one UTF8 character */
            ++(b->cursor);

            while( ((b->buffer[ b->cursor ]) & 0xC0) == 0x80 )
                ++(b->cursor);

            widget->need_redraw = 1;
            sgui_edit_box_determine_offset( b );
        }
    }
    else if( (type == SGUI_CHAR_EVENT) && (b->num_entered < b->max_chars) )
    {   
        unsigned int len;

        len = strlen( (const char*)event->char_event.as_utf8_str );

        /* move entire text block right by the character length */
        memmove( b->buffer + b->cursor + len, b->buffer + b->cursor,
                 b->end - b->cursor + 1 );

        /* insert the character */
        memcpy( b->buffer + b->cursor, event->char_event.as_utf8_str, len );

        /* update state */
        b->end += len;
        b->cursor += len;
        b->num_entered += 1;
        widget->need_redraw = 1;
        sgui_edit_box_determine_offset( b );
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
    b->cursor                       = 0;
    b->offset                       = 0;
    b->draw_cursor                  = 0;
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

