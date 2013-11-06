/*
 * mem_pixmap.c
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

#include <stdlib.h>
#include <string.h>



#ifndef SGUI_NO_MEM_CANVAS
typedef struct
{
    sgui_pixmap super;

    unsigned char* buffer;
    int format, swaprb;
}
mem_pixmap;



static void mem_pixmap_load_rgb8( sgui_pixmap* super, int dstx, int dsty,
                                  const unsigned char* data, unsigned int scan,
                                  unsigned int width, unsigned int height,
                                  int format )
{
    unsigned char *dst, *dstrow, temp;
    const unsigned char *src, *row;
    unsigned int i, j, dxd;
    mem_pixmap* this = (mem_pixmap*)super;

    dst = this->buffer + (dstx + dsty*super->width) * 3;
    dxd = super->width*3;

    if( format==SGUI_A8 )
    {
        for( src=data, j=0; j<height; ++j, src+=scan, dst+=dxd )
        {
            for( dstrow=dst, row=src, i=0; i<width; ++i, ++row, dstrow+=3 )
            {
                dstrow[0] = dstrow[1] = dstrow[2] = *row;
            }
        }
    }
    else if( format==SGUI_RGB8 )
    {
        for( src=data, j=0; j<height; ++j, src+=3*scan, dst+=dxd )
        {
            memcpy( dst, src, width );
        }
    }
    else
    {
        for( src=data, j=0; j<height; ++j, src+=4*scan, dst+=dxd )
        {
            for( dstrow=dst, row=src, i=0; i<width; ++i, ++row )
            {
                *(dstrow++) = *(row++);
                *(dstrow++) = *(row++);
                *(dstrow++) = *(row++);
            }
        }
    }

    if( this->swaprb )
    {
        dst = this->buffer + (dstx + dsty*super->width) * 3;

        for( j=0; j<height; ++j, dst+=dxd )
        {
            for( dstrow=dst, i=0; i<width; ++i, ++row, dstrow+=3 )
            {
                temp = dstrow[0];
                dstrow[0] = dstrow[2];
                dstrow[2] = temp;
            }
        }
    }
}

static void mem_pixmap_load_rgba8( sgui_pixmap* super, int dstx, int dsty,
                                   const unsigned char* data,
                                   unsigned int scan, unsigned int width,
                                   unsigned int height, int format )
{
    unsigned char temp, *dst, *dstrow;
    const unsigned char *src, *row;
    unsigned int i, j, alpha;
    mem_pixmap* this = (mem_pixmap*)super;

    dst = this->buffer + (dstx + dsty*super->width) * 4;

    if( format==SGUI_A8 )
    {
        for( src=data, j=0; j<height; ++j, src+=scan, dst+=super->width*4 )
        {
            for( dstrow=dst, row=src, i=0; i<width; ++i, ++row, dstrow+=4 )
            {
                dstrow[0] = dstrow[1] = dstrow[2] = dstrow[3] = *row;
            }
        }
    }
    else if( format==SGUI_RGB8 )
    {
        for( src=data, j=0; j<height; ++j, src+=scan*3, dst+=super->width*4 )
        {
            for( dstrow=dst, row=src, i=0; i<width; ++i, row+=3, dstrow+=4 )
            {
                dstrow[0] = row[0];
                dstrow[1] = row[1];
                dstrow[2] = row[2];
                dstrow[3] = 0xFF;
            }
        }
    }
    else
    {
        for( src=data, j=0; j<height; ++j, src+=scan*4, dst+=super->width*4 )
        {
            for( dstrow=dst, row=src, i=0; i<width; ++i, row+=4, dstrow+=4 )
            {
                alpha = row[3];

                dstrow[0] = (row[0] * alpha) >>8;
                dstrow[1] = (row[1] * alpha) >>8;
                dstrow[2] = (row[2] * alpha) >>8;
                dstrow[3] = alpha;
            }
        }
    }

    if( this->swaprb )
    {
        dst = this->buffer + (dstx + dsty*super->width) * 4;

        for( j=0; j<height; ++j, dst+=super->width*4 )
        {
            for( dstrow=dst, i=0; i<width; ++i, ++row, dstrow+=4 )
            {
                temp = dstrow[0];
                dstrow[0] = dstrow[2];
                dstrow[2] = temp;
            }
        }
    }
}

static void mem_pixmap_load_a8( sgui_pixmap* super, int dstx, int dsty,
                                const unsigned char* data,
                                unsigned int scan, unsigned int width,
                                unsigned int height, int format )
{
    unsigned char *dst, *dstrow;
    const unsigned char *src, *row;
    unsigned int i, j;
    mem_pixmap* this = (mem_pixmap*)super;

    dst = this->buffer + (dstx + dsty*super->width);

    if( format==SGUI_A8 )
    {
        for( src=data, j=0; j<height; ++j, src+=scan, dst+=super->width )
        {
            memcpy( dst, src, width );
        }
    }
    else if( format==SGUI_RGB8 )
    {
        for( src=data, j=0; j<height; ++j, src+=scan, dst+=super->width )
        {
            for( dstrow=dst, row=src, i=0; i<width; ++i, row+=3 )
            {
                *(dstrow++) = *row;
            }
        }
    }
    else
    {
        for( src=data, j=0; j<height; ++j, src+=scan, dst+=super->width )
        {
            for( dstrow=dst, row=src, i=0; i<width; ++i, row+=4 )
            {
                *(dstrow++) = row[3];
            }
        }
    }
}

static void mem_pixmap_destroy( sgui_pixmap* super )
{
    mem_pixmap* this = (mem_pixmap*)super;

    free( this->buffer );
    free( this );
}

/****************************************************************************/

sgui_pixmap* sgui_internal_mem_pixmap_create( unsigned int width,
                                              unsigned int height,
                                              int format, int swaprb )
{
    mem_pixmap* this;
    sgui_pixmap* super;

    if( !width || !height )
        return NULL;

    this = malloc( sizeof(mem_pixmap) );
    super = (sgui_pixmap*)this;

    if( !this )
        return NULL;

    this->buffer = malloc( width*height*(format==SGUI_RGB8 ? 3 : 
                                         format==SGUI_RGBA8 ? 4 : 1) );

    if( !this->buffer )
    {
        free( this );
        return NULL;
    }

    this->format   = format;
    this->swaprb   = swaprb;
    super->width   = width;
    super->height  = height;
    super->destroy = mem_pixmap_destroy;
    super->load    = format==SGUI_RGBA8 ? mem_pixmap_load_rgba8 :
                     format==SGUI_RGB8  ? mem_pixmap_load_rgb8 :
                                          mem_pixmap_load_a8;

    return (sgui_pixmap*)this;
}

unsigned char* sgui_internal_mem_pixmap_buffer( sgui_pixmap* this )
{
    return this ? ((mem_pixmap*)this)->buffer : NULL;
}

int sgui_internal_mem_pixmap_format( sgui_pixmap* this )
{
    return this ? ((mem_pixmap*)this)->format : 0;
}
#endif

