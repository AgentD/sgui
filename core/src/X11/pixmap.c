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
#include "platform.h"
#include "sgui_config.h"



void xlib_pixmap_destroy( sgui_pixmap* super )
{
    xlib_pixmap* this = (xlib_pixmap*)super;

    if( this->is_stencil )
    {
        free( this->data.pixels );
    }
    else
    {
        sgui_internal_lock_mutex( );
        XFreePixmap( x11.dpy, this->data.xpm );
        sgui_internal_unlock_mutex( );
    }

    free( this );
}

void xlib_pixmap_load( sgui_pixmap* super, int dstx, int dsty,
                       const unsigned char* data, unsigned int scan,
                       unsigned int width, unsigned int height,
                       int format )
{
    xlib_pixmap* this = (xlib_pixmap*)super;
    const unsigned char *src, *row;
    unsigned char *dst;
    unsigned long r, g, b, a;
    unsigned int i, j;

    if( this->is_stencil && format!=SGUI_A8 )
        return;

    sgui_internal_lock_mutex( );

    if( this->is_stencil )
    {
        dst = this->data.pixels + (dsty*super->width + dstx);

        for( j=0; j<height; ++j, data+=scan, dst+=super->width )
            memcpy( dst, data, width );
    }
    else if( format==SGUI_RGBA8 )
    {
        for( src=data, j=0; j<height; ++j, src+=scan*4 )
        {
            for( row=src, i=0; i<width; ++i, row+=4 )
            {
                a =   row[3];
                r = ((row[0]*a) >> 8) & 0x00FF;
                g = ((row[1]*a) >> 8) & 0x00FF;
                b = ((row[2]*a) >> 8) & 0x00FF;
                XSetForeground( x11.dpy, this->owner->gc, (r<<16)|(g<<8)|b );
                XDrawPoint( x11.dpy, this->data.xpm, this->owner->gc,
                            dstx+i, dsty+j );
            }
        }
    }
    else if( format==SGUI_RGB8 )
    {
        for( src=data, j=0; j<height; ++j, src+=scan*3 )
        {
            for( row=src, i=0; i<width; ++i, row+=3 )
            {
                a = (row[0]<<16) | (row[1]<<8) | row[2];
                XSetForeground( x11.dpy, this->owner->gc, a );
                XDrawPoint( x11.dpy, this->data.xpm, this->owner->gc,
                            dstx+i, dsty+j );
            }
        }
    }

    sgui_internal_unlock_mutex( );
}

sgui_pixmap* xlib_pixmap_create( sgui_canvas* cv, unsigned int width,
                                 unsigned int height, int format )
{
    sgui_canvas_xlib* owner = (sgui_canvas_xlib*)cv;
    Window wnd = ((sgui_canvas_x11*)cv)->wnd;
    sgui_pixmap* super;
    xlib_pixmap* this;

    this = malloc( sizeof(xlib_pixmap) );
    super = (sgui_pixmap*)this;

    if( this )
    {
        super->width   = width;
        super->height  = height;
        super->destroy = xlib_pixmap_destroy;
        super->load    = xlib_pixmap_load;

        this->is_stencil = format==SGUI_A8;
        this->owner      = owner;

        if( format==SGUI_A8 )
        {
            this->data.pixels = malloc( width*height );

            if( !this->data.pixels )
                goto fail;
        }
        else
        {
            sgui_internal_lock_mutex( );
            this->data.xpm = XCreatePixmap( x11.dpy, wnd, width, height, 24 );
            sgui_internal_unlock_mutex( );

            if( !this->data.xpm )
                goto fail;
        }
    }

    return (sgui_pixmap*)this;
fail:
    free( this );
    return NULL;
}
/****************************************************************************/
#ifndef SGUI_NO_XRENDER
void xrender_pixmap_destroy( sgui_pixmap* super )
{
    xrender_pixmap* this = (xrender_pixmap*)super;

    sgui_internal_lock_mutex( );
    XRenderFreePicture( x11.dpy, this->pic );
    XFreePixmap( x11.dpy, this->pix );
    sgui_internal_unlock_mutex( );
    free( this );
}

void xrender_pixmap_load( sgui_pixmap* super, int dstx, int dsty,
                          const unsigned char* data, unsigned int scan,
                          unsigned int width, unsigned int height,
                          int format )
{
    xrender_pixmap* this = (xrender_pixmap*)super;
    const unsigned char *src, *row;
    unsigned int i, j;
    XRenderColor c;
    Picture pic;

    sgui_internal_lock_mutex( );

    pic = this->pic;

    if( format==SGUI_RGBA8 )
    {
        for( src=data, j=0; j<height; ++j, src+=scan*4 )
        {
            for( row=src, i=0; i<width; ++i, row+=4 )
            {
                /* Xrender expects premultiplied alpha! */
                c.red   = row[0]*row[3];
                c.green = row[1]*row[3];
                c.blue  = row[2]*row[3];
                c.alpha = row[3]<<8;

                XRenderFillRectangle( x11.dpy, PictOpSrc, pic, &c,
                                      dstx+i, dsty+j, 1, 1 );
            }
        }
    }
    else if( format==SGUI_RGB8 )
    {
        for( src=data, j=0; j<height; ++j, src+=scan*3 )
        {
            for( row=src, i=0; i<width; ++i, row+=3 )
            {
                c.red   = row[0]<<8;
                c.green = row[1]<<8;
                c.blue  = row[2]<<8;
                c.alpha = 0xFFFF;

                XRenderFillRectangle( x11.dpy, PictOpSrc, pic, &c,
                                      dstx+i, dsty+j, 1, 1 );
            }
        }
    }
    else
    {
        for( src=data, j=0; j<height; ++j, src+=scan )
        {
            for( row=src, i=0; i<width; ++i, ++row )
            {
                c.red = c.green = c.blue = c.alpha = (*row)<<8;

                XRenderFillRectangle( x11.dpy, PictOpSrc, pic, &c,
                                      dstx+i, dsty+j, 1, 1 );
            }
        }
    }

    sgui_internal_unlock_mutex( );
}

sgui_pixmap* xrender_pixmap_create( sgui_canvas* cv, unsigned int width,
                                    unsigned int height, int format )
{
    Window wnd = ((sgui_canvas_x11*)cv)->wnd;
    xrender_pixmap* this = NULL;
    XRenderPictFormat* fmt;
    sgui_pixmap* super;
    int type;

    /* create pixmap structure */
    this = malloc( sizeof(xrender_pixmap) ); 
    super = (sgui_pixmap*)this;

    if( !this )
        return NULL;

    super->width   = width;
    super->height  = height;
    super->destroy = xrender_pixmap_destroy;
    super->load    = xrender_pixmap_load;

    sgui_internal_lock_mutex( );

    /* try to create an X11 Pixmap */
    this->pix = XCreatePixmap( x11.dpy, wnd, width, height,
                               format==SGUI_RGBA8 ? 32 :
                               format==SGUI_RGB8 ? 24 : 8 );

    if( !this->pix )
        goto fail;

    /* try to create XRender picture */
    type = (format==SGUI_RGBA8) ? PictStandardARGB32 :
           (format==SGUI_RGB8 ? PictStandardRGB24 : PictStandardA8);

    fmt = XRenderFindStandardFormat( x11.dpy, type );
    this->pic = XRenderCreatePicture( x11.dpy, this->pix, fmt, 0, NULL );

    if( !this->pic )
    {
        XFreePixmap( x11.dpy, this->pix );
        goto fail;
    }

    sgui_internal_unlock_mutex( );
    return (sgui_pixmap*)this;
fail:
    sgui_internal_unlock_mutex( );
    free( this );
    return NULL;
}
#endif /* !SGUI_NO_XRENDER */

