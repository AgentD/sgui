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



struct sgui_pixmap
{
    unsigned int width, height;
    int format, backend;

    union
    {
        struct
        {
            HDC hDC;
            HBITMAP bitmap;
            unsigned char* ptr;
        }
        native;

        unsigned int opengl;
    }
    pm;
};



sgui_pixmap* sgui_pixmap_create( unsigned int width, unsigned int height,
                                 int format, int backend )
{
    sgui_pixmap* pix;

    if( !width || !height || !format )
        return NULL;

    pix = malloc( sizeof(sgui_pixmap) );

    if( !pix )
        return NULL;

    pix->width   = width;
    pix->height  = height;
    pix->format  = format;
    pix->backend = backend;

    if( backend==SGUI_OPENGL_CORE || backend==SGUI_OPENGL_COMPAT )
    {
        pix->pm.opengl = sgui_opengl_pixmap_create( width, height, format );

        if( !pix->pm.opengl )
        {
            free( pix );
            pix = NULL;
        }
    }
    else
    {
        BITMAPINFO info;

        info.bmiHeader.biSize        = sizeof(info.bmiHeader);
        info.bmiHeader.biBitCount    = 32;
        info.bmiHeader.biCompression = BI_RGB;
        info.bmiHeader.biPlanes      = 1;
        info.bmiHeader.biWidth       = width;
        info.bmiHeader.biHeight      = -((int)height);

        pix->pm.native.hDC = CreateCompatibleDC( NULL );

        if( !pix->pm.native.hDC )
        {
            free( pix );
            return NULL;
        }

        pix->pm.native.bitmap = CreateDIBSection( pix->pm.native.hDC, &info,
                                                  DIB_RGB_COLORS,
                                                  (void**)&pix->pm.native.ptr,
                                                  0, 0 );

        if( !pix->pm.native.bitmap )
        {
            DeleteDC( pix->pm.native.hDC );
            free( pix );
            return NULL;
        }

        SelectObject( pix->pm.native.hDC, pix->pm.native.bitmap );
    }

    return pix;
}

void sgui_pixmap_load( sgui_pixmap* pixmap, sgui_rect* dstrect,
                       const unsigned char* data, int srcx, int srcy,
                       unsigned int width, unsigned int height,
                       int format )
{
    if( !pixmap || !data || !width || !height )
        return;

    if( pixmap->backend==SGUI_OPENGL_CORE ||
        pixmap->backend==SGUI_OPENGL_COMPAT )
    {
        sgui_opengl_pixmap_load( pixmap->pm.opengl, dstrect, data, srcx, srcy,
                                 width, height, format );
    }
    else
    {
        unsigned char *dst, *dstrow;
        const unsigned char *src, *row;
        int i, j, bpp = format==SGUI_RGB8 ? 3 : 4, alpha;

        int dstx = dstrect ? dstrect->left : 0;
        int dsty = dstrect ? dstrect->top  : 0;
        int subw = dstrect ? SGUI_RECT_WIDTH_V( dstrect ) : (int)width;
        int subh = dstrect ? SGUI_RECT_HEIGHT_V( dstrect ) : (int)height;

        data += (srcy*width + srcx)*bpp;
        dst = pixmap->pm.native.ptr + (dstx + dsty*pixmap->width)*4;

        for( src=data, j=0; j<subh; ++j, src+=width*bpp,
                                         dst+=pixmap->width*4 )
        {
            for( dstrow=dst, row=src, i=0; i<subw; ++i, row+=bpp, dstrow+=4 )
            {
                alpha = bpp==4 ? row[3] : 0xFF;

                dstrow[0] = row[2]*alpha >> 8;
                dstrow[1] = row[1]*alpha >> 8;
                dstrow[2] = row[0]*alpha >> 8;
                dstrow[3] = alpha;
            }
        }
    }
}

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

unsigned int sgui_pixmap_opengl_get_handle( sgui_pixmap* pixmap )
{
    if( !pixmap )
        return 0;

    if( pixmap->backend==SGUI_OPENGL_CORE ||
        pixmap->backend==SGUI_OPENGL_COMPAT )
    {
        return pixmap->pm.opengl;
    }

    return 0;
}

void sgui_pixmap_destroy( sgui_pixmap* pixmap )
{
    if( !pixmap )
        return;

    if( pixmap->backend==SGUI_OPENGL_CORE ||
        pixmap->backend==SGUI_OPENGL_COMPAT )
    {
        sgui_opengl_pixmap_destroy( pixmap->pm.opengl );
    }
    else
    {
        DeleteObject( pixmap->pm.native.bitmap );
        DeleteDC( pixmap->pm.native.hDC );
    }

    free( pixmap );
}

/****************************************************************************/

void pixmap_blit( sgui_pixmap* pixmap, HDC hDC, int x, int y,
                  int srcx, int srcy,
                  unsigned int width, unsigned int height )
{
    if( pixmap && pixmap->backend==SGUI_NATIVE )
    {
        BitBlt( hDC, x, y, width, height, pixmap->pm.native.hDC,
                srcx, srcy, SRCCOPY );
    }
}

void pixmap_blend( sgui_pixmap* pixmap, HDC hDC, int x, int y,
                   int srcx, int srcy,
                   unsigned int width, unsigned int height )
{
    BLENDFUNCTION ftn;

    if( pixmap && pixmap->backend==SGUI_NATIVE )
    {
        ftn.BlendOp             = AC_SRC_OVER;
        ftn.BlendFlags          = 0;
        ftn.SourceConstantAlpha = 0xFF;
        ftn.AlphaFormat         = AC_SRC_ALPHA;

        AlphaBlend( hDC, x, y, width, height, pixmap->pm.native.hDC,
                    srcx, srcy, width, height, ftn );
    }
}

