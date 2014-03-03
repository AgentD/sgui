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
    sgui_widget super;

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
static void determine_offset( sgui_edit_box* this )
{
    unsigned int cx, w;

    /* only adjust if there are characters entered */
    if( !this->num_entered || !this->cursor )
    {
        this->offset = 0;
        return;
    }

    w = SGUI_RECT_WIDTH( this->super.area );
    w -= sgui_skin_get_edit_box_border_width( ) << 1;

    /* if the cursor moved out of the edit box to the left */
    if( this->offset && (this->cursor <= this->offset) )
    {
        this->offset = this->cursor;

        /* roll offset back until the cursor moves out to the right */
        do
        {
            ROLL_BACK_UTF8( this->buffer, this->offset );

            cx = sgui_skin_default_font_extents( this->buffer + this->offset,
                                                 this->cursor - this->offset,
                                                 0, 0 );
        }
        while( this->offset && (cx < w) );

        /* move offset forward by one, so the cursor is at the far right */
        if( this->offset )
        {
            ADVANCE_UTF8( this->buffer, this->offset );
        }
    }
    else
    {
        /* get the extents of the text from the left of the
           edit box to the cursor */
        cx = sgui_skin_default_font_extents( this->buffer + this->offset,
                                             this->cursor - this->offset,
                                             0, 0 );

        /* the cursor has moved outside the edit box to the right */
        if( cx > w )
        {
            /* adjust the rendered region, so the cursor
              is at the left after the first character */
            this->offset = this->cursor;

            ROLL_BACK_UTF8( this->buffer, this->offset );
        }
    }
}

/* get a cursor position from a mouse offset */
static unsigned int cursor_from_mouse( sgui_edit_box* this, int mouse_x )
{
    unsigned int len = 0, cur = this->offset;

    mouse_x -= sgui_skin_get_edit_box_border_width( );

    /* move 'cur' to the right until it the text extents from
       the beginning to 'cur' catch up with the mouse offset */
    while( (len < (unsigned int)mouse_x) && (cur < this->end) )
    {
        ADVANCE_UTF8( this->buffer, cur );

        /* get the text extents from the rendering offset to 'cur' */
        len = sgui_skin_default_font_extents( this->buffer+this->offset,
                                              cur         -this->offset,
                                              0, 0 );
    }

    return cur;
}

static void sync_cursors( sgui_edit_box* super )
{
    sgui_edit_box_passwd* this = (sgui_edit_box_passwd*)super;
    unsigned int i;

    if( super->mode==SGUI_EDIT_PASSWORD )
    {
        for( this->cursor=0, i=0; i<super->cursor; ++i )
        {
            ADVANCE_UTF8( this->shadow, this->cursor );
        }

        for( this->selection=0, i=0; i<super->selection; ++i )
        {
            ADVANCE_UTF8( this->shadow, this->selection );
        }
    }
}

static void remove_selection( sgui_edit_box* super )
{
    sgui_edit_box_passwd* this = (sgui_edit_box_passwd*)super;
    int start, end, i, len;

    start = MIN( super->cursor, super->selection );
    end = MAX( super->cursor, super->selection );

    for( len=0, i=start; i<end; ++len )
    {
        ++i;
        while( (super->buffer[i] & 0xC0)==0x80 )
            ++i;
    }

    memmove( super->buffer+start, super->buffer+end, super->end-end+1 );
    super->num_entered -= len;
    super->end -= (end - start);
    super->selection = super->cursor = start;

    if( super->mode==SGUI_EDIT_PASSWORD )
    {
        start = MIN( this->cursor, this->selection );
        end = MAX( this->cursor, this->selection );

        memmove( this->shadow+start, this->shadow+end, this->end-end+1 );
        this->end -= (end - start);
        this->selection = this->cursor = start;
    }
}

static int insert( sgui_edit_box* super, unsigned int len, const char* utf8 )
{
    sgui_edit_box_passwd* this = (sgui_edit_box_passwd*)super;
    unsigned int i, ulen;

    /* clip length to maximum allowed characters */
    ulen = sgui_utf8_strlen( utf8 );

    if( (ulen+super->num_entered) > super->max_chars )
    {
        ulen = super->max_chars - super->num_entered;

        for( len=0, i=0; i<ulen && utf8[len]; ++i )
        {
            ADVANCE_UTF8( utf8, len );
        }
    }

    /* sanity check */
    if( !len || !ulen )
        return 0;

    if( super->mode == SGUI_EDIT_NUMERIC )
    {
        for( i=0; i<len; ++i )
        {
            if( !isdigit( utf8[i] ) )
                return 0;
        }
    }

    /* copy data */
    remove_selection( super );

    if( super->mode == SGUI_EDIT_PASSWORD )
    {
        /* move text block after curser to the right */
        memmove( this->shadow + this->cursor + len,
                 this->shadow + this->cursor,
                 this->end - this->cursor + 1 );

        /* insert */
        memcpy( this->shadow+this->cursor, utf8, len );
        this->cursor += len;
        this->end += len;

        /* update display buffer */
        memset( super->buffer + super->end, PASSWD_DISPLAY, ulen );
        super->buffer[ super->end+ulen ] = '\0';
        len = ulen;
    }
    else
    {
        /* move text block after curser to the right */
        memmove( super->buffer + super->cursor + len,
                 super->buffer + super->cursor,
                 super->end - super->cursor + 1 );

        /* insert */
        memcpy( super->buffer+super->cursor, utf8, len );
    }

    super->num_entered += ulen;
    super->end += len;
    super->cursor += len;
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

static void edit_box_on_event( sgui_widget* super, const sgui_event* e )
{
    sgui_edit_box* this = (sgui_edit_box*)super;
    const char* ptr;
    int update = 0;
    sgui_event se;
    sgui_rect r;

    se.widget = super;

    sgui_internal_lock_mutex( );

    if( e->type == SGUI_FOCUS_EVENT )
    {
        this->selection = this->cursor;
        this->selecting = 0;
        this->draw_cursor = 1;
        update = 1;
        sync_cursors( this );
    }
    else if( e->type == SGUI_FOCUS_LOSE_EVENT )
    {
        this->selection = this->cursor;
        this->selecting = 0;
        this->draw_cursor = 0;
        update = 1;
        sync_cursors( this );

        /* fire a text changed event */
        se.type = SGUI_EDIT_BOX_TEXT_CHANGED;
        sgui_event_post( &se );
    }
    else if( (e->type == SGUI_MOUSE_RELEASE_EVENT) &&
             (e->arg.i3.z == SGUI_MOUSE_BUTTON_LEFT) &&
             this->num_entered )
    {
        this->selecting = 0;
        this->cursor = cursor_from_mouse( this, e->arg.i3.x );
        this->selection = this->cursor;
        sync_cursors( this );
        update = 1;
    }
    else if( e->type == SGUI_KEY_RELEASED_EVENT &&
             (e->arg.i==SGUI_KC_LSHIFT || e->arg.i==SGUI_KC_RSHIFT ||
              e->arg.i==SGUI_KC_SHIFT) )
    {
        this->selecting = 0;
    }
    else if( e->type == SGUI_KEY_PRESSED_EVENT )
    {
        switch( e->arg.i )
        {
        case SGUI_KC_SELECT_ALL:
            if( this->num_entered )
            {
                this->selecting = 0;
                this->cursor = 0;
                this->selection = this->end;
                sync_cursors( this );
                determine_offset( this );
                update = 1;
            }
            break;
        case SGUI_KC_COPY:
            if( this->num_entered && this->selection!=this->cursor )
            {
                int start = MIN(this->selection,this->cursor);
                int end = MAX(this->selection,this->cursor);
                sgui_window_write_clipboard( e->window, this->buffer+start,
                                             end-start );
            }
            break;
        case SGUI_KC_PASTE:
            ptr = sgui_window_read_clipboard( e->window );

            if( insert( this, strlen( ptr ), ptr ) )
            {
                this->selection = this->cursor;
                sync_cursors( this );
                determine_offset( this );
                update = 1;
            }
            break;
        case SGUI_KC_CUT:
            if( this->num_entered && this->selection!=this->cursor )
            {
                int start = MIN(this->selection,this->cursor);
                int end = MAX(this->selection,this->cursor);
                sgui_window_write_clipboard( e->window, this->buffer+start,
                                             end-start );
                remove_selection( this );
                determine_offset( this );
                update = 1;
            }
            break;
        case SGUI_KC_LSHIFT:
        case SGUI_KC_RSHIFT:
        case SGUI_KC_SHIFT:
            this->selection=this->selecting ? this->selection : this->cursor;
            this->selecting=1;
            break;
        case SGUI_KC_BACK:
            if( this->num_entered )
            {
                if( this->selection==this->cursor )
                {
                    if( !this->cursor )
                        break;
                    ROLL_BACK_UTF8( this->buffer, this->cursor );
                    sync_cursors( this );
                }
                remove_selection( this );
                determine_offset( this );
                update = 1;
            }
            break;
        case SGUI_KC_DELETE:
            if( this->num_entered )
            {
                if( this->selection==this->cursor )
                {
                    if( this->selection==this->end )
                        break;
                    ADVANCE_UTF8( this->buffer, this->selection );
                    sync_cursors( this );
                }
                remove_selection( this );
                determine_offset( this );
                update = 1;
            }
            break;
        case SGUI_KC_LEFT:
            if( this->cursor )
            {
                ROLL_BACK_UTF8( this->buffer, this->cursor );
                this->selection=this->selecting?this->selection:this->cursor;
                sync_cursors( this );
                determine_offset( this );
                update = 1;
            }
            break;
        case SGUI_KC_RIGHT:
            if( this->cursor < this->end )
            {
                ADVANCE_UTF8( this->buffer, this->cursor );
                this->selection=this->selecting?this->selection:this->cursor;
                sync_cursors( this );
                determine_offset( this );
                update = 1;
            }
            break;
        case SGUI_KC_HOME:
            if( this->offset || this->cursor )
            {
                this->cursor = this->offset = 0;
                this->selection=this->selecting?this->selection:this->cursor;
                sync_cursors( this );
                update = 1;
            }
            break;
        case SGUI_KC_END:
            if( this->cursor < this->end )
            {
                this->offset = this->cursor = this->end;
                this->selection=this->selecting?this->selection:this->cursor;
                sync_cursors( this );
                determine_offset( this );
                update = 1;
            }
            break;
        case SGUI_KC_RETURN:
            se.type = SGUI_EDIT_BOX_TEXT_ENTERED;
            sgui_event_post( &se );
            this->selecting = 0;
            this->selection = this->cursor;
            break;
        }
    }
    else if( e->type==SGUI_CHAR_EVENT && (this->num_entered<this->max_chars) )
    {   
        if( insert( this, strlen( e->arg.utf8 ), e->arg.utf8 ) )
        {
            this->selection = this->cursor;
            sync_cursors( this );
            determine_offset( this );
            update = 1;
        }
    }

    if( update )
    {
        sgui_widget_get_absolute_rect( super, &r );
        sgui_canvas_add_dirty_rect( super->canvas, &r );
    }
    sgui_internal_unlock_mutex( );
}

static void edit_box_destroy( sgui_widget* this )
{
    free( ((sgui_edit_box*)this)->buffer );
    free( this );
}



sgui_widget* sgui_edit_box_create( int x, int y, unsigned int width,
                                   unsigned int max_chars, int mode )
{
    sgui_edit_box_passwd* pw;
    unsigned int size;
    sgui_edit_box* this;
    sgui_widget* super;

    /* allocate structure */
    size = mode==SGUI_EDIT_PASSWORD ? sizeof(sgui_edit_box_passwd) :
                                      sizeof(sgui_edit_box);

    this = malloc( size );
    super = (sgui_widget*)this;
    pw = (sgui_edit_box_passwd*)this;

    if( !this )
        return NULL;

    memset( this, 0, size );

    /* allocate storage for the text buffer */
    this->buffer = malloc( max_chars * 6 + 1 );

    if( !this->buffer )
    {
        free( this );
        return NULL;
    }

    /* allocate shadow buffer for password edit box */
    if( mode==SGUI_EDIT_PASSWORD )
    {
        pw->shadow = this->buffer;
        this->buffer = malloc( max_chars + 1 );

        if( !this->buffer )
        {
            free( pw->shadow );
            free( this );
            return NULL;
        }

        pw->shadow[0] = '\0';
    }

    /* initialise and store state */
    sgui_internal_widget_init( super, x, y, width,
                               sgui_skin_get_edit_box_height( ) );

    super->window_event_callback = edit_box_on_event;
    super->destroy               = edit_box_destroy;
    super->draw_callback         = edit_box_draw;
    this->mode                   = mode;
    this->max_chars              = max_chars;
    this->buffer[0]              = '\0';
    super->focus_policy          = SGUI_FOCUS_ACCEPT|SGUI_FOCUS_DROP_ESC|
                                   SGUI_FOCUS_DROP_TAB;

    return super;
}

const char* sgui_edit_box_get_text( sgui_widget* this )
{
    if( this )
    {
        if( ((sgui_edit_box*)this)->mode == SGUI_EDIT_PASSWORD )
            return ((sgui_edit_box_passwd*)this)->shadow;

        return ((sgui_edit_box*)this)->buffer;
    }

    return NULL;
}

void sgui_edit_box_set_text( sgui_widget* super, const char* text )
{
    sgui_rect r;
    unsigned int i;
    sgui_edit_box* this = (sgui_edit_box*)super;
    sgui_edit_box_passwd* pw = (sgui_edit_box_passwd*)super;

    if( !this )
        return;

    sgui_internal_lock_mutex( );

    /* text = NULL means clear */
    if( !text )
    {
        if( this->mode == SGUI_EDIT_PASSWORD )
        {
            pw->shadow[ 0 ] = '\0';
            pw->cursor = 0;
        }

        this->buffer[ 0 ] = '\0';
    }
    else if( this->mode == SGUI_EDIT_PASSWORD )
    {
        pw->end = sgui_utf8_strncpy( pw->shadow, text, -1, this->max_chars );
        pw->cursor = 0;

        /* generate sequence of display characters */
        i = sgui_utf8_strlen( pw->shadow );
        memset( this->buffer, PASSWD_DISPLAY, i );
        this->buffer[ i ] = '\0';
    }
    else if( this->mode == SGUI_EDIT_NUMERIC )
    {
        for( i=0; i<this->max_chars && *text; ++text )
        {
            if( isdigit( *text ) )
            {
                this->buffer[ i++ ] = *text;
            }
        }

        this->buffer[ i ] = '\0';
    }
    else
    {
        sgui_utf8_strncpy( this->buffer, text, -1, this->max_chars );
    }

    this->cursor = 0;          /* set cursor to the beginning */
    this->offset = 0;
    this->end = strlen( this->buffer );
    this->num_entered = sgui_utf8_strlen( this->buffer );

    /* flag area dirt */
    sgui_widget_get_absolute_rect( super, &r );
    sgui_canvas_add_dirty_rect( super->canvas, &r );

    sgui_internal_unlock_mutex( );
}

