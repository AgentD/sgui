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




typedef struct GLYPH GLYPH;



struct GLYPH
{
    int x, y, bearing;
    unsigned int width, height, codepoint;

    int red;

    sgui_font* font;

    GLYPH* left;
    GLYPH* right;
};



static Pixmap pixmap = 0;
static Picture pic = 0;
static int next_x = 0, next_y = 0;
static unsigned int row_height = 0;
static int refcount = 0;

static GLYPH* root = NULL;



#define FONT_MAP_WIDTH 256
#define FONT_MAP_HEIGHT 256



#define IS_RED( g ) ((g) && (g)->red)



static void create_font_map( void )
{
    XRenderPictFormat* fmt;
    XRenderColor c;

    if( !refcount )
    {
        root = NULL;
        next_x = 0;
        next_y = 0;
        row_height = 0;

        /* create pixmap and picture */
        pixmap = XCreatePixmap( dpy, DefaultRootWindow(dpy),
                                FONT_MAP_WIDTH, FONT_MAP_HEIGHT, 8 );

        if( !pixmap )
            return;

        fmt = XRenderFindStandardFormat( dpy, PictStandardA8 );
        pic = XRenderCreatePicture( dpy, pixmap, fmt, 0, NULL );

        if( !pic )
        {
            XFreePixmap( dpy, pixmap );
            pixmap = 0;
            return;        
        }

        /* "initialise" the font pixmap */
        c.red = c.green = c.blue = c.alpha = 0x0000;

        XRenderFillRectangle( dpy, PictOpSrc, pic, &c,
                              0, 0, FONT_MAP_WIDTH, FONT_MAP_HEIGHT );
    }

    ++refcount;
}

static void destroy_tree( GLYPH* g )
{
    if( g )
    {
        destroy_tree( g->left );
        destroy_tree( g->right );
    }

    free( g );
}

static void destroy_font_map( void )
{
    --refcount;

    if( !refcount )
    {
        if( pic )
            XRenderFreePicture( dpy, pic );

        if( pixmap )
            XFreePixmap( dpy, pixmap );

        destroy_tree( root );

        root = NULL;
        next_x = 0;
        next_y = 0;
        pic = 0;
        pixmap = 0;
        row_height = 0;
    }
}

/****************************************************************************/

static GLYPH* create_glyph( sgui_font* font, unsigned int codepoint )
{
    const unsigned char* src;
    unsigned int w, h, i, j;
    XRenderColor c;
    GLYPH* g;
    int b;

    /* load glyph and get metrics */
    sgui_font_load_glyph( font, codepoint );
    sgui_font_get_glyph_metrics( font, &w, &h, &b );
    src = sgui_font_get_glyph( font );

    /* calculate position for new glyph */
    if( (next_x + w) >= FONT_MAP_WIDTH )
    {
        next_x  = 0;
        next_y += row_height;
        row_height = 0;
    }

    if( h > row_height )
        row_height = h;

    /* create glyph */
    g = malloc( sizeof(GLYPH) );
    g->left = NULL;
    g->right = NULL;
    g->red = 1;
    g->codepoint = codepoint;
    g->x = next_x;
    g->y = next_y;
    g->width = w;
    g->height = h;
    g->bearing = b;
    g->font = font;

    /* copy glyph to pixmap */
    if( src )
    {
        for( j=0; j<h; ++j )
        {
            for( i=0; i<w; ++i, ++src )
            {
                c.red = c.green = c.blue = c.alpha = (*src)<<8;

                XRenderFillRectangle( dpy, PictOpSrc, pic, &c,
                                      g->x+i, g->y+j, 1, 1 );
            }
        }
    }

    /* advance next glyph position */
    next_x += w;

    return g;
}

static GLYPH* insert( sgui_font* font, GLYPH* g, unsigned int codepoint )
{ 
    GLYPH* x;

    /* Reached a NULL node? Create a new node. */
    if( !g )
        return create_glyph( font, codepoint );

    /* continue traversing down the tree */
    if( codepoint == g->codepoint )
    {
        if( font < g->font )
            g->left = insert( font, g->left, codepoint ); 
        else if( font > g->font )
            g->right = insert( font, g->right, codepoint ); 
    }
    else
    {
        if( codepoint < g->codepoint )
            g->left = insert( font, g->left, codepoint ); 
        else
            g->right = insert( font, g->right, codepoint ); 
    }

    /* rotate left */
    if( IS_RED(g->right) && !IS_RED(g->left) )
    {
        x = g->right;
        g->right = x->left;
        x->left = g;
        x->red = x->left->red;
        x->left->red = 1;
        g = x;
    }

    /* rotate right */
    if( IS_RED(g->left) && IS_RED(g->left->left) )
    {
        x = g->left;
        g->left = x->right;
        x->right = g;
        x->red = x->right->red;
        x->right->red = 1;
        g = x;
    }

    /* flip colors */
    if( IS_RED(g->left) && IS_RED(g->right) )
    {
        g->red = !g->red;
        g->left->red = !g->left->red;
        g->right->red = !g->right->red;
    }

    return g;
}

static void insert_glyph( sgui_font* font, unsigned int codepoint )
{
    root = insert( font, root, codepoint );
    root->red = 0;
}

static GLYPH* find_glyph( sgui_font* font, unsigned int codepoint )
{
    GLYPH* g = root;

    while( g )
    {
        if( g->codepoint == codepoint )
        {
            if( g->font == font )
                return g;

            g = (font < g->font) ? g->left : g->right;
        }
        else
            g = (codepoint < g->codepoint) ? g->left : g->right;
    }

    return NULL;
}

int draw_glyph( sgui_font* font, unsigned int codepoint, int x, int y,
                Picture dst, Picture pen )
{
    GLYPH* g;

    if( font )
    {
        g = find_glyph( font, codepoint );

        if( !g )
        {
            insert_glyph( font, codepoint );
            g = find_glyph( font, codepoint );
        }

        if( g )
        {
            XRenderComposite( dpy, PictOpOver, pen, pic, dst,
                              0, 0, g->x, g->y, x, y + g->bearing,
                              g->width, g->height );

            return g->width;
        }
    }

    return 0;
}

/****************************************************************************/

struct sgui_font
{
    FT_Face face;
    void* buffer;
    unsigned int height;
    unsigned int current_glyph;
};

static sgui_font* sgui_font_load_common( unsigned int pixel_height )
{
    sgui_font* font;

    create_font_map( );

    /* allocate font structure */
    font = malloc( sizeof(sgui_font) );

    if( !font )
        return NULL;

    /* initialise the remaining fields */
    font->buffer        = NULL;
    font->height        = pixel_height;
    font->current_glyph = 0;

    return font;
}

sgui_font* sgui_font_load( const char* filename, unsigned int pixel_height )
{
    sgui_font* font;

    /* sanity check */
    if( !filename )
        return NULL;

    /* allocate font structure */
    font = sgui_font_load_common( pixel_height );

    if( !font )
        return NULL;

    /* load the font file */
    if( FT_New_Face( freetype, filename, 0, &font->face ) )
    {
        free( font );
        return NULL;
    }

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
    font = sgui_font_load_common( pixel_height );

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

    FT_Set_Pixel_Sizes( font->face, 0, pixel_height );

    return font;
}

void sgui_font_destroy( sgui_font* font )
{
    if( font )
    {
        destroy_font_map( );

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

