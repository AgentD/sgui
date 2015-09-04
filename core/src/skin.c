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



#define ITALIC 0x01
#define BOLD 0x02



static struct
{
    const char* entity;
    const char* subst;
}
entities[] =
{
    { "&lt;",  "<" },
    { "&gt;",  ">" },
    { "&amp;", "&" },
};



/*
    text:   text string to process
    canvas: canvas for drawing if "draw" is non-zero
    x:      offset from the left if drawing text
    y:      offset from the topy if drawing text
    r:      if "draw" is zero, a pointer to a rect returning the text outlines
    draw:   non-zero to draw the text, zero to measure the outlines
 */
static void process_text( const char* text, sgui_canvas* canvas, int x, int y,
                          sgui_rect* r, int draw )
{
    unsigned int i, c, X = 0, Y = 0, longest = 0, font_stack_index = 0;
    unsigned char col[3], font_stack[10], f = 0;
    char *end, buffer[8];
    const char* subst;

    memcpy( col, skin->font_color, 3 );

    while( text && *text )
    {
        /* count characters until tag, entity, line break or terminator */
        for( i=0; text[i]&&text[i]!='<'&&text[i]!='&'&&text[i]!='\n'; ++i )
        {
        }

        /* process what we got so far with the current settings */
        if( draw )
        {
            X += sgui_canvas_draw_text_plain( canvas, x+X, y+Y,
                                              f&BOLD, f&ITALIC,
                                              col, text, i );
        }
        else
        {
            X += sgui_skin_default_font_extents( text, i, f&BOLD, f&ITALIC );
        }

        if( text[ i ] == '<' )
        {
            if( !strncmp( text+i, "<color=\"default\">", 17 ) )
            {
                memcpy( col, skin->font_color, 3 );
            }
            else if( !strncmp( text+i, "<color=\"#", 9 ) )
            {
                c = strtol( text+i+9, &end, 16 );
                if( end && !strncmp( end, "\">", 2 ) && (end-(text+i+9))==6 )
                {
                    col[0] = (c>>16) & 0xFF;
                    col[1] = (c>>8 ) & 0xFF;
                    col[2] =  c      & 0xFF;
                }
            }
            else if( !strncmp( text+i, "<b>", 3 ) )
            {
                if( (font_stack_index+1)<sizeof(font_stack) )
                {
                    font_stack[ font_stack_index++ ] = f;
                    f |= BOLD;
                }
            }
            else if( !strncmp( text+i, "<i>", 3 ) )
            {
                if( (font_stack_index+1)<sizeof(font_stack) )
                {
                    font_stack[ font_stack_index++ ] = f;
                    f |= ITALIC;
                }
            }
            else if( !strncmp( text+i, "</b>", 4 ) && font_stack_index )
            {
                if( (f&BOLD) && !(font_stack[font_stack_index-1]&BOLD) )
                    f = font_stack[ --font_stack_index ];
            }
            else if( !strncmp( text+i, "</i>", 4 ) && font_stack_index )
            {
                if( (f&ITALIC) && !(font_stack[font_stack_index-1]&ITALIC) )
                    f = font_stack[ --font_stack_index ];
            }

            while( text[ i ] && text[ i ]!='>' )
                ++i;
        }
        else if( text[ i ]=='&' )
        {
            for(subst=NULL, c=0; c<sizeof(entities)/sizeof(entities[0]); ++c)
            {
                if( !strncmp( text+i, entities[c].entity,
                              strlen(entities[c].entity) ) )
                {
                    subst = entities[c].subst;
                    break;
                }
            }

            if( !subst && !strncmp( text+i, "&#h", 3 ) )
            {
                memset( buffer, 0, sizeof(buffer) );
                sgui_utf8_encode( strtol( text+i+3, NULL, 16 ), buffer );
                subst = buffer;
            }
            else if( !subst && !strncmp( text+i, "&#", 2 ) )
            {
                memset( buffer, 0, sizeof(buffer) );
                sgui_utf8_encode( strtol( text+i+2, NULL, 10 ), buffer );
                subst = buffer;
            }

            if( draw )
            {
                X += sgui_canvas_draw_text_plain( canvas, x+X, y+Y, f&BOLD,
                                                  f&ITALIC, col, subst, -1 );
            }
            else
            {
                X += sgui_skin_default_font_extents( subst, -1,
                                                     f&BOLD, f&ITALIC );
            }

            while( text[ i ] && text[ i ]!=';' )
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

    /* account for last line */
    longest = X>longest ? X : longest;
    Y += skin->font_height;

    /* HACK: Add font height/2 because characters can peek below the line */
    if( !draw )
        sgui_rect_set_size( r, x, y, longest, Y + skin->font_height/2 );
}

/****************************************************************************/

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

    /* for each character */
    for( i=0; i<length && (*text) && (*text!='\n'); text+=len, i+=len )
    {
        /* load the next glyph */
        character = sgui_utf8_decode( text, &len );
        font_face->load_glyph( font_face, character );

        /* advance cursor */
        x += font_face->get_kerning_distance(font_face, previous, character);
        font_face->get_glyph_metrics( font_face, &w, NULL, NULL );

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

