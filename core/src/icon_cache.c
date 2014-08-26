/*
 * sgui_icon_cache.c
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
#include "sgui_icon_cache.h"
#include "sgui_internal.h"
#include "sgui_canvas.h"
#include "sgui_pixmap.h"
#include "sgui_rect.h"

#include <stdlib.h>
#include <string.h>



#ifndef SGUI_NO_ICON_CACHE
#define IS_RED( i ) ((i) && (i)->red)



static void tree_destroy( sgui_icon* this )
{
    if( this )
    {
        tree_destroy( this->left );
        tree_destroy( this->right );
        free( this );
    }
}

static sgui_icon* tree_balance( sgui_icon* this )
{
    sgui_icon* i;

    if( IS_RED(this->right) && !IS_RED(this->left) )
    {
        /* rotate left */
        i = this->right;
        this->right = i->left;
        i->left = this;
        i->red = i->left->red;
        i->left->red = 1;
        this = i;
    }

    if( IS_RED(this->left) && IS_RED(this->left->left) )
    {
        /* rotate right */
        i = this->left;
        this->left = i->right;
        i->right = this;
        i->red = i->right->red;
        i->right->red = 1;
        this = i;
    }

    if( IS_RED(this->left) && IS_RED(this->right) )
    {
        /* flip colors */
        this->red = !this->red;
        this->left->red = !this->left->red;
        this->right->red = !this->right->red;
    }

    return this;
}

static sgui_icon* tree_insert( sgui_icon* this, sgui_icon* new )
{ 
    if( !this )
        return new;

    if( new->id < this->id )
        this->left = tree_insert( this->left, new );
    else
        this->right = tree_insert( this->right, new );

    return tree_balance( this );
}

static sgui_icon* find_icon( sgui_icon* this, unsigned int id )
{
    while( this && this->id!=id )
    {
        this = (id < this->id) ? this->left : this->right;
    }

    return this;
}

/****************************************************************************/

sgui_icon_cache* sgui_icon_cache_create( sgui_canvas* canvas,
                                         unsigned int width,
                                         unsigned int height,
                                         int alpha )
{
    sgui_icon_cache* this;

    /* sanity check */
    if( !canvas || !width || !height )
        return NULL;

    /* allocate structure */
    this = malloc( sizeof(sgui_icon_cache) );

    if( !this )
        return NULL;

    /* initialize */
    memset( this, 0, sizeof(sgui_icon_cache) );

    this->owner = canvas;
    this->width = width;
    this->height = height;
    this->format = alpha ? SGUI_RGBA8 : SGUI_RGB8;
    this->pixmap = sgui_canvas_create_pixmap( canvas, width, height,
                                              this->format );

    if( !this->pixmap )
    {
        free( this );
        return NULL;
    }

    return this;
}

void sgui_icon_cache_destroy( sgui_icon_cache* this )
{
    if( this )
    {
        tree_destroy( this->root );
        sgui_pixmap_destroy( this->pixmap );
        free( this );
    }
}

int sgui_icon_cache_add_icon( sgui_icon_cache* this, unsigned int id,
                              unsigned int width, unsigned int height )
{
    sgui_icon* i;

    /* sanity check */
    if( !this || !width || !height || find_icon( this->root, id ) )
        return 0;

    sgui_internal_lock_mutex( );

    /* check if there is enought space for the icon */
    if( (this->next_x + width) > this->width )
    {
        if( (this->next_y + this->row_height + height) > this->height )
            goto fail;
    }
    else
    {
        if( (this->next_y + height) > this->height )
            goto fail;
    }

    /* create icon */
    i = malloc( sizeof(sgui_icon) );

    if( !i )
        goto fail;

    memset( i, 0, sizeof(sgui_icon) );
    i->red = 1;
    i->id = id;

    /* allocate icon rectangle */
    if( (this->next_x + width) > this->width )
    {
        this->next_x  = 0;
        this->next_y += this->row_height;
        this->row_height = height;
    }

    sgui_rect_set_size( &(i->area), this->next_x, this->next_y,
                        width, height );

    this->next_x += width;
    this->row_height = height>this->row_height ? height : this->row_height;

    /* insert into tree */
    this->root = tree_insert( this->root, i );
    this->root->red = 0;

    sgui_internal_unlock_mutex( );
    return 1;
fail:
    sgui_internal_unlock_mutex( );
    return 0;
}

void sgui_icon_cache_load_icon( sgui_icon_cache* this, unsigned int id,
                                unsigned char* data, unsigned int scan,
                                int format )
{
    sgui_icon* i;

    if( !this || !data )
        return;

    sgui_internal_lock_mutex( );
    i = find_icon( this->root, id );

    if( !i )
        return;

    sgui_pixmap_load( this->pixmap, i->area.left, i->area.top, data, 0, 0,
                      SGUI_RECT_WIDTH(i->area), SGUI_RECT_HEIGHT(i->area),
                      scan, format );
    sgui_internal_unlock_mutex( );
}

void sgui_icon_cache_draw_icon( const sgui_icon_cache* this, unsigned int id,
                                int x, int y )
{
    sgui_icon* i;

    if( !this )
        return;

    sgui_internal_lock_mutex( );
    i = find_icon( this->root, id );

    if( i )
    {
        sgui_canvas_draw_pixmap( this->owner, x, y, this->pixmap,
                                 &(i->area), this->format==SGUI_RGBA8 );
    }

    sgui_internal_unlock_mutex( );
}

int sgui_icon_cache_get_icon_area( const sgui_icon_cache* this,
                                   unsigned int id, sgui_rect* out )
{
    sgui_icon* i;

    if( !this )
        return 0;

    sgui_internal_lock_mutex( );

    i = find_icon( this->root, id );

    if( !i )
    {
        sgui_internal_unlock_mutex( );
        return 0;
    }

    if( out )
        sgui_rect_copy( out, &(i->area) );

    sgui_internal_unlock_mutex( );
    return 1;
}
#elif defined(SGUI_NOP_IMPLEMENTATIONS)
sgui_icon_cache* sgui_icon_cache_create( sgui_canvas* canvas,
                                         unsigned int width,
                                         unsigned int height,
                                         int alpha )
{
    (void)canvas; (void)width; (void)height; (void)alpha;
    return NULL;
}

void sgui_icon_cache_destroy( sgui_icon_cache* this )
{
    (void)this;
}

int sgui_icon_cache_add_icon( sgui_icon_cache* this, unsigned int id,
                              unsigned int width, unsigned int height )
{
    (void)this; (void)id; (void)width; (void)height;
    return 0;
}

void sgui_icon_cache_load_icon( sgui_icon_cache* this, unsigned int id,
                                unsigned char* data, unsigned int scan,
                                int format )
{
    (void)this; (void)id; (void)data; (void)scan; (void)format;
}

void sgui_icon_cache_draw_icon( const sgui_icon_cache* this, unsigned int id,
                                int x, int y )
{
    (void)this; (void)id; (void)x; (void)y;
}

int sgui_icon_cache_get_icon_area( const sgui_icon_cache* this,
                                   unsigned int id, sgui_rect* out )
{
    (void)this; (void)id;
    if( out ) { out->left=out->top=out->right=out->bottom; }
    return 0;
}
#endif /* !SGUI_NO_ICON_CACHE */


