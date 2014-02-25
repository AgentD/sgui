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
    sgui_font* this;

    /* sanity check */
    if( !filename )
        return NULL;

    /* allocate font structure */
    this = malloc( sizeof(sgui_font) );

    if( !this )
        return NULL;

    /* load the font file */
    if( FT_New_Face( freetype, filename, 0, &this->face ) )
    {
        free( this );
        return NULL;
    }

    /* initialise the remaining fields */
    this->buffer        = NULL;
    this->height        = pixel_height;
    this->current_glyph = 0;

    FT_Set_Pixel_Sizes( this->face, 0, pixel_height );

    return this;
}

sgui_font* sgui_font_load_memory( const void* data, unsigned long size,
                                  unsigned int pixel_height )
{
    sgui_font* this;

    /* sanity check */
    if( !data || !size )
        return NULL;

    /* allocate font structure */
    this = malloc( sizeof(sgui_font) );

    if( !this )
        return NULL;

    /* allocate a buffer for the file */
    this->buffer = malloc( size );

    if( !this->buffer )
    {
        free( this );
        return NULL;
    }

    /* initialise the font */
    if( FT_New_Memory_Face( freetype, this->buffer, size, 0, &this->face ) )
    {
        free( this->buffer );
        free( this );
        return NULL;
    }

    /* initialise the remaining fields */
    this->height        = pixel_height;
    this->current_glyph = 0;

    FT_Set_Pixel_Sizes( this->face, 0, pixel_height );

    return this;
}

void sgui_font_destroy( sgui_font* this )
{
    if( this )
    {
        FT_Done_Face( this->face );

        free( this->buffer );
        free( this );
    }
}

unsigned int sgui_font_get_height( sgui_font* this )
{
    return this ? this->height : 0;
}

void sgui_font_load_glyph( sgui_font* this, unsigned int codepoint )
{
    FT_UInt i;

    if( this )
    {
        this->current_glyph = codepoint;

        i = FT_Get_Char_Index( this->face, codepoint );

        FT_Load_Glyph( this->face, i, FT_LOAD_DEFAULT );
        FT_Render_Glyph( this->face->glyph, FT_RENDER_MODE_NORMAL );
    }
}

int sgui_font_get_kerning_distance( sgui_font* this, unsigned int first,
                                    unsigned int second )
{
    FT_Vector delta;
    FT_UInt index_a, index_b;

    if( this && FT_HAS_KERNING( this->face ) )
    {
        index_a = FT_Get_Char_Index( this->face, first );
        index_b = FT_Get_Char_Index( this->face, second );

        FT_Get_Kerning( this->face, index_a, index_b,
                        FT_KERNING_DEFAULT, &delta );

        return -((delta.x < 0 ? -delta.x : delta.x) >> 6);
    }

    return 0;
}

void sgui_font_get_glyph_metrics( sgui_font* this, unsigned int* width,
                                  unsigned int* height, int* bearing )
{
    unsigned int w = 0, h = 0;
    int b = 0;

    if( this )
    {
        if( this->current_glyph == ' ' )
        {
            w = this->height / 3;
            h = this->height;
            b = 0;
        }
        else
        {
            w = this->face->glyph->bitmap.width;
            h = this->face->glyph->bitmap.rows;
            b = this->height - this->face->glyph->bitmap_top;
        }
    }

    if( width   ) *width   = w;
    if( height  ) *height  = h;
    if( bearing ) *bearing = b;
}

unsigned char* sgui_font_get_glyph( sgui_font* this )
{
    if( this && this->current_glyph != ' ' )
        return this->face->glyph->bitmap.buffer;

    return NULL;
}

