/*
 * canvas.c
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


#define COLOR_COPY_INV( a, b ) (a)[0]=(b)[2]; (a)[1]=(b)[1]; (a)[2]=(b)[0]

#define COLOR_BLEND_INV( a, b, A, iA )\
        (a)[0] = ((a)[0]*iA + (b)[2]*A)>>8;\
        (a)[1] = ((a)[1]*iA + (b)[1]*A)>>8;\
        (a)[2] = ((a)[2]*iA + (b)[0]*A)>>8;


typedef struct
{
    sgui_canvas canvas;

    Window wnd;

    Pixmap pixmap;
    GC gc;

    long stencil_map[ 256 ];
    unsigned char stencil_base[3];

    Picture pic;
}
sgui_canvas_xlib;



/************************* public canvas functions *************************/
void canvas_xlib_destroy( sgui_canvas* canvas )
{
    sgui_canvas_xlib* cv = (sgui_canvas_xlib*)canvas;

    if( cv->pixmap )
        XFreePixmap( dpy, cv->pixmap );

    if( cv->gc )
        XFreeGC( dpy, cv->gc );

    if( cv->pic )
        XRenderFreePicture( dpy, cv->pic );

    free( cv );
}

void canvas_xlib_resize( sgui_canvas* canvas, unsigned int width,
                         unsigned int height )
{
    sgui_canvas_xlib* cv = (sgui_canvas_xlib*)canvas;
    XRenderPictFormat* fmt;

    /* destroy the pixmap */
    if( cv->pic )
        XRenderFreePicture( dpy, cv->pic );

    XFreeGC( dpy, cv->gc );
    XFreePixmap( dpy, cv->pixmap );

    /* create a new pixmap */
    cv->pixmap = XCreatePixmap( dpy, cv->wnd, width, height, 24 );
    cv->gc = XCreateGC( dpy, cv->pixmap, 0, NULL );

    if( cv->pic )
    {
        fmt = XRenderFindStandardFormat( dpy, PictStandardRGB24 );
        cv->pic = XRenderCreatePicture( dpy, cv->pixmap, fmt, 0, NULL );
    }
}

void canvas_xlib_begin( sgui_canvas* canvas, sgui_rect* r )
{
    (void)canvas;
    (void)r;
}

void canvas_xlib_end( sgui_canvas* canvas )
{
    (void)canvas;
}

void canvas_xlib_clear( sgui_canvas* canvas, sgui_rect* r )
{
    sgui_canvas_xlib* cv = (sgui_canvas_xlib*)canvas;

    XSetForeground( dpy, cv->gc, (canvas->bg_color[0] << 16) |
                                 (canvas->bg_color[1] <<  8) |
                                  canvas->bg_color[2]           );

    XFillRectangle( dpy, cv->pixmap, cv->gc, r->left, r->top,
                    SGUI_RECT_WIDTH_V( r ),
                    SGUI_RECT_HEIGHT_V( r ) );
}

void canvas_xlib_blit( sgui_canvas* canvas, int x, int y, unsigned int width,
                       unsigned int height, unsigned int scanline_length,
                       SGUI_COLOR_FORMAT format, const void* data )
{
    sgui_canvas_xlib* cv = (sgui_canvas_xlib*)canvas;
    unsigned int i, endy, bpp = format==SCF_RGBA8 ? 4 : 3;
    unsigned char *src = (unsigned char*)data, *srow;

    for( endy=y+height; y<(int)endy; ++y, src+=scanline_length*bpp )
    {
        for( srow=src, i=0; i<width; ++i, srow+=bpp )
        {
            XSetForeground( dpy, cv->gc, srow[0]<<16 | srow[1]<<8 | srow[2] );
            XDrawPoint( dpy, cv->pixmap, cv->gc, x+i, y );
        }
    }
}

void canvas_xlib_blend( sgui_canvas* canvas, int x, int y, unsigned int width,
                        unsigned int height, unsigned int scanline_length,
                        const void* data )
{
    sgui_canvas_xlib* cv = (sgui_canvas_xlib*)canvas;
    unsigned int i, endy;
    unsigned char *src = (unsigned char*)data, *srow, A, iA;
    long c;

    for( endy=y+height; y<(int)endy; ++y, src+=scanline_length*4 )
    {
        for( srow=src, i=0; i<width; ++i, srow+=4 )
        {
            A = srow[3];
            iA = 0xFF - A;

            c  = (srow[0] * A + canvas->bg_color[0] * iA)>>8; c <<= 8;
            c |= (srow[1] * A + canvas->bg_color[1] * iA)>>8; c <<= 8;
            c |= (srow[2] * A + canvas->bg_color[2] * iA)>>8;

            XSetForeground( dpy, cv->gc, c );
            XDrawPoint( dpy, cv->pixmap, cv->gc, x+i, y );
        }
    }
}

void canvas_xlib_draw_box( sgui_canvas* canvas, sgui_rect* r,
                           unsigned char* color, SGUI_COLOR_FORMAT format )
{
    sgui_canvas_xlib* cv = (sgui_canvas_xlib*)canvas;
    long c;

    if( format==SCF_RGBA8 )
    {
        unsigned char A = color[3], iA = 0xFF - A;

        c  = (color[0] * A + canvas->bg_color[0] * iA)>>8; c <<= 8;
        c |= (color[1] * A + canvas->bg_color[1] * iA)>>8; c <<= 8;
        c |= (color[2] * A + canvas->bg_color[2] * iA)>>8;
    }
    else
    {
        c = (color[0]<<16) | (color[1]<<8) | color[2];
    }

    XSetForeground( dpy, cv->gc, c );

    XFillRectangle( dpy, cv->pixmap, cv->gc, r->left, r->top,
                    SGUI_RECT_WIDTH_V( r ), SGUI_RECT_HEIGHT_V( r ) );
}

void canvas_xlib_draw_line( sgui_canvas* canvas, int x, int y,
                            unsigned int length, int horizontal,
                            unsigned char* color, SGUI_COLOR_FORMAT format )
{
    sgui_canvas_xlib* cv = (sgui_canvas_xlib*)canvas;
    long c;

    if( format==SCF_RGBA8 )
    {
        unsigned char A = color[3], iA = 0xFF - A;

        c  = (color[0] * A + canvas->bg_color[0] * iA)>>8; c <<= 8;
        c |= (color[1] * A + canvas->bg_color[1] * iA)>>8; c <<= 8;
        c |= (color[2] * A + canvas->bg_color[2] * iA)>>8;
    }
    else
    {
        c = (color[0]<<16) | (color[1]<<8) | color[2];
    }

    XSetForeground( dpy, cv->gc, c );

    if( horizontal )
        XDrawLine( dpy, cv->pixmap, cv->gc, x, y, x+length-1, y );
    else
        XDrawLine( dpy, cv->pixmap, cv->gc, x, y, x, y+length-1 );
}

void canvas_xlib_blend_stencil( sgui_canvas* canvas, unsigned char* buffer,
                                int x, int y, unsigned int w, unsigned int h,
                                unsigned int scan, unsigned char* color )
{
    sgui_canvas_xlib* cv = (sgui_canvas_xlib*)canvas;
    unsigned char iA, *src;
    unsigned int i, j;

    /* rebuild color map if required */
    if( cv->stencil_base[0]!=color[0] || cv->stencil_base[1]!=color[1] ||
        cv->stencil_base[2]!=color[2] )
    {
        for( i=0; i<256; ++i )
        {
            iA = 0xFF-i;

            cv->stencil_map[i]  = (color[0]*i + canvas->bg_color[0]*iA)>>8;
            cv->stencil_map[i] <<= 8;

            cv->stencil_map[i] |= (color[1]*i + canvas->bg_color[1]*iA)>>8;
            cv->stencil_map[i] <<= 8;

            cv->stencil_map[i] |= (color[2]*i + canvas->bg_color[2]*iA)>>8;
        }
    }

    /* perform the stencil blending */
    for( j=0; j<h; ++j, buffer+=scan )
    {
        for( src=buffer, i=0; i<w; ++i, ++src )
        {
            if( *src )
            {
                XSetForeground( dpy, cv->gc, cv->stencil_map[ *src ] );
                XDrawPoint( dpy, cv->pixmap, cv->gc, x+i, y+j );
            }
        }
    }
}

/************************* XRender canvas functions *************************/

void canvas_xrender_clear( sgui_canvas* canvas, sgui_rect* r )
{
    sgui_canvas_xlib* cv = (sgui_canvas_xlib*)canvas;
    XRenderColor c;

    c.red   = canvas->bg_color[0]<<8;
    c.green = canvas->bg_color[1]<<8;
    c.blue  = canvas->bg_color[2]<<8;
    c.alpha = 0xFFFF;

    XRenderFillRectangle( dpy, PictOpSrc, cv->pic, &c, r->left, r->top,
                          SGUI_RECT_WIDTH_V( r ), SGUI_RECT_HEIGHT_V( r ) );
}

void canvas_xrender_draw_box( sgui_canvas* canvas, sgui_rect* r,
                              unsigned char* color, SGUI_COLOR_FORMAT format )
{
    sgui_canvas_xlib* cv = (sgui_canvas_xlib*)canvas;
    XRenderColor c;

    c.red   = color[0]<<8;
    c.green = color[1]<<8;
    c.blue  = color[2]<<8;
    c.alpha = format==SCF_RGBA8 ? (color[3]<<8) : 0xFFFF;

    XRenderFillRectangle( dpy, PictOpOver, cv->pic, &c, r->left, r->top,
                          SGUI_RECT_WIDTH_V( r ), SGUI_RECT_HEIGHT_V( r ) );
}

void canvas_xrender_draw_line( sgui_canvas* canvas, int x, int y,
                               unsigned int length, int horizontal,
                               unsigned char* color, SGUI_COLOR_FORMAT format )
{
    sgui_canvas_xlib* cv = (sgui_canvas_xlib*)canvas;
    XRenderColor c;

    c.red   = color[0]<<8;
    c.green = color[1]<<8;
    c.blue  = color[2]<<8;
    c.alpha = format==SCF_RGBA8 ? (color[3]<<8) : 0xFFFF;

    if( horizontal )
    {
        XRenderFillRectangle( dpy, PictOpOver, cv->pic, &c, x, y, length, 1 );
    }
    else
    {
        XRenderFillRectangle( dpy, PictOpOver, cv->pic, &c, x, y, 1, length );
    }
}

/************************ internal canvas functions ************************/
sgui_canvas* canvas_xlib_create( Window wnd, unsigned int width,
                                 unsigned int height )
{
    sgui_canvas_xlib* cv;
    XRenderPictFormat* fmt;
    int base, error;

    /* allocate xlib canvas */
    cv = malloc( sizeof(sgui_canvas_xlib) );

    if( !cv )
        return NULL;

    /* create a pixmap */
    cv->pixmap = XCreatePixmap( dpy, wnd, width, height, 24 );

    if( !cv->pixmap )
    {
        canvas_xlib_destroy( (sgui_canvas*)cv );
        return NULL;
    }

    /* create a graphics context */
    cv->gc = XCreateGC( dpy, cv->pixmap, 0, NULL );

    if( !cv->gc )
    {
        canvas_xlib_destroy( (sgui_canvas*)cv );
        return NULL;
    }

    /* finish initialisation */
    sgui_internal_canvas_init( (sgui_canvas*)cv, width, height );
    cv->wnd = wnd;

    cv->canvas.destroy = canvas_xlib_destroy;
    cv->canvas.resize  = canvas_xlib_resize;
    cv->canvas.begin   = canvas_xlib_begin;
    cv->canvas.end     = canvas_xlib_end;
    cv->canvas.blit    = canvas_xlib_blit;
    cv->canvas.blend   = canvas_xlib_blend;
    cv->canvas.blend_stencil = canvas_xlib_blend_stencil;

    cv->stencil_base[0] = cv->stencil_base[1] = cv->stencil_base[2] = 0;

    if( XRenderQueryExtension( dpy, &base, &error ) )
    {
        fmt = XRenderFindStandardFormat( dpy, PictStandardRGB24 );
        cv->pic = XRenderCreatePicture( dpy, cv->pixmap, fmt, 0, NULL );

        cv->canvas.clear     = canvas_xrender_clear;
        cv->canvas.draw_box  = canvas_xrender_draw_box;
        cv->canvas.draw_line = canvas_xrender_draw_line;
    }
    else
    {
        cv->pic = 0;

        cv->canvas.clear     = canvas_xlib_clear;
        cv->canvas.draw_box  = canvas_xlib_draw_box;
        cv->canvas.draw_line = canvas_xlib_draw_line;
    }

    return (sgui_canvas*)cv;
}

void canvas_xlib_display( sgui_canvas* cv, int x, int y,
                          unsigned int width, unsigned int height )
{
    sgui_canvas_xlib* canvas = (sgui_canvas_xlib*)cv;

    if( canvas )
    {
        XCopyArea( dpy, canvas->pixmap, canvas->wnd, canvas->gc,
                   x, y, width, height, x, y );
    }    
}

