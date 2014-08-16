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



static GLYPH* create_glyph( sgui_font_cache* this, sgui_font* font,
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
        sgui_pixmap_get_size( this->font_map, &pw, &ph );

        /* calculate position for new glyph */
        if( (this->next_x + w) >= pw )
        {
            this->next_x  = 0;
            this->next_y += this->row_height;
            this->row_height = 0;
        }

        if( h > this->row_height )
            this->row_height = h;

        /* TODO: pixmap full!! How should we handle this case? */
        if( (this->next_y + h) >= ph )
        {
            free( g );
            return NULL;
        }

        /* calculate glyph area */
        g->area.left = this->next_x;
        g->area.top = this->next_y;
        g->area.right = this->next_x + w-1;
        g->area.bottom = this->next_y + h-1;

        if( g->area.bottom==g->area.top )
            ++g->area.bottom;

        if( g->area.right==g->area.left )
            ++g->area.right;

        /* load glyph to pixmap */
        sgui_pixmap_load( this->font_map, g->area.left, g->area.top, src,
                          0, 0, w, h, w, SGUI_A8 );

        /* advance next glyph position */
        this->next_x += w;
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

static void tree_destroy( GLYPH* this )
{
    if( this )
    {
        tree_destroy( this->left );
        tree_destroy( this->right );
    }

    free( this );
}

static GLYPH* tree_balance( GLYPH* this )
{
    GLYPH* x;

    /* rotate left */
    if( IS_RED(this->right) && !IS_RED(this->left) )
    {
        x = this->right;
        this->right = x->left;
        x->left = this;
        x->red = x->left->red;
        x->left->red = 1;
        this = x;
    }

    /* rotate right */
    if( IS_RED(this->left) && IS_RED(this->left->left) )
    {
        x = this->left;
        this->left = x->right;
        x->right = this;
        x->red = x->right->red;
        x->right->red = 1;
        this = x;
    }

    /* flip colors */
    if( IS_RED(this->left) && IS_RED(this->right) )
    {
        this->red = !this->red;
        this->left->red = !this->left->red;
        this->right->red = !this->right->red;
    }

    return this;
}

static GLYPH* tree_insert( GLYPH* this, GLYPH* new_glyph )
{ 
    /* Reached a NULL node? Create a new node. */
    if( !this )
        return new_glyph;

    /* continue traversing down the tree */
    if( new_glyph->codepoint == this->codepoint )
    {
        if( new_glyph->font < this->font )
            this->left = tree_insert( this->left, new_glyph );
        else if( new_glyph->font > this->font )
            this->right = tree_insert( this->right, new_glyph );
    }
    else if( new_glyph->codepoint < this->codepoint )
        this->left = tree_insert( this->left, new_glyph );
    else
        this->right = tree_insert( this->right, new_glyph );

    /* balance current subtree */
    this = tree_balance( this );

    return this;
}

static void insert_glyph( sgui_font_cache* this, GLYPH* new_glyph )
{
    this->root = tree_insert( this->root, new_glyph );
    this->root->red = 0;
}

static GLYPH* find_glyph(GLYPH* this, sgui_font* font, unsigned int codepoint)
{
    while( this )
    {
        if( this->codepoint == codepoint )
        {
            if( this->font == font )
                return this;

            this = (font < this->font) ? this->left : this->right;
        }
        else
        {
            this = (codepoint < this->codepoint) ? this->left : this->right;
        }
    }

    return NULL;
}

/****************************************************************************/

sgui_font_cache* sgui_font_cache_create( sgui_pixmap* map )
{
    sgui_font_cache* this;

    this = malloc( sizeof(sgui_font_cache) );

    if( this )
    {
        memset( this, 0, sizeof(sgui_font_cache) );
        this->font_map = map;
    }

    return this;
}

void sgui_font_cache_destroy( sgui_font_cache* this )
{
    if( this )
    {
        sgui_pixmap_destroy( this->font_map );
        tree_destroy( this->root );
        free( this );
    }
}

int sgui_font_cache_draw_glyph( sgui_font_cache* this, sgui_font* font,
                                unsigned int codepoint, int x, int y,
                                sgui_canvas* cv, const unsigned char* color )
{
    GLYPH* g;

    if( this && font && cv && color )
    {
        sgui_internal_lock_mutex( );

        g = find_glyph( this->root, font, codepoint );

        if( !g )
        {
            g = create_glyph( this, font, codepoint );
            insert_glyph( this, g );
        }

        sgui_internal_unlock_mutex( );

        if( g )
        {
            /* only render glyphs with a non zero area (skip) dummies */
            if( g->area.top != g->area.bottom )
            {
                cv->blend_glyph( cv, x, y+g->bearing, this->font_map,
                                 &g->area, color );
            }

            return SGUI_RECT_WIDTH( g->area );
        }
    }

    return 0;
}

void sgui_font_cache_load_glyph( sgui_font_cache* this, sgui_font* font,
                                 unsigned int codepoint )
{
    GLYPH* g;

    if( this && font )
    {
        sgui_internal_lock_mutex( );

        g = find_glyph( this->root, font, codepoint );

        if( !g )
        {
            g = create_glyph( this, font, codepoint );
            insert_glyph( this, g );
        }

        sgui_internal_unlock_mutex( );
    }
}

sgui_pixmap* sgui_font_cache_get_pixmap( sgui_font_cache* this )
{
    return this ? this->font_map : NULL;
}

