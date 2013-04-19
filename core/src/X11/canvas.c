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
#include "sgui_utf8.h"



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



static void canvas_xlib_destroy( sgui_canvas* canvas )
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

static void canvas_xlib_resize( sgui_canvas* canvas, unsigned int width,
                                unsigned int height )
{
    sgui_canvas_xlib* cv = (sgui_canvas_xlib*)canvas;
    XRenderPictFormat* fmt;

    /* destroy the pixmap */
    XRenderFreePicture( dpy, cv->pic );
    XFreePixmap( dpy, cv->pixmap );

    /* create a new pixmap */
    cv->pixmap = XCreatePixmap( dpy, cv->wnd, width, height, 24 );

    fmt = XRenderFindStandardFormat( dpy, PictStandardRGB24 );
    cv->pic = XRenderCreatePicture( dpy, cv->pixmap, fmt, 0, NULL );
}

static void canvas_xlib_clear( sgui_canvas* canvas, sgui_rect* r )
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

static void canvas_xlib_blit( sgui_canvas* canvas, int x, int y,
                              sgui_pixmap* pixmap, sgui_rect* srcrect )
{
    sgui_canvas_xlib* cv = (sgui_canvas_xlib*)canvas;
    Picture pic = pixmap_get_picture( pixmap );

    XRenderComposite( dpy, PictOpSrc, pic, 0, cv->pic,
                      srcrect->left, srcrect->top, 0, 0, x, y,
                      SGUI_RECT_WIDTH_V(srcrect),
                      SGUI_RECT_HEIGHT_V(srcrect) );
}

static void canvas_xlib_blend( sgui_canvas* canvas, int x, int y,
                               sgui_pixmap* pixmap, sgui_rect* srcrect )
{
    sgui_canvas_xlib* cv = (sgui_canvas_xlib*)canvas;
    Picture pic = pixmap_get_picture( pixmap );

    XRenderComposite( dpy, PictOpOver, pic, 0, cv->pic,
                      srcrect->left, srcrect->top, 0, 0, x, y,
                      SGUI_RECT_WIDTH_V(srcrect),
                      SGUI_RECT_HEIGHT_V(srcrect) );
}

static void canvas_xlib_draw_box( sgui_canvas* canvas, sgui_rect* r,
                                  unsigned char* color, int format )
{
    sgui_canvas_xlib* cv = (sgui_canvas_xlib*)canvas;
    XRenderColor c;

    c.red   = color[0]<<8;
    c.green = color[1]<<8;
    c.blue  = color[2]<<8;
    c.alpha = format==SGUI_RGBA8 ? (color[3]<<8) : 0xFFFF;

    XRenderFillRectangle( dpy, PictOpOver, cv->pic, &c, r->left, r->top,
                          SGUI_RECT_WIDTH_V( r ), SGUI_RECT_HEIGHT_V( r ) );
}

static void canvas_xlib_blend_stencil( sgui_canvas* canvas,
                                       unsigned char* buffer,
                                       int x, int y,
                                       unsigned int w, unsigned int h,
                                       unsigned int scan,
                                       unsigned char* color )
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

static int canvas_xlib_draw_string( sgui_canvas* canvas, int x, int y,
                                    sgui_font* font, unsigned char* color,
                                    const char* text, unsigned int length )
{
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
        sgui_font_load_glyph( font, character );

        /* apply kerning */
        x += sgui_font_get_kerning_distance( font, previous, character );

        /* blend onto destination buffer */
        sgui_font_get_glyph_metrics( font, &w, &h, &bearing );
        buffer = sgui_font_get_glyph( font );

        sgui_rect_set_size( &r, x, y + bearing, w, h );

        if( buffer && sgui_rect_get_intersection( &r, &canvas->sc, &r ) )
        {
            buffer += (r.top - (y + bearing)) * w + (r.left - x);

            canvas_xlib_blend_stencil( canvas, buffer, r.left, r.top,
                                       SGUI_RECT_WIDTH( r ),
                                       SGUI_RECT_HEIGHT( r ),
                                       w, color );
        }

        /* advance cursor */
        x += w + 1;

        /* store previous glyph index for kerning */
        previous = character;
    }

    return x - oldx;
}

/************************ internal canvas functions ************************/
sgui_canvas* canvas_xlib_create( Window wnd, unsigned int width,
                                 unsigned int height )
{
    sgui_canvas_xlib* cv;
    XRenderPictFormat* fmt;
    int base, error;

    /* make sure that the XRender extension is present */
    if( !XRenderQueryExtension( dpy, &base, &error ) )
    {
        return NULL;
    }

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

    /* crate an Xrender picture */
    fmt = XRenderFindStandardFormat( dpy, PictStandardRGB24 );
    cv->pic = XRenderCreatePicture( dpy, cv->pixmap, fmt, 0, NULL );

    if( !cv->pic )
    {
        canvas_xlib_destroy( (sgui_canvas*)cv );
        return NULL;        
    }

    /* finish initialisation */
    sgui_internal_canvas_init( (sgui_canvas*)cv, width, height );
    cv->wnd = wnd;

    cv->canvas.destroy       = canvas_xlib_destroy;
    cv->canvas.resize        = canvas_xlib_resize;
    cv->canvas.blit          = canvas_xlib_blit;
    cv->canvas.blend         = canvas_xlib_blend;
    cv->canvas.clear         = canvas_xlib_clear;
    cv->canvas.draw_box      = canvas_xlib_draw_box;
    cv->canvas.draw_string   = canvas_xlib_draw_string;

    cv->stencil_base[0] = cv->stencil_base[1] = cv->stencil_base[2] = 0;

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

