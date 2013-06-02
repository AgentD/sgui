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



void xlib_pixmap_destroy( sgui_pixmap* pixmap )
{
    XRenderFreePicture( dpy, ((xlib_pixmap*)pixmap)->pic );
    XFreePixmap( dpy, ((xlib_pixmap*)pixmap)->pix );
    free( pixmap );
}

void xlib_pixmap_load( sgui_pixmap* pixmap, int dstx, int dsty,
                       const unsigned char* data, unsigned int scan,
                       unsigned int width, unsigned int height, int format )
{
    XRenderColor c;
    const unsigned char *src, *row;
    unsigned int i, j, alpha, bpp = format==SGUI_RGB8 ? 3 : 4;

    for( src=data, j=0; j<height; ++j, src+=scan*bpp )
    {
        for( row=src, i=0; i<width; ++i, row+=bpp )
        {
            alpha = bpp==4 ? row[3] : 0xFF;

            /* Xrender expects premultiplied alpha! */
            c.red   = row[0]*alpha;
            c.green = row[1]*alpha;
            c.blue  = row[2]*alpha;
            c.alpha = alpha<<8;

            XRenderFillRectangle( dpy, PictOpSrc, ((xlib_pixmap*)pixmap)->pic,
                                  &c, dstx+i, dsty+j, 1, 1 );
        }
    }
}

/****************************************************************************/

sgui_pixmap* xlib_pixmap_create( unsigned int width, unsigned int height,
                                 int format, Window wnd )
{
    xlib_pixmap* pixmap = NULL;
    XRenderPictFormat* fmt;

    /* create pixmap structure */
    pixmap = malloc( sizeof(xlib_pixmap) ); 

    if( !pixmap )
        return NULL;

    pixmap->pm.width   = width;
    pixmap->pm.height  = height;
    pixmap->pm.destroy = xlib_pixmap_destroy;
    pixmap->pm.load    = xlib_pixmap_load;

    /* try to create an X11 Pixmap */
    pixmap->pix = XCreatePixmap( dpy, wnd, width, height,
                                 format==SGUI_RGB8 ? 24 : 32 );

    if( !pixmap->pix )
    {
        free( pixmap );
        return NULL;
    }

    /* try to create XRender picture */
    if( format==SGUI_RGB8 )
        fmt = XRenderFindStandardFormat( dpy, PictStandardRGB24 );
    else
        fmt = XRenderFindStandardFormat( dpy, PictStandardARGB32 );

    pixmap->pic = XRenderCreatePicture( dpy, pixmap->pix, fmt, 0, NULL );

    if( !pixmap->pic )
    {
        XFreePixmap( dpy, pixmap->pix );
        free( pixmap );
        return NULL;
    }

    return (sgui_pixmap*)pixmap;
}

