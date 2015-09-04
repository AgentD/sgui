/*
 * pass_box.c
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
#include "sgui_pass_box.h"
#include "sgui_internal.h"
#include "sgui_utf8.h"

#include <stdlib.h>
#include <string.h>

#define PASSWD_DISPLAY '*'



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
sgui_pass_box;



static int insert( sgui_edit_box* super, unsigned int len, const char* utf8 )
{
    sgui_pass_box* this = (sgui_pass_box*)super;
    unsigned int i, ulen;

    /* clip length to maximum allowed characters */
    ulen = sgui_utf8_strlen( utf8 );

    if( (ulen+super->num_entered) > super->max_chars )
    {
        ulen = super->max_chars - super->num_entered;

        for( len=0, i=0; i<ulen && utf8[len]; ++i )
        {
            ++len;
            while( (utf8[ len ] & 0xC0) == 0x80 )
                ++len;
        }
    }

    /* sanity check */
    if( !len || !ulen )
        return 0;

    super->remove_selection( super );

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

    super->num_entered += ulen;
    super->end += len;
    super->cursor += len;
    return 1;
}

static void remove_selection( sgui_edit_box* super )
{
    sgui_pass_box* this = (sgui_pass_box*)super;
    int start = MIN( super->cursor, super->selection );
    int end = MAX( super->cursor, super->selection );
    int i, len;

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

    start = MIN( this->cursor, this->selection );
    end = MAX( this->cursor, this->selection );

    memmove( this->shadow+start, this->shadow+end, this->end-end+1 );
    this->end -= (end - start);
    this->selection = this->cursor = start;
}

static void sync_cursors( sgui_edit_box* super )
{
    sgui_pass_box* this = (sgui_pass_box*)super;
    unsigned int i;

    for( this->cursor=0, i=0; i<super->cursor; ++i )
    {
        ++this->cursor;
        while( (this->shadow[ this->cursor ] & 0xC0) == 0x80 )
            ++this->cursor;
    }

    for( this->selection=0, i=0; i<super->selection; ++i )
    {
        ++this->selection;
        while( (this->shadow[ this->selection ] & 0xC0) == 0x80 )
            ++this->selection;
    }
}

static void pass_box_destroy( sgui_widget* this )
{
    free( ((sgui_pass_box*)this)->shadow );
    free( ((sgui_edit_box*)this)->buffer );
    free( this );
}

/****************************************************************************/

sgui_widget* sgui_pass_box_create( int x, int y, unsigned int width,
                                   unsigned int max_chars )
{
    sgui_pass_box* this = malloc( sizeof(sgui_pass_box) );
    sgui_edit_box* super = (sgui_edit_box*)this;

    if( !this )
        return NULL;

    memset( this, 0, sizeof(sgui_pass_box) );

    /* allocate storage for the text buffer */
    if( !sgui_edit_box_init( super, x, y, width, max_chars ) )
    {
        free( this );
        return NULL;
    }

    /* allocate shadow buffer for password edit box */
    this->shadow = super->buffer;
    super->buffer = malloc( max_chars + 1 );

    if( !super->buffer )
    {
        free( this->shadow );
        free( this );
        return NULL;
    }

    ((sgui_widget*)this)->destroy = pass_box_destroy;
    super->sync_cursors = sync_cursors;
    super->insert = insert;
    super->remove_selection = remove_selection;
    this->shadow[0] = '\0';
    return (sgui_widget*)this;
}

const char* sgui_pass_box_get_text( sgui_widget* this )
{
    return ((sgui_pass_box*)this)->shadow;
}

