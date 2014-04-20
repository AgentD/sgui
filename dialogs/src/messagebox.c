/*
 * messagebox.c
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
#include "sgui_static_text.h"
#include "sgui_messagebox.h"
#include "sgui_internal.h"
#include "sgui_button.h"
#include "sgui_widget.h"
#include "sgui_image.h"
#include "sgui_event.h"
#include "sgui_rect.h"

#include <stdlib.h>
#include <string.h>



#define ICON_WIDTH  32
#define ICON_HEIGHT 32



static const unsigned char colormap[4*7] =
{
    0x00, 0x00, 0x00, 0x00,     /* fully transparent */
    0x00, 0x00, 0x00, 0xFF,     /* black */
    0xFF, 0xFF, 0xFF, 0x80,     /* semi transparent white */
    0xFF, 0xFF, 0xFF, 0xFF,     /* white */
    0x00, 0x00, 0xFF, 0xFF,     /* blue */
    0xFF, 0xFF, 0x00, 0xFF,     /* yellow */
    0xFF, 0x00, 0x00, 0xFF      /* red */
};

static const unsigned char info[ (ICON_WIDTH/2)*ICON_HEIGHT ] =
{
    000,000,000,000,000,001,011,011,011,010,000,000,000,000,000,000,
    000,000,000,000,011,013,033,033,033,031,011,000,000,000,000,000,
    000,000,000,011,033,033,033,033,033,033,033,011,000,000,000,000,
    000,000,001,033,033,033,033,033,033,033,033,033,010,000,000,000,
    000,000,013,033,033,033,034,044,043,033,033,033,031,000,000,000,
    000,001,033,033,033,033,044,044,044,033,033,033,033,010,000,000,
    000,013,033,033,033,033,044,044,044,033,033,033,033,031,000,000,
    001,033,033,033,033,033,034,044,043,033,033,033,033,033,010,000,
    001,033,033,033,033,033,033,033,033,033,033,033,033,033,012,000,
    013,033,033,033,033,033,033,033,033,033,033,033,033,033,031,020,
    013,033,033,033,033,034,044,044,044,033,033,033,033,033,031,020,
    013,033,033,033,033,033,034,044,044,033,033,033,033,033,031,022,
    013,033,033,033,033,033,034,044,044,033,033,033,033,033,031,022,
    013,033,033,033,033,033,034,044,044,033,033,033,033,033,031,022,
    013,033,033,033,033,033,034,044,044,033,033,033,033,033,031,022,
    013,033,033,033,033,033,034,044,044,033,033,033,033,033,031,022,
    001,033,033,033,033,033,034,044,044,033,033,033,033,033,012,022,
    001,033,033,033,033,033,034,044,044,033,033,033,033,033,012,022,
    000,013,033,033,033,033,034,044,044,033,033,033,033,031,022,020,
    000,001,033,033,033,034,044,044,044,044,033,033,033,012,022,020,
    000,000,013,033,033,033,033,033,033,033,033,033,031,022,022,000,
    000,000,001,033,033,033,033,033,033,033,033,033,012,022,020,000,
    000,000,000,011,033,033,033,033,033,033,033,011,022,022,000,000,
    000,000,000,002,011,013,033,033,033,031,011,022,022,020,000,000,
    000,000,000,000,022,021,011,033,033,012,022,022,022,000,000,000,
    000,000,000,000,000,022,022,013,033,012,022,022,000,000,000,000,
    000,000,000,000,000,000,002,013,033,012,020,000,000,000,000,000,
    000,000,000,000,000,000,000,001,033,012,020,000,000,000,000,000,
    000,000,000,000,000,000,000,000,013,012,020,000,000,000,000,000,
    000,000,000,000,000,000,000,000,001,012,020,000,000,000,000,000,
    000,000,000,000,000,000,000,000,000,022,020,000,000,000,000,000,
    000,000,000,000,000,000,000,000,000,002,020,000,000,000,000,000
};

static const unsigned char warning[ (ICON_WIDTH/2)*ICON_HEIGHT ]=
{
    000,000,000,000,000,000,001,011,000,000,000,000,000,000,000,000,
    000,000,000,000,000,000,015,055,010,000,000,000,000,000,000,000,
    000,000,000,000,000,001,055,055,051,020,000,000,000,000,000,000,
    000,000,000,000,000,001,055,055,051,022,000,000,000,000,000,000,
    000,000,000,000,000,015,055,055,055,012,020,000,000,000,000,000,
    000,000,000,000,000,015,055,055,055,012,020,000,000,000,000,000,
    000,000,000,000,001,055,055,055,055,051,022,000,000,000,000,000,
    000,000,000,000,001,055,055,055,055,051,022,000,000,000,000,000,
    000,000,000,000,015,055,055,055,055,055,012,020,000,000,000,000,
    000,000,000,000,015,055,051,011,055,055,012,020,000,000,000,000,
    000,000,000,001,055,055,011,011,015,055,051,022,000,000,000,000,
    000,000,000,001,055,055,011,011,015,055,051,022,000,000,000,000,
    000,000,000,015,055,055,011,011,015,055,055,012,020,000,000,000,
    000,000,000,015,055,055,011,011,015,055,055,012,020,000,000,000,
    000,000,001,055,055,055,011,011,015,055,055,051,022,000,000,000,
    000,000,001,055,055,055,011,011,015,055,055,051,022,000,000,000,
    000,000,015,055,055,055,051,011,055,055,055,055,012,020,000,000,
    000,000,015,055,055,055,051,011,055,055,055,055,012,020,000,000,
    000,001,055,055,055,055,051,011,055,055,055,055,051,022,000,000,
    000,001,055,055,055,055,055,015,055,055,055,055,051,022,000,000,
    000,015,055,055,055,055,055,015,055,055,055,055,055,012,020,000,
    000,015,055,055,055,055,055,055,055,055,055,055,055,012,020,000,
    001,055,055,055,055,055,055,011,055,055,055,055,055,051,022,000,
    001,055,055,055,055,055,051,011,015,055,055,055,055,051,022,000,
    015,055,055,055,055,055,051,011,015,055,055,055,055,055,012,020,
    015,055,055,055,055,055,055,011,055,055,055,055,055,055,012,020,
    015,055,055,055,055,055,055,055,055,055,055,055,055,055,012,022,
    015,055,055,055,055,055,055,055,055,055,055,055,055,055,012,022,
    001,055,055,055,055,055,055,055,055,055,055,055,055,051,022,022,
    000,011,011,011,011,011,011,011,011,011,011,011,011,012,022,022,
    000,000,022,022,022,022,022,022,022,022,022,022,022,022,022,020,
    000,000,002,022,022,022,022,022,022,022,022,022,022,022,022,000
};

static const unsigned char critical[ (ICON_WIDTH/2)*ICON_HEIGHT ]=
{
    000,000,000,000,000,001,011,011,011,010,000,000,000,000,000,000,
    000,000,000,000,001,016,066,066,066,061,010,000,000,000,000,000,
    000,000,000,001,016,066,066,066,066,066,061,010,000,000,000,000,
    000,000,000,016,066,066,066,066,066,066,066,061,000,000,000,000,
    000,000,001,066,066,066,066,066,066,066,066,066,012,000,000,000,
    000,000,016,066,066,066,066,066,066,066,066,066,061,020,000,000,
    000,001,066,066,066,066,066,066,066,066,066,066,066,012,000,000,
    000,016,066,066,063,066,066,066,066,066,036,066,066,061,020,000,
    000,016,066,066,033,036,066,066,066,063,033,066,066,061,020,000,
    001,066,066,063,033,033,066,066,066,033,033,036,066,066,012,000,
    001,066,066,066,033,033,036,066,063,033,033,066,066,066,012,000,
    016,066,066,066,063,033,033,066,033,033,036,066,066,066,061,020,
    016,066,066,066,066,033,033,033,033,033,066,066,066,066,061,020,
    016,066,066,066,066,063,033,033,033,036,066,066,066,066,061,022,
    016,066,066,066,066,066,033,033,033,066,066,066,066,066,061,022,
    016,066,066,066,066,066,033,033,033,066,066,066,066,066,061,022,
    016,066,066,066,066,063,033,033,033,036,066,066,066,066,061,022,
    016,066,066,066,066,033,033,033,033,033,066,066,066,066,061,022,
    016,066,066,066,063,033,033,066,033,033,036,066,066,066,061,022,
    001,066,066,066,033,033,036,066,063,033,033,066,066,066,012,020,
    001,066,066,063,033,033,066,066,066,033,033,036,066,066,012,020,
    000,016,066,066,033,036,066,066,066,063,033,066,066,061,022,020,
    000,016,066,066,063,066,066,066,066,066,036,066,066,061,022,000,
    000,001,066,066,066,066,066,066,066,066,066,066,066,012,022,000,
    000,000,016,066,066,066,066,066,066,066,066,066,061,022,020,000,
    000,000,021,066,066,066,066,066,066,066,066,066,012,022,000,000,
    000,000,002,016,066,066,066,066,066,066,066,061,022,020,000,000,
    000,000,000,021,016,066,066,066,066,066,066,012,022,000,000,000,
    000,000,000,002,021,016,066,066,066,061,012,022,020,000,000,000,
    000,000,000,000,002,021,011,011,011,012,022,022,000,000,000,000,
    000,000,000,000,000,002,022,022,022,022,022,000,000,000,000,000,
    000,000,000,000,000,000,002,022,022,022,000,000,000,000,000,000
};

static const unsigned char question[ (ICON_WIDTH/2)*ICON_HEIGHT ] =
{
    000,000,000,000,000,001,011,011,011,010,000,000,000,000,000,000,
    000,000,000,000,011,013,033,033,033,031,011,000,000,000,000,000,
    000,000,000,011,033,033,033,033,033,033,033,011,000,000,000,000,
    000,000,001,033,033,033,033,033,033,033,033,033,010,000,000,000,
    000,000,013,033,033,033,033,033,033,033,033,033,031,000,000,000,
    000,001,033,033,033,033,044,044,044,033,033,033,033,010,000,000,
    000,013,033,033,033,034,033,034,044,043,033,033,033,031,000,000,
    001,033,033,033,033,044,033,033,044,044,033,033,033,033,010,000,
    001,033,033,033,033,044,044,033,044,044,033,033,033,033,012,000,
    013,033,033,033,033,044,044,033,044,044,033,033,033,033,031,020,
    013,033,033,033,033,034,043,034,044,043,033,033,033,033,031,020,
    013,033,033,033,033,033,033,044,044,033,033,033,033,033,031,022,
    013,033,033,033,033,033,033,044,043,033,033,033,033,033,031,022,
    013,033,033,033,033,033,033,044,033,033,033,033,033,033,031,022,
    013,033,033,033,033,033,033,044,033,033,033,033,033,033,031,022,
    013,033,033,033,033,033,033,033,033,033,033,033,033,033,031,022,
    001,033,033,033,033,033,033,044,033,033,033,033,033,033,012,022,
    001,033,033,033,033,033,034,044,043,033,033,033,033,033,012,022,
    000,013,033,033,033,033,034,044,043,033,033,033,033,031,022,020,
    000,001,033,033,033,033,033,044,033,033,033,033,033,012,022,020,
    000,000,013,033,033,033,033,033,033,033,033,033,031,022,022,000,
    000,000,001,033,033,033,033,033,033,033,033,033,012,022,020,000,
    000,000,000,011,033,033,033,033,033,033,033,011,022,022,000,000,
    000,000,000,002,011,013,033,033,033,031,011,022,022,020,000,000,
    000,000,000,000,022,021,011,033,033,012,022,022,022,000,000,000,
    000,000,000,000,000,022,022,013,033,012,022,022,000,000,000,000,
    000,000,000,000,000,000,002,013,033,012,020,000,000,000,000,000,
    000,000,000,000,000,000,000,001,033,012,020,000,000,000,000,000,
    000,000,000,000,000,000,000,000,013,012,020,000,000,000,000,000,
    000,000,000,000,000,000,000,000,001,012,020,000,000,000,000,000,
    000,000,000,000,000,000,000,000,000,022,020,000,000,000,000,000,
    000,000,000,000,000,000,000,000,000,002,020,000,000,000,000,000
};




struct sgui_message_box
{
    sgui_window* window;
    sgui_widget* button1;
    sgui_widget* button2;
    sgui_widget* button3;
    sgui_widget* text;
    sgui_widget* icon;
};



static void message_box_button_pressed( sgui_message_box* this, int button )
{
    struct sgui_event ev;

    ev.widget = NULL;
    ev.window = (sgui_window*)this;

    switch( button )
    {
    case 0:  ev.type = SGUI_MESSAGE_BOX_BUTTON1_EVENT; break;
    case 1:  ev.type = SGUI_MESSAGE_BOX_BUTTON2_EVENT; break;
    case 2:  ev.type = SGUI_MESSAGE_BOX_BUTTON3_EVENT; break;
    default: ev.type = SGUI_MESSAGE_BOX_CLOSED_EVENT;  break;
    }

    sgui_window_set_visible( this->window, SGUI_INVISIBLE );
    sgui_event_post( &ev );
}



sgui_message_box* sgui_message_box_create( int icon, const char* caption,
                                           const char* text,
                                           const char* button1,
                                           const char* button2,
                                           const char* button3 )
{
    unsigned int text_w, text_h, w, h, x, y, b_h=0, b1_w=0, b2_w=0, b3_w=0;
    unsigned char icon_image[ICON_WIDTH*ICON_HEIGHT*4], c;
    const unsigned char* iptr;
    sgui_message_box* this;
    unsigned char* dptr;
    sgui_rect r;

    this = malloc( sizeof(sgui_message_box) );

    if( !this )
        return NULL;

    memset( this, 0, sizeof(sgui_message_box) );

    /* determine element sizes */
    sgui_skin_get_text_extents( text, &r );
    text_w = SGUI_RECT_WIDTH(r);
    text_h = SGUI_RECT_HEIGHT(r);

    if( button1 )
    {
        sgui_skin_get_text_extents( button1, &r );
        b1_w = SGUI_RECT_WIDTH(r)+20;
        h = SGUI_RECT_HEIGHT(r);
        b_h = MAX(h,b_h);
    }

    if( button2 )
    {
        sgui_skin_get_text_extents( button2, &r );
        b2_w = SGUI_RECT_WIDTH(r)+20;
        h = SGUI_RECT_HEIGHT(r);
        b_h = MAX(h,b_h);
    }

    if( button3 )
    {
        sgui_skin_get_text_extents( button3, &r );
        b3_w = SGUI_RECT_WIDTH(r)+20;
        h = SGUI_RECT_HEIGHT(r);
        b_h = MAX(h,b_h);
    }

    w = b1_w + b2_w + b3_w;
    w = MAX(w, (10+ICON_WIDTH+10+text_w+10));
    h = 10+MAX(text_h,ICON_HEIGHT)+10+b_h+10;

    /* create the message box window */
    this->window = sgui_window_create( NULL, w, h, SGUI_FIXED_SIZE );

    if( !this->window )
        goto fail;

    sgui_window_set_title( this->window, caption );

    /* decode the icon image */
         if( icon==SGUI_MB_WARNING  ) iptr = warning;
    else if( icon==SGUI_MB_CRITICAL ) iptr = critical;
    else if( icon==SGUI_MB_QUESTION ) iptr = question;
    else                              iptr = info;

    for( dptr=icon_image, y=0; y<ICON_HEIGHT; ++y )
    {
        for( x=0; x<ICON_WIDTH; x+=2, ++iptr )
        {
            c = ((*iptr & 070)>>3)*4;
            *(dptr++) = colormap[ c   ];
            *(dptr++) = colormap[ c+1 ];
            *(dptr++) = colormap[ c+2 ];
            *(dptr++) = colormap[ c+3 ];

            c = (*iptr & 007)*4;
            *(dptr++) = colormap[ c   ];
            *(dptr++) = colormap[ c+1 ];
            *(dptr++) = colormap[ c+2 ];
            *(dptr++) = colormap[ c+3 ];
        }
    }

    /* create widgets */
    y = ICON_HEIGHT>text_h ? (ICON_HEIGHT-text_h)/2 : 0;
    this->text = sgui_static_text_create( 10+ICON_WIDTH+10, y+10, text );

    if( !this->text )
        goto fail;

    y = ICON_HEIGHT>text_h ? 0 : (text_h-ICON_HEIGHT)/2;
    this->icon = sgui_image_create( 10, 10+y, ICON_WIDTH, ICON_HEIGHT,
                                    icon_image, SGUI_RGBA8, 1, 0 );

    if( !this->icon )
        goto fail;

    x = w/2 - (b1_w+b2_w+b3_w)/2;
    y = 10 + MAX(text_h,ICON_HEIGHT) + 10;

    if( button1 )
    {
        this->button1 = sgui_button_create( x+5, y, b1_w-10, b_h, button1, 0 );
        x += b1_w;
        if( !this->button1 )
            goto fail;
    }
    if( button2 )
    {
        this->button2 = sgui_button_create( x+5, y, b2_w-10, b_h, button2, 0 );
        x += b2_w;
        if( !this->button2 )
            goto fail;
    }
    if( button3 )
    {
        this->button3 = sgui_button_create( x+5, y, b3_w-10, b_h, button3, 0 );
        x += b3_w;
        if( !this->button3 )
            goto fail;
    }

    /* add widgets to the window */
    sgui_window_add_widget( this->window, this->text    );
    sgui_window_add_widget( this->window, this->button1 );
    sgui_window_add_widget( this->window, this->button2 );
    sgui_window_add_widget( this->window, this->button3 );
    sgui_window_add_widget( this->window, this->icon    );

    /* connect widgets */
    if( this->button1 )
    {
        sgui_event_connect( this->button1, SGUI_BUTTON_OUT_EVENT, 1,
                            message_box_button_pressed, this,
                            SGUI_INT, 0 );
    }
    if( this->button2 )
    {
        sgui_event_connect( this->button2, SGUI_BUTTON_OUT_EVENT, 1,
                            message_box_button_pressed, this,
                            SGUI_INT, 1 );
    }

    if( this->button3 )
    {
        sgui_event_connect( this->button3, SGUI_BUTTON_OUT_EVENT, 1,
                            message_box_button_pressed, this,
                            SGUI_INT, 2 );
    }

    sgui_event_connect( this->window, SGUI_USER_CLOSED_EVENT, 0,
                        message_box_button_pressed, this, SGUI_INT, -1 );

    return this;
fail:
    sgui_message_box_destroy( this );
    return NULL;
}

void sgui_message_box_destroy( sgui_message_box* this )
{
    if( this )
    {
        if( this->window && this->button1 )
        {
            sgui_event_disconnect( this->button1, SGUI_BUTTON_OUT_EVENT,
                                   (sgui_function)message_box_button_pressed,
                                   this );
        }

        if( this->window && this->button2 )
        {
            sgui_event_disconnect( this->button2, SGUI_BUTTON_OUT_EVENT,
                                   (sgui_function)message_box_button_pressed,
                                   this );
        }

        if( this->window && this->button3 )
        {
            sgui_event_disconnect( this->button3, SGUI_BUTTON_OUT_EVENT,
                                   (sgui_function)message_box_button_pressed,
                                   this );
        }

        if( this->window )
        {
            sgui_event_disconnect( this->window, SGUI_USER_CLOSED_EVENT,
                                   (sgui_function)message_box_button_pressed,
                                   this );

            sgui_window_destroy( this->window );
        }

        if( this->button1 ) sgui_widget_destroy( this->button1 );
        if( this->button2 ) sgui_widget_destroy( this->button2 );
        if( this->button3 ) sgui_widget_destroy( this->button3 );
        if( this->text    ) sgui_widget_destroy( this->text    );
        if( this->icon    ) sgui_widget_destroy( this->icon    );

        free( this );
    }
}

void sgui_message_box_display( sgui_message_box* this )
{
    if( this )
    {
        sgui_window_set_visible( this->window, SGUI_VISIBLE );
        sgui_window_move_center( this->window );
    }
}

