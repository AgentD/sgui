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
#include "sgui_skin.h"

#include "sgui_progress_bar.h"
#include "sgui_window.h"
#include "sgui_font_manager.h"
#include "sgui_canvas.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>



static sgui_font* font_norm;
static sgui_font* font_bold;
static sgui_font* font_ital;
static sgui_font* font_boit;
static unsigned int font_height;

/***************************************************************************/

void sgui_skin_init( void )
{
    font_norm = NULL;
    font_bold = NULL;
    font_ital = NULL;
    font_boit = NULL;
    font_height = 0;
}

void sgui_skin_deinit( void )
{
}

/***************************************************************************/

void sgui_skin_set_default_font( sgui_font* normal, sgui_font* bold,
                                 sgui_font* italic, sgui_font* bold_italic,
                                 unsigned int height )
{
    font_norm = normal;
    font_bold = bold;
    font_ital = italic;
    font_boit = bold_italic;
    font_height = height;
}

void sgui_skin_get_window_background_color( unsigned char* color )
{
    color[0] = 0x64;
    color[1] = 0x64;
    color[2] = 0x64;
}

unsigned int sgui_skin_default_font_extents( const unsigned char* text,
                                             unsigned int length,
                                             int bold, int italic )
{
    if( bold && italic )
        return sgui_font_extents( text, font_boit, font_height, length );

    if( bold )
        return sgui_font_extents( text, font_bold, font_height, length );

    if( italic )
        return sgui_font_extents( text, font_ital, font_height, length );

    return sgui_font_extents( text, font_norm, font_height, length );
}

void sgui_skin_get_button_extents( const unsigned char* text,
                                   unsigned int* width,
                                   unsigned int* height,
                                   unsigned int* text_w )
{
    unsigned int len;

    len = strlen( (const char*)text );

    *text_w = sgui_font_extents( text, font_norm, font_height, len );
    *width  = *text_w + 2*font_height;
    *height = 2*font_height - font_height/4;
}

void sgui_skin_get_text_extents( const unsigned char* text,
                                 unsigned int* width, unsigned int* height )
{
    unsigned int lines = 1, linestart = 0, i = 0, line_width = 0;
    sgui_font* f = font_norm;
    sgui_font* font_stack[10];
    int font_stack_index = 0;

    *width = 0;
    *height = 0;

    /* determine size of the text buffer */
    for( ; text[ i ]; ++i )
    {
        if( text[ i ] == '<' )
        {
            if( !strncmp( (const char*)text+i+1, "color=", 6 ) )
            {
                i += 14;
            }
            else if( text[i+1] == 'b' )
            {
                font_stack[ font_stack_index++ ] = f;
                f = f==font_ital ? font_boit : font_bold;
            }
            else if( text[i+1] == 'i' )
            {
                font_stack[ font_stack_index++ ] = f;
                f = f==font_bold ? font_boit : font_ital;
            }
            else if( text[i+1] == '/' )
            {
                if( font_stack_index )
                    f = font_stack[ --font_stack_index ];

                ++i;
            }

            i += 2;
        }
        else if( text[ i ] == '\n' )
        {
            line_width = sgui_font_extents( text+linestart, f, font_height,
                                            i-linestart );

            if( line_width > *width )
                *width = line_width;

            ++lines;
            linestart = i+1;
        }
    }

    line_width = sgui_font_extents( text+linestart, f, font_height,
                                    i-linestart );

    if( line_width > *width )
        *width = line_width;

    *height = lines * font_height;
}

void sgui_skin_get_checkbox_extents( const unsigned char* text,
                                     unsigned int* width,
                                     unsigned int* height,
                                     unsigned int* text_w )
{
    unsigned int len = strlen( (const char*)text );

    *text_w = sgui_font_extents( text, font_norm, font_height, len );
    *width  = *text_w + 20;
    *height = font_height > 12 ? font_height : 12;
}

void sgui_skin_get_radio_menu_extents( const unsigned char** text,
                                       unsigned int num_lines,
                                       unsigned int* width,
                                       unsigned int* height )
{
    unsigned int i, w;

    *width  = 0;
    *height = (font_height + 10) * num_lines;

    for( i=0; i<num_lines; ++i )
    {
        w = sgui_font_extents( text[i], font_norm, font_height,
                               strlen((const char*)text[i]) );

        if( w > *width )
            *width = w;
    }

    *width += 20;
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

/***************************************************************************/

unsigned int sgui_skin_get_radio_menu_option_from_point( int y )
{
    return (y / (font_height + 10));
}

/***************************************************************************/

void sgui_skin_draw_progress_bar( sgui_canvas* cv, int x, int y,
                                  unsigned int length, int vertical,
                                  int style, float value )
{
    int ox, oy;
    unsigned char color[4];
    unsigned int segments, i, ww = vertical ? 30 : length;
    unsigned int wh = vertical ? length : 30;
    unsigned int width, height;

    /* draw background box */
    color[0] = color[1] = color[2] = 0x00; color[3] = 0x80;

    sgui_canvas_draw_box( cv, x, y, ww, wh, color, SCF_RGBA8 );

    color[0] = color[1] = color[2] = 0x00; color[3] = 0xFF;

    sgui_canvas_draw_line( cv, x, y, ww, 1, color, SCF_RGB8 );
    sgui_canvas_draw_line( cv, x, y, wh, 0, color, SCF_RGB8 );

    color[0] = color[1] = color[2] = 0xFF;

    sgui_canvas_draw_line( cv, x,      y+wh-1, ww, 1, color, SCF_RGB8 );
    sgui_canvas_draw_line( cv, x+ww-1,      y, wh, 0, color, SCF_RGB8 );

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
        height = (wh - 2*oy) * value;
        width  =  ww - 2*ox;
        ox += x;

        if( height )
        {
            if( style == SGUI_PROGRESS_BAR_CONTINUOUS )
            {
                sgui_canvas_draw_box( cv, ox, y+wh-oy-height, width, height,
                                      color, SCF_RGBA8 );
            }
            else
            {
                segments = height / 12;

                for( i=0; i<segments; ++i )
                {
                    sgui_canvas_draw_box( cv, ox, y+wh-oy-(int)i*12 - 7,
                                          width, 7, color, SCF_RGBA8 );
                }
            }
        }
    }
    else
    {
        width  = (ww - 2*ox) * value;
        height =  wh - 2*oy;
        ox += x;
        oy += y;

        if( width )
        {
            if( style == SGUI_PROGRESS_BAR_CONTINUOUS )
            {
                sgui_canvas_draw_box( cv, ox, oy, width, height, color,
                                      SCF_RGBA8 );
            }
            else
            {
                segments = width / 12;

                for( i=0; i<segments; ++i )
                {
                    sgui_canvas_draw_box( cv, ox+(int)i*12, oy, 7, height,
                                          color, SCF_RGBA8 );
                }
            }
        }
    }
}

void sgui_skin_draw_button( sgui_canvas* cv, int x, int y, int state,
                            unsigned int width, unsigned int text_w,
                            unsigned int height, const unsigned char* text )
{
    unsigned char color[4] = { 0x00, 0x00, 0x00, 0xFF };

    color[0] = color[1] = color[2] = state ? 0x00 : 0xFF;
    sgui_canvas_draw_line( cv, x, y, width, 1, color, SCF_RGB8 );
    sgui_canvas_draw_line( cv, x, y, height, 0, color, SCF_RGB8 );

    color[0] = color[1] = color[2] = state ? 0xFF : 0x00;
    sgui_canvas_draw_line( cv, x, y+height-1, width, 1, color, SCF_RGB8 );
    sgui_canvas_draw_line( cv, x+width-1, y, height, 0, color, SCF_RGB8 );

    color[0] = color[1] = color[2] = color[3] = 0xFF;

    x += width/2  - text_w/2      - state;
    y += height/2 - font_height/2 - font_height/8 - state;

    sgui_canvas_draw_text_plain( cv, x, y, font_norm, font_height, color,
                                 SCF_RGB8, text, strlen((const char*)text),
                                 0xFFFF );
}

void sgui_skin_draw_text( sgui_canvas* cv, int x, int y,
                          const unsigned char* text )
{
    unsigned char color[3] = { 0xFF, 0xFF, 0xFF };

    sgui_canvas_draw_text( cv, x, y, font_norm, font_bold, font_ital,
                           font_boit, font_height, color, SCF_RGB8, text );
}

void sgui_skin_draw_checkbox( sgui_canvas* cv, int x, int y,
                              const unsigned char* text, int state )
{
    unsigned char color[4] = { 0x00, 0x00, 0x00, 0x80 };
    int oy = font_height > 12 ? font_height/2-6 : 0;

    sgui_canvas_draw_box( cv, x, y+oy, 12, 12, color, SCF_RGBA8 );

    color[0] = color[1] = color[2] = 0x00; color[3] = 0xFF;

    sgui_canvas_draw_line( cv, x, y+oy, 12, 1, color, SCF_RGB8 );
    sgui_canvas_draw_line( cv, x, y+oy, 12, 0, color, SCF_RGB8 );

    color[0] = color[1] = color[2] = 0xFF;

    sgui_canvas_draw_line( cv, x,    y+11+oy, 12, 1, color, SCF_RGB8 );
    sgui_canvas_draw_line( cv, x+11, y+   oy, 12, 0, color, SCF_RGB8 );

    color[0] = color[1] = color[2] = 0xFF;

    sgui_canvas_draw_text_plain( cv, x+20, y-oy, font_norm, font_height,
                                 color, SCF_RGB8, text,
                                 strlen( (const char*)text ),
                                 0xFFFF );

    if( state )
    {
        sgui_canvas_draw_line( cv, x+2, y+4+oy, 3, 0, color, SCF_RGB8 );
        sgui_canvas_draw_line( cv, x+3, y+5+oy, 3, 0, color, SCF_RGB8 );
        sgui_canvas_draw_line( cv, x+4, y+6+oy, 3, 0, color, SCF_RGB8 );
        sgui_canvas_draw_line( cv, x+5, y+5+oy, 3, 0, color, SCF_RGB8 );
        sgui_canvas_draw_line( cv, x+6, y+4+oy, 3, 0, color, SCF_RGB8 );
        sgui_canvas_draw_line( cv, x+7, y+3+oy, 3, 0, color, SCF_RGB8 );
        sgui_canvas_draw_line( cv, x+8, y+2+oy, 3, 0, color, SCF_RGB8 );
    }
}

void sgui_skin_draw_radio_menu( sgui_canvas* cv, int x, int y,
                                const unsigned char** text,
                                unsigned int num_lines,
                                unsigned int selected )
{
    unsigned char color[4] = { 0xFF, 0xFF, 0xFF, 0xFF };
    unsigned int i, dy = font_height + 10;
    int oy = dy / 2 - 10;

    for( i=0; i<num_lines; ++i, y+=dy )
    {
        color[0] = color[1] = color[2] = 0x00; color[3] = 0x80;

        sgui_canvas_draw_box( cv, x+2, y+2+oy, 8, 8, color, SCF_RGBA8 );

        sgui_canvas_draw_line( cv, x+4,  y+ 1+oy, 4, 1, color, SCF_RGBA8 );
        sgui_canvas_draw_line( cv, x+4,  y+10+oy, 4, 1, color, SCF_RGBA8 );
        sgui_canvas_draw_line( cv, x+1,  y+ 4+oy, 4, 0, color, SCF_RGBA8 );
        sgui_canvas_draw_line( cv, x+10, y+ 4+oy, 4, 0, color, SCF_RGBA8 );

        color[3] = 0xFF;

        sgui_canvas_draw_line( cv, x+2, y+1+oy, 2, 1, color, SCF_RGB8 );
        sgui_canvas_draw_line( cv, x+4, y  +oy, 4, 1, color, SCF_RGB8 );
        sgui_canvas_draw_line( cv, x+8, y+1+oy, 2, 1, color, SCF_RGB8 );

        sgui_canvas_draw_line( cv, x+2, y+1+oy, 2, 1, color, SCF_RGB8 );
        sgui_canvas_draw_line( cv, x+4, y+  oy, 4, 1, color, SCF_RGB8 );
        sgui_canvas_draw_line( cv, x+8, y+1+oy, 2, 1, color, SCF_RGB8 );

        sgui_canvas_draw_line( cv, x+1, y+2+oy, 2, 0, color, SCF_RGB8 );
        sgui_canvas_draw_line( cv, x,   y+4+oy, 4, 0, color, SCF_RGB8 );
        sgui_canvas_draw_line( cv, x+1, y+8+oy, 2, 0, color, SCF_RGB8 );

        color[0] = color[1] = color[2] = 0xFF;

        sgui_canvas_draw_line( cv, x+10, y+2+oy, 2, 0, color, SCF_RGB8 );
        sgui_canvas_draw_line( cv, x+11, y+4+oy, 4, 0, color, SCF_RGB8 );
        sgui_canvas_draw_line( cv, x+10, y+8+oy, 2, 0, color, SCF_RGB8 );

        sgui_canvas_draw_line( cv, x+ 2, y+10+oy, 2, 1, color, SCF_RGB8 );
        sgui_canvas_draw_line( cv, x+ 4, y+11+oy, 4, 1, color, SCF_RGB8 );
        sgui_canvas_draw_line( cv, x+ 8, y+10+oy, 2, 1, color, SCF_RGB8 );

        if( i==selected )
        {
            sgui_canvas_draw_box( cv, x+4, y+3+oy, 4, 6, color, SCF_RGB8 );
            sgui_canvas_draw_box( cv, x+3, y+4+oy, 6, 4, color, SCF_RGB8 );
        }

        sgui_canvas_draw_text_plain( cv, x+20, y, font_norm, font_height,
                                     color, SCF_RGB8, text[ i ],
                                     strlen( (const char*)text[i] ),
                                     0xFFFF );
    }
}

void sgui_skin_draw_edit_box( sgui_canvas* cv, int x, int y,
                              const unsigned char* text,
                              unsigned int width, int cursor )
{
    unsigned char color[4];
    unsigned int height, cx;

    height = font_height + (font_height / 2) + 4;

    /* draw background box */
    color[0] = color[1] = color[2] = 0x00; color[3] = 0x80;

    sgui_canvas_draw_box( cv, x, y, width, height, color, SCF_RGBA8 );

    /* draw text */
    color[0] = color[1] = color[2] = color[3] = 0xFF;

    sgui_canvas_draw_text_plain( cv, x+2, y+2, font_norm, font_height,
                                 color, SCF_RGB8, text,
                                 strlen( (const char*)text ), width-2 );

    /* draw borders */
    color[0] = color[1] = color[2] = 0x00;

    sgui_canvas_draw_line( cv, x, y, width,  1, color, SCF_RGB8 );
    sgui_canvas_draw_line( cv, x, y, height, 0, color, SCF_RGB8 );

    color[0] = color[1] = color[2] = 0xFF;

    sgui_canvas_draw_line( cv, x, y+height-1, width,  1, color, SCF_RGB8 );
    sgui_canvas_draw_line( cv, x+width-1, y,  height, 0, color, SCF_RGB8 );

    /* draw cursor */
    if( cursor >= 0 )
    {
        cx = sgui_font_extents( text, font_norm, font_height, cursor );

        if( cx == 0 )
            cx = 3;

        if( cx < (width-2) )
        {
            color[0] = color[1] = color[2] = 0x7F;

            sgui_canvas_draw_line( cv, x+cx, y+5, height-10, 0, color,
                                   SCF_RGB8 );
        }
    }
}

