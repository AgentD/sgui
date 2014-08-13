/*
 * slider.c
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
#include "sgui_internal.h"
#include "sgui_slider.h"
#include "sgui_widget.h"
#include "sgui_canvas.h"
#include "sgui_event.h"

#include <stdlib.h>
#include <string.h>



typedef struct
{
    sgui_widget super;

    int min, max, value, steps, vertical, dragging;
    unsigned int draglen, border;
}
sgui_slider;



static void get_slider_rect( sgui_widget* super, sgui_rect* r )
{
    sgui_slider* this = (sgui_slider*)super;
    int i;

    if( this->steps )
    {
        i = ((this->steps-1)*(this->value-this->min)) / (this->max-this->min);
        i *= this->draglen / (this->steps-1);
    }
    else
    {
        i = this->draglen - 1;
        i = (i * (this->value - this->min)) / (this->max - this->min);
    }

    if( this->vertical )
    {
        r->left   = super->area.left;
        r->right  = super->area.right;
        r->top    = super->area.bottom - this->border - i;
        r->top    = MAX(r->top, super->area.top);
        r->bottom = r->top + this->border;
        r->bottom = MIN(r->bottom, super->area.bottom);
    }
    else
    {
        r->top    = super->area.top;
        r->bottom = super->area.bottom;
        r->left   = super->area.left + i;
        r->left   = MAX(r->left, super->area.left);
        r->right  = r->left + this->border;
        r->right  = MIN(r->right, super->area.right);
    }
}

static int value_from_position( sgui_widget* super, int x, int y )
{
    sgui_slider* this = (sgui_slider*)super;
    int i;

    i  = this->vertical ? (SGUI_RECT_HEIGHT(super->area) - 1 - y) : x;
    i -= this->border/2;
    i  = i<0 ? 0 : i;
    return this->min + ((this->max - this->min) * i)/this->draglen;
}

static void slider_on_event( sgui_widget* super, const sgui_event* e )
{
    sgui_slider* this = (sgui_slider*)super;
    int delta = this->steps ? ((this->max - this->min) / (this->steps-1)) : 1;
    int old_val = this->value;
    int new_val = this->value;
    sgui_event ev;
    sgui_rect r;

    switch( e->type )
    {
    case SGUI_KEY_PRESSED_EVENT:
        if( this->vertical )
        {
            if( e->arg.i==SGUI_KC_UP )
                new_val += delta;
            else if( e->arg.i==SGUI_KC_DOWN )
                new_val -= delta;
        }
        else
        {
            if( e->arg.i==SGUI_KC_RIGHT )
                new_val += delta;
            else if( e->arg.i==SGUI_KC_LEFT )
                new_val -= delta;
        }
        break;
    case SGUI_MOUSE_PRESS_EVENT:
        if( e->arg.i3.z!=SGUI_MOUSE_BUTTON_LEFT )
            break;
        get_slider_rect( super, &r );
        sgui_rect_add_offset( &r, -super->area.left, -super->area.top );

        if( sgui_rect_is_point_inside( &r, e->arg.i3.x, e->arg.i3.y ) )
            this->dragging = 1;
        else
            new_val = value_from_position( super, e->arg.i3.x, e->arg.i3.y );
        break;
    case SGUI_MOUSE_MOVE_EVENT:
        if( this->dragging )
            new_val = value_from_position( super, e->arg.i2.x, e->arg.i2.y );
        break;
    case SGUI_MOUSE_RELEASE_EVENT:
    case SGUI_MOUSE_LEAVE_EVENT:
        this->dragging = 0;
        break;
    case SGUI_MOUSE_WHEEL_EVENT:
        new_val = (e->arg.i > 0) ? (new_val + delta) : (new_val - delta);
        break;
    }

    /* set value and fire event if changed */
    sgui_slider_set_value( super, new_val );

    if( old_val != this->value )
    {
        ev.type       = SGUI_SLIDER_CHANGED_EVENT;
        ev.src.widget = super;
        ev.arg.i      = this->value;
        sgui_event_post( &ev );
    }
}

static void slider_draw( sgui_widget* super )
{
    sgui_slider* this = (sgui_slider*)super;

    sgui_skin_draw_slider( super->canvas, &super->area,
                           this->vertical, this->min, this->max,
                           this->value, this->steps );
}

static void slider_destroy( sgui_widget* this )
{
    free( this );
}

/****************************************************************************/

sgui_widget* sgui_slider_create( int x, int y, unsigned int length,
                                 int vertical, int min, int max,
                                 unsigned int steps )
{
    sgui_widget* super;
    sgui_slider* this;
    sgui_rect r;
    int temp;

    /* sanity check */
    if( min > max )
    {
        temp = min;
        min = max;
        max = temp;
    }

    if( min==max || length==0 )
        return NULL;

    /* create object */
    this = malloc( sizeof(sgui_slider) );
    super = (sgui_widget*)this;

    if( !this )
        return NULL;

    memset( this, 0, sizeof(sgui_slider) );

    /* initialize */
    sgui_skin_get_slider_extents( &r, vertical );

    if( vertical )
        sgui_widget_init( super, x, y, SGUI_RECT_WIDTH(r), length );
    else
        sgui_widget_init( super, x, y, length, SGUI_RECT_HEIGHT(r) );

    this->min      = min;
    this->max      = max;
    this->value    = min;
    this->steps    = steps;
    this->vertical = vertical;
    this->border   = vertical ? SGUI_RECT_HEIGHT(r) : SGUI_RECT_WIDTH(r);
    this->draglen  = length - this->border;

    super->window_event_callback = slider_on_event;
    super->draw_callback         = slider_draw;
    super->destroy               = slider_destroy;

    return super;
}

int sgui_slider_get_value( sgui_widget* this )
{
    return this ? ((sgui_slider*)this)->value : 0;
}

void sgui_slider_set_value( sgui_widget* super, int value )
{
    sgui_slider* this = (sgui_slider*)super;
    int delta, reminder;
    sgui_rect r;

    if( this )
    {
        /* sanitize value */
        if( this->steps )
        {
            delta = (this->max - this->min) / (this->steps - 1);
            reminder = value % delta;

            if( reminder < (delta/2) )
                value -= reminder;
            else
                value += delta - reminder;
        }

        value = MIN(value, this->max);
        value = MAX(value, this->min);

        /* if changed, set and request redraw */
        if( this->value != value )
        {
            this->value = value;
            sgui_widget_get_absolute_rect( super, &r );
            sgui_canvas_add_dirty_rect( this->super.canvas, &r );
        }
    }
}

