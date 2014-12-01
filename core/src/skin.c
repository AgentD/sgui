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
#include "sgui_filesystem.h"
#include "sgui_internal.h"
#include "sgui_skin.h"
#include "sgui_font.h"
#include "sgui_utf8.h"

#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>



static sgui_skin* skin;



/*
    text:   text string to process
    canvas: canvas for drawing if "draw" is non-zero
    x:      offset from the left if drawing text
    y:      offset from the topy if drawing text
    r:      if "draw" is zero, a pointer to a rect returning the outlines of
            the text
    draw:   non-zero to draw the text, zero to measure the outlines
 */
static void process_text( const char* text, sgui_canvas* canvas, int x, int y,
                          sgui_rect* r, int draw )
{
    unsigned int i, X = 0, Y = 0, longest = 0, font_stack_index = 0;
    unsigned char col[3], font_stack[10], f = 0;
    long c;

    /* sanity check */
    if( !text || (draw && !canvas) || (!draw && !r) )
        return;

    memcpy( col, skin->font_color, 3 );

    while( text && *text )
    {
        /* count characters until tag, line break or terminator */
        for( i=0; text[i] && text[i]!='<' && text[i]!='\n'; ++i ) { }

        /* process what we got so far with the current settings */
        if( draw )
        {
            X += sgui_canvas_draw_text_plain( canvas, x+X, y+Y, f&0x02, f&0x01,
                                              col, text, i );
        }
        else
        {
            X += sgui_skin_default_font_extents( text, i, f&0x02, f&0x01 );
        }

        if( text[ i ] == '<' )
        {
            if( !strncmp( text+i+1, "color=\"", 7 ) )
            {
                if( !strncmp( text+i+8, "default", 7 ) )
                {
                    memcpy( col, skin->font_color, 3 );
                }
                else if( text[ i+8 ]=='#' )
                {
                    c = strtol( text+i+9, NULL, 16 );

                    col[0] = (c>>16) & 0xFF;
                    col[1] = (c>>8 ) & 0xFF;
                    col[2] =  c      & 0xFF;
                }
            }
            else if( text[ i+1 ] == 'b' )
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

            while( text[ i ] && text[ i ]!='>' )
                ++i;
        }
        else if( text[ i ]=='\n' )
        {
            longest = X>longest ? X : longest;
            X = 0;                  /* carriage return */
            Y += skin->font_height; /* line feed */
        }

        text += text[i] ? (i + 1) : i;
    }

    if( r )
    {
        longest = X>longest ? X : longest;
        Y += skin->font_height;

        /* Add font height/2 as fudge factor because
           characters can peek out below the line */
        sgui_rect_set_size( r, x, y, longest, Y + skin->font_height/2 );
    }
}





void sgui_skin_set( sgui_skin* ui_skin )
{
    sgui_interal_skin_init_default( );
    skin = ui_skin ? ui_skin : &sgui_default_skin;
}

sgui_skin* sgui_skin_get( void )
{
    return skin ? skin : &sgui_default_skin;
}

sgui_font* sgui_skin_get_default_font( int bold, int italic )
{
    if( bold && italic ) return skin->font_boit;
    if( bold           ) return skin->font_bold;
    if( italic         ) return skin->font_ital;

    return skin->font_norm;
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
    process_text( text, NULL, 0, 0, r, 0 );
}

void sgui_skin_draw_text( sgui_canvas* canvas, int x, int y,
                          const char* text )
{
    process_text( text, canvas, x, y, NULL, 1 );
}

