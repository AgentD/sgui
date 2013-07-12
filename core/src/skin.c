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
#include "sgui_pixmap.h"
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



static sgui_skin skin;



void sgui_skin_set( sgui_skin* ui_skin )
{
    if( ui_skin )
    {
        memcpy( &skin, ui_skin, sizeof(sgui_skin) );
    }
    else
    {
        sgui_skin_init_default( &skin );
    }
}

void sgui_skin_get_pixmap_size( unsigned int* width, unsigned int* height )
{
    if( width  ) *width  = skin.pixmap_width;
    if( height ) *height = skin.pixmap_height;
}

void sgui_skin_to_pixmap( sgui_pixmap* pixmap )
{
    skin.load_to_pixmap( pixmap );
}

void sgui_skin_get_element( unsigned int element, sgui_rect* r )
{
    if( r && (element < SGUI_SKIN_ELEMENTS) )
    {
        sgui_rect_copy( r, skin.elements + element );
    }
}

void sgui_skin_set_default_font( sgui_font* normal, sgui_font* bold,
                                 sgui_font* italic, sgui_font* bold_italic )
{
    font_norm = normal;
    font_bold = bold;
    font_ital = italic;
    font_boit = bold_italic;
}

void sgui_skin_get_window_background_color( unsigned char* color )
{
    if( color )
    {
        color[0] = skin.window_color[0];
        color[1] = skin.window_color[1];
        color[2] = skin.window_color[2];
        color[3] = skin.window_color[3];
    }
}

void sgui_skin_get_default_font_color( unsigned char* color )
{
    if( color )
    {
        color[0] = skin.font_color[0];
        color[1] = skin.font_color[1];
        color[2] = skin.font_color[2];
        color[3] = skin.font_color[3];
    }
}

unsigned int sgui_skin_get_default_font_height( void )
{
    return skin.font_height;
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
    r->bottom = lines * skin.font_height + skin.font_height/2 - 1;
}

