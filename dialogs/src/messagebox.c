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
#include "sgui_messagebox.h"
#include "sgui_internal.h"
#include "sgui_dialog.h"
#include "sgui_button.h"
#include "sgui_widget.h"
#include "sgui_image.h"
#include "sgui_label.h"
#include "sgui_event.h"
#include "sgui_rect.h"

#include <stdlib.h>
#include <string.h>



#ifndef SGUI_NO_MESSAGEBOX
#define ICON_WIDTH  32
#define ICON_HEIGHT 32



/* COLORMAP 7 */
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

/* SIZE 32x32 */
static const unsigned char info[ 255 ] =
{
0x85,0,1,0x83,011,010,0x8A,0,011,013,0x83,033,031,011,0x88,0,011,0x87,033,011,
0x86,0,1,0x89,033,010,0x85,0,013,0x83,033,034,044,043,0x83,033,031,0x84,0,1,
0x84,033,0x83,044,0x84,033,010,0x83,0,013,0x84,033,0x83,044,0x84,033,031,0x40,
1,0x85,033,034,044,043,0x85,033,010,0,1,0x8D,033,012,0,013,0x8D,033,031,020,
013,0x84,033,034,0x83,044,0x85,033,031,020,013,0x85,033,034,0x64,0x85,033,031,
022,013,0x85,033,034,0x64,0x85,033,031,022,013,0x85,033,034,0x64,0x85,033,031,
022,013,0x85,033,034,0x64,0x85,033,031,022,013,0x85,033,034,0x64,0x85,033,031,
022,1,0x85,033,034,0x64,0x85,033,012,022,1,0x85,033,034,0x64,0x85,033,012,022,
0,013,0x84,033,034,0x64,0x84,033,031,022,020,0,1,0x83,033,034,0x84,044,0x83,
033,012,022,020,0x40,013,0x89,033,031,0x52,0x83,0,1,0x89,033,012,022,020,0x84,
0,011,0x87,033,011,0x52,0x85,0,2,011,013,0x83,033,031,011,0x52,020,0x86,0,022,
021,011,0x5B,012,0x83,022,0x88,0,0x52,013,033,012,0x52,0x8A,0,2,013,033,012,
020,0x8C,0,1,033,012,020,0x8D,0,013,012,020,0x8D,0,1,012,020,0x8E,0,022,020,
0x8E,0,2,020,0x85,0
};

/* SIZE 32x32 */
static const unsigned char warning[ 248 ] =
{
0x86,0,1,011,0x8E,0,015,055,010,0x8C,0,1,0x6D,051,020,0x8B,0,1,0x6D,051,022,
0x8B,0,015,0x83,055,012,020,0x8A,0,015,0x83,055,012,020,0x89,0,1,0x84,055,051,
022,0x89,0,1,0x84,055,051,022,0x89,0,015,0x85,055,012,020,0x88,0,015,055,051,
011,0x6D,012,020,0x87,0,1,0x6D,0x49,015,055,051,022,0x87,0,1,0x6D,0x49,015,
055,051,022,0x87,0,015,0x6D,0x49,015,0x6D,012,020,0x86,0,015,0x6D,0x49,015,
0x6D,012,020,0x85,0,1,0x83,055,0x49,015,0x6D,051,022,0x85,0,1,0x83,055,0x49,
015,0x6D,051,022,0x85,0,015,0x83,055,051,011,0x84,055,012,020,0x84,0,015,0x83,
055,051,011,0x84,055,012,020,0x83,0,1,0x84,055,051,011,0x84,055,051,022,0x83,
0,1,0x85,055,015,0x84,055,051,022,0x83,0,015,0x85,055,015,0x85,055,012,020,
0x40,015,0x8B,055,012,020,0,1,0x86,055,011,0x85,055,051,022,0,1,0x85,055,051,
011,015,0x84,055,051,022,0,015,0x85,055,051,011,015,0x85,055,012,020,015,0x86,
055,011,0x86,055,012,020,015,0x8D,055,012,022,015,0x8D,055,012,022,1,0x8C,055,
051,0x52,0,0x8C,011,012,0x52,0x40,0x8D,022,020,0x40,2,0x8C,022,0
};

/* SIZE 32x32 */
static const unsigned char critical[ 291 ] =
{
0x85,0,1,0x83,011,010,0x8A,0,1,016,0x83,066,061,010,0x88,0,1,016,0x85,066,061,
010,0x87,0,016,0x87,066,061,0x86,0,1,0x89,066,012,0x85,0,016,0x89,066,061,020,
0x83,0,1,0x8B,066,012,0x83,0,016,0x76,063,0x85,066,036,0x76,061,020,0x40,016,
0x76,033,036,0x83,066,063,033,0x76,061,020,0,1,0x76,063,0x5B,0x83,066,0x5B,
036,0x76,012,0,1,0x83,066,0x5B,036,066,063,0x5B,0x83,066,012,0,016,0x83,066,
063,0x5B,066,0x5B,036,0x83,066,061,020,016,0x84,066,0x85,033,0x84,066,061,020,
016,0x84,066,063,0x83,033,036,0x84,066,061,022,016,0x85,066,0x83,033,0x85,066,
061,022,016,0x85,066,0x83,033,0x85,066,061,022,016,0x84,066,063,0x83,033,036,
0x84,066,061,022,016,0x84,066,0x85,033,0x84,066,061,022,016,0x83,066,063,0x5B,
066,0x5B,036,0x83,066,061,022,1,0x83,066,0x5B,036,066,063,0x5B,0x83,066,012,
020,1,0x76,063,0x5B,0x83,066,0x5B,036,0x76,012,020,0,016,0x76,033,036,0x83,
066,063,033,0x76,061,022,020,0,016,0x76,063,0x85,066,036,0x76,061,022,0x40,
1,0x8B,066,012,022,0x83,0,016,0x89,066,061,022,020,0x83,0,021,0x89,066,012,
022,0x84,0,2,016,0x87,066,061,022,020,0x85,0,021,016,0x86,066,012,022,0x86,
0,2,021,016,0x83,066,061,012,022,020,0x87,0,2,021,0x83,011,012,0x52,0x89,0,
2,0x85,022,0x8B,0,2,0x83,022,0x86,0
};

/* SIZE 32x32 */
static const unsigned char question[ 257 ] =
{
0x85,0,1,0x83,011,010,0x8A,0,011,013,0x83,033,031,011,0x88,0,011,0x87,033,011,
0x86,0,1,0x89,033,010,0x85,0,013,0x89,033,031,0x84,0,1,0x84,033,0x83,044,0x84,
033,010,0x83,0,013,0x83,033,034,033,034,044,043,0x83,033,031,0x40,1,0x84,033,
044,0x5B,0x64,0x84,033,010,0,1,0x84,033,0x64,033,0x64,0x84,033,012,0,013,0x84,
033,0x64,033,0x64,0x84,033,031,020,013,0x84,033,034,043,034,044,043,0x84,033,
031,020,013,0x86,033,0x64,0x85,033,031,022,013,0x86,033,044,043,0x85,033,031,
022,013,0x86,033,044,0x86,033,031,022,013,0x86,033,044,0x86,033,031,022,013,
0x8D,033,031,022,1,0x86,033,044,0x86,033,012,022,1,0x85,033,034,044,043,0x85,
033,012,022,0,013,0x84,033,034,044,043,0x84,033,031,022,020,0,1,0x85,033,044,
0x85,033,012,022,020,0x40,013,0x89,033,031,0x52,0x83,0,1,0x89,033,012,022,020,
0x84,0,011,0x87,033,011,0x52,0x85,0,2,011,013,0x83,033,031,011,0x52,020,0x86,
0,022,021,011,0x5B,012,0x83,022,0x88,0,0x52,013,033,012,0x52,0x8A,0,2,013,033,
012,020,0x8C,0,1,033,012,020,0x8D,0,013,012,020,0x8D,0,1,012,020,0x8E,0,022,
020,0x8E,0,2,020,0x85,0
};




typedef struct
{
    sgui_dialog super;

    sgui_widget* text;
    sgui_widget* icon;
}
sgui_message_box;



static void message_box_button_pressed( sgui_dialog* super, int button )
{
    sgui_message_box* this = (sgui_message_box*)super;
    struct sgui_event ev;

    ev.src.other = this;

    switch( button )
    {
    case 0:  ev.type = SGUI_MESSAGE_BOX_BUTTON1_EVENT; break;
    case 1:  ev.type = SGUI_MESSAGE_BOX_BUTTON2_EVENT; break;
    case 2:  ev.type = SGUI_MESSAGE_BOX_BUTTON3_EVENT; break;
    default: ev.type = SGUI_DIALOG_REJECTED;           break;
    }

    sgui_window_set_visible( super->window, SGUI_INVISIBLE );
    sgui_event_post( &ev );
}

static void sgui_message_box_destroy( sgui_dialog* super )
{
    sgui_message_box* this = (sgui_message_box*)super;
    sgui_widget_destroy( this->text );
    sgui_widget_destroy( this->icon );
    free( this );
}

/****************************************************************************/

sgui_dialog* sgui_message_box_create( int icon, const char* caption,
                                      const char* text,
                                      const char* button1,
                                      const char* button2,
                                      const char* button3 )
{
    unsigned char icon_image[ICON_WIDTH*ICON_HEIGHT*4], a, b;
    unsigned int text_w, text_h, w, h, x, y, count=0;
    const unsigned char* iptr;
    sgui_message_box* this;
    unsigned char* dptr;
    sgui_dialog* super;
    sgui_rect r;

    /* input decoding & sanity check */
    switch( icon )
    {
    case SGUI_MB_WARNING:  iptr = warning;  break;
    case SGUI_MB_CRITICAL: iptr = critical; break;
    case SGUI_MB_QUESTION: iptr = question; break;
    case SGUI_MB_INFO:     iptr = info;     break;
    default:               return NULL;
    }

    if( (!button1 && !button2 && !button3) || !text || !caption )
        return NULL;

    /* create dialog structure */
    this = malloc( sizeof(sgui_message_box) );
    super = (sgui_dialog*)this;

    if( !this )
        return NULL;

    memset( this, 0, sizeof(sgui_message_box) );
    super->destroy = sgui_message_box_destroy;
    super->handle_button = message_box_button_pressed;

    /* determine element sizes */
    sgui_skin_get_text_extents( text, &r );
    text_w = SGUI_RECT_WIDTH(r);
    text_h = SGUI_RECT_HEIGHT(r);

    w = 10+ICON_WIDTH+10+text_w+10;
    h = 10+MAX(text_h,ICON_HEIGHT)+5;

    /* create the message box window */
    super->window = sgui_window_create( NULL, w, h, SGUI_FIXED_SIZE );

    if( !super->window )
        goto fail;

    sgui_window_set_title( super->window, caption );

    /* decode the icon image */
    for( dptr=icon_image, y=0; y<ICON_HEIGHT; ++y )
    {
        for( x=0; x<ICON_WIDTH; x+=2 )
        {
            if( count )
            {
                --count;
            }
            else
            {
                count = ( *iptr & 0x80) ? ((*(iptr++) & 0x7F)-1) :
                        ((*iptr & 0x40) ? 1 : 0);
                a = ((*iptr & 070)>>3)*4;
                b =  (*iptr & 007)    *4;
                ++iptr;
            }

            *(dptr++) = colormap[ a   ];
            *(dptr++) = colormap[ a+1 ];
            *(dptr++) = colormap[ a+2 ];
            *(dptr++) = colormap[ a+3 ];

            *(dptr++) = colormap[ b   ];
            *(dptr++) = colormap[ b+1 ];
            *(dptr++) = colormap[ b+2 ];
            *(dptr++) = colormap[ b+3 ];
        }
    }

    /* create widgets */
    y = ICON_HEIGHT>text_h ? (ICON_HEIGHT-text_h)/2 : 0;
    this->text = sgui_label_create( 10+ICON_WIDTH+10, y+10, text );

    if( !this->text )
        goto fail;

    y = ICON_HEIGHT>text_h ? 0 : (text_h-ICON_HEIGHT)/2;
    this->icon = sgui_image_create( 10, 10+y, ICON_WIDTH, ICON_HEIGHT,
                                    icon_image, SGUI_RGBA8, 1, 0 );

    if( !this->icon )
        goto fail;

    if( !sgui_dialog_init( super, button1, button2, button3, SGUI_CENTER ) )
        goto fail;

    sgui_window_add_widget( super->window, this->text );
    sgui_window_add_widget( super->window, this->icon );
    return (sgui_dialog*)this;
fail:
    sgui_dialog_destroy( super );
    return NULL;
}
#elif defined(SGUI_NOP_IMPLEMENTATIONS)
sgui_dialog* sgui_message_box_create( int icon, const char* caption,
                                      const char* text, const char* button1,
                                      const char* button2,
                                      const char* button3 )
{
    (void)icon; (void)caption; (void)text;
    (void)button1; (void)button2; (void)button3;
    return NULL;
}
#endif /* !SGUI_NO_MESSAGEBOX */

