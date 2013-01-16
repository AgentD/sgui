/*
 * font.c
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
#include "sgui_canvas.h"
#include "sgui_font.h"
#include "sgui_filesystem.h"
#include "sgui_skin.h"
#include "sgui_utf8.h"

#include <string.h>
#include <stdlib.h>

#include <ft2build.h>
#include FT_FREETYPE_H



struct sgui_font
{
    FT_Library freetype;
    FT_Face face;
    void* buffer;
    unsigned int height;
    unsigned int current_glyph;
};



sgui_font* sgui_font_load( const sgui_filesystem* fs, const char* filename )
{
    sgui_font* font;
    void* file;
    void* buffer = NULL;
    size_t size;

#ifdef SGUI_NO_STDIO
    /* make sure we have a valid filesystem pointer */
    if( !fs )
        return NULL;
#endif

    /* allocate font structure */
    font = malloc( sizeof(sgui_font) );

    if( !font )
        return NULL;

    if( FT_Init_FreeType( &font->freetype ) )
    {
        free( font );
        return NULL;
    }

    /* read the file if we use a virtual filesystem */
#ifndef SGUI_NO_STDIO
    if( fs )
    {
#endif
        /* try to open the file */
        file = fs->file_open_read( filename );

        if( !file )
        {
            free( font );
            return NULL;
        }

        /* determine the length of the file */
        size = fs->file_get_length( file );

        if( !size )
        {
            fs->file_close( file );
            free( font );
            return NULL;
        }

        /* allocate a buffer for the file */
        buffer = malloc( size );

        if( !buffer )
        {
            fs->file_close( file );
            free( font );
            return NULL;
        }

        /* load the file into the buffer */
        fs->file_read( file, buffer, 1, size );
        fs->file_close( file );

        /* initialise the font */
        if( FT_New_Memory_Face(font->freetype, buffer, size, 0, &font->face) )
        {
            FT_Done_FreeType( font->freetype );
            free( buffer );
            free( font );
            return NULL;
        }
#ifndef SGUI_NO_STDIO
    }
    else
    {
        if( FT_New_Face( font->freetype, filename, 0, &font->face ) )
        {
            FT_Done_FreeType( font->freetype );
            free( font );
            return NULL;
        }
    }
#endif

    font->buffer = buffer;
    font->height = 0;

    return font;
}

void sgui_font_destroy( sgui_font* font )
{
    if( font )
    {
        FT_Done_Face( font->face );
        FT_Done_FreeType( font->freetype );

        free( font->buffer );
        free( font );
    }
}

void sgui_font_set_height( sgui_font* font, unsigned int pixel_height )
{
    if( font )
    {
        font->height = pixel_height;
        FT_Set_Pixel_Sizes( font->face, 0, pixel_height );
    }
}

void sgui_font_load_glyph( sgui_font* font, unsigned int codepoint )
{
    FT_UInt index;

    if( font )
    {
        font->current_glyph = codepoint;

        index = FT_Get_Char_Index( font->face, codepoint );

        FT_Load_Glyph( font->face, index, FT_LOAD_DEFAULT );
        FT_Render_Glyph( font->face->glyph, FT_RENDER_MODE_NORMAL );
    }
}

int sgui_font_get_kerning_distance( sgui_font* font, unsigned int first,
                                    unsigned int second )
{
    FT_Vector delta;
    FT_UInt index_a, index_b;

    if( font && FT_HAS_KERNING( font->face ) )
    {
        index_a = FT_Get_Char_Index( font->face, first );
        index_b = FT_Get_Char_Index( font->face, second );

        FT_Get_Kerning( font->face, index_a, index_b,
                        FT_KERNING_DEFAULT, &delta );

        return -((delta.x < 0 ? -delta.x : delta.x) >> 6);
    }

    return 0;
}

void sgui_font_get_glyph_metrics( sgui_font* font, unsigned int* width,
                                  unsigned int* height, int* bearing )
{
    unsigned int w = 0, h = 0;
    int b = 0;

    if( font )
    {
        if( font->current_glyph == ' ' )
        {
            w = font->height / 3;
            h = font->height;
            b = 0;
        }
        else
        {
            w = font->face->glyph->bitmap.width;
            h = font->face->glyph->bitmap.rows;
            b = font->height - font->face->glyph->bitmap_top;
        }
    }

    if( width   ) *width   = w;
    if( height  ) *height  = h;
    if( bearing ) *bearing = b;
}

unsigned char* sgui_font_get_glyph( sgui_font* font )
{
    if( font && font->current_glyph != ' ' )
    {
        return font->face->glyph->bitmap.buffer;
    }

    return NULL;
}






unsigned int sgui_font_get_text_extents_plain( sgui_font* font_face,
                                               const char* text,
                                               unsigned int length )
{
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

        /* store previous glyph index for kerning */
        previous = character;
    }

    return x;
}

void sgui_font_get_text_extents( const char* text,
                                 unsigned int* width, unsigned int* height )
{
    int i = 0, font_stack_index = 0;
    unsigned int X = 0, longest = 0, lines = 1;
    sgui_font *f, *font_norm, *font_bold, *font_ital, *font_boit;
    sgui_font* font_stack[10];

    /* sanity check */
    if( !text || (!width && !height) )
        return;

    font_norm = sgui_skin_get_default_font( 0, 0 );
    font_bold = sgui_skin_get_default_font( 1, 0 );
    font_ital = sgui_skin_get_default_font( 0, 1 );
    font_boit = sgui_skin_get_default_font( 1, 1 );

    f = font_norm;

    for( ; text && text[ i ]; ++i )
    {
        if( text[ i ] == '<' )  /* we found a tag */
        {
            /* get extends for what we found so far */
            X += sgui_font_get_text_extents_plain( f, text, i );

            /* change fonts accordingly */
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
            X += sgui_font_get_text_extents_plain( f, text, i );

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
    X += sgui_font_get_text_extents_plain( f, text, i );

    if( X > longest )
        longest = X;

    /* store width and height */
    if( width  ) *width  = longest;
    if( height ) *height = lines * font_norm->height;

    /* Add font height/2 as fudge factor to the height, because our crude
       computation here does not take into account that characters can peek
       out below the line */
    if( height && *height )
        *height += font_norm->height/2;
}




void sgui_font_draw_text_plain( sgui_canvas* canvas, int x, int y,
                                sgui_font* font_face,
                                unsigned char* color,
                                const char* text, unsigned int length )
{
    int bearing;
    unsigned int i, w, h, len = 0;
    unsigned long character, previous=0;
    unsigned char* buffer;

    /* sanity check */
    if( !canvas || !font_face || !color || !text )
        return;

    /* for each character */
    for( i=0; i<length && (*text) && (*text!='\n'); text+=len, i+=len )
    {
        /* load the next glyph */
        character = sgui_utf8_decode( text, &len );
        sgui_font_load_glyph( font_face, character );

        /* apply kerning */
        x += sgui_font_get_kerning_distance( font_face, previous, character );

        /* blend onto destination buffer */
        sgui_font_get_glyph_metrics( font_face, &w, &h, &bearing );
        buffer = sgui_font_get_glyph( font_face );

        if( sgui_canvas_blend_stencil( canvas, buffer,
                                       x, y+bearing, w, h, color ) > 0 )
        {
            break;
        }

        /* advance cursor */
        x += w + 1;

        /* store previous glyph index for kerning */
        previous = character;
    }
}

void sgui_font_draw_text(sgui_canvas* canvas, int x, int y, const char* text)
{
    int i = 0, X = 0, font_stack_index = 0;
    sgui_font *f, *font_norm, *font_bold, *font_ital, *font_boit;
    sgui_font* font_stack[10];
    unsigned char col[3];
    long c;

    /* sanity check */
    if( !canvas || !text )
        return;

    sgui_skin_get_default_font_color( col );

    font_norm = sgui_skin_get_default_font( 0, 0 );
    font_bold = sgui_skin_get_default_font( 1, 0 );
    font_ital = sgui_skin_get_default_font( 0, 1 );
    font_boit = sgui_skin_get_default_font( 1, 1 );

    f = font_norm;

    for( ; text && text[ i ]; ++i )
    {
        if( text[ i ] == '<' )  /* we encountered a tag */
        {
            /* draw what we got so far with the current settings */
            sgui_font_draw_text_plain( canvas, x+X, y, f, col, text, i );

            /* advance cursor */
            X += sgui_font_get_text_extents_plain( f, text, i );

            if( !strncmp( text+i+1, "color=", 6 ) ) /* it's a color tag */
            {
                if( !strncmp( text+i+9, "default", 7 ) )
                {
                    sgui_skin_get_default_font_color( col );
                }
                else
                {
                    c = strtol( text+i+9, NULL, 16 );

                    col[0] = (c>>16) & 0xFF;
                    col[1] = (c>>8 ) & 0xFF;
                    col[2] =  c      & 0xFF;
                }
            }
            else if( text[ i+1 ] == 'b' )   /* it's a <b> tag */
            {
                font_stack[ font_stack_index++ ] = f;
                f = f==font_ital ? font_boit : font_bold;
            }
            else if( text[ i+1 ] == 'i' )   /* it's an <i> tag */
            {
                font_stack[ font_stack_index++ ] = f;
                f = f==font_bold ? font_boit : font_ital;
            }
            else if( text[ i+1 ] == '/' && font_stack_index )   /* end tag */
            {
                f = font_stack[ --font_stack_index ];   /* pop from stack */
            }

            /* skip to the end of the tag */
            if( (text = strchr( text+i, '>' )) )
                ++text;

            i = -1; /* reset i to 0 at next iteration */
        }
        else if( text[ i ] == '\n' )
        {
            /* draw what we got so far */
            sgui_font_draw_text_plain( canvas, x+X, y, f, col, text, i );

            text += i + 1;    /* skip to next line */
            i = -1;           /* reset i to 0 at next iteration */
            X = 0;            /* adjust move cursor */
            y += font_norm->height;
        }
    }

    /* draw what is still left */
    sgui_font_draw_text_plain( canvas, x+X, y, f, col, text, i );
}

