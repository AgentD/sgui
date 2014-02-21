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
#define SGUI_BUILDING_DLL
#include "sgui_edit_box.h"
#include "sgui_skin.h"
#include "sgui_event.h"
#include "sgui_internal.h"
#include "sgui_widget.h"

#include <stdlib.h>
#include <string.h>



typedef struct
{
    sgui_widget widget;

    /* maximum number of UTF8 characters the user can enter and
       the number of UTF8 characters that have already been entered */
    unsigned int max_chars, num_entered;

    /* BYTE OFFSET of the last character in the text buffer */
    unsigned int end;

    /* BYTE OFFSET of the character after which to draw the cursor */
    unsigned int cursor;

    /* BYTE OFFSET of the first character
       visible at the left side of the box */
    unsigned int offset;

    int draw_cursor;    /* boolean: draw the cursor? */

    char* buffer;       /* text buffer */
}
sgui_edit_box;



/* move index back to the start of the preceeding
   UTF8 character in the buffer */
#define ROLL_BACK_UTF8( buffer, index )\
            --(index);\
            while( (index) && (((buffer)[ index ]) & 0xC0) == 0x80 )\
                --(index);

/* move index forward to the start of the next UTF8 character in the buffer */
#define ADVANCE_UTF8( buffer, index )\
            ++(index);\
            while( (((buffer)[ index ]) & 0xC0) == 0x80 )\
                ++(index);


/* Adjust text render offset after moving the cursor */
static void determine_offset( sgui_edit_box* b )
{
    unsigned int cx, w;

    /* only adjust if there are characters entered */
    if( !b->num_entered )
        return;

    w = SGUI_RECT_WIDTH( b->widget.area );
    w -= sgui_skin_get_edit_box_border_width( ) << 1;

    /* if the cursor moved out of the edit box to the left */
    if( b->offset && (b->cursor <= b->offset) )
    {
        b->offset = b->cursor;

        /* roll offset back until the cursor moves out to the right */
        do
        {
            ROLL_BACK_UTF8( b->buffer, b->offset );

            cx = sgui_skin_default_font_extents( b->buffer + b->offset,
                                                 b->cursor - b->offset,
                                                 0, 0 );
        }
        while( b->offset && (cx < w) );

        /* move offset forward by one, so the cursor is at the far right */
        if( b->offset )
        {
            ADVANCE_UTF8( b->buffer, b->offset );
        }
    }
    else
    {
        /* get the extents of the text from the left of the
           edit box to the cursor */
        cx = sgui_skin_default_font_extents( b->buffer + b->offset,
                                             b->cursor - b->offset, 0, 0 );

        /* the cursor has moved outside the edit box to the right */
        if( cx > w )
        {
            /* adjust the rendered region, so the cursor
              is at the left after the first character */
            b->offset = b->cursor;

            ROLL_BACK_UTF8( b->buffer, b->offset );
        }
    }
}

/* get a cursor position from a mouse offset */
static unsigned int cursor_from_mouse( sgui_edit_box* b, int mouse_x )
{
    unsigned int len = 0, cur = b->offset;

    mouse_x -= sgui_skin_get_edit_box_border_width( );

    /* move 'cur' to the right until it the text extents from
       the beginning to 'cur' catch up with the mouse offset */
    while( (len < (unsigned int)mouse_x) && (cur < b->end) )
    {
        ADVANCE_UTF8( b->buffer, cur );

        /* get the text extents from the rendering offset to 'cur' */
        len = sgui_skin_default_font_extents( b->buffer+b->offset,
                                              cur      -b->offset, 0, 0 );
    }

    return cur;
}



static void edit_box_draw( sgui_widget* super )
{
    sgui_edit_box* this = (sgui_edit_box*)super;

    sgui_skin_draw_editbox( super->canvas, &(super->area),
                            this->buffer, this->offset,
                            this->draw_cursor ? (int)this->cursor : -1 );
}

static void edit_box_on_event( sgui_widget* widget, sgui_event* e )
{
    sgui_edit_box* b = (sgui_edit_box*)widget;
    sgui_event se;
    sgui_rect r;

    se.widget = widget;

    if( e->type == SGUI_FOCUS_EVENT )
    {
        /* enable cursor drawing, flag area as dirty */
        b->draw_cursor = 1;
        sgui_widget_get_absolute_rect( widget, &r );
        sgui_canvas_add_dirty_rect( widget->canvas, &r );
    }
    else if( e->type == SGUI_FOCUS_LOSE_EVENT )
    {
        /* disable cursor drawing, flag area as dirty */
        b->draw_cursor = 0;
        sgui_widget_get_absolute_rect( widget, &r );
        sgui_canvas_add_dirty_rect( widget->canvas, &r );

        /* fire a text changed event */
        se.type = SGUI_EDIT_BOX_TEXT_CHANGED;
        sgui_canvas_fire_widget_event( widget->canvas, &se );
    }
    else if( (e->type == SGUI_MOUSE_RELEASE_EVENT) &&
             (e->arg.mouse_press.button == SGUI_MOUSE_BUTTON_LEFT) &&
             b->num_entered )
    {
        unsigned int new_cur;

        /* get the cursor offset from the mouse position */
        new_cur = cursor_from_mouse( b, e->arg.mouse_press.x );

        /* store new position and flag area dirty if the
           cursor position changed */
        if( new_cur != b->cursor )
        {
            b->cursor = new_cur;
            sgui_widget_get_absolute_rect( widget, &r );
            sgui_canvas_add_dirty_rect( widget->canvas, &r );
        }
    }
    else if( e->type == SGUI_KEY_PRESSED_EVENT )
    {
        /* backspace pressed, characters have been entered, cursor is not 0 */
        if( (e->arg.keyboard_event.code==SGUI_KC_BACK) &&
            b->num_entered && b->cursor )
        {
            unsigned int old = b->cursor;   /* store old cursor position */

            /* move back by one character */
            ROLL_BACK_UTF8( b->buffer, b->cursor );

            /* move entire text block back by the difference */
            memmove( b->buffer+b->cursor, b->buffer+old, b->end-old+1 );

            /* update state */
            b->num_entered -= 1;
            b->end -= (old - b->cursor);

            /* flag dirty and adjust offset */
            sgui_widget_get_absolute_rect( widget, &r );
            sgui_canvas_add_dirty_rect( widget->canvas, &r );
            determine_offset( b );
        }
        /* delete pressed, cursor is not at end, chars have been entered */
        else if( (e->arg.keyboard_event.code==SGUI_KC_DELETE) &&
                 (b->cursor < b->end) && b->num_entered )
        {
            unsigned int offset = b->cursor;  /* store old cursor position */

            /* move forward by one character */
            ADVANCE_UTF8( b->buffer, offset );

            /* move entire text block back by difference */
            memmove( b->buffer+b->cursor, b->buffer+offset, b->end-offset+1 );

            /* update state */
            b->num_entered -= 1;
            b->end -= (offset - b->cursor);

            /* flag dirty and adjust offset */
            sgui_widget_get_absolute_rect( widget, &r );
            sgui_canvas_add_dirty_rect( widget->canvas, &r );
            determine_offset( b );
        }
        /* move cursor left pressed and cursor is not 0 */
        else if( (e->arg.keyboard_event.code==SGUI_KC_LEFT) && b->cursor )
        {
            ROLL_BACK_UTF8( b->buffer, b->cursor );

            sgui_widget_get_absolute_rect( widget, &r );
            sgui_canvas_add_dirty_rect( widget->canvas, &r );
            determine_offset( b );
        }
        /* move cursor right pressed and cursor is not at the end */
        else if( (e->arg.keyboard_event.code==SGUI_KC_RIGHT) &&
                 (b->cursor < b->end) )
        {
            ADVANCE_UTF8( b->buffer, b->cursor );

            sgui_widget_get_absolute_rect( widget, &r );
            sgui_canvas_add_dirty_rect( widget->canvas, &r );
            determine_offset( b );
        }
        /* home key pressed and we have an offset OR the cursor is not */
        else if( (e->arg.keyboard_event.code==SGUI_KC_HOME) &&
                 (b->offset || b->cursor) )
        {
            b->cursor = 0;
            b->offset = 0;
            sgui_widget_get_absolute_rect( widget, &r );
            sgui_canvas_add_dirty_rect( widget->canvas, &r );
        }
        /* end key pressed and the cursor is not at the end */
        else if( (e->arg.keyboard_event.code==SGUI_KC_END) &&
                 (b->cursor < b->end) )
        {
            b->cursor = b->end;
            b->offset = b->end;
            sgui_widget_get_absolute_rect( widget, &r );
            sgui_canvas_add_dirty_rect( widget->canvas, &r );
            determine_offset( b );
        }
        /* ENTER key pressed */
        else if( e->arg.keyboard_event.code==SGUI_KC_RETURN )
        {
            se.type = SGUI_EDIT_BOX_TEXT_ENTERED;
            sgui_canvas_fire_widget_event( widget->canvas, &se );
        }
    }
    else if( (e->type == SGUI_CHAR_EVENT) && (b->num_entered < b->max_chars) )
    {   
        unsigned int len;

        /* get the length of the UTF8 string to insert */
        len = strlen( e->arg.char_event.as_utf8_str );

        /* move entire text block after curser right by that length */
        memmove( b->buffer + b->cursor + len, b->buffer + b->cursor,
                 b->end - b->cursor + 1 );

        /* insert the character */
        memcpy( b->buffer+b->cursor, e->arg.char_event.as_utf8_str, len );

        /* update state */
        b->end += len;
        b->cursor += len;
        b->num_entered += 1;

        /* flag dirty and determine cursor position */
        sgui_widget_get_absolute_rect( widget, &r );
        sgui_canvas_add_dirty_rect( widget->canvas, &r );
        determine_offset( b );
    }
}

static void edit_box_destroy( sgui_widget* box )
{
    free( ((sgui_edit_box*)box)->buffer );
    free( box );
}



sgui_widget* sgui_edit_box_create( int x, int y, unsigned int width,
                                   unsigned int max_chars )
{
    sgui_edit_box* b;

    /* allocate structure */
    b = malloc( sizeof(sgui_edit_box) );

    if( !b )
        return NULL;

    memset( b, 0, sizeof(sgui_edit_box) );

    /* allocate storage for the text buffer */
    b->buffer = malloc( max_chars * 6 + 1 );

    if( !b->buffer )
    {
        free( b );
        return NULL;
    }

    /* initialise and store state */
    sgui_internal_widget_init( (sgui_widget*)b, x, y, width,
                               sgui_skin_get_edit_box_height( ) );

    b->widget.window_event_callback = edit_box_on_event;
    b->widget.destroy               = edit_box_destroy;
    b->widget.draw_callback         = edit_box_draw;
    b->max_chars                    = max_chars;
    b->buffer[0]                    = '\0';

    return (sgui_widget*)b;
}

const char* sgui_edit_box_get_text( sgui_widget* box )
{
    return box ? ((sgui_edit_box*)box)->buffer : NULL;
}

void sgui_edit_box_set_text( sgui_widget* box, const char* text )
{
    sgui_rect r;
    unsigned int i;
    sgui_edit_box* b = (sgui_edit_box*)box;

    if( !box )
        return;

    /* text = NULL means clear */
    if( !text )
    {
        b->num_entered = 0;
        b->buffer[ 0 ] = '\0';
        b->end = 0;
        b->cursor = 0;
        b->offset = 0;
    }
    else
    {
        /* copy text and count entered characters (UTF8!!) */
        for( b->num_entered=0, i=0; b->num_entered<b->max_chars && *text;
             ++b->num_entered )
        {
            b->buffer[ i++ ] = *(text++);

            while( (*text & 0xC0) == 0x80 )
                b->buffer[ i++ ] = *(text++);
        }

        b->buffer[ i ] = '\0';  /* append null-terminator */
        b->end = i;             /* store offset of end */

        b->cursor = 0;          /* set cursor to the beginning */
        b->offset = 0;

        /* flag area dirt */
        sgui_widget_get_absolute_rect( box, &r );
        sgui_canvas_add_dirty_rect( box->canvas, &r );
    }
}

