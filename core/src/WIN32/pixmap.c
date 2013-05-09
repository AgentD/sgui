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
#include "internal.h"
#include "sgui_opengl.h"



static void gdi_pixmap_load( sgui_pixmap* pixmap, int dstx, int dsty,
                             const unsigned char* data, unsigned int scan,
                             unsigned int width, unsigned int height,
                             int format )
{
    unsigned char *dst, *dstrow;
    const unsigned char *src, *row;
    unsigned int i, j, alpha, srcbpp = format==SGUI_RGBA8 ? 4 : 3;
    int bpp = ((gdi_pixmap*)pixmap)->format==SGUI_RGBA8 ? 4 : 3;

    dst = ((gdi_pixmap*)pixmap)->ptr + (dstx + dsty*pixmap->width)*srcbpp;

    for( src=data, j=0; j<height; ++j, src+=scan*srcbpp,
                                       dst+=pixmap->width*bpp )
    {
        for( dstrow=dst, row=src, i=0; i<width; ++i, row+=srcbpp,
                                                     dstrow+=bpp )
        {
            alpha = bpp==4 ? row[3] : 0xFF;

            dstrow[0] = row[2]*alpha >> 8;
            dstrow[1] = row[1]*alpha >> 8;
            dstrow[2] = row[0]*alpha >> 8;

            if( srcbpp==4 )
                dstrow[3] = alpha;
        }
    }
}

static void gdi_pixmap_destroy( sgui_pixmap* pixmap )
{
    DeleteObject( ((gdi_pixmap*)pixmap)->bitmap );
    DeleteDC( ((gdi_pixmap*)pixmap)->hDC );
    free( pixmap );
}

sgui_pixmap* gdi_pixmap_create( unsigned int width, unsigned int height,
                                int format )
{
    gdi_pixmap* pix = malloc( sizeof(gdi_pixmap) );
    BITMAPINFO info;

    if( !pix )
        return NULL;

    pix->format     = format;
    pix->pm.width   = width;
    pix->pm.height  = height;
    pix->pm.destroy = gdi_pixmap_destroy;
    pix->pm.load    = gdi_pixmap_load;

    info.bmiHeader.biSize        = sizeof(info.bmiHeader);
    info.bmiHeader.biBitCount    = format==SGUI_RGBA8 ? 32 : 24;
    info.bmiHeader.biCompression = BI_RGB;
    info.bmiHeader.biPlanes      = 1;
    info.bmiHeader.biWidth       = width;
    info.bmiHeader.biHeight      = -((int)height);

    pix->hDC = CreateCompatibleDC( NULL );

    if( !pix->hDC )
    {
        free( pix );
        return NULL;
    }

    pix->bitmap = CreateDIBSection( pix->hDC, &info, DIB_RGB_COLORS,
                                    (void**)&pix->ptr, 0, 0 );

    if( !pix->bitmap )
    {
        DeleteDC( pix->hDC );
        free( pix );
        return NULL;
    }

    SelectObject( pix->hDC, pix->bitmap );

    return (sgui_pixmap*)pix;
}

