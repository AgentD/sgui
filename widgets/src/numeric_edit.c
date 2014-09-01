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
#include <stdio.h>
#include <ctype.h>



typedef struct
{
    sgui_edit_box super;

    int min, max, current;   /* minimum, maximum and current values */
}
sgui_numeric_edit;



static int charlen( int number )
{
    number = number<0 ? -number : number;
    if( number >= 1000000000 ) return 10;
    if( number >= 100000000  ) return 9;
    if( number >= 10000000   ) return 8;
    if( number >= 1000000    ) return 7;
    if( number >= 100000     ) return 6;
    if( number >= 10000      ) return 5;
    if( number >= 1000       ) return 4;
    if( number >= 100        ) return 3;
    if( number >= 10         ) return 2;
    return 1;
}

static int insert( sgui_edit_box* super, unsigned int len, const char* utf8 )
{
    sgui_numeric_edit* this = (sgui_numeric_edit*)super;
    char temp[ 128 ];
    unsigned int i;
    int val;

    for( i = utf8[0]=='-' ? 1 : 0; i<len; ++i )
    {
        if( !isdigit( utf8[i] ) )
            return 0;
    }

    /* clip length to maximum allowed characters */
    if( (len + super->num_entered) > super->max_chars )
        len = super->max_chars - super->num_entered;

    if( !len )
        return 0;

    /* stitch together */
    memcpy( temp, super->buffer, super->cursor );
    memcpy( temp+super->cursor, utf8, len );
    memcpy( temp+super->cursor+len, super->buffer+super->cursor,
            super->end - super->cursor );

    /* check */
    val = strtol( temp, NULL, 10 );

    if( val < this->min || val > this->max )
        return 0;

    /* insert */
    memcpy( super->buffer, temp, super->num_entered+len );

    this->current = val;
    super->num_entered += len;
    super->end += len;
    super->cursor += len;
    return 1;
}

/****************************************************************************/

sgui_widget* sgui_numeric_edit_create( int x, int y, unsigned int width,
                                       int min, int max, int current )
{
    sgui_numeric_edit* this = malloc( sizeof(sgui_numeric_edit) );
    sgui_edit_box* super = (sgui_edit_box*)this;
    unsigned int max_chars, minlen, maxlen;

    if( !this )
        return NULL;

    memset( this, 0, sizeof(sgui_numeric_edit) );

    /* determine maximum number of characters */
    minlen    = charlen( min );
    maxlen    = charlen( max );
    max_chars = MAX(minlen,maxlen);

    if( min<0 || max<0 )
        ++max_chars;

    current = MAX(current,min);
    current = MIN(current,max);

    /* init */
    if( !sgui_edit_box_init( (sgui_edit_box*)this, x, y, width, max_chars ) )
    {
        free( this );
        return NULL;
    }

    this->min = min;
    this->max = max;
    this->current = current;
    super->insert = insert;

    sprintf( super->buffer, "%d", current );
    super->end = super->num_entered = strlen( super->buffer );
    super->selecting=super->offset=super->selection=super->cursor = 0;
    return (sgui_widget*)this;
}

int sgui_numeric_edit_get_value( sgui_widget* this )
{
    return this ? ((sgui_numeric_edit*)this)->current : 0;
}

void sgui_numeric_edit_set_value( sgui_widget* box, int value )
{
    sgui_numeric_edit* this = (sgui_numeric_edit*)box;
    sgui_edit_box* super = (sgui_edit_box*)box;
    sgui_rect r;

    if( this )
    {
        value = MAX(this->min, value);
        value = MIN(this->max, value);

        sprintf( super->buffer, "%d", value );

        super->end = super->num_entered = strlen( super->buffer );
        super->selecting=super->offset=super->selection=super->cursor = 0;

        super->sync_cursors( super );

        this->current = value;

        /* flag area dirty */
        sgui_widget_get_absolute_rect( (sgui_widget*)this, &r );
        sgui_canvas_add_dirty_rect( ((sgui_widget*)this)->canvas, &r );
    }
}

