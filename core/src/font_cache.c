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
#include "sgui_font_cache.h"
#include "sgui_internal.h"
#include "sgui_pixmap.h"
#include "sgui_font.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>



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

struct sgui_font_cache
{
    int next_x, next_y;
    unsigned int row_height;
    sgui_pixmap* font_map;

    GLYPH* root;
};



#define IS_RED( g ) ((g) && (g)->red)



static GLYPH* create_glyph( sgui_font_cache* cache, sgui_font* font,
                            unsigned int codepoint )
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
        sgui_pixmap_get_size( cache->font_map, &pw, &ph );

        /* calculate position for new glyph */
        if( (cache->next_x + w) >= pw )
        {
            cache->next_x  = 0;
            cache->next_y += cache->row_height;
            cache->row_height = 0;
        }

        if( h > cache->row_height )
            cache->row_height = h;

        /* TODO: pixmap full!! How should we handle this case? */
        if( (cache->next_y + h) >= ph )
        {
            free( g );
            return NULL;
        }

        /* calculate glyph area */
        g->area.left = cache->next_x;
        g->area.top = cache->next_y;
        g->area.right = cache->next_x + w-1;
        g->area.bottom = cache->next_y + h-1;

        if( g->area.bottom==g->area.top )
            ++g->area.bottom;

        if( g->area.right==g->area.left )
            ++g->area.right;

        /* load glyph to pixmap */
        sgui_pixmap_load( cache->font_map, g->area.left, g->area.top, src,
                          0, 0, w, h, w, SGUI_A8 );

        /* advance next glyph position */
        cache->next_x += w;
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

static void tree_destroy( GLYPH* g )
{
    if( g )
    {
        tree_destroy( g->left );
        tree_destroy( g->right );
    }

    free( g );
}

static GLYPH* tree_balance( GLYPH* g )
{
    GLYPH* x;

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

static GLYPH* tree_insert( GLYPH* g, GLYPH* new_glyph )
{ 
    /* Reached a NULL node? Create a new node. */
    if( !g )
        return new_glyph;

    /* continue traversing down the tree */
    if( new_glyph->codepoint == g->codepoint )
    {
        if( new_glyph->font < g->font )
            g->left = tree_insert( g->left, new_glyph );
        else if( new_glyph->font > g->font )
            g->right = tree_insert( g->right, new_glyph );
    }
    else if( new_glyph->codepoint < g->codepoint )
        g->left = tree_insert( g->left, new_glyph );
    else
        g->right = tree_insert( g->right, new_glyph );

    /* balance current subtree */
    g = tree_balance( g );

    return g;
}

static void insert_glyph( sgui_font_cache* cache, GLYPH* new_glyph )
{
    cache->root = tree_insert( cache->root, new_glyph );
    cache->root->red = 0;
}

static GLYPH* find_glyph( GLYPH* g, sgui_font* font, unsigned int codepoint )
{
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

sgui_font_cache* sgui_font_cache_create( sgui_pixmap* map )
{
    sgui_font_cache* cache;

    cache = malloc( sizeof(sgui_font_cache) );

    if( cache )
    {
        memset( cache, 0, sizeof(sgui_font_cache) );
        cache->font_map = map;
    }

    return cache;
}

void sgui_font_cache_destroy( sgui_font_cache* cache )
{
    if( cache )
    {
        sgui_pixmap_destroy( cache->font_map );
        tree_destroy( cache->root );
        free( cache );
    }
}

int sgui_font_cache_draw_glyph( sgui_font_cache* cache, sgui_font* font,
                                unsigned int codepoint, int x, int y,
                                sgui_canvas* cv, unsigned char* color )
{
    GLYPH* g;

    if( cache && font && cv && color )
    {
        sgui_internal_lock_mutex( );

        g = find_glyph( cache->root, font, codepoint );

        if( !g )
        {
            g = create_glyph( cache, font, codepoint );
            insert_glyph( cache, g );
        }

        sgui_internal_unlock_mutex( );

        if( g )
        {
            /* only render glyphs with a non zero area (skip) dummies */
            if( g->area.top != g->area.bottom )
            {
                cv->blend_glyph( cv, x, y+g->bearing, cache->font_map,
                                 &g->area, color );
            }

            return SGUI_RECT_WIDTH( g->area );
        }
    }

    return 0;
}

void sgui_font_cache_load_glyph( sgui_font_cache* cache, sgui_font* font,
                                 unsigned int codepoint )
{
    GLYPH* g;

    if( cache && font )
    {
        sgui_internal_lock_mutex( );

        g = find_glyph( cache->root, font, codepoint );

        if( !g )
        {
            g = create_glyph( cache, font, codepoint );
            insert_glyph( cache, g );
        }

        sgui_internal_unlock_mutex( );
    }
}

sgui_pixmap* sgui_font_cache_get_pixmap( sgui_font_cache* cache )
{
    return cache ? cache->font_map : NULL;
}

