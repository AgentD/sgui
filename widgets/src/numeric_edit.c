/*
 * numeric_edit.c
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
#include "sgui_numeric_edit.h"
#include "sgui_internal.h"
#include "sgui_utf8.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>



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
            ++len;
            while( (utf8[ len ] & 0xC0) == 0x80 )
                ++len;
        }
    }

    /* sanity check */
    if( !len || !ulen )
        return 0;

    for( i=0; i<len; ++i )
    {
        if( !isdigit( utf8[i] ) )
            return 0;
    }

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

/****************************************************************************/

sgui_widget* sgui_numeric_edit_create( int x, int y, unsigned int width,
                                       unsigned int max_chars )
{
    sgui_edit_box* this = malloc( sizeof(sgui_edit_box) );

    if( !this )
        return NULL;

    memset( this, 0, sizeof(sgui_edit_box) );

    if( !sgui_edit_box_init( this, x, y, width, max_chars ) )
    {
        free( this );
        return NULL;
    }

    this->insert = insert;
    return (sgui_widget*)this;
}

