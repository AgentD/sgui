/*
 * color_dialog.c
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
#include "sgui_color_dialog.h"
#include "sgui_color_picker.h"
#include "sgui_numeric_edit.h"
#include "sgui_window.h"
#include "sgui_button.h"
#include "sgui_label.h"
#include "sgui_event.h"
#include "sgui_skin.h"

#include <stdlib.h>
#include <string.h>



#ifndef SGUI_NO_COLOR_DIALOG
struct sgui_color_dialog
{
    sgui_window* window;

    sgui_widget* picker;

    sgui_widget *spin_h, *spin_s, *spin_v;
    sgui_widget *spin_r, *spin_g, *spin_b;

    sgui_widget *label_h, *label_s, *label_v;
    sgui_widget *label_r, *label_g, *label_b;

    sgui_widget *spin_a, *label_a;

    sgui_widget *button_accept, *button_reject;
};



static void update_hsv_from_spinbox( sgui_color_dialog* this )
{
    unsigned char hsva[4];

    hsva[0] = sgui_numeric_edit_get_value( this->spin_h );
    hsva[1] = sgui_numeric_edit_get_value( this->spin_s );
    hsva[2] = sgui_numeric_edit_get_value( this->spin_v );
    hsva[3] = sgui_numeric_edit_get_value( this->spin_a );
    sgui_color_dialog_set_hsva( this, hsva );
}

static void update_rgb_from_spinbox( sgui_color_dialog* this )
{
    unsigned char rgba[4];

    rgba[0] = sgui_numeric_edit_get_value( this->spin_r );
    rgba[1] = sgui_numeric_edit_get_value( this->spin_g );
    rgba[2] = sgui_numeric_edit_get_value( this->spin_b );
    rgba[3] = sgui_numeric_edit_get_value( this->spin_a );
    sgui_color_dialog_set_rgba( this, rgba );
}

static void on_accept( sgui_color_dialog* this )
{
    sgui_event ev;

    ev.src.other = this;
    ev.type = SGUI_COLOR_SELECTED_RGBA_EVENT;
    sgui_color_picker_get_rgb( this->picker, ev.arg.color );
    sgui_event_post( &ev );

    ev.src.other = this;
    ev.type = SGUI_COLOR_SELECTED_HSVA_EVENT;
    sgui_color_picker_get_hsv( this->picker, ev.arg.color );
    sgui_event_post( &ev );

    sgui_window_set_visible( this->window, SGUI_INVISIBLE );
}

static void on_reject( sgui_color_dialog* this )
{
    sgui_event ev;

    ev.src.other = this;
    ev.type = SGUI_DIALOG_REJECTED;
    sgui_event_post( &ev );

    sgui_window_set_visible( this->window, SGUI_INVISIBLE );
}

/****************************************************************************/

static int add_spinbox( sgui_widget** box, sgui_widget** label,
                        unsigned int width, unsigned int* height,
                        int x, int y, const char* caption )
{
    unsigned int fh = sgui_skin_get( )->font_height, wh;
    unsigned int textlen = strlen( caption ) * fh;

    *box = sgui_spin_box_create(x+textlen,y,width-textlen,0,0xFF,0,1,1);

    if( !*box )
        return 0;

    sgui_widget_get_size( *box, NULL, &wh );

    if( wh < fh )
    {
        sgui_widget_set_position( *box, x+textlen, y+(fh-wh)/2 );
        *height = fh;
    }
    else
    {
        y += (wh-fh)/2;
        *height = wh;
    }

    *label = sgui_label_create( x, y, caption );

    return (*label != NULL);
}

/****************************************************************************/

sgui_color_dialog* sgui_color_dialog_create( const char* caption,
                                             const char* accept,
                                             const char* reject )
{
    sgui_color_dialog* this = malloc( sizeof(sgui_color_dialog) );
    unsigned int x, y, w, h, ws;
    unsigned char color[4];
    sgui_rect r, wr;

    if( !this )
        return NULL;

    memset( this, 0, sizeof(sgui_color_dialog) );

    /* dialog window */
    this->window = sgui_window_create( NULL, 640, 480, SGUI_FIXED_SIZE );

    if( !this->window )
    {
        free( this );
        return NULL;
    }

    sgui_window_set_title( this->window, caption );

    /* color picker */
    if( !(this->picker = sgui_color_picker_create( 10, 10 )) )
        goto fail;

    sgui_widget_get_size( this->picker, &w, &h );

    /* spin boxes */
    x = 10; y = h + 20;
    if(!add_spinbox(&this->spin_h,&this->label_h,100,&ws,x,y,"H:"))goto fail;
    y += ws+5;
    if(!add_spinbox(&this->spin_s,&this->label_s,100,&ws,x,y,"S:"))goto fail;
    y += ws+5;
    if(!add_spinbox(&this->spin_v,&this->label_v,100,&ws,x,y,"V:"))goto fail;

    x += 120; y = h + 20;
    if(!add_spinbox(&this->spin_r,&this->label_r,100,&ws,x,y,"R:"))goto fail;
    y += ws+5;
    if(!add_spinbox(&this->spin_g,&this->label_g,100,&ws,x,y,"G:"))goto fail;
    y += ws+5;
    if(!add_spinbox(&this->spin_b,&this->label_b,100,&ws,x,y,"B:"))goto fail;

    x += 120; y = h + 20;
    if(!add_spinbox(&this->spin_a,&this->label_a,100,&ws,x,y,"A:"))goto fail;

    /* calculate window size */
    sgui_rect_set_size( &r, 0, 0, 0, 0 );

    sgui_widget_get_rect( this->picker, &wr );
    sgui_rect_join( &r, &wr, 0 );
    sgui_widget_get_rect( this->spin_v, &wr );
    sgui_rect_join( &r, &wr, 0 );
    sgui_widget_get_rect( this->label_v, &wr );
    sgui_rect_join( &r, &wr, 0 );
    sgui_widget_get_rect( this->spin_b, &wr );
    sgui_rect_join( &r, &wr, 0 );
    sgui_widget_get_rect( this->label_b, &wr );
    sgui_rect_join( &r, &wr, 0 );
    sgui_widget_get_rect( this->spin_a, &wr );
    sgui_rect_join( &r, &wr, 0 );
    sgui_widget_get_rect( this->label_a, &wr );
    sgui_rect_join( &r, &wr, 0 );

    w = SGUI_RECT_WIDTH(r)+10;
    h = SGUI_RECT_HEIGHT(r)+10;

    /* buttons */
    ws = sgui_skin_default_font_extents( reject, -1, 0, 0 );

    x = r.right-ws-10;
    y = r.bottom+10;
    this->button_reject = sgui_button_create( x, y, ws+10, 30, reject, 0 );
    if( !this->button_reject )
        goto fail;

    ws = sgui_skin_default_font_extents( accept, -1, 0, 0 );
    x -= ws+15;
    this->button_accept = sgui_button_create( x, y, ws+10, 30, accept, 0 );
    if( !this->button_accept )
        goto fail;

    h += 40;

    /* add widgets */
    sgui_window_add_widget( this->window, this->picker );
    sgui_window_add_widget( this->window, this->spin_h );
    sgui_window_add_widget( this->window, this->spin_s );
    sgui_window_add_widget( this->window, this->spin_v );
    sgui_window_add_widget( this->window, this->spin_r );
    sgui_window_add_widget( this->window, this->spin_g );
    sgui_window_add_widget( this->window, this->spin_b );
    sgui_window_add_widget( this->window, this->label_h );
    sgui_window_add_widget( this->window, this->label_s );
    sgui_window_add_widget( this->window, this->label_v );
    sgui_window_add_widget( this->window, this->label_r );
    sgui_window_add_widget( this->window, this->label_g );
    sgui_window_add_widget( this->window, this->label_b );
    sgui_window_add_widget( this->window, this->spin_a );
    sgui_window_add_widget( this->window, this->label_a );
    sgui_window_add_widget( this->window, this->button_accept );
    sgui_window_add_widget( this->window, this->button_reject );

    /* reshape window */
    sgui_window_set_size( this->window, w, h );

    /* events */
    sgui_event_connect( this->picker, SGUI_HSVA_CHANGED_EVENT,
                        sgui_color_dialog_set_hsva, this,
                        SGUI_FROM_EVENT, SGUI_COLOR );
    sgui_event_connect( this->picker, SGUI_RGBA_CHANGED_EVENT,
                        sgui_color_dialog_set_rgba, this,
                        SGUI_FROM_EVENT, SGUI_COLOR );
    sgui_event_connect( this->spin_h, SGUI_EDIT_VALUE_CHANGED,
                        update_hsv_from_spinbox, this, SGUI_VOID );
    sgui_event_connect( this->spin_s, SGUI_EDIT_VALUE_CHANGED,
                        update_hsv_from_spinbox, this, SGUI_VOID );
    sgui_event_connect( this->spin_v, SGUI_EDIT_VALUE_CHANGED,
                        update_hsv_from_spinbox, this, SGUI_VOID );
    sgui_event_connect( this->spin_a, SGUI_EDIT_VALUE_CHANGED,
                        update_hsv_from_spinbox, this, SGUI_VOID );
    sgui_event_connect( this->spin_r, SGUI_EDIT_VALUE_CHANGED,
                        update_rgb_from_spinbox, this, SGUI_VOID );
    sgui_event_connect( this->spin_g, SGUI_EDIT_VALUE_CHANGED,
                        update_rgb_from_spinbox, this, SGUI_VOID );
    sgui_event_connect( this->spin_b, SGUI_EDIT_VALUE_CHANGED,
                        update_rgb_from_spinbox, this, SGUI_VOID );
    sgui_event_connect( this->button_accept, SGUI_BUTTON_OUT_EVENT,
                        on_accept, this, SGUI_VOID );
    sgui_event_connect( this->button_reject, SGUI_BUTTON_OUT_EVENT,
                        on_reject, this, SGUI_VOID );

    /* init */
    sgui_color_picker_get_hsv( this->picker, color );
    sgui_color_dialog_set_hsva( this, color );

    return this;
fail:
    sgui_color_dialog_destroy( this );
    return NULL;
}

void sgui_color_dialog_destroy( sgui_color_dialog* this )
{
    if( this )
    {
        sgui_event_disconnect(this->picker, SGUI_HSVA_CHANGED_EVENT,
                              (sgui_function)sgui_color_dialog_set_hsva,this);
        sgui_event_disconnect(this->picker, SGUI_RGBA_CHANGED_EVENT,
                              (sgui_function)sgui_color_dialog_set_rgba,this);
        sgui_event_disconnect(this->spin_h, SGUI_EDIT_VALUE_CHANGED,
                              (sgui_function)update_hsv_from_spinbox,this);
        sgui_event_disconnect(this->spin_s, SGUI_EDIT_VALUE_CHANGED,
                              (sgui_function)update_hsv_from_spinbox,this);
        sgui_event_disconnect(this->spin_v, SGUI_EDIT_VALUE_CHANGED,
                              (sgui_function)update_hsv_from_spinbox,this);
        sgui_event_disconnect(this->spin_a, SGUI_EDIT_VALUE_CHANGED,
                              (sgui_function)update_hsv_from_spinbox,this);
        sgui_event_disconnect(this->spin_r, SGUI_EDIT_VALUE_CHANGED,
                              (sgui_function)update_rgb_from_spinbox,this);
        sgui_event_disconnect(this->spin_g, SGUI_EDIT_VALUE_CHANGED,
                              (sgui_function)update_rgb_from_spinbox,this);
        sgui_event_disconnect(this->spin_b, SGUI_EDIT_VALUE_CHANGED,
                              (sgui_function)update_rgb_from_spinbox,this);
        sgui_event_disconnect( this->button_accept, SGUI_BUTTON_OUT_EVENT,
                               (sgui_function)on_accept, this );
        sgui_event_disconnect( this->button_reject, SGUI_BUTTON_OUT_EVENT,
                               (sgui_function)on_reject, this );

        sgui_window_destroy( this->window );

        sgui_widget_destroy( this->picker );
        sgui_widget_destroy( this->spin_h );
        sgui_widget_destroy( this->spin_s );
        sgui_widget_destroy( this->spin_v );
        sgui_widget_destroy( this->spin_r );
        sgui_widget_destroy( this->spin_g );
        sgui_widget_destroy( this->spin_b );
        sgui_widget_destroy( this->label_h );
        sgui_widget_destroy( this->label_s );
        sgui_widget_destroy( this->label_v );
        sgui_widget_destroy( this->label_r );
        sgui_widget_destroy( this->label_g );
        sgui_widget_destroy( this->label_b );
        sgui_widget_destroy( this->spin_a );
        sgui_widget_destroy( this->label_a );
        sgui_widget_destroy( this->button_accept );
        sgui_widget_destroy( this->button_reject );

        free( this );
    }
}

void sgui_color_dialog_display( sgui_color_dialog* this )
{
    if( this )
    {
        sgui_window_set_visible( this->window, SGUI_VISIBLE );
        sgui_window_move_center( this->window );
    }
}

void sgui_color_dialog_set_rgba( sgui_color_dialog* this,
                                 const unsigned char* rgba )
{
    unsigned char hsva[4];

    if( !this || !rgba )
        return;

    sgui_numeric_edit_set_value( this->spin_r, rgba[0] );
    sgui_numeric_edit_set_value( this->spin_g, rgba[1] );
    sgui_numeric_edit_set_value( this->spin_b, rgba[2] );
    sgui_numeric_edit_set_value( this->spin_a, rgba[3] );

    sgui_color_picker_set_rgb( this->picker, rgba );
    sgui_color_picker_get_hsv( this->picker, hsva );

    sgui_numeric_edit_set_value( this->spin_h, hsva[0] );
    sgui_numeric_edit_set_value( this->spin_s, hsva[1] );
    sgui_numeric_edit_set_value( this->spin_v, hsva[2] );
}

void sgui_color_dialog_set_hsva( sgui_color_dialog* this,
                                 const unsigned char* hsva )
{
    unsigned char rgba[4];

    if( !this || !hsva )
        return;

    sgui_numeric_edit_set_value( this->spin_h, hsva[0] );
    sgui_numeric_edit_set_value( this->spin_s, hsva[1] );
    sgui_numeric_edit_set_value( this->spin_v, hsva[2] );
    sgui_numeric_edit_set_value( this->spin_a, hsva[3] );

    sgui_color_picker_set_hsv( this->picker, hsva );
    sgui_color_picker_get_rgb( this->picker, rgba );

    sgui_numeric_edit_set_value( this->spin_r, rgba[0] );
    sgui_numeric_edit_set_value( this->spin_g, rgba[1] );
    sgui_numeric_edit_set_value( this->spin_b, rgba[2] );
}
#elif defined(SGUI_NOP_IMPLEMENTATIONS)
sgui_color_dialog* sgui_color_dialog_create( const char* caption,
                                             const char* accept,
                                             const char* reject )
{
    (void)caption; (void)accept; (void)reject;
    return NULL;
}
void sgui_color_dialog_destroy( sgui_color_dialog* this )
{
    (void)this;
}
void sgui_color_dialog_display( sgui_color_dialog* this )
{
    (void)this;
}
void sgui_color_dialog_set_rgba( sgui_color_dialog* this,
                                 const unsigned char* rgba )
{
    (void)this; (void)rgba;
}
void sgui_color_dialog_set_hsva( sgui_color_dialog* this,
                                 const unsigned char* hsva )
{
    (void)this; (void)hsva;
}
#endif

