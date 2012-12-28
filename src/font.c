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

#include <string.h>
#include <stdlib.h>

#include <ft2build.h>
#include FT_FREETYPE_H



struct sgui_font
{
    FT_Library freetype;
    FT_Face face;
    void* buffer;
};



int utf8_char_length( unsigned char c )
{
    c >>= 3;

    if( c == 0x1E )
        return 4;

    c >>= 1;

    if( c == 0x0E )
        return 3;

    c >>= 1;

    return (c==0x06) ? 2 : 1;
}

unsigned long to_utf32( const unsigned char* utf8, int* length )
{
    unsigned long ch = 0;
    int i;

    *length = utf8_char_length( *utf8 );

    switch( *length )
    {
    case 4: ch = (*utf8 ^ 0xf0); break;
    case 3: ch = (*utf8 ^ 0xe0); break;
    case 2: ch = (*utf8 ^ 0xc0); break;
    case 1: ch =  *utf8;         break;
    }

    ++utf8;

    for( i=*length; i>1; --i, ++utf8 )
    {
        ch <<= 6;
        ch |= (*utf8 ^ 0x80);
    }

    return ch;
}

sgui_font* sgui_font_load( const sgui_filesystem* fs, const char* filename )
{
    sgui_font* font;
    void* file;
    void* buffer;
    size_t size;

    /* make sure we have a valid filesystem pointer */
    if( !fs )
        fs = sgui_filesystem_get_default( );

    if( !fs )
        return NULL;

    /* try to open the file */
    file = fs->file_open_read( filename );

    if( !file )
        return NULL;

    /* determine the length of the file */
    size = fs->file_get_length( file );

    if( !size )
    {
        fs->file_close( file );
        return NULL;
    }

    /* allocate a buffer for the file */
    buffer = malloc( size );

    if( !buffer )
    {
        fs->file_close( file );
        return NULL;
    }

    /* load the file into the buffer */
    fs->file_read( file, buffer, 1, size );
    fs->file_close( file );

    /* allocate font structure */
    font = malloc( sizeof(sgui_font) );

    if( !font )
    {
        free( buffer );
        return NULL;
    }

    /* load font */
    if( FT_Init_FreeType( &font->freetype ) )
    {
        free( buffer );
        free( font );
        return NULL;
    }

    if( FT_New_Memory_Face( font->freetype, buffer, size, 0, &font->face ) )
    {
        FT_Done_FreeType( font->freetype );
        free( buffer );
        free( font );
        return NULL;
    }

    font->buffer = buffer;

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

unsigned int sgui_font_get_text_extents_plain( sgui_font* font_face,
                                               unsigned int font_height,
                                               const char* text,
                                               unsigned int length )
{
    unsigned int x = 0;
    unsigned long character;
    int len = 0;
    FT_UInt glyph_index = 0;
    FT_UInt previous = 0;
    FT_Bool useKerning;
    unsigned int i;

    /* sanity check */
    if( !text || !font_face || !font_height || !length )
        return 0;

    /* set rendering pixel size */
    FT_Set_Pixel_Sizes( font_face->face, 0, font_height );

    useKerning = FT_HAS_KERNING( font_face->face );

    /* for each character */
    for( i=0; i<length && (*text) && (*text!='\n'); text+=len, i+=len )
    {
        /* space must be handled manually */
        if( *text == ' ' )
        {
            x += (font_height/3);
            len = 1;
            continue;
        }

        /* UTF8 -> UTF32 -> glyph index */
        character = to_utf32( (const unsigned char*)text, &len );
        glyph_index = FT_Get_Char_Index( font_face->face, character );

        /* load and render */
        FT_Load_Glyph( font_face->face, glyph_index, FT_LOAD_DEFAULT );
        FT_Render_Glyph( font_face->face->glyph, FT_RENDER_MODE_NORMAL );

        /* apply kerning to cursor position */
        if( useKerning && previous && glyph_index )
        {
            FT_Vector delta;
            FT_Get_Kerning( font_face->face, previous, glyph_index,
                            FT_KERNING_DEFAULT, &delta );
            x -= (abs( delta.x ) >> 6);
        }

        /* advance cursor */
        x += font_face->face->glyph->bitmap.width + 1;

        /* store previous glyph index for kerning */
        previous = glyph_index;
    }

    return x;
}

void sgui_font_get_text_extents( sgui_font* font_norm, sgui_font* font_bold,
                                 sgui_font* font_ital, sgui_font* font_boit,
                                 unsigned int font_height, const char* text,
                                 unsigned int* width, unsigned int* height )
{
    int i = 0, font_stack_index = 0;
    unsigned int X = 0, longest = 0, lines = 1;
    sgui_font* f = font_norm;
    sgui_font* font_stack[10];

    /* sanity check */
    if( !font_norm || !font_height || !text || (!width && !height) )
        return;

    for( ; text && text[ i ]; ++i )
    {
        if( text[ i ] == '<' )  /* we found a tag */
        {
            /* get extends for what we found so far */
            X += sgui_font_get_text_extents_plain( f, font_height, text, i );

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
            X += sgui_font_get_text_extents_plain( f, font_height, text, i );

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
    X += sgui_font_get_text_extents_plain( f, font_height, text, i );

    if( X > longest )
        longest = X;

    /* store width and height */
    if( width  ) *width  = longest;
    if( height ) *height = lines * font_height;

    /* Add font_height/2 as fudge factor to the height, because our crude
       computation here does not take into account that characters can peek
       out below the line */
    if( height && *height )
        *height += font_height/2;
}




void sgui_font_draw_text_plain( sgui_canvas* canvas, int x, int y,
                                sgui_font* font_face,
                                unsigned int font_height,
                                unsigned char* color,
                                const char* text, unsigned int length )
{
    FT_UInt glyph_index = 0;
    FT_UInt previous = 0;
    FT_GlyphSlot glyph;
    FT_Bool useKerning;
    int len = 0, bearing;
    unsigned int i;
    unsigned long character;

    /* sanity check */
    if( !canvas || !font_face || !font_height )
        return;

    /* set rendering parameters */
    FT_Set_Pixel_Sizes( font_face->face, 0, font_height );

    useKerning = FT_HAS_KERNING( font_face->face );

    /* for each character */
    for( i=0; i<length && (*text) && (*text!='\n'); text+=len, i+=len )
    {
        /* space must be handled manually */
        if( *text == ' ' )
        {
            x += ((int)font_height/3);
            len = 1;
            continue;
        }

        /* UTF8 -> UTF32 -> glyph index */
        character = to_utf32( (const unsigned char*)text, &len );
        glyph_index = FT_Get_Char_Index( font_face->face, character );

        /* load and render glyph */
        FT_Load_Glyph( font_face->face, glyph_index, FT_LOAD_DEFAULT );
        FT_Render_Glyph( font_face->face->glyph, FT_RENDER_MODE_NORMAL );

        /* apply kerning */
        if( useKerning && previous && glyph_index )
        {
            FT_Vector delta;
            FT_Get_Kerning( font_face->face, previous, glyph_index,
                            FT_KERNING_DEFAULT, &delta );
            x -= abs( delta.x ) >> 6;
        } 

        /* blend onto destination buffer */
        glyph = font_face->face->glyph;

        bearing = font_height - glyph->bitmap_top;

        if( sgui_canvas_blend_stencil( canvas, glyph->bitmap.buffer,
                                       x, y+bearing,
                                       glyph->bitmap.width,
                                       glyph->bitmap.rows,
                                       color ) > 0 )
        {
            break;
        }

        /* advance cursor */
        x += font_face->face->glyph->bitmap.width + 1;

        /* store previous glyph index for kerning */
        previous = glyph_index;
    }
}

void sgui_font_draw_text( sgui_canvas* canvas, int x, int y,
                          sgui_font* font_norm, sgui_font* font_bold,
                          sgui_font* font_ital, sgui_font* font_boit,
                          unsigned int font_height, unsigned char* color,
                          const char* text )
{
    int i = 0, X = 0, font_stack_index = 0;
    sgui_font* f = font_norm;
    sgui_font* font_stack[10];
    unsigned char col[3];
    long c;

    /* sanity check */
    if( !canvas || !font_norm || !font_height || !color || !text )
        return;

    col[0] = color[0];
    col[1] = color[1];
    col[2] = color[2];

    for( ; text && text[ i ]; ++i )
    {
        if( text[ i ] == '<' )  /* we encountered a tag */
        {
            /* draw what we got so far with the current settings */
            sgui_font_draw_text_plain( canvas, x+X, y, f, font_height, col,
                                       text, i );

            /* advance cursor */
            X += sgui_font_get_text_extents_plain( f, font_height, text, i );

            if( !strncmp( text+i+1, "color=", 6 ) ) /* it's a color tag */
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
            sgui_font_draw_text_plain( canvas, x+X, y, f, font_height, col,
                                       text, i );

            text += i + 1;    /* skip to next line */
            i = -1;           /* reset i to 0 at next iteration */
            X = 0;            /* adjust move cursor */
            y += font_height;
        }
    }

    /* draw what is still left */
    sgui_font_draw_text_plain( canvas, x+X, y, f, font_height, col, text, i );
}

