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
    sgui_skin* skin;

    /* only adjust if there are characters entered */
    if( !this->num_entered || !this->cursor )
    {
        this->offset = 0;
        return;
    }

    skin = sgui_skin_get( );
    w = SGUI_RECT_WIDTH( this->super.area );
    w -= skin->get_edit_box_border_width( skin ) << 1;

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
    sgui_skin* skin = sgui_skin_get( );

    mouse_x -= skin->get_edit_box_border_width( skin );

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

static void sync_cursors( sgui_edit_box* this )
{
    (void)this;
}

static void remove_selection( sgui_edit_box* this )
{
    int start = MIN( this->cursor, this->selection );
    int end = MAX( this->cursor, this->selection );
    int i, len;

    for( len=0, i=start; i<end; ++len )
    {
        ++i;
        while( (this->buffer[i] & 0xC0)==0x80 )
            ++i;
    }

    memmove( this->buffer+start, this->buffer+end, this->end-end+1 );
    this->num_entered -= len;
    this->end -= (end - start);
    this->selection = this->cursor = start;
}

static int insert( sgui_edit_box* this, unsigned int len, const char* utf8 )
{
    unsigned int i, ulen;

    /* clip length to maximum allowed characters */
    ulen = sgui_utf8_strlen( utf8 );

    if( (ulen+this->num_entered) > this->max_chars )
    {
        ulen = this->max_chars - this->num_entered;

        for( len=0, i=0; i<ulen && utf8[len]; ++i )
        {
            ADVANCE_UTF8( utf8, len );
        }
    }

    /* sanity check */
    if( !len || !ulen )
        return 0;

    /* copy data */
    this->remove_selection( this );

    /* move text block after curser to the right */
    memmove( this->buffer + this->cursor + len,
             this->buffer + this->cursor,
             this->end - this->cursor + 1 );

    /* insert */
    memcpy( this->buffer + this->cursor, utf8, len );

    this->num_entered += ulen;
    this->end += len;
    this->cursor += len;
    return 1;
}

static void edit_box_draw( sgui_widget* super )
{
    sgui_edit_box* this = (sgui_edit_box*)super;
    sgui_skin* skin = sgui_skin_get( );

    skin->draw_editbox( skin, super->canvas, &(super->area),
                              this->buffer, this->offset,
                              this->draw_cursor ? (int)this->cursor : -1,
                              this->selection, 0, 0 );
}

static void edit_box_text_changed( sgui_edit_box* this, int type )
{
    sgui_event se;

    se.src.widget = (sgui_widget*)this;
    se.type = type;
    sgui_event_post( &se );
}

static void edit_box_on_event( sgui_widget* super, const sgui_event* e )
{
    sgui_edit_box* this = (sgui_edit_box*)super;
    const char* ptr;
    int update = 0;
    sgui_rect r;

    sgui_internal_lock_mutex( );

    if( e->type == SGUI_FOCUS_EVENT )
    {
        this->selection = this->cursor;
        this->selecting = 0;
        this->draw_cursor = 1;
        update = 1;
        this->sync_cursors( this );
    }
    else if( e->type == SGUI_FOCUS_LOSE_EVENT )
    {
        this->selection = this->cursor;
        this->selecting = 0;
        this->draw_cursor = 0;
        update = 1;
        this->sync_cursors( this );
        this->text_changed( this, SGUI_EDIT_BOX_TEXT_CHANGED );
    }
    else if( (e->type == SGUI_MOUSE_RELEASE_EVENT) &&
             (e->arg.i3.z == SGUI_MOUSE_BUTTON_LEFT) &&
             this->num_entered )
    {
        this->selecting = 0;
        this->cursor = this->offset_from_position( this, e->arg.i3.x );
        this->selection = this->cursor;
        this->sync_cursors( this );
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
                this->sync_cursors( this );
                determine_offset( this );
                update = 1;
            }
            break;
        case SGUI_KC_COPY:
            if( this->num_entered && this->selection!=this->cursor )
            {
                int start = MIN(this->selection,this->cursor);
                int end = MAX(this->selection,this->cursor);
                sgui_window_write_clipboard( e->src.window, this->buffer+start,
                                             end-start );
            }
            break;
        case SGUI_KC_PASTE:
            ptr = sgui_window_read_clipboard( e->src.window );

            if( this->insert( this, strlen( ptr ), ptr ) )
            {
                this->selection = this->cursor;
                this->sync_cursors( this );
                determine_offset( this );
                update = 1;
            }
            break;
        case SGUI_KC_CUT:
            if( this->num_entered && this->selection!=this->cursor )
            {
                int start = MIN(this->selection,this->cursor);
                int end = MAX(this->selection,this->cursor);
                sgui_window_write_clipboard( e->src.window,this->buffer+start,
                                             end-start );
                this->remove_selection( this );
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
                    this->sync_cursors( this );
                }
                this->remove_selection( this );
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
                    this->sync_cursors( this );
                }
                this->remove_selection( this );
                determine_offset( this );
                update = 1;
            }
            break;
        case SGUI_KC_LEFT:
            if( this->cursor )
            {
                ROLL_BACK_UTF8( this->buffer, this->cursor );
                this->selection=this->selecting?this->selection:this->cursor;
                this->sync_cursors( this );
                determine_offset( this );
                update = 1;
            }
            break;
        case SGUI_KC_RIGHT:
            if( this->cursor < this->end )
            {
                ADVANCE_UTF8( this->buffer, this->cursor );
                this->selection=this->selecting?this->selection:this->cursor;
                this->sync_cursors( this );
                determine_offset( this );
                update = 1;
            }
            break;
        case SGUI_KC_HOME:
            if( this->offset || this->cursor )
            {
                this->cursor = this->offset = 0;
                this->selection=this->selecting?this->selection:this->cursor;
                this->sync_cursors( this );
                update = 1;
            }
            break;
        case SGUI_KC_END:
            if( this->cursor < this->end )
            {
                this->offset = this->cursor = this->end;
                this->selection=this->selecting?this->selection:this->cursor;
                this->sync_cursors( this );
                determine_offset( this );
                update = 1;
            }
            break;
        case SGUI_KC_RETURN:
            this->text_changed( this, SGUI_EDIT_BOX_TEXT_ENTERED );
            this->selecting = 0;
            this->selection = this->cursor;
            break;
        }
    }
    else if( e->type==SGUI_CHAR_EVENT && (this->num_entered<this->max_chars) )
    {   
        if( this->insert( this, strlen( e->arg.utf8 ), e->arg.utf8 ) )
        {
            this->selection = this->cursor;
            this->sync_cursors( this );
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

/****************************************************************************/

sgui_widget* sgui_edit_box_create( int x, int y, unsigned int width,
                                   unsigned int max_chars )
{
    sgui_edit_box* this = malloc( sizeof(sgui_edit_box) );
    sgui_widget* super = (sgui_widget*)this;

    if( !this )
        return NULL;

    memset( this, 0, sizeof(sgui_edit_box) );

    if( !sgui_edit_box_init( this, x, y, width, max_chars ) )
    {
        free( this );
        return NULL;
    }

    return super;
}

int sgui_edit_box_init( sgui_edit_box* this, int x, int y, unsigned int width,
                        unsigned int max_chars )
{
    sgui_widget* super = (sgui_widget*)this;
    sgui_skin* skin;

    if( !this )
        return 0;

    this->buffer = malloc( max_chars * 6 + 1 );

    if( !this->buffer )
        return 0;

    skin = sgui_skin_get( );
    sgui_widget_init( super, x, y, width, skin->get_edit_box_height( skin ) );

    super->window_event          = edit_box_on_event;
    super->destroy               = edit_box_destroy;
    super->draw                  = edit_box_draw;
    this->max_chars              = max_chars;
    this->buffer[0]              = '\0';
    this->insert                 = insert;
    this->remove_selection       = remove_selection;
    this->sync_cursors           = sync_cursors;
    this->offset_from_position   = cursor_from_mouse;
    this->text_changed           = edit_box_text_changed;
    super->flags                 = SGUI_FOCUS_ACCEPT|SGUI_FOCUS_DROP_ESC|
                                   SGUI_FOCUS_DROP_TAB|SGUI_WIDGET_VISIBLE;
    return 1;
}

const char* sgui_edit_box_get_text( sgui_widget* this )
{
    return this ? ((sgui_edit_box*)this)->buffer : NULL;
}

void sgui_edit_box_set_text( sgui_widget* super, const char* text )
{
    sgui_edit_box* this = (sgui_edit_box*)super;
    sgui_rect r;

    if( this )
    {
        sgui_internal_lock_mutex( );

        if( this->num_entered )
        {
            /* select all */
            this->selecting = this->cursor = 0;
            this->selection = this->end;
            this->sync_cursors( this );

            /* clear */
            this->remove_selection( this );
        }

        /* insert */
        if( text )
            this->insert( this, strlen(text), text );

        /* flag area dirty */
        this->cursor = this->offset = 0;
        this->sync_cursors( this );

        sgui_widget_get_absolute_rect( super, &r );
        sgui_canvas_add_dirty_rect( super->canvas, &r );

        sgui_internal_unlock_mutex( );
    }
}

