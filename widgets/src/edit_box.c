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
#include "sgui_utf8.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>



#define PASSWD_DISPLAY '*'



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

    /* BYTE OFFSET of the cursor before selection started */
    unsigned int selection;

    /* BYTE OFFSET of the first character
       visible at the left side of the box */
    unsigned int offset;

    int selecting;      /* boolean: currently in selection mode? */
    int draw_cursor;    /* boolean: draw the cursor? */
    char* buffer;       /* text buffer */
    int mode;           /* editing mode */
}
sgui_edit_box;

/* password mode edit box, derived from edit box */
typedef struct
{
    sgui_edit_box super;

    char* shadow;           /* buffer with actual text */

    /* BYTE OFFSET of the last character in the shadow buffer */
    unsigned int end;

    /* BYTE OFFSET of the shadow character after which to draw the cursor */
    unsigned int cursor;

    /* BYTE OFFSET of the cursor before selection started */
    unsigned int selection;
}
sgui_edit_box_passwd;



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
    if( !b->num_entered || !b->cursor )
    {
        b->offset = 0;
        return;
    }

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

static void sync_cursors( sgui_edit_box* b )
{
    sgui_edit_box_passwd* pw = (sgui_edit_box_passwd*)b;
    unsigned int i;

    if( b->mode==SGUI_EDIT_PASSWORD )
    {
        for( pw->cursor=0, i=0; i<b->cursor; ++i )
        {
            ADVANCE_UTF8( pw->shadow, pw->cursor );
        }

        for( pw->selection=0, i=0; i<b->selection; ++i )
        {
            ADVANCE_UTF8( pw->shadow, pw->selection );
        }
    }
}

static void remove_selection( sgui_edit_box* b )
{
    sgui_edit_box_passwd* pw = (sgui_edit_box_passwd*)b;
    int start, end, i, len;

    start = MIN( b->cursor, b->selection );
    end = MAX( b->cursor, b->selection );

    for( len=0, i=start; i<end; ++len )
    {
        ++i;
        while( (b->buffer[i] & 0xC0)==0x80 )
            ++i;
    }

    memmove( b->buffer+start, b->buffer+end, b->end-end+1 );
    b->num_entered -= len;
    b->end -= (end - start);
    b->selection = b->cursor = start;

    if( b->mode==SGUI_EDIT_PASSWORD )
    {
        start = MIN( pw->cursor, pw->selection );
        end = MAX( pw->cursor, pw->selection );

        memmove( pw->shadow+start, pw->shadow+end, pw->end-end+1 );
        pw->end -= (end - start);
        pw->selection = pw->cursor = start;
    }
}

static int insert( sgui_edit_box* b, unsigned int len, const char* utf8 )
{
    sgui_edit_box_passwd* pw = (sgui_edit_box_passwd*)b;
    unsigned int i, ulen;

    /* clip length to maximum allowed characters */
    ulen = sgui_utf8_strlen( utf8 );

    if( (ulen+b->num_entered) > b->max_chars )
    {
        ulen = b->max_chars - b->num_entered;

        for( len=0, i=0; i<ulen && utf8[len]; ++i )
        {
            ADVANCE_UTF8( utf8, len );
        }
    }

    /* sanity check */
    if( !len || !ulen )
        return 0;

    if( b->mode == SGUI_EDIT_NUMERIC )
    {
        for( i=0; i<len; ++i )
        {
            if( !isdigit( utf8[i] ) )
                return 0;
        }
    }

    /* copy data */
    remove_selection( b );

    if( b->mode == SGUI_EDIT_PASSWORD )
    {
        /* move text block after curser to the right */
        memmove( pw->shadow + pw->cursor + len, pw->shadow + pw->cursor,
                 pw->end - pw->cursor + 1 );

        /* insert */
        memcpy( pw->shadow+pw->cursor, utf8, len );
        pw->cursor += len;
        pw->end += len;

        /* update display buffer */
        memset( b->buffer + b->end, PASSWD_DISPLAY, ulen );
        b->buffer[ b->end+ulen ] = '\0';
        len = ulen;
    }
    else
    {
        /* move text block after curser to the right */
        memmove( b->buffer + b->cursor + len, b->buffer + b->cursor,
                 b->end - b->cursor + 1 );

        /* insert */
        memcpy( b->buffer+b->cursor, utf8, len );
    }

    b->num_entered += ulen;
    b->end += len;
    b->cursor += len;
    return 1;
}

/****************************************************************************/

static void edit_box_draw( sgui_widget* super )
{
    sgui_edit_box* this = (sgui_edit_box*)super;

    sgui_skin_draw_editbox( super->canvas, &(super->area),
                            this->buffer, this->offset,
                            this->draw_cursor ? (int)this->cursor : -1,
                            this->selection );
}

static void edit_box_on_event( sgui_widget* widget, sgui_event* e )
{
    sgui_edit_box* b = (sgui_edit_box*)widget;
    const char* ptr;
    int update = 0;
    sgui_event se;
    sgui_rect r;

    se.widget = widget;

    sgui_internal_lock_mutex( );

    if( e->type == SGUI_FOCUS_EVENT )
    {
        b->selection = b->cursor;
        b->selecting = 0;
        b->draw_cursor = 1;
        update = 1;
        sync_cursors( b );
    }
    else if( e->type == SGUI_FOCUS_LOSE_EVENT )
    {
        b->selection = b->cursor;
        b->selecting = 0;
        b->draw_cursor = 0;
        update = 1;
        sync_cursors( b );

        /* fire a text changed event */
        se.type = SGUI_EDIT_BOX_TEXT_CHANGED;
        sgui_event_post( &se );
    }
    else if( (e->type == SGUI_MOUSE_RELEASE_EVENT) &&
             (e->arg.i3.z == SGUI_MOUSE_BUTTON_LEFT) &&
             b->num_entered )
    {
        b->selecting = 0;
        b->cursor = cursor_from_mouse( b, e->arg.i3.x );
        b->selection = b->cursor;
        sync_cursors( b );
        update = 1;
    }
    else if( e->type == SGUI_KEY_RELEASED_EVENT &&
             (e->arg.i==SGUI_KC_LSHIFT || e->arg.i==SGUI_KC_RSHIFT ||
              e->arg.i==SGUI_KC_SHIFT) )
    {
        b->selecting = 0;
    }
    else if( e->type == SGUI_KEY_PRESSED_EVENT )
    {
        switch( e->arg.i )
        {
        case SGUI_KC_SELECT_ALL:
            if( b->num_entered )
            {
                b->selecting = 0;
                b->cursor = 0;
                b->selection = b->end;
                sync_cursors( b );
                determine_offset( b );
                update = 1;
            }
            break;
        case SGUI_KC_COPY:
            if( b->num_entered && b->selection!=b->cursor )
            {
                int start = MIN(b->selection,b->cursor);
                int end = MAX(b->selection,b->cursor);
                sgui_window_write_clipboard( e->window, b->buffer+start,
                                             end-start );
            }
            break;
        case SGUI_KC_PASTE:
            ptr = sgui_window_read_clipboard( e->window );

            if( insert( b, strlen( ptr ), ptr ) )
            {
                b->selection = b->cursor;
                sync_cursors( b );
                determine_offset( b );
                update = 1;
            }
            break;
        case SGUI_KC_CUT:
            if( b->num_entered && b->selection!=b->cursor )
            {
                int start = MIN(b->selection,b->cursor);
                int end = MAX(b->selection,b->cursor);
                sgui_window_write_clipboard( e->window, b->buffer+start,
                                             end-start );
                remove_selection( b );
                determine_offset( b );
                update = 1;
            }
            break;
        case SGUI_KC_LSHIFT:
        case SGUI_KC_RSHIFT:
        case SGUI_KC_SHIFT:
            b->selection = b->selecting ? b->selection : b->cursor;
            b->selecting = 1;
            break;
        case SGUI_KC_BACK:
            if( b->num_entered )
            {
                if( b->selection==b->cursor )
                {
                    if( !b->cursor )
                        break;
                    ROLL_BACK_UTF8( b->buffer, b->cursor );
                    sync_cursors( b );
                }
                remove_selection( b );
                determine_offset( b );
                update = 1;
            }
            break;
        case SGUI_KC_DELETE:
            if( b->num_entered )
            {
                if( b->selection==b->cursor )
                {
                    if( b->selection==b->end )
                        break;
                    ADVANCE_UTF8( b->buffer, b->selection );
                    sync_cursors( b );
                }
                remove_selection( b );
                determine_offset( b );
                update = 1;
            }
            break;
        case SGUI_KC_LEFT:
            if( b->cursor )
            {
                ROLL_BACK_UTF8( b->buffer, b->cursor );
                b->selection = b->selecting ? b->selection : b->cursor;
                sync_cursors( b );
                determine_offset( b );
                update = 1;
            }
            break;
        case SGUI_KC_RIGHT:
            if( b->cursor < b->end )
            {
                ADVANCE_UTF8( b->buffer, b->cursor );
                b->selection = b->selecting ? b->selection : b->cursor;
                sync_cursors( b );
                determine_offset( b );
                update = 1;
            }
            break;
        case SGUI_KC_HOME:
            if( b->offset || b->cursor )
            {
                b->cursor = b->offset = 0;
                b->selection = b->selecting ? b->selection : b->cursor;
                sync_cursors( b );
                update = 1;
            }
            break;
        case SGUI_KC_END:
            if( b->cursor < b->end )
            {
                b->offset = b->cursor = b->end;
                b->selection = b->selecting ? b->selection : b->cursor;
                sync_cursors( b );
                determine_offset( b );
                update = 1;
            }
            break;
        case SGUI_KC_RETURN:
            se.type = SGUI_EDIT_BOX_TEXT_ENTERED;
            sgui_event_post( &se );
            b->selecting = 0;
            b->selection = b->cursor;
            break;
        }
    }
    else if( (e->type == SGUI_CHAR_EVENT) && (b->num_entered < b->max_chars) )
    {   
        if( insert( b, strlen( e->arg.utf8 ), e->arg.utf8 ) )
        {
            b->selection = b->cursor;
            sync_cursors( b );
            determine_offset( b );
            update = 1;
        }
    }

    if( update )
    {
        sgui_widget_get_absolute_rect( widget, &r );
        sgui_canvas_add_dirty_rect( widget->canvas, &r );
    }
    sgui_internal_unlock_mutex( );
}

static void edit_box_destroy( sgui_widget* box )
{
    free( ((sgui_edit_box*)box)->buffer );
    free( box );
}



sgui_widget* sgui_edit_box_create( int x, int y, unsigned int width,
                                   unsigned int max_chars, int mode )
{
    sgui_edit_box_passwd* pw;
    unsigned int size;
    sgui_edit_box* b;

    /* allocate structure */
    size = mode==SGUI_EDIT_PASSWORD ? sizeof(sgui_edit_box_passwd) :
                                      sizeof(sgui_edit_box);

    b = malloc( size );
    pw = (sgui_edit_box_passwd*)b;

    if( !b )
        return NULL;

    memset( b, 0, size );

    /* allocate storage for the text buffer */
    b->buffer = malloc( max_chars * 6 + 1 );

    if( !b->buffer )
    {
        free( b );
        return NULL;
    }

    /* allocate shadow buffer for password edit box */
    if( mode==SGUI_EDIT_PASSWORD )
    {
        pw->shadow = b->buffer;
        b->buffer = malloc( max_chars + 1 );

        if( !b->buffer )
        {
            free( pw->shadow );
            free( b );
            return NULL;
        }

        pw->shadow[0] = '\0';
    }

    /* initialise and store state */
    sgui_internal_widget_init( (sgui_widget*)b, x, y, width,
                               sgui_skin_get_edit_box_height( ) );

    b->widget.window_event_callback = edit_box_on_event;
    b->widget.destroy               = edit_box_destroy;
    b->widget.draw_callback         = edit_box_draw;
    b->mode                         = mode;
    b->max_chars                    = max_chars;
    b->buffer[0]                    = '\0';
    b->widget.focus_policy          = SGUI_FOCUS_ACCEPT|SGUI_FOCUS_DROP_ESC|
                                      SGUI_FOCUS_DROP_TAB;

    return (sgui_widget*)b;
}

const char* sgui_edit_box_get_text( sgui_widget* box )
{
    if( box )
    {
        if( ((sgui_edit_box*)box)->mode == SGUI_EDIT_PASSWORD )
            return ((sgui_edit_box_passwd*)box)->shadow;

        return ((sgui_edit_box*)box)->buffer;
    }

    return NULL;
}

void sgui_edit_box_set_text( sgui_widget* box, const char* text )
{
    sgui_rect r;
    unsigned int i;
    sgui_edit_box* b = (sgui_edit_box*)box;
    sgui_edit_box_passwd* pw = (sgui_edit_box_passwd*)box;

    if( !box )
        return;

    sgui_internal_lock_mutex( );

    /* text = NULL means clear */
    if( !text )
    {
        if( b->mode == SGUI_EDIT_PASSWORD )
        {
            pw->shadow[ 0 ] = '\0';
            pw->cursor = 0;
        }

        b->buffer[ 0 ] = '\0';
    }
    else if( b->mode == SGUI_EDIT_PASSWORD )
    {
        pw->end = sgui_utf8_strncpy( pw->shadow, text, -1, b->max_chars );
        pw->cursor = 0;

        /* generate sequence of display characters */
        i = sgui_utf8_strlen( pw->shadow );
        memset( b->buffer, PASSWD_DISPLAY, i );
        b->buffer[ i ] = '\0';
    }
    else if( b->mode == SGUI_EDIT_NUMERIC )
    {
        for( i=0; i<b->max_chars && *text; ++text )
        {
            if( isdigit( *text ) )
            {
                b->buffer[ i++ ] = *text;
            }
        }

        b->buffer[ i ] = '\0';
    }
    else
    {
        sgui_utf8_strncpy( b->buffer, text, -1, b->max_chars );
    }

    b->cursor = 0;          /* set cursor to the beginning */
    b->offset = 0;
    b->end = strlen( b->buffer );
    b->num_entered = sgui_utf8_strlen( b->buffer );

    /* flag area dirt */
    sgui_widget_get_absolute_rect( box, &r );
    sgui_canvas_add_dirty_rect( box->canvas, &r );

    sgui_internal_unlock_mutex( );
}

