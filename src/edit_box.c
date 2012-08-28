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




/* Adjust text render offset after moving the cursor */
void sgui_edit_box_determine_offset( sgui_edit_box* b )
{
    unsigned int cx;

    if( !b->num_entered )
        return;

    cx = sgui_skin_default_font_extents( b->buffer + b->offset,
                                         b->cursor - b->offset, 0, 0 );

    if( b->offset && (b->cursor <= b->offset) )
    {
        /* roll offset back, until render space is exceeded */
        while( b->offset && (cx < b->widget.width) )
        {
            --(b->offset);

            while( b->offset && ((b->buffer[ b->offset ]) & 0xC0) == 0x80 )
                --(b->offset);

            cx = sgui_skin_default_font_extents( b->buffer + b->offset,
                                                 b->cursor - b->offset,
                                                 0, 0 );
        }

        /* move offset forward by one, to fit exactely into the bar */
        if( b->offset )
        {
            ++(b->offset);

            while( ((b->buffer[ b->offset ]) & 0xC0) == 0x80 )
                ++(b->offset);
        }
    }
    else if( cx > b->widget.width )
    {
        b->offset = b->cursor - 1;

        while( ((b->buffer[ b->offset ]) & 0xC0) == 0x80 )
            --(b->offset);
    }
}

/* get a cursor position from a mouse offset */
unsigned int sgui_edit_box_cursor_from_mouse( sgui_edit_box* b, int mouse_x )
{
    unsigned int len = 0, cur = b->offset;

    while( (len < (unsigned int)mouse_x) && (cur < b->end) )
    {
        ++cur;

        while( ((b->buffer[ cur ]) & 0xC0) == 0x80 )
            ++(cur);

        len = sgui_skin_default_font_extents( b->buffer+b->offset,
                                              cur      -b->offset, 0, 0 );
    }

    return cur;
}





void sgui_edit_box_draw( sgui_widget* widget, sgui_canvas* cv )
{
    sgui_edit_box* b = (sgui_edit_box*)widget;

    sgui_skin_draw_edit_box( cv, widget->x, widget->y,
                             b->buffer + b->offset, widget->width,
                             b->draw_cursor ?
                             (int)(b->cursor-b->offset) : -1 );
}

void sgui_edit_box_on_event( sgui_widget* widget, int type,
                             sgui_event* event )
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
    else if( (type == SGUI_MOUSE_RELEASE_EVENT) &&
             (event->mouse_press.button == SGUI_MOUSE_BUTTON_LEFT) &&
             b->num_entered )
    {
        unsigned int new_cur;
        int x = event->mouse_press.x - widget->x;
    
        new_cur = sgui_edit_box_cursor_from_mouse( b, x );

        if( new_cur != b->cursor )
        {
            b->cursor = new_cur;
            widget->need_redraw = 1;
        }
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
        else if( (event->keyboard_event.code==SGUI_KC_HOME) &&
                 (b->cursor || b->offset) )
        {
            b->cursor = 0;
            b->offset = 0;
            widget->need_redraw = 1;
        }
        else if( (event->keyboard_event.code==SGUI_KC_END) &&
                 (b->cursor < b->end) )
        {
            b->cursor = b->end;
            b->offset = b->end;
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
    b->widget.draw_callback         = sgui_edit_box_draw;
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

const unsigned char* sgui_edit_box_get_text( sgui_widget* box )
{
    return box ? ((sgui_edit_box*)box)->buffer : NULL;
}

void sgui_edit_box_set_text( sgui_widget* box, const unsigned char* text )
{
    unsigned int i;
    sgui_edit_box* b = (sgui_edit_box*)box;

    if( !box )
        return;

    if( !text )
    {
        b->num_entered = 0;
        b->buffer[ 0 ] = '\0';
        b->end = 0;
        b->cursor = 0;
        b->offset = 0;
        return;
    }

    for( b->num_entered=0, i=0; b->num_entered<b->max_chars && *text;
         ++b->num_entered )
    {
        b->buffer[ i++ ] = *(text++);

        while( (*text & 0xC0) == 0x80 )
            b->buffer[ i++ ] = *(text++);
    }

    b->buffer[ i ] = '\0';
    b->end = i;

    b->cursor = 0;
    b->offset = 0;

    box->need_redraw = 1;
}

