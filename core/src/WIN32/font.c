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
#include "internal.h"




struct sgui_font
{
    FT_Face face;
    void* buffer;
    unsigned int height;
    unsigned int current_glyph;
};



sgui_font* sgui_font_load( const char* filename, unsigned int pixel_height )
{
    sgui_font* font;

    /* sanity check */
    if( !filename )
        return NULL;

    /* allocate font structure */
    font = malloc( sizeof(sgui_font) );

    if( !font )
        return NULL;

    /* load the font file */
    if( FT_New_Face( freetype, filename, 0, &font->face ) )
    {
        free( font );
        return NULL;
    }

    /* initialise the remaining fields */
    font->buffer        = NULL;
    font->height        = pixel_height;
    font->current_glyph = 0;

    FT_Set_Pixel_Sizes( font->face, 0, pixel_height );

    return font;
}

sgui_font* sgui_font_load_memory( const void* data, unsigned long size,
                                  unsigned int pixel_height )
{
    sgui_font* font;

    /* sanity check */
    if( !data || !size )
        return NULL;

    /* allocate font structure */
    font = malloc( sizeof(sgui_font) );

    if( !font )
        return NULL;

    /* allocate a buffer for the file */
    font->buffer = malloc( size );

    if( !font->buffer )
    {
        free( font );
        return NULL;
    }

    /* initialise the font */
    if( FT_New_Memory_Face( freetype, font->buffer, size, 0, &font->face ) )
    {
        free( font->buffer );
        free( font );
        return NULL;
    }

    /* initialise the remaining fields */
    font->height        = pixel_height;
    font->current_glyph = 0;

    FT_Set_Pixel_Sizes( font->face, 0, pixel_height );

    return font;
}

void sgui_font_destroy( sgui_font* font )
{
    if( font )
    {
        FT_Done_Face( font->face );

        free( font->buffer );
        free( font );
    }
}

unsigned int sgui_font_get_height( sgui_font* font )
{
    return font ? font->height : 0;
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
        return font->face->glyph->bitmap.buffer;

    return NULL;
}

