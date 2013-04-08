/*
 * skin.c
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
#include "sgui_skin.h"

#include "sgui_progress_bar.h"
#include "sgui_window.h"
#include "sgui_canvas.h"
#include "sgui_font.h"
#include "sgui_rect.h"
#include "sgui_utf8.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>



static sgui_font* font_norm = NULL;
static sgui_font* font_bold = NULL;
static sgui_font* font_ital = NULL;
static sgui_font* font_boit = NULL;
static unsigned int font_height = 0;



void sgui_skin_set_default_font( sgui_font* normal, sgui_font* bold,
                                 sgui_font* italic, sgui_font* bold_italic )
{
    font_norm = normal;
    font_bold = bold;
    font_ital = italic;
    font_boit = bold_italic;
    font_height = sgui_font_get_height( font_norm );
}

void sgui_skin_get_window_background_color( unsigned char* color )
{
    color[0] = 0x64;
    color[1] = 0x64;
    color[2] = 0x64;
}

void sgui_skin_get_default_font_color( unsigned char* color )
{
    color[0] = 0xFF;
    color[1] = 0xFF;
    color[2] = 0xFF;
}

unsigned int sgui_skin_get_default_font_height( void )
{
    return font_height;
}

sgui_font* sgui_skin_get_default_font( int bold, int italic )
{
    if( bold && italic ) return font_boit;
    if( bold           ) return font_bold;
    if( italic         ) return font_ital;

    return font_norm;
}

unsigned int sgui_skin_default_font_extents( const char* text,
                                             unsigned int length,
                                             int bold, int italic )
{
    sgui_font* font_face = sgui_skin_get_default_font( bold, italic );
    unsigned int x = 0, w, len = 0, i;
    unsigned long character, previous = 0;

    /* sanity check */
    if( !text || !font_face || !length )
        return 0;

    /* for each character */
    for( i=0; i<length && (*text) && (*text!='\n'); text+=len, i+=len )
    {
        /* load the next glyph */
        character = sgui_utf8_decode( text, &len );
        sgui_font_load_glyph( font_face, character );

        /* advance cursor */
        x += sgui_font_get_kerning_distance( font_face, previous, character );
        sgui_font_get_glyph_metrics( font_face, &w, NULL, NULL );

        x += w + 1;

        /* store previous glyph character for kerning */
        previous = character;
    }

    return x;
}

void sgui_skin_get_checkbox_extents( unsigned int* width,
                                     unsigned int* height )
{
    *width  = 20;
    *height = 12;
}

void sgui_skin_get_radio_button_extents( unsigned int* width,
                                         unsigned int* height )
{
    *width  = 20;
    *height = 12;
}

void sgui_skin_get_progress_bar_extents( unsigned int length, int style,
                                         int vertical, unsigned int* width,
                                         unsigned int* height )
{
    (void)style;

    if( vertical )
    {
        *width  = 30;
        *height = length;
    }
    else
    {
        *width  = length;
        *height = 30;
    }
}

unsigned int sgui_skin_get_edit_box_height( void )
{
    return (font_height + (font_height / 2) + 4);
}

void sgui_skin_get_scroll_bar_extents( int horizontal, unsigned int length,
                                       unsigned int* width,
                                       unsigned int* height,
                                       unsigned int* bw, unsigned int* bh )
{
    *bw = *bh = 20;

    if( horizontal )
    {
        *width = length;
        *height = 20;
    }
    else
    {
        *width = 20;
        *height = length;
    }
}

unsigned int sgui_skin_get_frame_border_width( void )
{
    return 1;
}

unsigned int sgui_skin_get_tab_caption_width( const char* caption )
{
    unsigned int w = sgui_skin_default_font_extents( caption, -1, 0, 0 );

    return w + 20;
}

unsigned int sgui_skin_get_tab_caption_height( void )
{
    return font_height + font_height / 2;
}

void sgui_skin_get_text_extents( const char* text,
                                 unsigned int* width, unsigned int* height )
{
    unsigned int X = 0, longest = 0, lines = 1, i = 0, font_stack_index = 0;
    char font_stack[10], f = 0;

    /* sanity check */
    if( !text || (!width && !height) )
        return;

    for( ; text && text[ i ]; ++i )
    {
        if( text[ i ] == '<' )  /* we found a tag */
        {
            /* get extends for what we found so far */
            X += sgui_skin_default_font_extents( text, i, f&0x02, f&0x01 );

            /* change fonts accordingly */
            if( text[ i+1 ] == 'b' )
            {
                font_stack[ font_stack_index++ ] = f;
                f |= 0x02;
            }
            else if( text[ i+1 ] == 'i' )
            {
                font_stack[ font_stack_index++ ] = f;
                f |= 0x01;
            }
            else if( text[ i+1 ] == '/' && font_stack_index )
            {
                f = font_stack[ --font_stack_index ];
            }

            /* skip to tag end */
            text = strchr( text+i, '>' );

            if( text )
                ++text;

            /* reset i to -1, so it starts with 0 in the next iteration */
            i = -1;
        }
        else if( text[ i ] == '\n' )
        {
            /* get extends for what we found so far */
            X += sgui_skin_default_font_extents( text, i, f&0x02, f&0x01 );

            /* store the length of the longest line */
            if( X > longest )
                longest = X;

            ++lines;        /* increment line counter */
            text += i + 1;  /* skip to next line */
            i = -1;         /* restart with 0 at next iteration */
            X = 0;          /* move cursor back to the left */
        }
    }

    /* get the extents of what we didn't get so far */
    X += sgui_skin_default_font_extents( text, i, f&0x02, f&0x01 );

    if( X > longest )
        longest = X;

    /* store width and height */
    if( width  ) *width  = longest;
    if( height ) *height = lines * font_height;

    /* Add font height/2 as fudge factor to the height, because our crude
       computation here does not take into account that characters can peek
       out below the line */
    if( height && *height )
        *height += font_height/2;
}

/***************************************************************************/

void sgui_skin_draw_progress_bar( sgui_canvas* cv, int x, int y,
                                  unsigned int length, int vertical,
                                  int style, unsigned int value )
{
    int ox, oy;
    unsigned char color[4];
    unsigned int segments, i, ww = vertical ? 30 : length;
    unsigned int wh = vertical ? length : 30;
    unsigned int width, height;
    sgui_rect r;

    sgui_rect_set_size( &r, x, y, ww, wh );

    /* draw background box */
    color[0] = color[1] = color[2] = 0x00; color[3] = 0x80;

    sgui_canvas_draw_box( cv, &r, color, SGUI_RGBA8 );

    color[0] = color[1] = color[2] = 0x00; color[3] = 0xFF;

    sgui_canvas_draw_line( cv, x, y, ww, 1, color, SGUI_RGB8 );
    sgui_canvas_draw_line( cv, x, y, wh, 0, color, SGUI_RGB8 );

    color[0] = color[1] = color[2] = 0xFF;

    sgui_canvas_draw_line( cv, x,      y+wh-1, ww, 1, color, SGUI_RGB8 );
    sgui_canvas_draw_line( cv, x+ww-1,      y, wh, 0, color, SGUI_RGB8 );

    /* draw bar */
    if( style == SGUI_PROGRESS_BAR_STIPPLED )
    {
        ox = oy = 5;
        color[0] = color[1] = color[2] = 0xFF;
    }
    else
    {
        ox = oy = 1;
        color[0] = color[1] = 0xFF; color[2] = 0x00;
    }

    if( vertical )
    {
        height = ((wh - 2*oy) * value) / 100;
        width  =  ww - 2*ox;
        ox += x;

        if( height )
        {
            if( style == SGUI_PROGRESS_BAR_CONTINUOUS )
            {
                sgui_rect_set_size( &r, ox, y+wh-oy-height, width, height );
                sgui_canvas_draw_box( cv, &r, color, SGUI_RGBA8 );
            }
            else
            {
                segments = height / 12;
                sgui_rect_set_size( &r, ox, y+wh-oy - 7, width, 7 );

                for( i=0; i<segments; ++i )
                {
                    sgui_canvas_draw_box( cv, &r, color, SGUI_RGBA8 );
                    r.top    -= 12;
                    r.bottom -= 12;
                }
            }
        }
    }
    else
    {
        width  = ((ww - 2*ox) * value) / 100;
        height =  wh - 2*oy;
        ox += x;
        oy += y;

        if( width )
        {
            if( style == SGUI_PROGRESS_BAR_CONTINUOUS )
            {
                sgui_rect_set_size( &r, ox, oy, width, height );
                sgui_canvas_draw_box( cv, &r, color, SGUI_RGBA8 );
            }
            else
            {
                segments = width / 12;
                sgui_rect_set_size( &r, ox, oy, 7, height );

                for( i=0; i<segments; ++i )
                {
                    sgui_canvas_draw_box( cv, &r, color, SGUI_RGBA8 );
                    r.left += 12;
                    r.right += 12;
                }
            }
        }
    }
}

void sgui_skin_draw_button( sgui_canvas* cv, sgui_rect* r, int state )
{
    unsigned char color[4] = { 0x00, 0x00, 0x00, 0xFF };
    unsigned int w, h;

    w = SGUI_RECT_WIDTH_V( r );
    h = SGUI_RECT_HEIGHT_V( r );

    color[0] = color[1] = color[2] = state ? 0x00 : 0xFF;
    sgui_canvas_draw_line( cv, r->left, r->top, w, 1, color, SGUI_RGB8 );
    sgui_canvas_draw_line( cv, r->left, r->top, h, 0, color, SGUI_RGB8 );

    color[0] = color[1] = color[2] = state ? 0xFF : 0x00;
    sgui_canvas_draw_line( cv, r->left, r->top+h-1, w, 1, color, SGUI_RGB8 );
    sgui_canvas_draw_line( cv, r->left+w-1, r->top, h, 0, color, SGUI_RGB8 );
}

void sgui_skin_draw_checkbox( sgui_canvas* cv, int x, int y, int state )
{
    unsigned char color[4] = { 0x00, 0x00, 0x00, 0x80 };
    sgui_rect r;

    sgui_rect_set_size( &r, x, y, 12, 12 );
    sgui_canvas_draw_box( cv, &r, color, SGUI_RGBA8 );

    color[0] = color[1] = color[2] = 0x00; color[3] = 0xFF;

    sgui_canvas_draw_line( cv, x, y, 12, 1, color, SGUI_RGB8 );
    sgui_canvas_draw_line( cv, x, y, 12, 0, color, SGUI_RGB8 );

    color[0] = color[1] = color[2] = 0xFF;

    sgui_canvas_draw_line( cv, x,    y+11, 12, 1, color, SGUI_RGB8 );
    sgui_canvas_draw_line( cv, x+11, y,    12, 0, color, SGUI_RGB8 );

    if( state )
    {
        sgui_canvas_draw_line( cv, x+2, y+4, 3, 0, color, SGUI_RGB8 );
        sgui_canvas_draw_line( cv, x+3, y+5, 3, 0, color, SGUI_RGB8 );
        sgui_canvas_draw_line( cv, x+4, y+6, 3, 0, color, SGUI_RGB8 );
        sgui_canvas_draw_line( cv, x+5, y+5, 3, 0, color, SGUI_RGB8 );
        sgui_canvas_draw_line( cv, x+6, y+4, 3, 0, color, SGUI_RGB8 );
        sgui_canvas_draw_line( cv, x+7, y+3, 3, 0, color, SGUI_RGB8 );
        sgui_canvas_draw_line( cv, x+8, y+2, 3, 0, color, SGUI_RGB8 );
    }
}

void sgui_skin_draw_radio_button( sgui_canvas* cv, int x, int y,
                                  int selected )
{
    unsigned char color[4] = { 0x00, 0x00, 0x00, 0x80 };
    sgui_rect r;

    color[0] = color[1] = color[2] = 0x00; color[3] = 0x80;

    sgui_rect_set_size( &r, x+2, y+2, 8, 8 );
    sgui_canvas_draw_box( cv, &r, color, SGUI_RGBA8 );

    sgui_canvas_draw_line( cv, x+4,  y+ 1, 4, 1, color, SGUI_RGBA8 );
    sgui_canvas_draw_line( cv, x+4,  y+10, 4, 1, color, SGUI_RGBA8 );
    sgui_canvas_draw_line( cv, x+1,  y+ 4, 4, 0, color, SGUI_RGBA8 );
    sgui_canvas_draw_line( cv, x+10, y+ 4, 4, 0, color, SGUI_RGBA8 );

    color[3] = 0xFF;

    sgui_canvas_draw_line( cv, x+2, y+1, 2, 1, color, SGUI_RGB8 );
    sgui_canvas_draw_line( cv, x+4, y,   4, 1, color, SGUI_RGB8 );
    sgui_canvas_draw_line( cv, x+8, y+1, 2, 1, color, SGUI_RGB8 );

    sgui_canvas_draw_line( cv, x+2, y+1, 2, 1, color, SGUI_RGB8 );
    sgui_canvas_draw_line( cv, x+4, y,   4, 1, color, SGUI_RGB8 );
    sgui_canvas_draw_line( cv, x+8, y+1, 2, 1, color, SGUI_RGB8 );

    sgui_canvas_draw_line( cv, x+1, y+2, 2, 0, color, SGUI_RGB8 );
    sgui_canvas_draw_line( cv, x,   y+4, 4, 0, color, SGUI_RGB8 );
    sgui_canvas_draw_line( cv, x+1, y+8, 2, 0, color, SGUI_RGB8 );

    color[0] = color[1] = color[2] = 0xFF;

    sgui_canvas_draw_line( cv, x+10, y+2, 2, 0, color, SGUI_RGB8 );
    sgui_canvas_draw_line( cv, x+11, y+4, 4, 0, color, SGUI_RGB8 );
    sgui_canvas_draw_line( cv, x+10, y+8, 2, 0, color, SGUI_RGB8 );

    sgui_canvas_draw_line( cv, x+ 2, y+10, 2, 1, color, SGUI_RGB8 );
    sgui_canvas_draw_line( cv, x+ 4, y+11, 4, 1, color, SGUI_RGB8 );
    sgui_canvas_draw_line( cv, x+ 8, y+10, 2, 1, color, SGUI_RGB8 );

    if( selected )
    {
        sgui_rect_set_size( &r, x+4, y+3, 4, 6 );
        sgui_canvas_draw_box( cv, &r, color, SGUI_RGB8 );

        sgui_rect_set_size( &r, x+3, y+4, 6, 4 );
        sgui_canvas_draw_box( cv, &r, color, SGUI_RGB8 );
    }
}

void sgui_skin_draw_edit_box( sgui_canvas* cv, int x, int y,
                              const char* text, unsigned int width,
                              int cursor )
{
    unsigned char color[4];
    unsigned int height, cx;
    sgui_rect r, old_sc;

    height = font_height + (font_height / 2) + 4;

    /* draw background box */
    color[0] = color[1] = color[2] = 0x00; color[3] = 0x80;

    sgui_rect_set_size( &r, x, y, width, height );
    sgui_canvas_draw_box( cv, &r, color, SGUI_RGBA8 );

    /* draw text */
    color[0] = color[1] = color[2] = color[3] = 0xFF;

    r.left += 2;
    r.top += 2;
    r.right -= 2;
    r.bottom -= 2;
    sgui_canvas_get_scissor_rect( cv, &old_sc );
    sgui_canvas_merge_scissor_rect( cv, &r );

    sgui_canvas_draw_text_plain( cv, x+2, y+2, 0, 0, color, text, -1 );

    sgui_canvas_set_scissor_rect( cv, &old_sc );

    /* draw borders */
    color[0] = color[1] = color[2] = 0x00;

    sgui_canvas_draw_line( cv, x, y, width,  1, color, SGUI_RGB8 );
    sgui_canvas_draw_line( cv, x, y, height, 0, color, SGUI_RGB8 );

    color[0] = color[1] = color[2] = 0xFF;

    sgui_canvas_draw_line( cv, x, y+height-1, width,  1, color, SGUI_RGB8 );
    sgui_canvas_draw_line( cv, x+width-1, y,  height, 0, color, SGUI_RGB8 );

    /* draw cursor */
    if( cursor >= 0 )
    {
        cx = sgui_skin_default_font_extents( text, cursor, 0, 0 );

        if( cx == 0 )
            cx = 3;

        if( cx < (width-2) )
        {
            color[0] = color[1] = color[2] = 0x7F;

            sgui_canvas_draw_line( cv, x+cx, y+5, height-10, 0, color,
                                   SGUI_RGB8 );
        }
    }
}

void sgui_skin_draw_frame( sgui_canvas* cv, int x, int y, unsigned int width,
                           unsigned int height )
{
    unsigned char color[4] = { 0x00, 0x00, 0x00, 0x80 };
    sgui_rect r;

    sgui_rect_set_size( &r, x, y, width, height );
    sgui_canvas_draw_box( cv, &r, color, SGUI_RGBA8 );

    sgui_canvas_draw_line( cv, x, y, width,  1, color, SGUI_RGB8 );
    sgui_canvas_draw_line( cv, x, y, height, 0, color, SGUI_RGB8 );

    color[0] = color[1] = color[2] = 0xFF;
    sgui_canvas_draw_line( cv, x, y+height-1, width,  1, color, SGUI_RGB8 );
    sgui_canvas_draw_line( cv, x+width-1, y,  height, 0, color, SGUI_RGB8 );
}

void sgui_skin_draw_scroll_bar( sgui_canvas* cv, int x, int y,
                                int horizontal, unsigned int length,
                                unsigned int p_offset, unsigned int p_length,
                                int inc_button_state, int dec_button_state )
{
    unsigned char color[4] = { 0x64, 0x64, 0x64, 0xFF };
    int i, ox=0, oy=0;
    sgui_rect r;

    if( horizontal )
    {
        sgui_rect_set_size( &r, x+20, y, length-40, 20 );
        sgui_canvas_draw_box( cv, &r, color, SGUI_RGB8 );

        color[0] = color[1] = color[2] = 0xFF;

        /* left button */
        sgui_rect_set_size( &r, x, y, 20, 20 );
        sgui_skin_draw_button( cv, &r, dec_button_state );

        ox = oy = dec_button_state ? 1 : 0;
        ox += x + 12;
        oy += y + 5;

        for( i=0; i<6; ++i )
            sgui_canvas_draw_line( cv, ox-i, oy+i, 11-2*i, 0,
                                   color, SGUI_RGB8 );

        /* right button */
        sgui_rect_set_size( &r, x+length-20, y, 20, 20 );
        sgui_skin_draw_button( cv, &r, inc_button_state );

        ox = oy = inc_button_state ? 1 : 0;
        ox += x+length-1-12;
        oy += y+5;

        for( i=0; i<6; ++i )
            sgui_canvas_draw_line( cv, ox+i, oy+i, 11-2*i, 0,
                                   color, SGUI_RGB8 );

        /* pane */
        sgui_rect_set_size( &r, x+20+p_offset, y, p_length, 20 );
        sgui_skin_draw_button( cv, &r, 0 );
    }
    else
    {
        sgui_rect_set_size( &r, x, y+20, 20, length-40 );
        sgui_canvas_draw_box( cv, &r, color, SGUI_RGB8 );

        color[0] = color[1] = color[2] = 0xFF;

        /* upper button */
        sgui_rect_set_size( &r, x, y, 20, 20 );
        sgui_skin_draw_button( cv, &r, dec_button_state );

        ox = oy = dec_button_state ? 1 : 0;
        ox += x+5;
        oy += y+12;

        for( i=0; i<6; ++i )
            sgui_canvas_draw_line( cv, ox+i, oy-i, 11-2*i, 1,
                                   color, SGUI_RGB8 );

        /* lower button */
        sgui_rect_set_size( &r, x, y+length-20, 20, 20 );
        sgui_skin_draw_button( cv, &r, inc_button_state );

        ox = oy = inc_button_state ? 1 : 0;
        ox += x + 5;
        oy += y+length-1-12;

        for( i=0; i<6; ++i )
            sgui_canvas_draw_line( cv, ox+i, oy+i, 11-2*i, 1,
                                   color, SGUI_RGB8 );

        /* pane */
        sgui_rect_set_size( &r, x, y+20+p_offset, 20, p_length );
        sgui_skin_draw_button( cv, &r, 0 );
    }
}

void sgui_skin_draw_group_box( sgui_canvas* cv, int x, int y,
                               unsigned int width, unsigned int height,
                               const char* caption )
{
    unsigned char color[3] = { 0xFF, 0xFF, 0xFF };
    unsigned int len;

    len = sgui_skin_default_font_extents( caption, -1, 0, 0 );

    sgui_canvas_draw_text_plain( cv, x+13, y, 0, 0, color, caption, -1 );

    y += font_height/2;
    height -= font_height/2;

    /* the top line has a gap for the caption */
    sgui_canvas_draw_line(cv,x+1,     y+1, 9,            1, color, SGUI_RGB8);
    sgui_canvas_draw_line(cv,x+16+len,y+1, width-15-len, 1, color, SGUI_RGB8);

    /* left, right and bottom lines */
    sgui_canvas_draw_line( cv, x+1, y+1,      height, 0, color, SGUI_RGB8 );
    sgui_canvas_draw_line( cv, x+width, y+1,  height, 0, color, SGUI_RGB8 );
    sgui_canvas_draw_line( cv, x+1, y+height, width,  1, color, SGUI_RGB8 );

    color[0] = color[1] = color[2] = 0x00;

    /* again, a gap for the caption */
    sgui_canvas_draw_line(cv, x,        y,           10, 1, color, SGUI_RGB8);
    sgui_canvas_draw_line(cv, x+16+len, y, width-16-len, 1, color, SGUI_RGB8);

    /* left right and bottom lines */
    sgui_canvas_draw_line( cv, x, y,          height, 0, color, SGUI_RGB8 );
    sgui_canvas_draw_line( cv, x+width-1, y,  height, 0, color, SGUI_RGB8 );
    sgui_canvas_draw_line( cv, x, y+height-1, width,  1, color, SGUI_RGB8 );
}

void sgui_skin_draw_tab_caption( sgui_canvas* cv, int x, int y,
                                 unsigned int width, const char* caption )
{
    unsigned char color[4] = { 0xFF, 0xFF, 0xFF, 0xFF };
    unsigned int h = font_height + font_height / 2;

    sgui_canvas_draw_line( cv, x, y, width, 1, color, SGUI_RGB8 );
    sgui_canvas_draw_line( cv, x, y, h,     0, color, SGUI_RGB8 );

    color[0] = color[1] = color[2] = 0x00;
    sgui_canvas_draw_line( cv, x+width-1, y, h, 0, color, SGUI_RGB8 );

    color[0] = color[1] = color[2] = 0xFF;
    sgui_canvas_draw_text_plain( cv, x+10, y, 0, 0, color, caption, -1 );
}

void sgui_skin_draw_tab( sgui_canvas* cv, int x, int y, unsigned int width,
                         unsigned int height, unsigned int gap,
                         unsigned int gap_width )
{
    unsigned char color[4] = { 0xFF, 0xFF, 0xFF, 0xFF };
    unsigned int w_remain = width - gap - gap_width;
    int x_after = x + gap + gap_width;

    sgui_canvas_draw_line( cv, x,       y, gap+1,     1, color, SGUI_RGB8 );
    sgui_canvas_draw_line( cv, x_after, y, w_remain,  1, color, SGUI_RGB8 );
    sgui_canvas_draw_line( cv, x,       y, height,    0, color, SGUI_RGB8 );

    color[0] = color[1] = color[2] = 0x00;
    sgui_canvas_draw_line( cv, x, y+height-1, width, 1, color, SGUI_RGB8 );
    sgui_canvas_draw_line( cv, x+width-1, y, height, 0, color, SGUI_RGB8 );
}

