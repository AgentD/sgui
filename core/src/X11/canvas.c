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

    Picture pen;
    Pixmap penmap;

    Picture pic;
    Pixmap pixmap;
    GC gc;
}
sgui_canvas_xlib;



static void canvas_xlib_destroy( sgui_canvas* canvas )
{
    sgui_canvas_xlib* cv = (sgui_canvas_xlib*)canvas;

    if( cv->pic ) XRenderFreePicture( dpy, cv->pic );
    if( cv->pen ) XRenderFreePicture( dpy, cv->pen );

    if( cv->pixmap ) XFreePixmap( dpy, cv->pixmap );
    if( cv->penmap ) XFreePixmap( dpy, cv->penmap );

    if( cv->gc )
        XFreeGC( dpy, cv->gc );

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

    XRenderComposite( dpy, PictOpSrc, ((xlib_pixmap*)pixmap)->pic, 0, cv->pic,
                      srcrect->left, srcrect->top, 0, 0,
                      x, y,
                      SGUI_RECT_WIDTH_V(srcrect),
                      SGUI_RECT_HEIGHT_V(srcrect) );
}

static void canvas_xlib_blend( sgui_canvas* canvas, int x, int y,
                               sgui_pixmap* pixmap, sgui_rect* srcrect )
{
    sgui_canvas_xlib* cv = (sgui_canvas_xlib*)canvas;

    XRenderComposite( dpy, PictOpOver, ((xlib_pixmap*)pixmap)->pic, 0,
                      cv->pic, srcrect->left, srcrect->top, 0, 0,
                      x, y,
                      SGUI_RECT_WIDTH_V(srcrect),
                      SGUI_RECT_HEIGHT_V(srcrect) );
}

static void canvas_xlib_blend_glyph( sgui_canvas* canvas, int x, int y,
                                     sgui_pixmap* pixmap, sgui_rect* r,
                                     unsigned char* color )
{
    sgui_canvas_xlib* cv = (sgui_canvas_xlib*)canvas;
    XRenderColor c;

    c.red   = color[0]<<8;
    c.green = color[1]<<8;
    c.blue  = color[2]<<8;
    c.alpha = 0xFFFF;

    XRenderFillRectangle( dpy, PictOpSrc, cv->pen, &c, 0, 0, 1, 1 );

    XRenderComposite( dpy, PictOpOver, cv->pen, ((xlib_pixmap*)pixmap)->pic,
                      cv->pic, 0, 0, r->left, r->top, x, y,
                      SGUI_RECT_WIDTH_V(r), SGUI_RECT_HEIGHT_V(r) );
}

static void canvas_xlib_draw_box( sgui_canvas* canvas, sgui_rect* r,
                                  unsigned char* color, int format )
{
    sgui_canvas_xlib* cv = (sgui_canvas_xlib*)canvas;
    XRenderColor c;

    if( format==SGUI_RGB8 || format==SGUI_RGBA8 )
    {
        c.red   = color[0]<<8;
        c.green = color[1]<<8;
        c.blue  = color[2]<<8;
        c.alpha = format==SGUI_RGBA8 ? (color[3]<<8) : 0xFFFF;
    }
    else
    {
        c.red = c.green = c.blue = color[0]<<8;
        c.alpha = 0xFFFF;
    }

    XRenderFillRectangle( dpy, PictOpOver, cv->pic, &c, r->left, r->top,
                          SGUI_RECT_WIDTH_V( r ), SGUI_RECT_HEIGHT_V( r ) );
}

static int canvas_xlib_draw_string( sgui_canvas* canvas, int x, int y,
                                    sgui_font* font, unsigned char* color,
                                    const char* text, unsigned int length )
{
    int oldx = x;
    unsigned int i, len = 0;
    unsigned long character, previous=0;
    sgui_canvas_xlib* cv = (sgui_canvas_xlib*)canvas;
    XRectangle r;

    r.x = canvas->sc.left;
    r.y = canvas->sc.top;
    r.width = SGUI_RECT_WIDTH( canvas->sc );
    r.height = SGUI_RECT_HEIGHT( canvas->sc );
    XRenderSetPictureClipRectangles( dpy, cv->pic, 0, 0, &r, 1 );

    /* for each character */
    for( i=0; i<length && (*text) && (*text!='\n'); text+=len, i+=len )
    {
        /* load the next glyph */
        character = sgui_utf8_decode( text, &len );

        /* apply kerning */
        x += sgui_font_get_kerning_distance( font, previous, character );

        /* blend onto destination buffer */
        x += sgui_font_cache_draw_glyph( glyph_cache, font, character,
                                         x, y, canvas, color ) + 1;

        /* store previous glyph index for kerning */
        previous = character;
    }

    r.x = 0;
    r.y = 0;
    r.width = canvas->width;
    r.height = canvas->height;
    XRenderSetPictureClipRectangles( dpy, cv->pic, 0, 0, &r, 1 );

    return x - oldx;
}

sgui_pixmap* canvas_xlib_create_pixmap( sgui_canvas* canvas,
                                        unsigned int width,
                                        unsigned int height, int format )
{
    sgui_canvas_xlib* cv = (sgui_canvas_xlib*)canvas;

    return xlib_pixmap_create( width, height, format, cv->wnd );
}

/************************ internal canvas functions ************************/
sgui_canvas* canvas_xlib_create( Window wnd, unsigned int width,
                                 unsigned int height )
{
    sgui_canvas_xlib* cv;
    XRenderPictFormat* fmt;
    XRenderPictureAttributes attr;
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

    /* create pixmaps */
    cv->pixmap = XCreatePixmap( dpy, wnd, width, height, 24 );

    if( !cv->pixmap )
    {
        canvas_xlib_destroy( (sgui_canvas*)cv );
        return NULL;
    }

    cv->penmap = XCreatePixmap( dpy, wnd, 1, 1, 24 );

    if( !cv->penmap )
    {
        canvas_xlib_destroy( (sgui_canvas*)cv );
        return NULL;
    }

    /* crate Xrender pictures */
    fmt = XRenderFindStandardFormat( dpy, PictStandardRGB24 );
    cv->pic = XRenderCreatePicture( dpy, cv->pixmap, fmt, 0, NULL );

    if( !cv->pic )
    {
        canvas_xlib_destroy( (sgui_canvas*)cv );
        return NULL;        
    }

    attr.repeat = RepeatNormal;
    cv->pen = XRenderCreatePicture( dpy, cv->penmap, fmt, CPRepeat, &attr );

    if( !cv->pen )
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

    cv->canvas.destroy       = canvas_xlib_destroy;
    cv->canvas.resize        = canvas_xlib_resize;
    cv->canvas.blit          = canvas_xlib_blit;
    cv->canvas.blend         = canvas_xlib_blend;
    cv->canvas.blend_glyph   = canvas_xlib_blend_glyph;
    cv->canvas.clear         = canvas_xlib_clear;
    cv->canvas.draw_box      = canvas_xlib_draw_box;
    cv->canvas.draw_string   = canvas_xlib_draw_string;
    cv->canvas.create_pixmap = canvas_xlib_create_pixmap;
    cv->canvas.skin_pixmap   = skin_pixmap;

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

