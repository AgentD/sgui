/*
 * pixmap.c
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
#include "sgui_pixmap.h"
#include "sgui_internal.h"



void sgui_pixmap_get_size( sgui_pixmap* pixmap, unsigned int* width,
                           unsigned int* height )
{
    if( pixmap )
    {
        if( width  ) *width  = pixmap->width;
        if( height ) *height = pixmap->height;
    }
    else
    {
        if( width  ) *width  = 0;
        if( height ) *height = 0;
    }
}

void sgui_pixmap_load( sgui_pixmap* pixmap, int dstx, int dsty,
                       const unsigned char* data, int srcx, int srcy,
                       unsigned int scan, unsigned int width,
                       unsigned int height, int format )
{
    int bpp =  format==SGUI_RGBA8 ? 4 :
              (format==SGUI_RGB8  ? 3 : 1);

    if( !pixmap || !data || !width || !height || srcx>=(int)scan )
        return;

    if( dstx>=(int)pixmap->width || dsty>=(int)pixmap->height )
        return;

    if( (srcx+width)>=scan )
        width = scan - dstx;

    if( (dstx+width)>=pixmap->width )
        width = pixmap->width - dstx;

    if( (dsty+height)>=pixmap->height )
        height = pixmap->height - dsty;

    data += (srcy*scan + srcx) * bpp;

    pixmap->load( pixmap, dstx, dsty, data, scan, width, height, format );
}

void sgui_pixmap_destroy( sgui_pixmap* pixmap )
{
    if( pixmap )
        pixmap->destroy( pixmap );
}

