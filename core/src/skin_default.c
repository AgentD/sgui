/*
 * skin_default.c
 * This file is part of sgui
 *
 * Copyright (C) 2012 - David Oberhollenzer
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions
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
#include "sgui_skin.h"
#include "sgui_font.h"
#include "sgui_rect.h"
#include "sgui_pixmap.h"
#include "sgui_internal.h"

#include <stddef.h>



#define FONT "SourceSansPro-Regular.ttf"
#define FONT_ITAL "SourceSansPro-It.ttf"
#define FONT_BOLD "SourceSansPro-Semibold.ttf"
#define FONT_BOLD_ITAL "SourceSansPro-SemiboldIt.ttf"

#define FONT_ALT_PATH "font/"
#define FONT_ALT_PATH2 "../font/"

#define FONT_HEIGHT 16



sgui_skin sgui_default_skin;
static int is_init = 0;
static const unsigned char black[4]       = { 0x00, 0x00, 0x00, 0xFF };
static const unsigned char white[4]       = { 0xFF, 0xFF, 0xFF, 0xFF };
static const unsigned char windowcolor[4] = { 0x64, 0x64, 0x64, 0xFF };
static const unsigned char darkoverlay[4] = { 0x00, 0x00, 0x00, 0x80 };
static const unsigned char whiteoverlay[4]= { 0xFF, 0xFF, 0xFF, 0xC0 };
static const unsigned char focusbox[4]    = { 0xFF, 0x80, 0x25, 0xFF };
static const unsigned char fontcolor[4]   = { 0xFF, 0xFF, 0xFF, 0xFF };
static const unsigned char yellow[4]      = { 0xFF, 0xFF, 0x00, 0xFF };



static void default_get_checkbox_extents( sgui_skin* this, sgui_rect* r )
{
    (void)this;
    sgui_rect_set_size( r, 0, 0, 20, 12 );
}

static unsigned int default_get_edit_box_height( sgui_skin* this )
{
    (void)this;
    return 30;
}

static unsigned int default_get_edit_box_border_width( sgui_skin* this )
{
    (void)this;
    return 2;
}

static unsigned int default_get_frame_border_width( sgui_skin* this )
{
    (void)this;
    return 1;
}

static unsigned int default_get_progess_bar_width( sgui_skin* this )
{
    (void)this;
    return 30;
}

static unsigned int default_get_scroll_bar_width( sgui_skin* this )
{
    (void)this;
    return 20;
}

static unsigned int default_get_focus_box_width( sgui_skin* skin )
{
    (void)skin;
    return 3;
}

static void default_get_scroll_bar_button_extents( sgui_skin* this,
                                                   sgui_rect* r )
{
    (void)this;
    sgui_rect_set_size( r, 0, 0, 20, 20 );
}

static void default_get_tap_caption_extents( sgui_skin* this, sgui_rect* r )
{
    (void)this;
    sgui_rect_set_size( r, 0, 0, 16, 25 );
}

static void default_get_slider_extents( sgui_skin* this, sgui_rect* r,
                                        int vertical )
{
    (void)this;
    sgui_rect_set_size( r, 0, 0, vertical ? 20 : 10, vertical ? 10 : 20 );
}

static void default_get_spin_buttons( sgui_skin* this, sgui_rect* up,
                                      sgui_rect* down )
{
    (void)this;
    sgui_rect_set_size( up,   5,  5, 15, 10 );
    sgui_rect_set_size( down, 5, 15, 15, 10 );
}

static void default_draw_checkbox( sgui_skin* this, sgui_canvas* canvas,
                                   int x, int y, int checked )
{
    sgui_rect r;
    (void)this;

    sgui_rect_set_size( &r, x+1, y+1, 10, 10 );
    sgui_canvas_draw_box( canvas, &r, darkoverlay, SGUI_RGBA8 );

    sgui_canvas_draw_line( canvas, x,    y,    12, 1, black, SGUI_RGB8 );
    sgui_canvas_draw_line( canvas, x,    y,    12, 0, black, SGUI_RGB8 );
    sgui_canvas_draw_line( canvas, x,    y+11, 12, 1, white, SGUI_RGB8 );
    sgui_canvas_draw_line( canvas, x+11, y,    12, 0, white, SGUI_RGB8 );

    if( checked )
    {
        sgui_canvas_draw_line( canvas, x+2, y+4, 3, 0, white, SGUI_RGB8 );
        sgui_canvas_draw_line( canvas, x+3, y+5, 3, 0, white, SGUI_RGB8 );
        sgui_canvas_draw_line( canvas, x+4, y+6, 3, 0, white, SGUI_RGB8 );
        sgui_canvas_draw_line( canvas, x+5, y+5, 3, 0, white, SGUI_RGB8 );
        sgui_canvas_draw_line( canvas, x+6, y+4, 3, 0, white, SGUI_RGB8 );
        sgui_canvas_draw_line( canvas, x+7, y+3, 3, 0, white, SGUI_RGB8 );
        sgui_canvas_draw_line( canvas, x+8, y+2, 3, 0, white, SGUI_RGB8 );
    }
}

static void default_draw_radio_button( sgui_skin* this, sgui_canvas* canvas,
                                       int x, int y, int checked )
{
    sgui_rect r;
    (void)this;

    sgui_rect_set_size( &r, x+4, y+1, 4, 10 );
    sgui_canvas_draw_box( canvas, &r, darkoverlay, SGUI_RGBA8 );

    sgui_rect_set_size( &r, x+2, y+2, 2, 8 );
    sgui_canvas_draw_box( canvas, &r, darkoverlay, SGUI_RGBA8 );

    sgui_rect_set_size( &r, x+8, y+2, 2, 8 );
    sgui_canvas_draw_box( canvas, &r, darkoverlay, SGUI_RGBA8 );

    sgui_canvas_draw_line( canvas, x+1, y+4, 4, 0, darkoverlay, SGUI_RGBA8 );
    sgui_canvas_draw_line( canvas, x+10, y+4, 4, 0, darkoverlay, SGUI_RGBA8 );

    sgui_canvas_draw_line( canvas, x+4, y,   4, 1, black, SGUI_RGB8 );
    sgui_canvas_draw_line( canvas, x+2, y+1, 2, 1, black, SGUI_RGB8 );
    sgui_canvas_draw_line( canvas, x+8, y+1, 2, 1, black, SGUI_RGB8 );

    sgui_canvas_draw_line( canvas, x,   y+4, 4, 0, black, SGUI_RGB8 );
    sgui_canvas_draw_line( canvas, x+1, y+2, 2, 0, black, SGUI_RGB8 );
    sgui_canvas_draw_line( canvas, x+1, y+8, 2, 0, black, SGUI_RGB8 );

    sgui_canvas_draw_line( canvas, x+4, y+11, 4, 1, white, SGUI_RGB8 );
    sgui_canvas_draw_line( canvas, x+2, y+10, 2, 1, white, SGUI_RGB8 );
    sgui_canvas_draw_line( canvas, x+8, y+10, 2, 1, white, SGUI_RGB8 );

    sgui_canvas_draw_line( canvas, x+11, y+4, 4, 0, white, SGUI_RGB8 );
    sgui_canvas_draw_line( canvas, x+10, y+2, 2, 0, white, SGUI_RGB8 );
    sgui_canvas_draw_line( canvas, x+10, y+8, 2, 0, white, SGUI_RGB8 );

    if( checked )
    {
        sgui_rect_set_size( &r, x+4, y+3, 4, 6 );
        sgui_canvas_draw_box( canvas, &r, white, SGUI_RGB8 );

        sgui_rect_set_size( &r, x+3, y+4, 6, 4 );
        sgui_canvas_draw_box( canvas, &r, white, SGUI_RGB8 );
    }
}

static void default_draw_button( sgui_skin* skin, sgui_canvas* canvas,
                                 sgui_rect* r, int pressed )
{
    int x=r->left, y=r->top, w=SGUI_RECT_WIDTH_V(r), h=SGUI_RECT_HEIGHT_V(r);
    (void)skin;

    if( pressed )
    {
        sgui_canvas_draw_line( canvas, x,     y,     w, 1, black, SGUI_RGB8 );
        sgui_canvas_draw_line( canvas, x,     y,     h, 0, black, SGUI_RGB8 );
        sgui_canvas_draw_line( canvas, x,     y+h-1, w, 1, white, SGUI_RGB8 );
        sgui_canvas_draw_line( canvas, x+w-1, y,     h, 0, white, SGUI_RGB8 );
    }
    else
    {
        sgui_canvas_draw_line( canvas, x,     y,     w, 1, white, SGUI_RGB8 );
        sgui_canvas_draw_line( canvas, x,     y,     h, 0, white, SGUI_RGB8 );
        sgui_canvas_draw_line( canvas, x,     y+h-1, w, 1, black, SGUI_RGB8 );
        sgui_canvas_draw_line( canvas, x+w-1, y,     h, 0, black, SGUI_RGB8 );
    }
}

static void default_draw_editbox( sgui_skin* this, sgui_canvas* canvas,
                                  sgui_rect* r, const char* text, int offset,
                                  int cursor, int selection, int numeric,
                                  int spinbuttons )
{
    unsigned char cur[4] = { 0x7F, 0x7F, 0x7F, 0xFF };
    unsigned char selcolor[4] = { 0xFF, 0x80, 0x25, 0xFF };
    int x=r->left, y=r->top, w=SGUI_RECT_WIDTH_V(r), h=SGUI_RECT_HEIGHT_V(r);
    int cx, i, dx=0;
    sgui_rect r0;
    (void)this;

    sgui_canvas_draw_box( canvas, r, darkoverlay, SGUI_RGBA8 );
    text += offset;
    cursor -= offset;
    selection -= offset;

    if( numeric )
    {
        dx = sgui_skin_default_font_extents( text, -1, 0, 0 );
        dx = w - 2 - dx;
    }

    /* draw selection */
    if( cursor>=0 && cursor!=selection )
    {
        r0.left = sgui_skin_default_font_extents( text, cursor, 0, 0 );

        if( selection>0 )
            r0.right = sgui_skin_default_font_extents(text, selection, 0, 0);
        else
            r0.right = 0;

        if( r0.left > r0.right )
        {
            cx = r0.left;
            r0.left = r0.right;
            r0.right = cx;
        }

        r0.left += x+2 + dx;
        r0.right += x+2 + dx;
        r0.top = y+2;
        r0.bottom = y+h-3;
        sgui_canvas_draw_box( canvas, &r0, selcolor, SGUI_RGB8 );
    }

    /* draw text */
    sgui_canvas_draw_text_plain(canvas, x+2+dx, y+4, 0, 0, white, text, -1);

    /* draw cursor */
    if( cursor>=0 )
    {
        cx = sgui_skin_default_font_extents(text, cursor, 0, 0) + 2;
        sgui_canvas_draw_line(canvas, dx+x+cx, y+5, h-10, 0, cur, SGUI_RGB8);
    }

    /* draw borders */
    sgui_canvas_draw_line( canvas, x,     y,     w, 1, black, SGUI_RGB8 );
    sgui_canvas_draw_line( canvas, x,     y,     h, 0, black, SGUI_RGB8 );
    sgui_canvas_draw_line( canvas, x,     y+h-1, w, 1, white, SGUI_RGB8 );
    sgui_canvas_draw_line( canvas, x+w-1, y,     h, 0, white, SGUI_RGB8 );

    /* spin box buttons */
    if( spinbuttons )
    {
        for( i=0; i<4; ++i )
        {
            sgui_canvas_draw_line( canvas, x+5+7-i, y+5+2+i,
                                   i*2+1, 1, whiteoverlay, SGUI_RGBA8 );
        }

        for( i=0; i<4; ++i )
        {
            sgui_canvas_draw_line( canvas, x+5+7-i, y+15+6-i,
                                   i*2+1, 1, whiteoverlay, SGUI_RGBA8 );
        }
    }
}

static void default_draw_frame( sgui_skin* this, sgui_canvas* canvas,
                                sgui_rect* r )
{
    int x=r->left, y=r->top, w=SGUI_RECT_WIDTH_V(r), h=SGUI_RECT_HEIGHT_V(r);
    (void)this;

    sgui_canvas_draw_box( canvas, r, darkoverlay, SGUI_RGBA8 );

    sgui_canvas_draw_line( canvas, x,     y,     w, 1, black, SGUI_RGB8 );
    sgui_canvas_draw_line( canvas, x,     y,     h, 0, black, SGUI_RGB8 );
    sgui_canvas_draw_line( canvas, x,     y+h-1, w, 1, white, SGUI_RGB8 );
    sgui_canvas_draw_line( canvas, x+w-1, y,     h, 0, white, SGUI_RGB8 );
}

static void default_draw_group_box( sgui_skin* this, sgui_canvas* canvas,
                                    sgui_rect* r, const char* caption )
{
    int x=r->left, y=r->top, w=SGUI_RECT_WIDTH_V(r), h=SGUI_RECT_HEIGHT_V(r);
    int txw = sgui_skin_default_font_extents( caption, -1, 0, 0 );
    (void)this;

    y += 8;
    h -= 8;

    sgui_canvas_draw_line( canvas, x+1,   y+1,   h-1, 0, white, SGUI_RGB8 );
    sgui_canvas_draw_line( canvas, x+1,   y+h-1, w-1, 1, white, SGUI_RGB8 );
    sgui_canvas_draw_line( canvas, x+w-1, y+1,   h-1, 0, white, SGUI_RGB8 );
    sgui_canvas_draw_line( canvas, x+1,   y+1,   12,  1, white, SGUI_RGB8 );

    sgui_canvas_draw_line( canvas, x,     y,     h-1, 0, black, SGUI_RGB8 );
    sgui_canvas_draw_line( canvas, x,     y+h-2, w-2, 1, black, SGUI_RGB8 );
    sgui_canvas_draw_line( canvas, x+w-2, y,     h-1, 0, black, SGUI_RGB8 );
    sgui_canvas_draw_line( canvas, x,     y,     13,  1, black, SGUI_RGB8 );

    sgui_canvas_draw_line(canvas, x+18+txw, y+1, w-txw-20,1,white,SGUI_RGB8);
    sgui_canvas_draw_line(canvas, x+18+txw, y,   w-txw-20,1,black,SGUI_RGB8);

    sgui_canvas_draw_text_plain( canvas, x + 15, r->top, 0, 0, white,
                                 caption, -1 );
}

static void default_draw_progress_bar( sgui_skin* this, sgui_canvas* canvas,
                                       int x, int y, unsigned int length,
                                       int vertical, int percentage )
{
    unsigned int bar = (percentage*(length-2))/100;
    sgui_rect r;
    (void)this;

    if( vertical )
    {
        sgui_rect_set_size( &r, x, y, 30, length-bar-1 );
        sgui_canvas_draw_box( canvas, &r, darkoverlay, SGUI_RGBA8 );

        sgui_canvas_draw_line(canvas, x, y, 30, 1, black, SGUI_RGB8);
        sgui_canvas_draw_line(canvas, x, y, length, 0, black, SGUI_RGB8);
        sgui_canvas_draw_line(canvas, x, y+length-1, 30, 1, white, SGUI_RGB8);
        sgui_canvas_draw_line(canvas, x+29, y, length, 0, white, SGUI_RGB8);

        sgui_rect_set_size( &r, x+1, y+length-bar-1, 28, bar );
        sgui_canvas_draw_box( canvas, &r, yellow, SGUI_RGB8 );
    }
    else
    {
        sgui_rect_set_size( &r, x, y, length, 30 );
        sgui_canvas_draw_box( canvas, &r, darkoverlay, SGUI_RGBA8 );

        sgui_canvas_draw_line(canvas, x, y, 30, 0, black, SGUI_RGB8);
        sgui_canvas_draw_line(canvas, x, y, length, 1, black, SGUI_RGB8);
        sgui_canvas_draw_line(canvas, x, y+29, length, 1, white, SGUI_RGB8);
        sgui_canvas_draw_line(canvas, x+length-1, y, 30, 0, white, SGUI_RGB8);

        sgui_rect_set_size( &r, x+1, y+1, bar, 28 );
        sgui_canvas_draw_box( canvas, &r, yellow, SGUI_RGB8 );
    }
}

static void default_draw_progress_stippled( sgui_skin* this,
                                            sgui_canvas* canvas, int x, int y,
                                            unsigned int length, int vertical,
                                            int percentage )
{
    unsigned int i, bar = (percentage*(length-2))/100;
    sgui_rect r;
    (void)this;

    if( vertical )
    {
        sgui_rect_set_size( &r, x, y, 30, length );
        sgui_canvas_draw_box( canvas, &r, darkoverlay, SGUI_RGBA8 );

        sgui_canvas_draw_line(canvas, x, y, 30, 1, black, SGUI_RGB8);
        sgui_canvas_draw_line(canvas, x, y, length, 0, black, SGUI_RGB8);
        sgui_canvas_draw_line(canvas, x, y+length-1, 30, 1, white, SGUI_RGB8);
        sgui_canvas_draw_line(canvas, x+29, y, length, 0, white, SGUI_RGB8);

        for( i=5; i<bar; i+=12 )
        {
            sgui_rect_set_size( &r, x+5, y+length-7-i, 20, 7 );
            sgui_canvas_draw_box( canvas, &r, white, SGUI_RGBA8 );
        }
    }
    else
    {
        sgui_rect_set_size( &r, x, y, length, 30 );
        sgui_canvas_draw_box( canvas, &r, darkoverlay, SGUI_RGBA8 );

        sgui_canvas_draw_line(canvas, x, y, 30, 0, black, SGUI_RGB8);
        sgui_canvas_draw_line(canvas, x, y, length, 1, black, SGUI_RGB8);
        sgui_canvas_draw_line(canvas, x, y+29, length, 1, white, SGUI_RGB8);
        sgui_canvas_draw_line(canvas, x+length-1, y, 30, 0, white, SGUI_RGB8);

        for( i=5; i<bar; i+=12 )
        {
            sgui_rect_set_size( &r, x+i, y+5, 7, 20 );
            sgui_canvas_draw_box( canvas, &r, white, SGUI_RGBA8 );
        }
    }
}

static void default_draw_scroll_bar( sgui_skin* this, sgui_canvas* canvas,
                                     int x, int y, unsigned int length,
                                     int vertical, int pane_offset,
                                     unsigned int pane_length,
                                     int decbutton, int incbutton )
{
    sgui_rect r;
    int i;

    if( vertical )
    {
        /* background */
        sgui_rect_set_size( &r, x, y, 20, length );
        sgui_canvas_draw_box( canvas, &r, windowcolor, SGUI_RGB8 );

        /* upper button */
        sgui_rect_set_size( &r, x, y, 20, 20 );
        default_draw_button( this, canvas, &r, incbutton );

        for( i=5; i<=10; ++i )
        {
            sgui_canvas_draw_line( canvas, x+i-incbutton, y+17-i-incbutton,
                                   11-(i-5)*2, 1, white, SGUI_RGB8 );
        }

        /* lower button */
        sgui_rect_set_size( &r, x, y+length-20, 20, 20 );
        default_draw_button( this, canvas, &r, decbutton );

        for( i=5; i<=10; ++i )
        {
            sgui_canvas_draw_line( canvas, x+i-decbutton,
                                   y+length-18+i-decbutton,
                                   11-(i-5)*2, 1, white, SGUI_RGB8 );
        }

        /* pane */
        sgui_rect_set_size( &r, x, y+20+pane_offset, 20, pane_length );
        default_draw_button( this, canvas, &r, 0 );
    }
    else
    {
        /* background */
        sgui_rect_set_size( &r, x, y, length, 20 );
        sgui_canvas_draw_box( canvas, &r, windowcolor, SGUI_RGB8 );

        /* left button */
        sgui_rect_set_size( &r, x, y, 20, 20 );
        default_draw_button( this, canvas, &r, incbutton );

        for( i=7; i<=12; ++i )
        {
            sgui_canvas_draw_line( canvas, x+i-incbutton, y+17-i-incbutton,
                                   1+(i-7)*2, 0, white, SGUI_RGB8 );
        }

        /* right button */
        sgui_rect_set_size( &r, x+length-20, y, 20, 20 );
        default_draw_button( this, canvas, &r, decbutton );

        for( i=7; i<=12; ++i )
        {
            sgui_canvas_draw_line( canvas, x+length-20+i-incbutton,
                                   y+i-2-incbutton,
                                   11-(i-7)*2, 0, white, SGUI_RGB8 );
        }

        /* pane */
        sgui_rect_set_size( &r, x+20+pane_offset, y, pane_length, 20 );
        default_draw_button( this, canvas, &r, 0 );
    }
}

static void default_draw_tab_caption( sgui_skin* this, sgui_canvas* canvas,
                                      int x, int y, const char* caption,
                                      unsigned int text_width )
{
    (void)this;

    sgui_canvas_draw_line( canvas, x, y, text_width, 1, white, SGUI_RGB8 );
    sgui_canvas_draw_line( canvas, x, y,         25, 0, white, SGUI_RGB8 );
    sgui_canvas_draw_line(canvas, x+text_width-1, y, 25, 0, black, SGUI_RGB8);
    sgui_canvas_draw_text_plain(canvas, x+8, y+1, 0, 0, white, caption, -1);
}

static void default_draw_tab( sgui_skin* this, sgui_canvas* canvas,
                              sgui_rect* r, unsigned int gap,
                              unsigned int gap_width )
{
    int x=r->left, y=r->top, w=SGUI_RECT_WIDTH_V(r), h=SGUI_RECT_HEIGHT_V(r);
    (void)this;

    sgui_canvas_draw_line( canvas, x, y, h, 0, white, SGUI_RGB8 );
    sgui_canvas_draw_line( canvas, x, y, gap-x+1, 1, white, SGUI_RGB8 );
    sgui_canvas_draw_line( canvas, gap+gap_width-1, y, w-gap_width-gap, 1,
                           white, SGUI_RGB8 );

    sgui_canvas_draw_line( canvas, x,     y+h-1, w, 1, black, SGUI_RGB8 );
    sgui_canvas_draw_line( canvas, x+w-1, y,     h, 0, black, SGUI_RGB8 );
}

static void default_draw_focus_box( sgui_skin* skin, sgui_canvas* canvas,
                                    sgui_rect* r )
{
    int x=r->left, y=r->top, w=SGUI_RECT_WIDTH_V(r), h=SGUI_RECT_HEIGHT_V(r);
    int i, l;
    (void)skin;

    for( i=0; i<w-2; i+=6 )
    {
        l = w-2-i;
        l = l>=6 ? 3 : l;
        sgui_canvas_draw_line(canvas,x+1+i,y+1,  l,1,focusbox,SGUI_RGB8);
        sgui_canvas_draw_line(canvas,x+1+i,y+h-2,l,1,focusbox,SGUI_RGB8);
    }

    for( i=0; i<h-2; i+=6 )
    {
        l = h-2-i;
        l = l>=6 ? 3 : l;
        sgui_canvas_draw_line(canvas,x+1,  y+1+i,3,0,focusbox,SGUI_RGB8);
        sgui_canvas_draw_line(canvas,x+w-2,y+1+i,3,0,focusbox,SGUI_RGB8);
    }
}

static void default_draw_slider( sgui_skin* skin, sgui_canvas* canvas,
                                 sgui_rect* r, int vertical, int min, int max,
                                 int value, int steps )
{
    int x, y, i, delta, draglen;
    sgui_rect r0;
    (void)skin;

    draglen = vertical ? SGUI_RECT_HEIGHT_V(r) : SGUI_RECT_WIDTH_V(r);
    draglen -= 10;

    /* dashes for discrete slider */
    if( steps )
    {
        delta = draglen / (steps-1);

        if( vertical )
        {
            x = r->left;
            y = r->top + 5;

            for( i=0; i<steps; ++i, y+=delta )
                sgui_canvas_draw_line( canvas, x, y, 6, 1, white, SGUI_RGB8 );
        }
        else
        {
            x = r->left + 5;
            y = r->bottom - 6;

            for( i=0; i<steps; ++i, x+=delta )
                sgui_canvas_draw_line( canvas, x, y, 6, 0, white, SGUI_RGB8 );
        }
    }

    /* sliding area */
    if( vertical )
    {
        x = (r->left + r->right)/2;
        y = r->top;

        sgui_canvas_draw_line(canvas,x,  y+3,draglen+6,0,black,SGUI_RGB8);
        sgui_canvas_draw_line(canvas,x+1,y+3,draglen+6,0,white,SGUI_RGB8);
    }
    else
    {
        x = r->left;
        y = (r->top + r->bottom)/2;

        sgui_canvas_draw_line(canvas,x+3,y,  draglen+6,1,black,SGUI_RGB8);
        sgui_canvas_draw_line(canvas,x+3,y+1,draglen+6,1,white,SGUI_RGB8);
    }

    /* slider box */
    if( steps )
    {
        i = ((steps-1)*(value - min)) / (max - min);
        i *= draglen / (steps-1);
    }
    else
    {
        i = draglen - 1;
        i = (i * (value - min)) / (max - min);
    }

    if( vertical )
    {
        r0.left   = r->left;
        r0.right  = r->right;
        r0.top    = r->bottom - 10 - i;
        r0.top    = MAX(r0.top, r->top);
        r0.bottom = r0.top + 10;
        r0.bottom = MIN(r0.bottom, r->bottom);
    }
    else
    {
        r0.top    = r->top;
        r0.bottom = r->bottom;
        r0.left   = r->left + i;
        r0.left   = MAX(r0.left, r->left);
        r0.right  = r0.left + 10;
        r0.right  = MIN(r0.right, r->right);
    }

    sgui_canvas_draw_box( canvas, &r0, windowcolor, SGUI_RGB8 );

    sgui_canvas_draw_line( canvas, r0.left, r0.top,
                           SGUI_RECT_WIDTH(r0), 1, white, SGUI_RGB8 );

    sgui_canvas_draw_line( canvas, r0.left, r0.top,
                           SGUI_RECT_HEIGHT(r0), 0, white, SGUI_RGB8 );

    sgui_canvas_draw_line( canvas, r0.right, r0.top,
                           SGUI_RECT_HEIGHT(r0), 0, black, SGUI_RGB8 );

    sgui_canvas_draw_line( canvas, r0.left, r0.bottom,
                           SGUI_RECT_WIDTH(r0), 1, black, SGUI_RGB8 );
}

/****************************************************************************/

void sgui_interal_skin_init_default( void )
{
    if( is_init )
        return;

    sgui_default_skin.get_checkbox_extents = default_get_checkbox_extents;
    sgui_default_skin.get_radio_button_extents = default_get_checkbox_extents;
    sgui_default_skin.draw_checkbox = default_draw_checkbox;
    sgui_default_skin.draw_radio_button = default_draw_radio_button;
    sgui_default_skin.draw_button = default_draw_button;
    sgui_default_skin.draw_editbox = default_draw_editbox;
    sgui_default_skin.get_edit_box_height = default_get_edit_box_height;
    sgui_default_skin.draw_frame = default_draw_frame;
    sgui_default_skin.get_frame_border_width = default_get_frame_border_width;
    sgui_default_skin.draw_group_box = default_draw_group_box;
    sgui_default_skin.get_progess_bar_width = default_get_progess_bar_width;
    sgui_default_skin.draw_progress_bar = default_draw_progress_bar;
    sgui_default_skin.draw_progress_stippled = default_draw_progress_stippled;
    sgui_default_skin.draw_scroll_bar = default_draw_scroll_bar;
    sgui_default_skin.get_scroll_bar_width = default_get_scroll_bar_width;
    sgui_default_skin.draw_tab_caption = default_draw_tab_caption;
    sgui_default_skin.draw_tab = default_draw_tab;
    sgui_default_skin.get_focus_box_width = default_get_focus_box_width;
    sgui_default_skin.draw_focus_box = default_draw_focus_box;
    sgui_default_skin.draw_slider = default_draw_slider;
    sgui_default_skin.get_slider_extents = default_get_slider_extents;
    sgui_default_skin.get_spin_buttons = default_get_spin_buttons;
    sgui_default_skin.get_edit_box_border_width =
    default_get_edit_box_border_width;
    sgui_default_skin.get_scroll_bar_button_extents =
    default_get_scroll_bar_button_extents;
    sgui_default_skin.get_tap_caption_extents =
    default_get_tap_caption_extents;

    sgui_default_skin.window_color[0] = windowcolor[0];
    sgui_default_skin.window_color[1] = windowcolor[1];
    sgui_default_skin.window_color[2] = windowcolor[2];
    sgui_default_skin.window_color[3] = windowcolor[3];

    sgui_default_skin.font_color[0] = fontcolor[0];
    sgui_default_skin.font_color[1] = fontcolor[1];
    sgui_default_skin.font_color[2] = fontcolor[2];
    sgui_default_skin.font_color[3] = fontcolor[3];

    sgui_default_skin.font_height = FONT_HEIGHT;

    /* load default fonts */
    sgui_default_skin.font_norm = sgui_font_load( FONT, FONT_HEIGHT );

    if( !sgui_default_skin.font_norm )
        sgui_default_skin.font_norm =
        sgui_font_load( FONT_ALT_PATH FONT, FONT_HEIGHT );

    if( !sgui_default_skin.font_norm )
        sgui_default_skin.font_norm =
        sgui_font_load( FONT_ALT_PATH2 FONT,FONT_HEIGHT );

    sgui_default_skin.font_ital = sgui_font_load( FONT_ITAL, FONT_HEIGHT );

    if( !sgui_default_skin.font_ital )
        sgui_default_skin.font_ital =
        sgui_font_load( FONT_ALT_PATH FONT_ITAL, FONT_HEIGHT );

    if( !sgui_default_skin.font_ital )
        sgui_default_skin.font_ital =
        sgui_font_load( FONT_ALT_PATH2 FONT_ITAL, FONT_HEIGHT );

    sgui_default_skin.font_bold = sgui_font_load( FONT_BOLD, FONT_HEIGHT );

    if( !sgui_default_skin.font_bold )
        sgui_default_skin.font_bold =
        sgui_font_load( FONT_ALT_PATH FONT_BOLD, FONT_HEIGHT );

    if( !sgui_default_skin.font_bold )
        sgui_default_skin.font_bold =
        sgui_font_load( FONT_ALT_PATH2 FONT_BOLD, FONT_HEIGHT );

    sgui_default_skin.font_boit = sgui_font_load(FONT_BOLD_ITAL, FONT_HEIGHT);

    if( !sgui_default_skin.font_boit )
        sgui_default_skin.font_boit =
        sgui_font_load( FONT_ALT_PATH FONT_BOLD_ITAL, FONT_HEIGHT );

    if( !sgui_default_skin.font_boit )
        sgui_default_skin.font_boit =
        sgui_font_load( FONT_ALT_PATH2 FONT_BOLD_ITAL, FONT_HEIGHT );

    is_init = 1;
}

void sgui_interal_skin_deinit_default( void )
{
    if( is_init )
    {
        sgui_font_destroy( sgui_default_skin.font_bold );
        sgui_font_destroy( sgui_default_skin.font_ital );
        sgui_font_destroy( sgui_default_skin.font_boit );
        sgui_font_destroy( sgui_default_skin.font_norm );
        is_init = 0;
    }
}

