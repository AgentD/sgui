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

void sgui_skin_get_text_extents( const char* text, sgui_rect* r )
{
    unsigned int X = 0, longest = 0, lines = 1, i = 0, font_stack_index = 0;
    char font_stack[10], f = 0;

    /* sanity check */
    if( !text || !r )
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

    /* Add font height/2 as fudge factor to the height, because our crude
       computation here does not take into account that characters can peek
       out below the line */
    r->left   = 0;
    r->top    = 0;
    r->right  = longest - 1;
    r->bottom = lines * font_height + font_height/2 - 1;
}

void sgui_skin_get_widget_extents( int type, sgui_rect* r )
{
    if( r )
    {
        r->left = r->right = r->top = r->bottom = 0;

        switch( type )
        {
        case SGUI_CHECKBOX:
        case SGUI_CHECKBOX_SELECTED:
            r->right = 19;
            r->bottom = 11;
            break;
        case SGUI_RADIO_BUTTON:
        case SGUI_RADIO_BUTTON_SELECTED:
            r->right = 19;
            r->bottom = 11;
            break;
        case SGUI_PROGRESS_BAR_V_STIPPLED:
        case SGUI_PROGRESS_BAR_V_FILLED:
            r->right = 29;
            break;
        case SGUI_PROGRESS_BAR_H_STIPPLED:
        case SGUI_PROGRESS_BAR_H_FILLED:
            r->bottom = 29;
            break;
        case SGUI_EDIT_BOX:
            r->bottom = (font_height + (font_height / 2) + 4);
            break;
        case SGUI_SCROLL_BAR_V:
            r->right = 19;
            break;
        case SGUI_SCROLL_BAR_H:
            r->bottom = 19;
            break;
        case SGUI_SCROLL_BAR_V_BUTTON:
        case SGUI_SCROLL_BAR_H_BUTTON:
            r->right = r->bottom = 19;
            break;
        case SGUI_TAB_CAPTION:
            r->right = 19;
            r->bottom = font_height + font_height / 2 - 1;
            break;
        case SGUI_FRAME_BORDER:
            r->left = r->right = r->top = r->bottom = 1;
            break;
        }
    }
}

/***************************************************************************/

void sgui_skin_draw_progress_bar( sgui_canvas* cv, sgui_rect* area,
                                  int type, unsigned int value )
{
    int w, h, stippled, vertical;
    unsigned char c[4];
    sgui_rect r;

    if( !cv || !area )
        return;

    w = SGUI_RECT_WIDTH_V( area );
    h = SGUI_RECT_HEIGHT_V( area );

    /* draw background box */
    c[0] = c[1] = c[2] = 0x00; c[3] = 0x80;
    sgui_canvas_draw_box( cv, area, c, SGUI_RGBA8 );

    c[0] = c[1] = c[2] = 0x00; c[3] = 0xFF;
    sgui_canvas_draw_line( cv, area->left, area->top, w, 1, c, SGUI_RGB8 );
    sgui_canvas_draw_line( cv, area->left, area->top, h, 0, c, SGUI_RGB8 );

    c[0] = c[1] = c[2] = 0xFF;
    sgui_canvas_draw_line(cv, area->left,  area->bottom, w, 1, c, SGUI_RGB8);
    sgui_canvas_draw_line(cv, area->right, area->top,    h, 0, c, SGUI_RGB8);

    /* draw bar */
    stippled = (type==SGUI_PROGRESS_BAR_V_STIPPLED ||
                type==SGUI_PROGRESS_BAR_H_STIPPLED);
    vertical = (type==SGUI_PROGRESS_BAR_V_STIPPLED ||
                type==SGUI_PROGRESS_BAR_V_FILLED);

    if( !stippled )
        c[2] = 0x00;

    sgui_rect_copy( &r, area );
    r.left   += stippled ? 5 : 1;
    r.right  -= stippled ? 5 : 1;
    r.top    += stippled ? 5 : 1;
    r.bottom -= stippled ? 5 : 1;

    if( vertical )
        r.top += (h*(100-value)) / 100;
    else
        r.right -= (w*(100-value)) / 100;

    if( stippled )
    {
        if( vertical )
        {
            h = SGUI_RECT_HEIGHT( r );
            r.bottom = r.top + 6;

            for( ; h>=0; h-=12, r.top+=12, r.bottom+=12 )
                sgui_canvas_draw_box( cv, &r, c, SGUI_RGBA8 );
        }
        else
        {
            w = SGUI_RECT_WIDTH( r );
            r.right = r.left + 6;

            for( ; w>=0; w-=12, r.left+=12, r.right+=12 )
                sgui_canvas_draw_box( cv, &r, c, SGUI_RGBA8 );
        }
    }
    else
    {
        sgui_canvas_draw_box( cv, &r, c, SGUI_RGBA8 );
    }
}

void sgui_skin_draw_button( sgui_canvas* cv, sgui_rect* area, int type )
{
    unsigned char c[4] = { 0x00, 0x00, 0x00, 0xFF };
    unsigned int w, h;
    int x, y, x1, y1;
    sgui_rect r;

    x = area->left;
    y = area->top;
    x1 = area->right;
    y1 = area->bottom;

    if( type==SGUI_CHECKBOX || type==SGUI_CHECKBOX_SELECTED )
    {
        y += font_height/4;
        sgui_rect_set_size( &r, x, y, 12, 12 );
        sgui_canvas_draw_box( cv, &r, c, SGUI_RGBA8 );

        c[0] = c[1] = c[2] = 0x00; c[3] = 0xFF;
        sgui_canvas_draw_line( cv, x, y, 12, 1, c, SGUI_RGB8 );
        sgui_canvas_draw_line( cv, x, y, 12, 0, c, SGUI_RGB8 );

        c[0] = c[1] = c[2] = 0xFF;
        sgui_canvas_draw_line( cv, x,    y+11,12, 1, c, SGUI_RGB8 );
        sgui_canvas_draw_line( cv, x+11, y,   12, 0, c, SGUI_RGB8 );

        if( type==SGUI_CHECKBOX_SELECTED )
        {
            sgui_canvas_draw_line( cv, x+2, y+4, 3, 0, c, SGUI_RGB8);
            sgui_canvas_draw_line( cv, x+3, y+5, 3, 0, c, SGUI_RGB8);
            sgui_canvas_draw_line( cv, x+4, y+6, 3, 0, c, SGUI_RGB8);
            sgui_canvas_draw_line( cv, x+5, y+5, 3, 0, c, SGUI_RGB8);
            sgui_canvas_draw_line( cv, x+6, y+4, 3, 0, c, SGUI_RGB8);
            sgui_canvas_draw_line( cv, x+7, y+3, 3, 0, c, SGUI_RGB8);
            sgui_canvas_draw_line( cv, x+8, y+2, 3, 0, c, SGUI_RGB8);
        }
    }
    else if( type==SGUI_RADIO_BUTTON || type==SGUI_RADIO_BUTTON_SELECTED )
    {
        y += font_height/4;

        c[0] = c[1] = c[2] = 0x00; c[3] = 0x80;
        sgui_rect_set_size( &r, x+2, y+2, 8, 8 );
        sgui_canvas_draw_box( cv, &r, c, SGUI_RGBA8 );

        sgui_canvas_draw_line( cv, x+4,  y+ 1, 4, 1, c, SGUI_RGBA8 );
        sgui_canvas_draw_line( cv, x+4,  y+10, 4, 1, c, SGUI_RGBA8 );
        sgui_canvas_draw_line( cv, x+1,  y+ 4, 4, 0, c, SGUI_RGBA8 );
        sgui_canvas_draw_line( cv, x+10, y+ 4, 4, 0, c, SGUI_RGBA8 );

        c[3] = 0xFF;
        sgui_canvas_draw_line( cv, x+2, y+1, 2, 1, c, SGUI_RGB8 );
        sgui_canvas_draw_line( cv, x+4, y,   4, 1, c, SGUI_RGB8 );
        sgui_canvas_draw_line( cv, x+8, y+1, 2, 1, c, SGUI_RGB8 );

        sgui_canvas_draw_line( cv, x+2, y+1, 2, 1, c, SGUI_RGB8 );
        sgui_canvas_draw_line( cv, x+4, y,   4, 1, c, SGUI_RGB8 );
        sgui_canvas_draw_line( cv, x+8, y+1, 2, 1, c, SGUI_RGB8 );

        sgui_canvas_draw_line( cv, x+1, y+2, 2, 0, c, SGUI_RGB8 );
        sgui_canvas_draw_line( cv, x,   y+4, 4, 0, c, SGUI_RGB8 );
        sgui_canvas_draw_line( cv, x+1, y+8, 2, 0, c, SGUI_RGB8 );

        c[0] = c[1] = c[2] = 0xFF;
        sgui_canvas_draw_line( cv, x+10, y+2, 2, 0, c, SGUI_RGB8 );
        sgui_canvas_draw_line( cv, x+11, y+4, 4, 0, c, SGUI_RGB8 );
        sgui_canvas_draw_line( cv, x+10, y+8, 2, 0, c, SGUI_RGB8 );

        sgui_canvas_draw_line( cv, x+2, y+10, 2, 1, c, SGUI_RGB8 );
        sgui_canvas_draw_line( cv, x+4, y+11, 4, 1, c, SGUI_RGB8 );
        sgui_canvas_draw_line( cv, x+8, y+10, 2, 1, c, SGUI_RGB8 );

        if( type==SGUI_RADIO_BUTTON_SELECTED )
        {
            sgui_rect_set_size( &r, x+4, y+3, 4, 6 );
            sgui_canvas_draw_box( cv, &r, c, SGUI_RGB8 );

            sgui_rect_set_size( &r, x+3, y+4, 6, 4 );
            sgui_canvas_draw_box( cv, &r, c, SGUI_RGB8 );
        }
    }
    else
    {
        w = SGUI_RECT_WIDTH_V( area );
        h = SGUI_RECT_HEIGHT_V( area );

        c[0] = c[1] = c[2] = type==SGUI_BUTTON_SELECTED ? 0x00 : 0xFF;
        sgui_canvas_draw_line( cv, x, y, w, 1, c, SGUI_RGB8 );
        sgui_canvas_draw_line( cv, x, y, h, 0, c, SGUI_RGB8 );

        c[0] = c[1] = c[2] = type==SGUI_BUTTON_SELECTED ? 0xFF : 0x00;
        sgui_canvas_draw_line( cv, x,  y1, w, 1, c, SGUI_RGB8 );
        sgui_canvas_draw_line( cv, x1, y,  h, 0, c, SGUI_RGB8 );
    }
}

void sgui_skin_draw_edit_box( sgui_canvas* cv, sgui_rect* area,
                              const char* text, int cursor )
{
    unsigned int cx, width, height;
    unsigned char c[4];

    if( !cv || !area )
        return;

    width = SGUI_RECT_WIDTH_V( area );
    height = SGUI_RECT_HEIGHT_V( area );

    /* draw background box */
    c[0] = c[1] = c[2] = 0x00; c[3] = 0x80;
    sgui_canvas_draw_box( cv, area, c, SGUI_RGBA8 );

    /* draw text */
    c[0] = c[1] = c[2] = c[3] = 0xFF;
    sgui_canvas_draw_text_plain(cv,area->left+2,area->top+2,0,0,c,text,-1);

    /* draw borders */
    c[0] = c[1] = c[2] = 0x00;
    sgui_canvas_draw_line(cv, area->left, area->top, width,  1, c, SGUI_RGB8);
    sgui_canvas_draw_line(cv, area->left, area->top, height, 0, c, SGUI_RGB8);

    c[0] = c[1] = c[2] = 0xFF;
    sgui_canvas_draw_line(cv,area->left, area->bottom,width, 1,c,SGUI_RGB8);
    sgui_canvas_draw_line(cv,area->right,area->top,   height,0,c,SGUI_RGB8);

    /* draw cursor */
    if( cursor >= 0 )
    {
        cx = sgui_skin_default_font_extents( text, cursor, 0, 0 );

        if( cx == 0 )
            cx = 3;

        if( cx < (width-2) )
        {
            c[0] = c[1] = c[2] = 0x7F;
            sgui_canvas_draw_line( cv, area->left+cx, area->top+5,
                                   height-10, 0, c, SGUI_RGB8 );
        }
    }
}

void sgui_skin_draw_frame( sgui_canvas* cv, sgui_rect* area )
{
    unsigned char c[4] = { 0x00, 0x00, 0x00, 0x80 };
    unsigned int w, h;

    if( !cv || !area )
        return;

    w = SGUI_RECT_WIDTH_V( area );
    h = SGUI_RECT_HEIGHT_V( area );

    sgui_canvas_draw_box( cv, area, c, SGUI_RGBA8 );
    sgui_canvas_draw_line( cv, area->left, area->top, w, 1, c, SGUI_RGB8 );
    sgui_canvas_draw_line( cv, area->left, area->top, h, 0, c, SGUI_RGB8 );

    c[0] = c[1] = c[2] = 0xFF;
    sgui_canvas_draw_line(cv, area->left,  area->bottom, w, 1, c, SGUI_RGB8);
    sgui_canvas_draw_line(cv, area->right, area->top,    h, 0, c, SGUI_RGB8);
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
        sgui_rect_set_size( &r, x, y, length, 20 );
        sgui_canvas_draw_box( cv, &r, color, SGUI_RGB8 );

        color[0] = color[1] = color[2] = 0xFF;

        /* left button */
        sgui_rect_set_size( &r, x, y, 20, 20 );
        sgui_skin_draw_button( cv, &r, dec_button_state ?
                                       SGUI_BUTTON_SELECTED : SGUI_BUTTON );

        ox = oy = dec_button_state ? 1 : 0;
        ox += x + 12;
        oy += y + 5;

        for( i=0; i<6; ++i )
            sgui_canvas_draw_line( cv, ox-i, oy+i, 11-2*i, 0,
                                   color, SGUI_RGB8 );

        /* right button */
        sgui_rect_set_size( &r, x+length-20, y, 20, 20 );
        sgui_skin_draw_button( cv, &r, inc_button_state ? 
                                       SGUI_BUTTON_SELECTED : SGUI_BUTTON );

        ox = oy = inc_button_state ? 1 : 0;
        ox += x+length-1-12;
        oy += y+5;

        for( i=0; i<6; ++i )
            sgui_canvas_draw_line( cv, ox+i, oy+i, 11-2*i, 0,
                                   color, SGUI_RGB8 );

        /* pane */
        sgui_rect_set_size( &r, x+20+p_offset, y, p_length, 20 );
        sgui_skin_draw_button( cv, &r, SGUI_BUTTON );
    }
    else
    {
        sgui_rect_set_size( &r, x, y, 20, length );
        sgui_canvas_draw_box( cv, &r, color, SGUI_RGB8 );

        color[0] = color[1] = color[2] = 0xFF;

        /* upper button */
        sgui_rect_set_size( &r, x, y, 20, 20 );
        sgui_skin_draw_button( cv, &r, dec_button_state ?
                                       SGUI_BUTTON_SELECTED : SGUI_BUTTON );

        ox = oy = dec_button_state ? 1 : 0;
        ox += x+5;
        oy += y+12;

        for( i=0; i<6; ++i )
            sgui_canvas_draw_line( cv, ox+i, oy-i, 11-2*i, 1,
                                   color, SGUI_RGB8 );

        /* lower button */
        sgui_rect_set_size( &r, x, y+length-20, 20, 20 );
        sgui_skin_draw_button( cv, &r, inc_button_state ?
                                       SGUI_BUTTON_SELECTED : SGUI_BUTTON );

        ox = oy = inc_button_state ? 1 : 0;
        ox += x + 5;
        oy += y+length-1-12;

        for( i=0; i<6; ++i )
            sgui_canvas_draw_line( cv, ox+i, oy+i, 11-2*i, 1,
                                   color, SGUI_RGB8 );

        /* pane */
        sgui_rect_set_size( &r, x, y+20+p_offset, 20, p_length );
        sgui_skin_draw_button( cv, &r, SGUI_BUTTON );
    }
}

void sgui_skin_draw_group_box( sgui_canvas* cv, sgui_rect* area,
                               const char* caption )
{
    unsigned char color[3] = { 0xFF, 0xFF, 0xFF };
    unsigned int len;
    int x, y, x1, y1;

    if( !area || !cv )
        return;

    x   = area->left;
    y   = area->top;
    x1  = area->right;
    y1  = area->bottom;
    len = sgui_skin_default_font_extents( caption, -1, 0, 0 );

    sgui_canvas_draw_text_plain( cv, x+13, y, 0, 0, color, caption, -1 );
    y += font_height/2;

    /* the top line has a gap for the caption */
    sgui_canvas_draw_line(cv,x+1,     y+1, 9,           1, color, SGUI_RGB8);
    sgui_canvas_draw_line(cv,x+16+len,y+1, x1-x-15-len, 1, color, SGUI_RGB8);

    /* left, right and bottom lines */
    sgui_canvas_draw_line(cv, x+1, y+1, y1-y, 0, color, SGUI_RGB8);
    sgui_canvas_draw_line(cv, x1,  y+1, y1-y, 0, color, SGUI_RGB8);
    sgui_canvas_draw_line(cv, x+1, y1,  x1-x, 1, color, SGUI_RGB8);

    color[0] = color[1] = color[2] = 0x00;

    /* again, a gap for the caption */
    sgui_canvas_draw_line(cv, x,        y,          10, 1, color, SGUI_RGB8);
    sgui_canvas_draw_line(cv, x+16+len, y, x1-x-16-len, 1, color, SGUI_RGB8);

    /* left right and bottom lines */
    sgui_canvas_draw_line( cv, x,    y, y1-y, 0, color, SGUI_RGB8 );
    sgui_canvas_draw_line( cv, x1-1, y, y1-y, 0, color, SGUI_RGB8 );
    sgui_canvas_draw_line( cv, x, y1-1, x1-x, 1, color, SGUI_RGB8 );
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

