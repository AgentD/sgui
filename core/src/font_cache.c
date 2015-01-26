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

#include <stdlib.h>
#include <string.h>

#ifndef SGUI_NO_ICON_CACHE
typedef struct
{
    sgui_icon super;
    int bearing;            /* bearing of the glyph */
    unsigned int codepoint; /* unicode code point  */
    sgui_font* font;        /* the font used by the glyph */
}
GLYPH;


static int glyph_compare( const sgui_icon* left, const sgui_icon* right )
{
    if( ((GLYPH*)left)->codepoint == ((GLYPH*)right)->codepoint )
    {
        if( ((GLYPH*)left)->font == ((GLYPH*)right)->font )
            return 0;

        return ((GLYPH*)left)->font < ((GLYPH*)right)->font ? -1 : 1;
    }

    return ((GLYPH*)left)->codepoint < ((GLYPH*)right)->codepoint ? -1 : 1;
}

static GLYPH* create_glyph( sgui_icon_cache* this, sgui_font* font,
                            unsigned int codepoint )
{
    const unsigned char* src;
    unsigned int w, h;
    GLYPH* g;
    int b;

    /* load glyph and get metrics */
    font->load_glyph( font, codepoint );
    font->get_glyph_metrics( font, &w, &h, &b );
    src = font->get_glyph( font );

    /* create glyph */
    if( !(g = malloc( sizeof(GLYPH) )) )
        return NULL;

    memset( g, 0, sizeof(GLYPH) );
    g->super.red = 1;
    g->codepoint = codepoint;
    g->bearing = b;
    g->font = font;

    /* copy glyph to pixmap */
    if( w==1 ) ++w; /* FIXME: ugly hack */
    if( h==1 ) ++h;

    if( src && w && h )
    {
        if( !sgui_icon_cache_alloc_area( this, w, h, &g->super.area ) )
        {
            free( g );
            return NULL;
        }

        sgui_icon_cache_load_icon( this, (sgui_icon*)g, src, w, SGUI_A8 );
    }
    else
    {
        g->super.area.right = w-1;      /* empty dummy area */
    }

    this->root = sgui_icon_cache_tree_insert(this, this->root, (sgui_icon*)g);
    this->root->red = 0;
    return g;
}

static GLYPH* fetch_glyph( sgui_icon_cache* this,
                           sgui_font* font, unsigned int codepoint)
{
    GLYPH cmp, *g=NULL;

    if( this && font )
    {
        sgui_internal_lock_mutex( );
        cmp.font = font;
        cmp.codepoint = codepoint;
        g = (GLYPH*)sgui_icon_cache_find( this, (sgui_icon*)&cmp );
        g = g ? g : create_glyph( this, font, codepoint );
        sgui_internal_unlock_mutex( );
    }
    return g;
}

/****************************************************************************/

sgui_icon_cache* sgui_font_cache_create( sgui_pixmap* map )
{
    sgui_icon_cache* this = malloc( sizeof(sgui_icon_cache) );

    if( this )
    {
        memset( this, 0, sizeof(sgui_icon_cache) );
        sgui_pixmap_get_size( map, &this->width, &this->height );

        this->pixmap = map;
        this->icon_compare = glyph_compare;
    }
    return this;
}

int sgui_font_cache_draw_glyph( sgui_icon_cache* this, sgui_font* font,
                                unsigned int codepoint, int x, int y,
                                sgui_canvas* cv, const unsigned char* color )
{
    GLYPH* g = NULL;

    if( this && font && cv && color && (g=fetch_glyph(this,font,codepoint)) )
    {
        if( g->super.area.top != g->super.area.bottom )
        {
            cv->blend_glyph( cv, x, y+g->bearing, this->pixmap,
                             &g->super.area, color );
        }
    }

    return g ? SGUI_RECT_WIDTH( g->super.area ) : 0;
}

void sgui_font_cache_load_glyph( sgui_icon_cache* this, sgui_font* font,
                                 unsigned int codepoint )
{
    fetch_glyph( this, font, codepoint );
}
#elif defined(SGUI_NOP_IMPLEMENTATIONS)
sgui_icon_cache* sgui_font_cache_create( sgui_pixmap* map )
{
    (void)map;
    return NULL;
}

int sgui_font_cache_draw_glyph( sgui_icon_cache* this, sgui_font* font,
                                unsigned int codepoint, int x, int y,
                                sgui_canvas* cv, const unsigned char* color )
{
    (void)this; (void)font; (void)codepoint; (void)x; (void)y; (void)cv;
    (void)color;
    return 0;
}

void sgui_font_cache_load_glyph( sgui_icon_cache* this, sgui_font* font,
                                 unsigned int codepoint )
{
    (void)this; (void)font; (void)codepoint;
}
#endif /* !SGUI_NO_ICON_CACHE */

