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
    sgui_canvas super;

    Window wnd;

    Picture pen;
    Pixmap penmap;

    Picture pic;
    Pixmap pixmap;
    GC gc;
}
sgui_canvas_xlib;



static void canvas_xlib_destroy( sgui_canvas* super )
{
    sgui_canvas_xlib* this = (sgui_canvas_xlib*)super;

    if( this->pic ) XRenderFreePicture( dpy, this->pic );
    if( this->pen ) XRenderFreePicture( dpy, this->pen );

    if( this->pixmap ) XFreePixmap( dpy, this->pixmap );
    if( this->penmap ) XFreePixmap( dpy, this->penmap );

    if( this->gc )
        XFreeGC( dpy, this->gc );

    free( this );
}

static void canvas_xlib_resize( sgui_canvas* super, unsigned int width,
                                unsigned int height )
{
    sgui_canvas_xlib* this = (sgui_canvas_xlib*)super;
    XRenderPictFormat* fmt;

    /* destroy the pixmap */
    XRenderFreePicture( dpy, this->pic );
    XFreePixmap( dpy, this->pixmap );

    /* create a new pixmap */
    this->pixmap = XCreatePixmap( dpy, this->wnd, width, height, 24 );

    fmt = XRenderFindStandardFormat( dpy, PictStandardRGB24 );
    this->pic = XRenderCreatePicture( dpy, this->pixmap, fmt, 0, NULL );
}

static void canvas_xlib_clear( sgui_canvas* super, sgui_rect* r )
{
    sgui_canvas_xlib* this = (sgui_canvas_xlib*)super;
    XRenderColor c;

    c.red   = super->bg_color[0]<<8;
    c.green = super->bg_color[1]<<8;
    c.blue  = super->bg_color[2]<<8;
    c.alpha = 0xFFFF;

    XRenderFillRectangle( dpy, PictOpSrc, this->pic, &c, r->left, r->top,
                          SGUI_RECT_WIDTH_V( r ), SGUI_RECT_HEIGHT_V( r ) );
}

static void canvas_xlib_blit( sgui_canvas* super, int x, int y,
                              sgui_pixmap* pixmap, sgui_rect* srcrect )
{
    sgui_canvas_xlib* this = (sgui_canvas_xlib*)super;

    XRenderComposite( dpy, PictOpSrc, ((xlib_pixmap*)pixmap)->pic, 0,
                      this->pic, srcrect->left, srcrect->top, 0, 0, x, y,
                      SGUI_RECT_WIDTH_V(srcrect),
                      SGUI_RECT_HEIGHT_V(srcrect) );
}

static void canvas_xlib_blend( sgui_canvas* super, int x, int y,
                               sgui_pixmap* pixmap, sgui_rect* srcrect )
{
    sgui_canvas_xlib* this = (sgui_canvas_xlib*)super;

    XRenderComposite( dpy, PictOpOver, ((xlib_pixmap*)pixmap)->pic, 0,
                      this->pic, srcrect->left, srcrect->top, 0, 0, x, y,
                      SGUI_RECT_WIDTH_V(srcrect),
                      SGUI_RECT_HEIGHT_V(srcrect) );
}

static void canvas_xlib_blend_glyph( sgui_canvas* super, int x, int y,
                                     sgui_pixmap* pixmap, sgui_rect* r,
                                     unsigned char* color )
{
    sgui_canvas_xlib* this = (sgui_canvas_xlib*)super;
    XRenderColor c;

    c.red   = color[0]<<8;
    c.green = color[1]<<8;
    c.blue  = color[2]<<8;
    c.alpha = 0xFFFF;

    XRenderFillRectangle( dpy, PictOpSrc, this->pen, &c, 0, 0, 1, 1 );

    XRenderComposite( dpy, PictOpOver, this->pen, ((xlib_pixmap*)pixmap)->pic,
                      this->pic, 0, 0, r->left, r->top, x, y,
                      SGUI_RECT_WIDTH_V(r), SGUI_RECT_HEIGHT_V(r) );
}

static int canvas_xlib_draw_string( sgui_canvas* super, int x, int y,
                                    sgui_font* font, unsigned char* color,
                                    const char* text, unsigned int length )
{
    int oldx = x;
    unsigned int i, len = 0;
    unsigned long character, previous=0;
    sgui_canvas_xlib* this = (sgui_canvas_xlib*)super;
    sgui_font_cache* cache = get_glyph_cache( );
    XRectangle r;

    r.x = super->sc.left;
    r.y = super->sc.top;
    r.width = SGUI_RECT_WIDTH( super->sc );
    r.height = SGUI_RECT_HEIGHT( super->sc );
    XRenderSetPictureClipRectangles( dpy, this->pic, 0, 0, &r, 1 );

    /* for each character */
    for( i=0; i<length && (*text) && (*text!='\n'); text+=len, i+=len )
    {
        /* load the next glyph */
        character = sgui_utf8_decode( text, &len );

        /* apply kerning */
        x += sgui_font_get_kerning_distance( font, previous, character );

        /* blend onto destination buffer */
        x += sgui_font_cache_draw_glyph( cache, font, character,
                                         x, y, super, color ) + 1;

        /* store previous glyph index for kerning */
        previous = character;
    }

    r.x = 0;
    r.y = 0;
    r.width = super->width;
    r.height = super->height;
    XRenderSetPictureClipRectangles( dpy, this->pic, 0, 0, &r, 1 );

    return x - oldx;
}

sgui_pixmap* canvas_xlib_create_pixmap( sgui_canvas* super,
                                        unsigned int width,
                                        unsigned int height, int format )
{
    sgui_canvas_xlib* this = (sgui_canvas_xlib*)super;

    return xlib_pixmap_create( width, height, format, this->wnd );
}

/************************ internal canvas functions ************************/
sgui_canvas* canvas_xlib_create( Window wnd, unsigned int width,
                                 unsigned int height )
{
    sgui_canvas_xlib* this;
    sgui_canvas* super;
    XRenderPictFormat* fmt;
    XRenderPictureAttributes attr;
    int base, error;

    /* make sure that the XRender extension is present */
    if( !XRenderQueryExtension( dpy, &base, &error ) )
    {
        return NULL;
    }

    /* allocate xlib canvas */
    this = malloc( sizeof(sgui_canvas_xlib) );
    super = (sgui_canvas*)this;

    if( !this )
        return NULL;

    /* create pixmaps */
    this->pixmap = XCreatePixmap( dpy, wnd, width, height, 24 );

    if( !this->pixmap )
    {
        canvas_xlib_destroy( super );
        return NULL;
    }

    this->penmap = XCreatePixmap( dpy, wnd, 1, 1, 24 );

    if( !this->penmap )
    {
        canvas_xlib_destroy( super );
        return NULL;
    }

    /* crate Xrender pictures */
    fmt = XRenderFindStandardFormat( dpy, PictStandardRGB24 );
    this->pic = XRenderCreatePicture( dpy, this->pixmap, fmt, 0, NULL );

    if( !this->pic )
    {
        canvas_xlib_destroy( super );
        return NULL;        
    }

    attr.repeat = RepeatNormal;
    this->pen = XRenderCreatePicture(dpy, this->penmap, fmt, CPRepeat, &attr);

    if( !this->pen )
    {
        canvas_xlib_destroy( super );
        return NULL;        
    }

    /* create a graphics context */
    this->gc = XCreateGC( dpy, this->pixmap, 0, NULL );

    if( !this->gc )
    {
        canvas_xlib_destroy( super );
        return NULL;
    }

    /* finish initialisation */
    sgui_internal_canvas_init( super, width, height );
    this->wnd = wnd;

    super->destroy       = canvas_xlib_destroy;
    super->resize        = canvas_xlib_resize;
    super->blit          = canvas_xlib_blit;
    super->blend         = canvas_xlib_blend;
    super->blend_glyph   = canvas_xlib_blend_glyph;
    super->clear         = canvas_xlib_clear;
    super->draw_string   = canvas_xlib_draw_string;
    super->create_pixmap = canvas_xlib_create_pixmap;
    super->stretch_blend = NULL;
    super->stretch_blit  = NULL;
    super->skin_pixmap   = get_skin_pixmap( );

    return (sgui_canvas*)this;
}

void canvas_xlib_display( sgui_canvas* super, int x, int y,
                          unsigned int width, unsigned int height )
{
    sgui_canvas_xlib* this = (sgui_canvas_xlib*)super;

    if( this )
    {
        XCopyArea( dpy, this->pixmap, this->wnd, this->gc,
                   x, y, width, height, x, y );
    }    
}

