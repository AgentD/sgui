/*
 * canvas.c
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
#include "sgui_canvas.h"

#include <string.h>
#include <stdlib.h>



void sgui_font_get_text_extents( sgui_font* font_norm, sgui_font* font_bold,
                                 sgui_font* font_ital, sgui_font* font_boit,
                                 unsigned int font_height, const char* text,
                                 unsigned int* width, unsigned int* height )
{
    int i = 0, font_stack_index = 0;
    unsigned int X = 0, longest = 0, lines = 1;
    sgui_font* f = font_norm;
    sgui_font* font_stack[10];

    if( !font_norm || !font_height || !text || (!width && !height) )
        return;

    for( ; text && text[ i ]; ++i )
    {
        if( text[ i ] == '<' )
        {
            X += sgui_font_get_text_extents_plain( f, font_height, text, i );

            if( text[ i+1 ] == 'b' )
            {
                font_stack[ font_stack_index++ ] = f;
                f = f==font_ital ? font_boit : font_bold;
            }
            else if( text[ i+1 ] == 'i' )
            {
                font_stack[ font_stack_index++ ] = f;
                f = f==font_bold ? font_boit : font_ital;
            }
            else if( text[ i+1 ] == '/' && font_stack_index )
            {
                f = font_stack[ --font_stack_index ];
            }

            text = strchr( text+i, '>' );

            if( text )
                ++text;

            i = -1;
        }
        else if( text[ i ] == '\n' )
        {
            X += sgui_font_get_text_extents_plain( f, font_height, text, i );

            if( X > longest )
                longest = X;

            ++lines;
            text += i + 1;
            i = -1;
            X = 0;
        }
    }

    X += sgui_font_get_text_extents_plain( f, font_height, text, i );

    if( X > longest )
        longest = X;

    if( width  ) *width  = longest;
    if( height ) *height = lines * font_height;
}

void sgui_canvas_draw_text( sgui_canvas* canvas, int x, int y,
                            sgui_font* font_norm, sgui_font* font_bold,
                            sgui_font* font_ital, sgui_font* font_boit,
                            unsigned int font_height, unsigned char* color,
                            SGUI_COLOR_FORMAT format, const char* text )
{
    int i = 0, X = 0, font_stack_index = 0;
    sgui_font* f = font_norm;
    sgui_font* font_stack[10];
    unsigned char col[3];
    long c;

    if( !canvas || !font_norm || !font_height || !color || !text )
        return;

    col[0] = color[0];
    col[1] = color[1];
    col[2] = color[2];

    for( ; text && text[ i ]; ++i )
    {
        if( text[ i ] == '<' )
        {
            sgui_canvas_draw_text_plain( canvas, x+X, y, f, font_height, col,
                                         format, text, i );

            X += sgui_font_get_text_extents_plain( f, font_height, text, i );

            if( !strncmp( text+i+1, "color=", 6 ) )
            {
                if( !strncmp( text+i+9, "default", 7 ) )
                {
                    col[0] = color[0];
                    col[1] = color[1];
                    col[2] = color[2];
                }
                else
                {
                    c = strtol( text+i+9, NULL, 16 );

                    if( format==SCF_RGBA8 || format==SCF_RGB8 )
                    {
                        col[0] = (c>>16) & 0xFF;
                        col[1] = (c>>8 ) & 0xFF;
                        col[2] =  c      & 0xFF;
                    }
                    else
                    {
                        col[2] = (c>>16) & 0xFF;
                        col[1] = (c>>8 ) & 0xFF;
                        col[0] =  c      & 0xFF;
                    }
                }
            }
            else if( text[ i+1 ] == 'b' )
            {
                font_stack[ font_stack_index++ ] = f;
                f = f==font_ital ? font_boit : font_bold;
            }
            else if( text[ i+1 ] == 'i' )
            {
                font_stack[ font_stack_index++ ] = f;
                f = f==font_bold ? font_boit : font_ital;
            }
            else if( text[ i+1 ] == '/' && font_stack_index )
            {
                f = font_stack[ --font_stack_index ];
            }

            text = strchr( text+i, '>' );

            if( text )
                ++text;

            i = -1;
        }
        else if( text[ i ] == '\n' )
        {
            sgui_canvas_draw_text_plain( canvas, x+X, y, f, font_height, col,
                                         format, text, i );

            text += i + 1;
            i = -1;
            X = 0;
            y += font_height;
        }
    }

    sgui_canvas_draw_text_plain( canvas, x+X, y, f, font_height, col,
                                 format, text, i );
}

