/*
 * font_cache.c
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
#include "sgui_pixmap.h"
#include "sgui_font.h"

#include <stddef.h>
#include <stdlib.h>



typedef struct GLYPH GLYPH;



struct GLYPH
{
    unsigned int codepoint; /* unicode codepoint of the glyph */
    sgui_rect area;         /* area of the glyph on the font pixmap */
    int bearing;            /* bearing of the glyph */
    int red;                /* non zero if red (red-black tree color ) */

    sgui_font* font;        /* the font used by the glyph */

    GLYPH* left;            /* left hand tree node */
    GLYPH* right;           /* right hand tree node */
};



static int next_x = 0, next_y = 0;
static unsigned int row_height = 0;
static sgui_pixmap* font_map = NULL;

static GLYPH* root = NULL;



#define IS_RED( g ) ((g) && (g)->red)



int create_font_cache( sgui_pixmap* cache )
{
    root = NULL;
    next_x = 0;
    next_y = 0;
    row_height = 0;

    /* create pixmap and picture */
    font_map = cache;

    if( !font_map )
        return 0;

    return 1;
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

void destroy_font_cache( void )
{
    sgui_pixmap_destroy( font_map );

    destroy_tree( root );

    font_map = NULL;
    root = NULL;
    next_x = 0;
    next_y = 0;
    row_height = 0;
}

/****************************************************************************/

static GLYPH* create_glyph( sgui_font* font, unsigned int codepoint )
{
    const unsigned char* src;
    unsigned int w, h, pw, ph;
    GLYPH* g;
    int b;

    /* load glyph and get metrics */
    sgui_font_load_glyph( font, codepoint );
    sgui_font_get_glyph_metrics( font, &w, &h, &b );
    src = sgui_font_get_glyph( font );

    /* create glyph */
    g = malloc( sizeof(GLYPH) );
    g->left = NULL;
    g->right = NULL;
    g->red = 1;
    g->codepoint = codepoint;
    g->bearing = b;
    g->font = font;

    /* copy glyph to pixmap */
    if( src )
    {
        sgui_pixmap_get_size( font_map, &pw, &ph );

        /* calculate position for new glyph */
        if( (next_x + w) >= pw )
        {
            next_x  = 0;
            next_y += row_height;
            row_height = 0;
        }

        if( h > row_height )
            row_height = h;

        /* TODO: pixmap full!! How should we handle this case? */
        if( (next_y + h) >= ph )
        {
            free( g );
            return NULL;
        }

        /* calculate glyph area */
        g->area.left = next_x;
        g->area.top = next_y;
        g->area.right = next_x + w-1;
        g->area.bottom = next_y + h-1;

        /* load glyph to pixmap */
        sgui_pixmap_load( font_map, g->area.left, g->area.top, src, 0, 0,
                          w, h, w, SGUI_A8 );

        /* advance next glyph position */
        next_x += w;
    }
    else
    {
        /* create dummy glyph area, do not alter font pixmap */
        g->area.left = 0;
        g->area.top = 0;
        g->area.right = w-1;
        g->area.bottom = 0;
    }

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
        {
            g = (codepoint < g->codepoint) ? g->left : g->right;
        }
    }

    return NULL;
}

/****************************************************************************/

int draw_glyph( sgui_font* font, unsigned int codepoint, int x, int y,
                sgui_canvas* cv, unsigned char* color )
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
            /* only render glyphs with a non zero area (skip) dummies */
            if( g->area.top != g->area.bottom )
            {
                cv->blend_glyph( cv, x, y+g->bearing, font_map,
                                 &g->area, color );
            }

            return SGUI_RECT_WIDTH( g->area );
        }
    }

    return 0;
}

