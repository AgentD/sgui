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
#include "font.h"



static FT_Library freetype = 0;



int font_init( void )
{
    return (FT_Init_FreeType( &freetype )==0);
}

void font_deinit( void )
{
    if( freetype )
        FT_Done_FreeType( freetype );

    freetype = 0;
}

/****************************************************************************/

static void w32_font_destroy( sgui_font* this )
{
    if( this )
    {
        FT_Done_Face( ((sgui_w32_font*)this)->face );

        free( ((sgui_w32_font*)this)->buffer );
        free( this );
    }
}

static void w32_font_load_glyph( sgui_font* super, unsigned int codepoint )
{
    sgui_w32_font* this = (sgui_w32_font*)super;
    FT_UInt i;

    if( this )
    {
        this->current_glyph = codepoint;

        i = FT_Get_Char_Index( this->face, codepoint );

        FT_Load_Glyph( this->face, i, FT_LOAD_DEFAULT );
        FT_Render_Glyph( this->face->glyph, FT_RENDER_MODE_NORMAL );
    }
}

static int w32_font_get_kerning_distance( sgui_font* super,
                                          unsigned int first,
                                          unsigned int second )
{
    sgui_w32_font* this = (sgui_w32_font*)super;
    FT_UInt index_a, index_b;
    FT_Vector delta;

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

static void w32_font_get_glyph_metrics( sgui_font* super, unsigned int* width,
                                        unsigned int* height, int* bearing )
{
    sgui_w32_font* this = (sgui_w32_font*)super;
    unsigned int w = 0, h = 0;
    int b = 0;

    if( this )
    {
        if( this->current_glyph == ' ' )
        {
            w = super->height / 3;
            h = super->height;
            b = 0;
        }
        else
        {
            w = this->face->glyph->bitmap.width;
            h = this->face->glyph->bitmap.rows;
            b = super->height - this->face->glyph->bitmap_top;
        }
    }

    if( width   ) *width   = w;
    if( height  ) *height  = h;
    if( bearing ) *bearing = b;
}

static unsigned char* w32_font_get_glyph( sgui_font* this )
{
    if( this && ((sgui_w32_font*)this)->current_glyph != ' ' )
        return ((sgui_w32_font*)this)->face->glyph->bitmap.buffer;

    return NULL;
}

/****************************************************************************/

sgui_font* sgui_font_load( const char* filename, unsigned int pixel_height )
{
    sgui_w32_font* this;
    char buffer[ 512 ];
    sgui_font* super;

    /* sanity check */
    if( !filename )
        return NULL;

    /* allocate font structure */
    this = calloc( 1, sizeof(sgui_w32_font) );
    super = (sgui_font*)this;

    if( !this )
        return NULL;

    /* load the font file */
    if( !FT_New_Face( freetype, filename, 0, &this->face ) )
        goto cont;

    sprintf( buffer, "%s%s", SYS_FONT_PATH, filename );

    if( !FT_New_Face( freetype, buffer, 0, &this->face ) )
        goto cont;

    free( this );
    return NULL;

cont:
    /* initialise the remaining fields */
    this->buffer        = NULL;
    this->current_glyph = 0;

    super->height               = pixel_height;
    super->destroy              = w32_font_destroy;
    super->load_glyph           = w32_font_load_glyph;
    super->get_kerning_distance = w32_font_get_kerning_distance;
    super->get_glyph_metrics    = w32_font_get_glyph_metrics;
    super->get_glyph            = w32_font_get_glyph;

    FT_Set_Pixel_Sizes( this->face, 0, pixel_height );

    return super;
}

sgui_font* sgui_font_load_memory( const void* data, unsigned long size,
                                  unsigned int pixel_height )
{
    sgui_w32_font* this;
    sgui_font* super;

    /* sanity check */
    if( !data || !size )
        return NULL;

    /* allocate font structure */
    this = calloc( 1, sizeof(sgui_w32_font) );
    super = (sgui_font*)this;

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
    this->current_glyph = 0;

    super->height               = pixel_height;
    super->destroy              = w32_font_destroy;
    super->load_glyph           = w32_font_load_glyph;
    super->get_kerning_distance = w32_font_get_kerning_distance;
    super->get_glyph_metrics    = w32_font_get_glyph_metrics;
    super->get_glyph            = w32_font_get_glyph;

    FT_Set_Pixel_Sizes( this->face, 0, pixel_height );

    return super;
}

