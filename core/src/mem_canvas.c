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
#include <string.h>



#ifndef SGUI_NO_MEM_CANVAS
static void canvas_mem_resize( sgui_canvas* super, unsigned int width,
                               unsigned int height )
{
    (void)super; (void)width; (void)height;
}

static void canvas_mem_clear( sgui_canvas* super, sgui_rect* r )
{
    sgui_mem_canvas* this = (sgui_mem_canvas*)super;
    unsigned int deltax, deltay;
    unsigned char* dst;
    int i;

    dst = this->data + (r->top*super->width + r->left)*this->bpp;
    deltax = SGUI_RECT_WIDTH_V(r)*this->bpp;
    deltay = super->width*this->bpp;

    /* clear */
    for( i=r->top; i<=r->bottom; ++i, dst+=deltay )
    {
        memset( dst, 0, deltax );
    }
}

/****************************************************************************/

static void canvas_mem_draw_box_rgb( sgui_canvas* super, sgui_rect* r,
                                     const unsigned char* color, int format )
{
    sgui_mem_canvas* this = (sgui_mem_canvas*)super;
    unsigned int R=0, G=0, B=0, A=0, iA=0;
    unsigned char *dst, *row;
    int i, j;

    dst = this->data + (r->top*super->width + r->left)*3;

    if( format==SGUI_RGBA8 || format==SGUI_RGB8 )
    {
        R = this->swaprb ? color[2] : color[0];
        G = color[1];
        B = this->swaprb ? color[0] : color[2];
    }

    if( format==SGUI_RGBA8 )
    {
        A = color[3];
        iA = 0xFF - A;
        R *= A;
        G *= A;
        B *= A;

        for( j=r->top; j<=r->bottom; ++j, dst+=super->width*3 )
        {
            for( row=dst, i=r->left; i<=r->right; ++i, row+=3 )
            {
                row[0] = (row[0] * iA + R)>>8;
                row[1] = (row[1] * iA + G)>>8;
                row[2] = (row[2] * iA + B)>>8;
            }
        }
    }
    else if( format==SGUI_RGB8 )
    {
        for( j=r->top; j<=r->bottom; ++j, dst+=super->width*3 )
        {
            for( row=dst, i=r->left; i<=r->right; ++i, row+=3 )
            {
                row[0] = R;
                row[1] = G;
                row[2] = B;
            }
        }
    }
    else
    {
        for( j=r->top; j<=r->bottom; ++j, dst+=super->width*3 )
        {
            for( row=dst, i=r->left; i<=r->right; ++i, row+=3 )
            {
                row[0] = row[1] = row[2] = *color;
            }
        }
    }
}

static void canvas_mem_blit_rgb( sgui_canvas* super, int x, int y,
                                 sgui_pixmap* pixmap, sgui_rect* srcrect )
{
    sgui_mem_canvas* this = (sgui_mem_canvas*)super;
    unsigned int w = SGUI_RECT_WIDTH_V( srcrect ) * 3;
    unsigned int h = SGUI_RECT_HEIGHT_V( srcrect );
    unsigned char *dst = this->data + (y*super->width + x)*3, *row;
    unsigned char *src = sgui_internal_mem_pixmap_buffer( pixmap ), *srow;
    unsigned int format = sgui_internal_mem_pixmap_format( pixmap ), i, j;
    unsigned int dy = super->width*3, scan, lines;
    sgui_pixmap_get_size( pixmap, &scan, &lines );

    if( format==SGUI_RGBA8 )
    {
        src += (srcrect->top*scan + srcrect->left) * 3;
        scan <<= 2;

        for( j=0; j<h; ++j, src+=scan, dst+=dy )
        {
            for( srow=src, row=dst, i=0; i<w; ++i, ++srow )
            {
                *(row++) = *(srow++);
            }
        }
    }
    else if( format==SGUI_RGB8 )
    {
        src += (srcrect->top*scan + srcrect->left) * 3;
        scan *= 3;

        for( j=0; j<h; ++j, src+=scan, dst+=dy )
        {
            memcpy( dst, src, w );
        }
    }
}

static void canvas_mem_blend_rgb( sgui_canvas* super, int x, int y,
                                  sgui_pixmap* pixmap, sgui_rect* srcrect )
{
    sgui_mem_canvas* this = (sgui_mem_canvas*)super;
    unsigned int w = SGUI_RECT_WIDTH_V( srcrect );
    unsigned int h = SGUI_RECT_HEIGHT_V( srcrect );
    unsigned char iA, *src, *dst, *row, *srow;
    unsigned int i, j, scan, lines, dyd;

    if( sgui_internal_mem_pixmap_format( pixmap )!=SGUI_RGBA8 )
    {
        canvas_mem_blit_rgb( super, x, y, pixmap, srcrect );
        return;
    }

    sgui_pixmap_get_size( pixmap, &scan, &lines );
    dst = this->data + (y*super->width + x) * 3;
    src = sgui_internal_mem_pixmap_buffer( pixmap );
    src += (srcrect->top*scan + srcrect->left) * 4;
    dyd = super->width*3;
    scan <<= 2;

    for( j=0; j<h; ++j, src+=scan, dst+=dyd )
    {
        for( srow=src, row=dst, i=0; i<w; ++i, row+=3, srow+=4 )
        {
            iA = 0xFF - srow[3];

            row[0] = ((row[0] * iA)>>8) + srow[0];
            row[1] = ((row[1] * iA)>>8) + srow[1];
            row[2] = ((row[2] * iA)>>8) + srow[2];
        }
    }
}

static void canvas_mem_blend_stencil_rgb( sgui_canvas* super,
                                          unsigned char* buffer,
                                          int x, int y,
                                          unsigned int w, unsigned int h,
                                          unsigned int scan,
                                          const unsigned char* color )
{
    sgui_mem_canvas* this = (sgui_mem_canvas*)super;
    unsigned char A, iA, *src, *row, *dst = this->data+(y*super->width+x)*3;
    unsigned int i, j;

    unsigned int R = this->swaprb ? color[2] : color[0];
    unsigned int G = color[1];
    unsigned int B = this->swaprb ? color[0] : color[2];

    for( j=0; j<h; ++j, buffer+=scan, dst+=super->width*3 )
    {
        for( src=buffer, row=dst, i=0; i<w; ++i, row+=3, ++src )
        {
            A = *src;
            iA = 0xFF-A;

            row[0] = (row[0] * iA + R * A)>>8;
            row[1] = (row[1] * iA + G * A)>>8;
            row[2] = (row[2] * iA + B * A)>>8;
        }
    }
}

/****************************************************************************/

static void canvas_mem_draw_box_rgba( sgui_canvas* super, sgui_rect* r,
                                      const unsigned char* color, int format )
{
    sgui_mem_canvas* this = (sgui_mem_canvas*)super;
    unsigned int R=0, G=0, B=0, A=0, iA=0;
    unsigned char *dst, *row;
    int i, j;

    dst = this->data + (r->top*super->width + r->left)*4;

    if( format==SGUI_RGBA8 || format==SGUI_RGB8 )
    {
        R = this->swaprb ? color[2] : color[0];
        G = color[1];
        B = this->swaprb ? color[0] : color[2];
    }

    if( format==SGUI_RGBA8 )
    {
        A = color[3];
        iA = 0xFF - A;
        R *= A;
        G *= A;
        B *= A;
        A = A<<8;

        for( j=r->top; j<=r->bottom; ++j, dst+=super->width*4 )
        {
            for( row=dst, i=r->left; i<=r->right; ++i, row+=4 )
            {
                row[0] = (row[0] * iA + R)>>8;
                row[1] = (row[1] * iA + G)>>8;
                row[2] = (row[2] * iA + B)>>8;
                row[3] = (row[3] * iA + A)>>8;
            }
        }
    }
    else if( format==SGUI_RGB8 )
    {
        for( j=r->top; j<=r->bottom; ++j, dst+=super->width*4 )
        {
            for( row=dst, i=r->left; i<=r->right; ++i, row+=4 )
            {
                row[0] = R;
                row[1] = G;
                row[2] = B;
                row[3] = 0xFF;
            }
        }
    }
    else
    {
        for( j=r->top; j<=r->bottom; ++j, dst+=super->width*4 )
        {
            for( row=dst, i=r->left; i<=r->right; ++i, row+=4 )
            {
                row[0] = row[1] = row[2] = *color;
                row[3] = 0xFF;
            }
        }
    }
}

static void canvas_mem_blit_rgba( sgui_canvas* super, int x, int y,
                                  sgui_pixmap* pixmap, sgui_rect* srcrect )
{
    sgui_mem_canvas* this = (sgui_mem_canvas*)super;
    unsigned int w = SGUI_RECT_WIDTH_V( srcrect );
    unsigned int h = SGUI_RECT_HEIGHT_V( srcrect );
    unsigned char *dst = this->data + (y*super->width + x)*4, *row;
    unsigned char *src = sgui_internal_mem_pixmap_buffer( pixmap ), *srow;
    unsigned int format = sgui_internal_mem_pixmap_format( pixmap ), i, j;
    unsigned int dy = super->width*4, scan, lines;
    sgui_pixmap_get_size( pixmap, &scan, &lines );

    if( format==SGUI_RGBA8 )
    {
        src += (srcrect->top*scan + srcrect->left) << 2;
        scan <<= 2;

        for( j=0; j<h; ++j, src+=scan, dst+=dy )
        {
            memcpy( dst, src, w<<2 );

            for( row=dst, i=0; i<w; ++i, row+=4 )
                row[3] = 0xFF;
        }
    }
    else if( format==SGUI_RGB8 )
    {
        src += (srcrect->top*scan + srcrect->left) * 3;
        scan *= 3;

        for( j=0; j<h; ++j, src+=scan, dst+=dy )
        {
            for( srow=src, row=dst, i=0; i<w; ++i )
            {
                *(row++) = *(srow++);
                *(row++) = *(srow++);
                *(row++) = *(srow++);
                *(row++) = 0xFF;
            }
        }
    }
}

static void canvas_mem_blend_rgba( sgui_canvas* super, int x, int y,
                                   sgui_pixmap* pixmap, sgui_rect* srcrect )
{
    sgui_mem_canvas* this = (sgui_mem_canvas*)super;
    unsigned int w = SGUI_RECT_WIDTH_V( srcrect );
    unsigned int h = SGUI_RECT_HEIGHT_V( srcrect );
    unsigned char iA, *src, *dst, *row, *srow;
    unsigned int i, j, scan, lines, dyd;

    if( sgui_internal_mem_pixmap_format( pixmap )!=SGUI_RGBA8 )
    {
        canvas_mem_blit_rgba( super, x, y, pixmap, srcrect );
        return;
    }

    sgui_pixmap_get_size( pixmap, &scan, &lines );
    dst = this->data + (y*super->width + x) * 4;
    src = sgui_internal_mem_pixmap_buffer( pixmap );
    src += (srcrect->top*scan + srcrect->left) * 4;
    dyd = super->width*4;
    scan <<= 2;

    for( j=0; j<h; ++j, src+=scan, dst+=dyd )
    {
        for( srow=src, row=dst, i=0; i<w; ++i, row+=4, srow+=4 )
        {
            iA = 0xFF - srow[3];

            row[0] = ((row[0] * iA)>>8) + srow[0];
            row[1] = ((row[1] * iA)>>8) + srow[1];
            row[2] = ((row[2] * iA)>>8) + srow[2];
            row[3] = ((row[3] * iA)>>8) + srow[3];
        }
    }
}

static void canvas_mem_blend_stencil_rgba( sgui_canvas* super,
                                           unsigned char* buffer,
                                           int x, int y,
                                           unsigned int w, unsigned int h,
                                           unsigned int scan,
                                           const unsigned char* color )
{
    sgui_mem_canvas* this = (sgui_mem_canvas*)super;
    unsigned char A, iA, *src, *row, *dst = this->data+(y*super->width+x)*4;
    unsigned int i, j;

    unsigned int R = this->swaprb ? color[2] : color[0];
    unsigned int G = color[1];
    unsigned int B = this->swaprb ? color[0] : color[2];

    for( j=0; j<h; ++j, buffer+=scan, dst+=super->width*4 )
    {
        for( src=buffer, row=dst, i=0; i<w; ++i, row+=4, ++src )
        {
            A = *src;
            iA = 0xFF-A;

            row[0] = (row[0] * iA +  R * A)>>8;
            row[1] = (row[1] * iA +  G * A)>>8;
            row[2] = (row[2] * iA +  B * A)>>8;
            row[3] = (row[3] * iA + (A<<8)) >> 8;
        }
    }
}

/****************************************************************************/

static int canvas_mem_draw_string( sgui_canvas* super, int x, int y,
                                   sgui_font* font,
                                   const unsigned char* color,
                                   const char* text, unsigned int length )
{
    sgui_mem_canvas* this = (sgui_mem_canvas*)super;
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
        font->load_glyph( font, character );

        /* apply kerning */
        x += font->get_kerning_distance( font, previous, character );

        /* blend onto destination buffer */
        font->get_glyph_metrics( font, &w, &h, &bearing );
        buffer = font->get_glyph( font );

        sgui_rect_set_size( &r, x, y + bearing, w, h );

        if( buffer && sgui_rect_get_intersection( &r, &super->sc, &r ) )
        {
            buffer += (r.top - (y + bearing)) * w + (r.left - x);

            this->blend_stencil( super, buffer, r.left, r.top,
                                 SGUI_RECT_WIDTH( r ), SGUI_RECT_HEIGHT( r ),
                                 w, color );
        }

        /* advance cursor */
        x += w + 1;

        /* store previous glyph index for kerning */
        previous = character;
    }

    return x - oldx;
}

static sgui_pixmap* canvas_mem_create_pixmap( sgui_canvas* this,
                                              unsigned int width,
                                              unsigned int height,
                                              int format )
{
    return sgui_internal_mem_pixmap_create( width, height, format,
                                            ((sgui_mem_canvas*)this)->swaprb);
}

/****************************************************************************/

sgui_canvas* sgui_memory_canvas_create( unsigned char* buffer,
                                        unsigned int width,
                                        unsigned int height,
                                        int format, int swaprb )
{
    sgui_mem_canvas* this = malloc( sizeof(sgui_mem_canvas) );
    sgui_canvas* super = (sgui_canvas*)this;

    if( !this )
        return NULL;

    if( !sgui_memory_canvas_init(super,buffer,width,height,format,swaprb) )
    {
        free( this );
        this = NULL;
    }
    return (sgui_canvas*)this;
}

int sgui_memory_canvas_init( sgui_canvas* super, unsigned char* buffer,
                             unsigned int width,
                             unsigned int height,
                             int format, int swaprb )
{
    sgui_mem_canvas* this = (sgui_mem_canvas*)super;

    if( !this || !buffer || !width || !height )
        return 0;

    if( format!=SGUI_RGBA8 && format!=SGUI_RGB8 )
        return 0;

    if( !sgui_canvas_init( super, width, height ) )
        return 0;

    this->data = buffer;
    this->bpp = format==SGUI_RGBA8 ? 4 : 3;
    this->swaprb = swaprb;

    if( format==SGUI_RGBA8 )
    {
        super->blit = canvas_mem_blit_rgba;
        super->blend = canvas_mem_blend_rgba;
        super->draw_box = canvas_mem_draw_box_rgba;
        this->blend_stencil = canvas_mem_blend_stencil_rgba;
    }
    else
    {
        super->blit = canvas_mem_blit_rgb;
        super->blend = canvas_mem_blend_rgb;
        super->draw_box = canvas_mem_draw_box_rgb;
        this->blend_stencil = canvas_mem_blend_stencil_rgb;
    }

    super->destroy = (void(*)(sgui_canvas*))free;
    super->resize = canvas_mem_resize;
    super->clear = canvas_mem_clear;
    super->draw_string = canvas_mem_draw_string;
    super->create_pixmap = canvas_mem_create_pixmap;
    return 1;
}

void sgui_memory_canvas_set_buffer( sgui_canvas* this,
                                    unsigned char* buffer )
{
    if( this && buffer )
    {
        ((sgui_mem_canvas*)this)->data = buffer;
    }
}
#elif defined(SGUI_NOP_IMPLEMENTATIONS)
sgui_canvas* sgui_memory_canvas_create( unsigned char* buffer,
                                        unsigned int width,
                                        unsigned int height,
                                        int format, int swaprb )
{
    (void)buffer;
    (void)width;
    (void)height;
    (void)format;
    (void)swaprb;
    return NULL;
}

int sgui_memory_canvas_init( sgui_canvas* super, unsigned char* buffer,
                             unsigned int width,
                             unsigned int height,
                             int format, int swaprb )
{
    (void)super; (void)buffer; (void)width; (void)height;
    (void)format; (void)swaprb;
    return 0;
}

void sgui_memory_canvas_set_buffer( sgui_canvas* canvas,
                                    unsigned char* buffer )
{
    (void)canvas;
    (void)buffer;
}
#endif

