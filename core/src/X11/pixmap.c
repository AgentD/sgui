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
#include "sgui_pixmap.h"
#include "sgui_opengl.h"
#include "internal.h"



struct sgui_pixmap
{
    unsigned int width;
    unsigned int height;

    int backend;
    int format;

    union
    {
        struct
        {
            Pixmap pix;
            Picture pic;
        }
        native;

        unsigned int opengl;
    }
    pm;
};



sgui_pixmap* sgui_pixmap_create( unsigned int width, unsigned int height,
                                 int format, int backend )
{
    sgui_pixmap* pixmap = NULL;
    XRenderPictFormat* fmt;

    pixmap = malloc( sizeof(sgui_pixmap) ); 

    if( !pixmap )
        return NULL;

    pixmap->width   = width;
    pixmap->height  = height;
    pixmap->format  = format;
    pixmap->backend = backend;

    if( backend==SGUI_OPENGL_CORE || backend==SGUI_OPENGL_COMPAT )
    {
        pixmap->pm.opengl = sgui_opengl_pixmap_create(width, height, format);

        if( !pixmap->pm.opengl )
        {
            free( pixmap );
            pixmap = NULL;
        }
    }
    else
    {
        /* try to create an X11 Pixmap */
        pixmap->pm.native.pix = XCreatePixmap( dpy, DefaultRootWindow(dpy),
                                               width, height,
                                               format==SGUI_RGB8 ? 24 : 32 );

        if( !pixmap->pm.native.pix )
        {
            free( pixmap );
            return NULL;
        }

        if( format==SGUI_RGB8 )
            fmt = XRenderFindStandardFormat( dpy, PictStandardRGB24 );
        else
            fmt = XRenderFindStandardFormat( dpy, PictStandardARGB32 );

        pixmap->pm.native.pic = XRenderCreatePicture( dpy,
                                                      pixmap->pm.native.pix,
                                                      fmt, 0, NULL );

        if( !pixmap->pm.native.pic )
        {
            XFreePixmap( dpy, pixmap->pm.native.pix );
            free( pixmap );
            return NULL;
        }
    }

    return pixmap;
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
        XRenderColor c;
        const unsigned char *src, *row;
        int i, j, bpp = format==SGUI_RGB8 ? 3 : 4, alpha;

        int dstx = dstrect ? dstrect->left : 0;
        int dsty = dstrect ? dstrect->top  : 0;
        int subw = dstrect ? SGUI_RECT_WIDTH_V( dstrect ) : (int)width;
        int subh = dstrect ? SGUI_RECT_HEIGHT_V( dstrect ) : (int)height;

        data += (srcy*width + srcx)*(format==SGUI_RGB8 ? 3 : 4);

        for( src=data, j=0; j<subh; ++j, src+=width*bpp )
        {
            for( row=src, i=0; i<subw; ++i, row+=bpp )
            {
                alpha = bpp==4 ? row[3] : 0xFF;

                /* Xrender expects premultiplied alpha! */
                c.red   = row[0]*alpha;
                c.green = row[1]*alpha;
                c.blue  = row[2]*alpha;
                c.alpha = alpha<<8;

                XRenderFillRectangle( dpy, PictOpSrc, pixmap->pm.native.pic,
                                      &c, dstx+i, dsty+j, 1, 1 );
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
        XRenderFreePicture( dpy, pixmap->pm.native.pic );
        XFreePixmap( dpy, pixmap->pm.native.pix );
    }

    free( pixmap );
}

/****************************************************************************/

Picture pixmap_get_picture( sgui_pixmap* pixmap )
{
    if( pixmap->backend==SGUI_NATIVE )
        return pixmap->pm.native.pic;

    return 0;
}

Pixmap pixmap_get_pixmap( sgui_pixmap* pixmap )
{
    if( pixmap->backend==SGUI_NATIVE )
        return pixmap->pm.native.pix;

    return 0;
}

