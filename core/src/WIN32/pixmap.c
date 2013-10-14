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



static void gdi_pixmap_load( sgui_pixmap* super, int dstx, int dsty,
                             const unsigned char* data, unsigned int scan,
                             unsigned int width, unsigned int height,
                             int format )
{
    unsigned char *dst, *dstrow;
    const unsigned char *src, *row;
    unsigned int i, j, alpha, srcbpp;
    gdi_pixmap* this = (gdi_pixmap*)super;
    int bpp;

    srcbpp = format==SGUI_RGBA8 ? 4 : (format==SGUI_RGB8 ? 3 : 1);
    bpp = this->format==SGUI_RGBA8 ? 4 : (this->format==SGUI_RGB8 ? 3 : 1);
    dst = this->ptr + (dstx + dsty*super->width)*bpp;

    for( src=data, j=0; j<height; ++j, src+=scan*srcbpp,
                                       dst+=super->width*bpp )
    {
        for( dstrow=dst, row=src, i=0; i<width; ++i, row+=srcbpp,
                                                     dstrow+=bpp )
        {
            if( srcbpp>=3 )
            {
                alpha = srcbpp==4 ? row[3] : 0xFF;

                dstrow[0] = row[2]*alpha >> 8;
                dstrow[1] = row[1]*alpha >> 8;
                dstrow[2] = row[0]*alpha >> 8;

                if( bpp==4 )
                    dstrow[3] = alpha;
            }
            else
            {
                dstrow[0] = dstrow[1] = dstrow[2] = *row;

                if( bpp==4 )
                    dstrow[3] = *row;
            }
        }
    }
}

static void gdi_pixmap_destroy( sgui_pixmap* super )
{
    gdi_pixmap* this = (gdi_pixmap*)super;

    DeleteObject( this->bitmap );
    DeleteDC( this->hDC );
    free( this );
}

sgui_pixmap* gdi_pixmap_create( unsigned int width, unsigned int height,
                                int format )
{
    gdi_pixmap* this = malloc( sizeof(gdi_pixmap) );
    sgui_pixmap* super = (sgui_pixmap*)this;
    BITMAPINFO info;

    if( !this )
        return NULL;

    this->format   = format;
    super->width   = width;
    super->height  = height;
    super->destroy = gdi_pixmap_destroy;
    super->load    = gdi_pixmap_load;

    info.bmiHeader.biSize        = sizeof(info.bmiHeader);
    info.bmiHeader.biPlanes      = 1;
    info.bmiHeader.biWidth       = width;
    info.bmiHeader.biHeight      = -((int)height);
    info.bmiHeader.biCompression = BI_RGB;
    info.bmiHeader.biBitCount    = format==SGUI_RGBA8 ? 32 :
                                   format==SGUI_RGB8 ? 24 : 8;

    this->hDC = CreateCompatibleDC( NULL );

    if( !this->hDC )
    {
        free( this );
        return NULL;
    }

    this->bitmap = CreateDIBSection( this->hDC, &info, DIB_RGB_COLORS,
                                     (void**)&this->ptr, 0, 0 );

    if( !this->bitmap )
    {
        DeleteDC( this->hDC );
        free( this );
        return NULL;
    }

    SelectObject( this->hDC, this->bitmap );

    return (sgui_pixmap*)this;
}

