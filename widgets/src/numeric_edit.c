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
#include "sgui_event.h"
#include "sgui_utf8.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>



typedef struct
{
    sgui_edit_box super;

    int min, max;   /* minimum and maximum values */
}
sgui_numeric_edit;

typedef struct
{
    sgui_numeric_edit super;

    sgui_rect up, down; /* up and down buttons */
    int step;           /* step size */

    void (* on_event )( sgui_widget* widget, const sgui_event* event );
}
sgui_spin_box;



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
    char temp[ 128 ], *end;
    unsigned int i;
    int val;

    for( i = utf8[0]=='-' ? 1 : 0; i<len; ++i )
    {
        if( !isdigit( utf8[i] ) )
            return 0;
    }

    if( (len + super->num_entered) > super->max_chars )
        return 0;

    /* stitch together */
    memcpy( temp, super->buffer, super->cursor );
    memcpy( temp+super->cursor, utf8, len );
    strcpy( temp+super->cursor+len, super->buffer+super->cursor );

    /* check */
    val = strtol( temp, &end, 10 );

    if( val < this->min || val > this->max || *end )
        return 0;

    /* insert */
    strcpy( super->buffer, temp );

    super->num_entered += len;
    super->end += len;
    super->cursor += len;
    return 1;
}

static void remove_selection( sgui_edit_box* super )
{
    sgui_numeric_edit* this = (sgui_numeric_edit*)super;
    int start = MIN( super->cursor, super->selection );
    int end = MAX( super->cursor, super->selection );
    char temp[ 128 ];
    int val;

    memcpy( temp, super->buffer, start );
    strcpy( temp+start, super->buffer+end );

    val = strtol( temp, NULL, 10 );

    if( val >= this->min && val <= this->max )
    {
        strcpy( super->buffer, temp );

        super->num_entered -= (end - start);
        super->end -= (end - start);
        super->selection = super->cursor = start;
    }
}

static void numeric_edit_text_changed( sgui_edit_box* this, int type )
{
    sgui_event se;
    (void)type;

    se.src.widget = (sgui_widget*)this;
    se.arg.i = sgui_numeric_edit_get_value( (sgui_widget*)this );
    se.type = SGUI_EDIT_VALUE_CHANGED;

    sgui_event_post( &se );
}

static unsigned int offset_from_position( sgui_edit_box* this, int x )
{
    unsigned int len = 0, cur = this->offset;
    sgui_skin* skin = sgui_skin_get( );

    x -= skin->get_edit_box_border_width( skin );
    x -= SGUI_RECT_WIDTH(this->super.area) -
         sgui_skin_default_font_extents( this->buffer, -1, 0, 0 );

    /* move 'cur' to the right until it the text extents from
       the beginning to 'cur' catch up with the mouse offset */
    while( (len < (unsigned int)x) && (cur < this->end) )
    {
        ++cur;

        /* get the text extents from the rendering offset to 'cur' */
        len = sgui_skin_default_font_extents( this->buffer+this->offset,
                                              cur         -this->offset,
                                              0, 0 );
    }

    return cur;
}

static void numeric_edit_draw( sgui_widget* super )
{
    sgui_edit_box* this = (sgui_edit_box*)super;
    sgui_skin* skin = sgui_skin_get( );

    skin->draw_editbox( skin, super->canvas, &(super->area),
                              this->buffer, this->offset,
                              this->flags & SGUI_EDIT_DRAW_CURSOR ?
                              (int)this->cursor : -1,
                              this->selection, 1, 0 );
}

static void spin_box_draw( sgui_widget* super )
{
    sgui_edit_box* this = (sgui_edit_box*)super;
    sgui_skin* skin = sgui_skin_get( );

    skin->draw_editbox( skin, super->canvas, &(super->area),
                              this->buffer, this->offset,
                              this->flags & SGUI_EDIT_DRAW_CURSOR ?
                              (int)this->cursor : -1,
                              this->selection, 1, 1 );
}

static void spin_box_on_event( sgui_widget* super, const sgui_event* event )
{
    sgui_spin_box* this = (sgui_spin_box*)super;
    int value;

    switch( event->type )
    {
    case SGUI_MOUSE_MOVE_EVENT:
        if( sgui_rect_is_point_inside( &this->up, event->arg.i2.x,
                                                  event->arg.i2.y ) )
            return;
        if( sgui_rect_is_point_inside( &this->down, event->arg.i2.x,
                                                    event->arg.i2.y ) )
            return;
        break;
    case SGUI_MOUSE_PRESS_EVENT:
        if( sgui_rect_is_point_inside( &this->up, event->arg.i3.x,
                                                  event->arg.i3.y ) )
            return;
        if( sgui_rect_is_point_inside( &this->down, event->arg.i3.x,
                                                    event->arg.i3.y ) )
            return;
        break;
    case SGUI_MOUSE_RELEASE_EVENT:
        if( sgui_rect_is_point_inside( &this->up, event->arg.i3.x,
                                                  event->arg.i3.y ) )
        {
            value = sgui_numeric_edit_get_value( super );
            sgui_numeric_edit_set_value( super, value + this->step );
            numeric_edit_text_changed( (sgui_edit_box*)this, 0 );
            return;
        }
        if( sgui_rect_is_point_inside( &this->down, event->arg.i3.x,
                                                    event->arg.i3.y ) )
        {
            value = sgui_numeric_edit_get_value( super );
            sgui_numeric_edit_set_value( super, value - this->step );
            numeric_edit_text_changed( (sgui_edit_box*)this, 0 );
            return;
        }
        break;
    case SGUI_MOUSE_WHEEL_EVENT:
        value = sgui_numeric_edit_get_value( super );
        value = event->arg.i>0 ? (value + this->step) : (value - this->step);
        sgui_numeric_edit_set_value( super, value );
        numeric_edit_text_changed( (sgui_edit_box*)this, 0 );
        return;
    }

    if( this->on_event )
        this->on_event( super, event );
}

static int numeric_edit_init( sgui_numeric_edit* this,
                              int x, int y, unsigned int width,
                              int min, int max, int current )
{
    unsigned int minlen = charlen( min ), maxlen = charlen( max );
    unsigned int max_chars = MAX(minlen,maxlen);
    sgui_edit_box* super = (sgui_edit_box*)this;

    if( min<0 || max<0 )
        ++max_chars;

    current = MAX(current,min);
    current = MIN(current,max);

    if( !sgui_edit_box_init( (sgui_edit_box*)this, x, y, width, max_chars ) )
        return 0;

    this->min = min;
    this->max = max;
    super->insert = insert;
    super->remove_selection = remove_selection;
    super->text_changed = numeric_edit_text_changed;
    super->offset_from_position = offset_from_position;

    ((sgui_widget*)this)->draw = numeric_edit_draw;

    sprintf( super->buffer, "%d", current );
    super->end = super->num_entered = strlen( super->buffer );
    super->flags &= ~SGUI_EDIT_SELECTING; 
    super->offset = super->selection = super->cursor = 0;
    return 1;
}

/****************************************************************************/

sgui_widget* sgui_numeric_edit_create( int x, int y, unsigned int width,
                                       int min, int max, int current )
{
    sgui_numeric_edit* this = malloc( sizeof(sgui_numeric_edit) );

    if( this )
    {
        memset( this, 0, sizeof(sgui_numeric_edit) );

        if( !numeric_edit_init( this, x, y, width, min, max, current ) )
        {
            free( this );
            return NULL;
        }
    }
    return (sgui_widget*)this;
}

sgui_widget* sgui_spin_box_create( int x, int y, unsigned int width,
                                   int min, int max, int current,
                                   unsigned int stepsize, int editable )
{
    sgui_spin_box* this = malloc( sizeof(sgui_spin_box) );
    sgui_numeric_edit* super = (sgui_numeric_edit*)this;
    sgui_skin* skin;

    if( this )
    {
        memset( this, 0, sizeof(sgui_spin_box) );

        if( !numeric_edit_init( super, x, y, width, min, max, current ) )
        {
            free( this );
            return NULL;
        }

        ((sgui_widget*)this)->draw = spin_box_draw;

        skin = sgui_skin_get( );
        skin->get_spin_buttons( skin, &this->up, &this->down );
        this->step     = stepsize>0 ? stepsize : 1;

        if( editable )
            this->on_event = ((sgui_widget*)this)->window_event;

        ((sgui_widget*)this)->window_event = spin_box_on_event;
    }
    return (sgui_widget*)this;
}

int sgui_numeric_edit_get_value( sgui_widget* this )
{
    return this ? strtol( ((sgui_edit_box*)this)->buffer, NULL, 10 ) : 0;
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
        super->flags &= ~SGUI_EDIT_SELECTING;
        super->offset = super->selection = super->cursor = 0;

        super->sync_cursors( super );

        /* flag area dirty */
        sgui_widget_get_absolute_rect( (sgui_widget*)this, &r );
        sgui_canvas_add_dirty_rect( ((sgui_widget*)this)->canvas, &r );
    }
}

