/*
 * font_manager.c
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
#include "sgui_font_manager.h"

#include <ft2build.h>
#include FT_FREETYPE_H



FT_Library freetype;



struct sgui_font
{
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
    unsigned long ch;
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

    for( i=*length; i>1; --i )
    {
        ch <<= 6;
        ch |= (*utf8 ^ 0x80);
        ++utf8;
    }

    return ch;
}





int sgui_font_init( void )
{
    return (FT_Init_FreeType( &freetype )==0);
}

void sgui_font_deinit( void )
{
    FT_Done_FreeType( freetype );
}



sgui_font* sgui_font_load_from_file( const char* filename )
{
    sgui_font* font;

    /* allocate font structure */
    font = malloc( sizeof(sgui_font) );

    if( !font )
        return NULL;

    /* load font */
    if( FT_New_Face( freetype, filename, 0, &font->face ) )
    {
        free( font );
        return NULL;
    }

    font->buffer = NULL;

    return font;
}

sgui_font* sgui_font_load_from_mem( void* buffer, unsigned int buffersize )
{
    sgui_font* font;

    /* sanity check */
    if( !buffer || !buffersize )
        return NULL;

    /* allocate font structure */
    font = malloc( sizeof(sgui_font) );

    if( !font )
        return NULL;

    /* load font */
    if( FT_New_Memory_Face( freetype, buffer, buffersize, 0, &font->face ) )
    {
        free( font );
        free( buffer );
        return NULL;
    }

    font->buffer = buffer;

    return font;
}

void sgui_font_destroy( sgui_font* font )
{
    FT_Done_Face( font->face );

    free( font->buffer );
    free( font );
}

void sgui_font_print( const unsigned char* text, sgui_font* font_face,
                      unsigned int font_height, unsigned char* buffer,
                      int x, int y, unsigned int width, unsigned int height,
                      unsigned int max_width,
                      unsigned char* color, unsigned int length, int alpha )
{
    FT_UInt glyph_index = 0;
    FT_UInt previous = 0;
    int len = 0, X, Y, bearing;
    FT_Bool useKerning;
    unsigned long character;
    unsigned char *src, *dst;
    unsigned int i, bpp = alpha ? 4 : 3;

    FT_Set_Pixel_Sizes( font_face->face, 0, font_height );

    useKerning = FT_HAS_KERNING( font_face->face ); 

    for( i=0; i<length && (*text) && (*text!='\n'); text+=len, i+=len )
    {
        if( *text == ' ' )
        {
            x += ((int)font_height/3);
            len = 1;
            continue;
        }

        /* UTF8 -> UTF32 -> glyph index */
        character = to_utf32( text, &len );

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
        bearing = font_height - font_face->face->glyph->bitmap_top;
        src = font_face->face->glyph->bitmap.buffer;

        for( Y=0; Y<font_face->face->glyph->bitmap.rows; ++Y )
        {
            dst = buffer + ((y+Y+bearing)*width + x)*bpp;

            if( ((y+Y+bearing) < 0) || ((y+Y+bearing) >= (int)height) )
                continue;

            for( X=0; X<font_face->face->glyph->bitmap.width; ++X, ++src,
                                                              dst+=bpp )
            {
                float value = (*src) / 255.0f;

                if( ((x+X) < 0) || ((x+X) >= (int)max_width) )
                    continue;

                dst[0] = color[0]*value + dst[0]*(1.0f-value);
                dst[1] = color[1]*value + dst[1]*(1.0f-value);
                dst[2] = color[2]*value + dst[2]*(1.0f-value);
            }
        }

        x += font_face->face->glyph->bitmap.width + 1;
        previous = glyph_index;
    }
}

unsigned int sgui_font_extents( const unsigned char* text,
                                sgui_font* font_face,
                                unsigned int height, unsigned int length )
{
    unsigned int x = 0;
    unsigned long character;
    int len = 0;
    FT_UInt glyph_index = 0;
    FT_UInt previous = 0;
    FT_Bool useKerning;
    unsigned int i;

    /* sanity check */
    if( !text || !font_face || !height )
        return 0;

    /* set rendering pixel size */
    FT_Set_Pixel_Sizes( font_face->face, 0, height );

    useKerning = FT_HAS_KERNING( font_face->face );

    for( i=0; i<length && (*text) && (*text!='\n'); text+=len, i+=len )
    {
        if( *text == ' ' )
        {
            x += (height/3);
            len = 1;
            continue;
        }

        /* UTF8 -> UTF32 -> glyph index */
        character = to_utf32( text, &len );

        glyph_index = FT_Get_Char_Index( font_face->face, character );

        /* load and render */
        FT_Load_Glyph( font_face->face, glyph_index, FT_LOAD_DEFAULT );
        FT_Render_Glyph( font_face->face->glyph, FT_RENDER_MODE_NORMAL );

        /* apply kerning */
        if( useKerning && previous && glyph_index )
        {
            FT_Vector delta;
            FT_Get_Kerning( font_face->face, previous, glyph_index,
                            FT_KERNING_DEFAULT, &delta );
            x -= (abs( delta.x ) >> 6);
        }

        /* advance */
        x += font_face->face->glyph->bitmap.width + 1;

        previous = glyph_index;
    }

    return x;
}

