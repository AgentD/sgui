/*
 * dialog.c
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
#include "sgui_dialog.h"
#include "sgui_window.h"
#include "sgui_widget.h"
#include "sgui_button.h"
#include "sgui_event.h"
#include "sgui_skin.h"
#include "sgui_rect.h"



void sgui_dialog_destroy( sgui_dialog* this )
{
    if( this )
    {
        if( this->b0 )
        {
            sgui_event_disconnect( this->b0, SGUI_BUTTON_OUT_EVENT,
                                   (sgui_function)this->handle_button, this );
        }
        if( this->b1 )
        {
            sgui_event_disconnect( this->b1, SGUI_BUTTON_OUT_EVENT,
                                   (sgui_function)this->handle_button, this );
        }
        if( this->b2 )
        {
            sgui_event_disconnect( this->b2, SGUI_BUTTON_OUT_EVENT,
                                   (sgui_function)this->handle_button, this );
        }

        sgui_event_disconnect( this->window, SGUI_USER_CLOSED_EVENT,
                               (sgui_function)this->handle_button, this );

        sgui_widget_remove_from_parent( this->b0 );
        sgui_widget_remove_from_parent( this->b1 );
        sgui_widget_remove_from_parent( this->b2 );
        sgui_widget_destroy( this->b0 );
        sgui_widget_destroy( this->b1 );
        sgui_widget_destroy( this->b2 );
        sgui_window_destroy( this->window );
        this->b0 = this->b1 = this->b2 = 0;
        this->window = 0;
        this->destroy( this );
    }
}

void sgui_dialog_display( sgui_dialog* this )
{
    if( this )
    {
        sgui_window_set_visible( this->window, SGUI_VISIBLE );
        sgui_window_move_center( this->window );
    }
}

int sgui_dialog_init( sgui_dialog* this,
                      const char* button0, const char* button1,
                      const char* button2, int allignment )
{
    unsigned int total_width=0, total_height=0, height, w, h, x, y, count=0;
    sgui_rect r0, r1, r2;

    if( !this || !(button0 || button1 || button2) )
        return 0;

    if( this->b0 || this->b1 || this->b2 )
        return 0;

    /* compute text dimensions, adjust window size */
    if( button0 )
    {
        sgui_skin_get_text_extents( button0, &r0 );
        height = SGUI_RECT_HEIGHT(r0);
        ++count;
        total_height = MAX(height,total_height);
        total_width += SGUI_RECT_WIDTH(r0) + 10;
    }

    if( button1 )
    {
        sgui_skin_get_text_extents( button1, &r1 );
        height = SGUI_RECT_HEIGHT(r1);
        ++count;
        total_height = MAX(height,total_height);
        total_width += SGUI_RECT_WIDTH(r1) + 10;
    }

    if( button2 )
    {
        sgui_skin_get_text_extents( button2, &r2 );
        height = SGUI_RECT_HEIGHT(r2);
        ++count;
        total_height = MAX(height,total_height);
        total_width += SGUI_RECT_WIDTH(r2) + 10;
    }

    total_width += (count-1)*5;

    sgui_window_get_size( this->window, &w, &h );

    if( total_width > w )
        w = total_width + 20;

    sgui_window_set_size( this->window, w, h+total_height+15 );

    /* compute starting positions */
    y = h + 5;

    switch( allignment )
    {
    case SGUI_LEFT:   x = 5;                   break;
    case SGUI_CENTER: x = (w - total_width)/2; break;
    case SGUI_RIGHT:  x = w - 5 - total_width; break;
    default:                                   return 0;
    }

    /* create buttons */
    if( button0 )
    {
        this->b0 = sgui_button_create( 0, 0, SGUI_RECT_WIDTH(r0)+10,
                                       total_height, button0, 0 );

        if( !this->b0 )
            return 0;
    }

    if( button1 )
    {
        this->b1 = sgui_button_create( 0, 0, SGUI_RECT_WIDTH(r1)+10,
                                       total_height, button1, 0 );

        if( !this->b1 )
        {
            sgui_widget_destroy( this->b0 );
            return 0;
        }
    }

    if( button2 )
    {
        this->b2 = sgui_button_create( 0, 0, SGUI_RECT_WIDTH(r2)+10,
                                       total_height, button2, 0 );

        if( !this->b2 )
        {
            sgui_widget_destroy( this->b0 );
            sgui_widget_destroy( this->b1 );
            return 0;
        }
    }

    /* add and connect the buttons */
    if( button0 )
    {
        sgui_widget_set_position( this->b0, x, y );
        sgui_window_add_widget( this->window, this->b0 );
        x += SGUI_RECT_WIDTH(r0) + 15;
        sgui_event_connect( this->b0, SGUI_BUTTON_OUT_EVENT,
                            this->handle_button, this, SGUI_INT, 0 );
    }
    if( button1 )
    {
        sgui_widget_set_position( this->b1, x, y );
        sgui_window_add_widget( this->window, this->b1 );
        x += SGUI_RECT_WIDTH(r1) + 15;
        sgui_event_connect( this->b1, SGUI_BUTTON_OUT_EVENT,
                            this->handle_button, this, SGUI_INT, 1 );
    }
    if( button2 )
    {
        sgui_widget_set_position( this->b2, x, y );
        sgui_window_add_widget( this->window, this->b2 );
        sgui_event_connect( this->b2, SGUI_BUTTON_OUT_EVENT,
                            this->handle_button, this, SGUI_INT, 2 );
    }

    /* connect window closing event */
    sgui_event_connect( this->window, SGUI_USER_CLOSED_EVENT,
                        this->handle_button, this, SGUI_INT, -1 );
    return 1;
}

