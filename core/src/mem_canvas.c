/*
 * mem_canvas.c
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
#include "sgui_canvas.h"
#include "sgui_pixmap.h"
#include "sgui_font.h"
#include "sgui_utf8.h"
#include "sgui_internal.h"

#include <stdlib.h>
#include <stdio.h>



#ifndef SGUI_NO_MEM_CANVAS
typedef struct
{
    sgui_canvas super;

    unsigned char* data;
    int bpp;

    sgui_rect locked;
}
mem_canvas;



static void mem_canvas_swaprb( sgui_canvas* super )
{
    mem_canvas* this = (mem_canvas*)super;
    unsigned char temp, *dst, *row;
    int i, j, x, y, x1, y1;

    x = this->locked.left;
    y = this->locked.top;
    x1 = this->locked.right;
    y1 = this->locked.bottom;
    dst = (unsigned char*)this->data + (y*super->width + x)*4;

    /* for each pixel in the locked area */
    for( j=y; j<=y1; ++j, dst+=super->width*this->bpp )
    {
        for( row=dst, i=x; i<=x1; ++i, row+=this->bpp )
        {
            /* swap R and B */
            temp = row[0];
            row[0] = row[2];
            row[2] = temp;
        }
    }
}

/****************************************************************************/

static void canvas_mem_destroy( sgui_canvas* this )
{
    free( this );
}

static void canvas_mem_resize( sgui_canvas* super, unsigned int width,
                               unsigned int height )
{
    (void)super; (void)width; (void)height;
}

static void canvas_mem_clear( sgui_canvas* super, sgui_rect* r )
{
    mem_canvas* this = (mem_canvas*)super;
    unsigned char *dst, *row;
    int i, j;

    dst = (unsigned char*)this->data + (r->top*super->width + r->left)*4;

    /* clear */
    for( j=r->top; j<=r->bottom; ++j, dst+=super->width*this->bpp )
    {
        for( row=dst, i=r->left; i<=r->right; ++i, row+=this->bpp )
        {
            row[0] = super->bg_color[0];
            row[1] = super->bg_color[1];
            row[2] = super->bg_color[2];
        }
    }
}

static void canvas_mem_blit( sgui_canvas* super, int x, int y,
                             sgui_pixmap* pixmap, sgui_rect* srcrect )
{
    mem_canvas* this = (mem_canvas*)super;
    unsigned int w = SGUI_RECT_WIDTH_V( srcrect );
    unsigned int h = SGUI_RECT_HEIGHT_V( srcrect );
    unsigned char *src, *dst, *row, *srow;
    unsigned int i, j, srcbpp, scan, lines;

    dst = (unsigned char*)this->data + (y*super->width + x)*4;
    src = sgui_internal_mem_pixmap_buffer( pixmap );
    srcbpp = sgui_internal_mem_pixmap_format( pixmap );
    srcbpp = srcbpp==SGUI_RGBA8 ? 4 : (srcbpp==SGUI_RGB8 ? 3 : 1);
    sgui_pixmap_get_size( pixmap, &scan, &lines );
    src += (srcrect->top*scan + srcrect->left) * srcbpp;

    for( j=0; j<h; ++j, src+=scan*srcbpp, dst+=super->width*this->bpp )
    {
        for( srow=src, row=dst, i=0; i<w; ++i, row+=this->bpp, srow+=srcbpp )
        {
            if( srcbpp>=3 )
            {
                row[0] = srow[0];
                row[1] = srow[1];
                row[2] = srow[2];
            }
            else
            {
                row[0] = srow[0];
                row[1] = srow[0];
                row[2] = srow[0];
            }
        }
    }
}

static void canvas_mem_blend( sgui_canvas* super, int x, int y,
                              sgui_pixmap* pixmap, sgui_rect* srcrect )
{
    mem_canvas* this = (mem_canvas*)super;
    unsigned int w = SGUI_RECT_WIDTH_V( srcrect );
    unsigned int h = SGUI_RECT_HEIGHT_V( srcrect );
    unsigned char A, iA, *src, *dst, *row, *srow;
    unsigned int i, j, srcbpp, scan, lines;

    srcbpp = sgui_internal_mem_pixmap_format( pixmap );
    srcbpp = srcbpp==SGUI_RGBA8 ? 4 : (srcbpp==SGUI_RGB8 ? 3 : 1);

    if( srcbpp == 4 )
    {
        dst = (unsigned char*)this->data + (y*super->width + x)*4;
        src = sgui_internal_mem_pixmap_buffer( pixmap );
        sgui_pixmap_get_size( pixmap, &scan, &lines );
        src += (srcrect->top*scan + srcrect->left) * srcbpp;

        for( j=0; j<h; ++j, src+=scan*4, dst+=super->width*this->bpp )
        {
            for( srow=src, row=dst, i=0; i<w; ++i, row+=this->bpp, srow+=4 )
            {
                A = srow[3];
                iA = 0xFF - A;

                row[0] = (row[0] * iA + srow[0] * A)>>8;
                row[1] = (row[1] * iA + srow[1] * A)>>8;
                row[2] = (row[2] * iA + srow[2] * A)>>8;
            }
        }
    }
    else
    {
        canvas_mem_blit( super, x, y, pixmap, srcrect );
    }
}

static void canvas_mem_blend_stencil( sgui_canvas* super,
                                      unsigned char* buffer,
                                      int x, int y,
                                      unsigned int w, unsigned int h,
                                      unsigned int scan,
                                      unsigned char* color )
{
    mem_canvas* this = (mem_canvas*)super;
    unsigned char A, iA, *src, *dst, *row;
    unsigned int i, j;

    dst = (unsigned char*)this->data + (y*super->width + x)*this->bpp;

    for( j=0; j<h; ++j, buffer+=scan, dst+=super->width*this->bpp )
    {
        for( src=buffer, row=dst, i=0; i<w; ++i, row+=this->bpp, ++src )
        {
            A = *src;
            iA = 0xFF-A;

            row[0] = (row[0] * iA + color[0] * A)>>8;
            row[1] = (row[1] * iA + color[1] * A)>>8;
            row[2] = (row[2] * iA + color[2] * A)>>8;
        }
    }
}

static int canvas_mem_draw_string( sgui_canvas* super, int x, int y,
                                   sgui_font* font, unsigned char* color,
                                   const char* text, unsigned int length )
{
    int bearing, oldx = x;
    unsigned int i, w, h, len = 0;
    unsigned long character, previous=0;
    unsigned char* buffer;
    sgui_rect r;

    /* for each character */
    for( i=0; i<length && (*text) && (*text!='\n'); text+=len, i+=len )
    {
        /* load the next glyph */
        character = sgui_utf8_decode( text, &len );
        sgui_font_load_glyph( font, character );

        /* apply kerning */
        x += sgui_font_get_kerning_distance( font, previous, character );

        /* blend onto destination buffer */
        sgui_font_get_glyph_metrics( font, &w, &h, &bearing );
        buffer = sgui_font_get_glyph( font );

        sgui_rect_set_size( &r, x, y + bearing, w, h );

        if( buffer && sgui_rect_get_intersection( &r, &super->sc, &r ) )
        {
            buffer += (r.top - (y + bearing)) * w + (r.left - x);

            canvas_mem_blend_stencil( super, buffer, r.left, r.top,
                                      SGUI_RECT_WIDTH( r ),
                                      SGUI_RECT_HEIGHT( r ),
                                      w, color );
        }

        /* advance cursor */
        x += w + 1;

        /* store previous glyph index for kerning */
        previous = character;
    }

    return x - oldx;
}

static sgui_pixmap* canvas_mem_create_pixmap( sgui_canvas* super,
                                              unsigned int width,
                                              unsigned int height,
                                              int format )
{
    (void)super;

    return sgui_internal_mem_pixmap_create( width, height, format );
}

void canvas_mem_begin( sgui_canvas* this, sgui_rect* r )
{
    sgui_rect_copy( &(((mem_canvas*)this)->locked), r );
    mem_canvas_swaprb( this );
}

void canvas_mem_end( sgui_canvas* this )
{
    mem_canvas_swaprb( this );
}

/****************************************************************************/

sgui_canvas* sgui_memory_canvas_create( unsigned char* buffer,
                                        unsigned int width,
                                        unsigned int height,
                                        int format, int swaprb )
{
    mem_canvas* this;
    sgui_canvas* super;

    if( !buffer || !width || !height )
        return NULL;

    if( format!=SGUI_RGBA8 && format!=SGUI_RGB8 )
        return NULL;

    this = malloc( sizeof(mem_canvas) );
    super = (sgui_canvas*)this;

    if( !this )
        return NULL;

    this->data = buffer;
    this->bpp = format==SGUI_RGBA8 ? 4 : 3;

    sgui_internal_canvas_init( super, width, height );

    super->destroy = canvas_mem_destroy;
    super->resize = canvas_mem_resize;
    super->clear = canvas_mem_clear;
    super->blit = canvas_mem_blit;
    super->blend = canvas_mem_blend;
    super->stretch_blit = NULL;
    super->stretch_blend = NULL;
    super->draw_string = canvas_mem_draw_string;
    super->create_pixmap = canvas_mem_create_pixmap;
    super->skin_pixmap = NULL;

    if( swaprb )
    {
        super->begin = canvas_mem_begin;
        super->end = canvas_mem_end;
    }

    return (sgui_canvas*)this;
}

void sgui_memory_canvas_set_buffer( sgui_canvas* this,
                                    unsigned char* buffer )
{
    if( this && buffer )
    {
        ((mem_canvas*)this)->data = buffer;
    }
}
#endif

