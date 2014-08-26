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
#include "platform.h"

#include "sgui_utf8.h"



static int canvas_x11_draw_string( sgui_canvas* super, int x, int y,
                                   sgui_font* font,
                                   const unsigned char* color,
                                   const char* text, unsigned int length )
{
    sgui_canvas_x11* this = (sgui_canvas_x11*)super;
    unsigned int i, len, character, previous=0;
    sgui_pixmap* font_map;
    int oldx = x;

    sgui_internal_lock_mutex( );

    if( !this->cache )
    {
        font_map = super->create_pixmap( super,
                                         FONT_MAP_WIDTH, FONT_MAP_HEIGHT,
                                         SGUI_A8 );

        if( !font_map )
        {
            sgui_internal_unlock_mutex( );
            return super->width;
        }

        if( !(this->cache = sgui_font_cache_create( font_map )) )
        {
            sgui_pixmap_destroy( font_map );
            sgui_internal_unlock_mutex( );
            return super->width;
        }
    }

    this->set_clip_rect( this, super->sc.left, super->sc.top,
                         SGUI_RECT_WIDTH(super->sc),
                         SGUI_RECT_HEIGHT(super->sc) );

    /* for each character */
    for( i=0; i<length && (*text) && (*text!='\n'); text+=len, i+=len )
    {
        /* load the next glyph */
        character = sgui_utf8_decode( text, &len );

        /* apply kerning */
        x += sgui_font_get_kerning_distance( font, previous, character );

        /* blend onto destination buffer */
        x += sgui_font_cache_draw_glyph( this->cache, font, character,
                                         x, y, super, color ) + 1;

        /* store previous glyph index for kerning */
        previous = character;
    }

    this->set_clip_rect( this, 0, 0, super->width, super->height );
    sgui_internal_unlock_mutex( );
    return x - oldx;
}

/************************ xlib based implementation ************************/
static void canvas_xlib_destroy( sgui_canvas* super )
{
    sgui_canvas_xlib* this = (sgui_canvas_xlib*)super;

    sgui_internal_lock_mutex( );
    if( ((sgui_canvas_x11*)this)->cache )
        sgui_icon_cache_destroy( ((sgui_canvas_x11*)this)->cache );

    XFreeGC( dpy, this->gc );
    XFreePixmap( dpy, this->pixmap );
    sgui_internal_unlock_mutex( );

    free( this );
}

static void canvas_xlib_set_clip_rect( sgui_canvas_x11* super,
                                       int left, int top,
                                       int width, int height )
{
    sgui_canvas_xlib* this = (sgui_canvas_xlib*)super;
    XRectangle r;
    r.x      = left;
    r.y      = top;
    r.width  = width;
    r.height = height;
    sgui_internal_lock_mutex( );
    XSetClipRectangles( dpy, this->gc, 0, 0, &r, 1, Unsorted );
    sgui_internal_unlock_mutex( );
}

static void canvas_xlib_resize( sgui_canvas* super, unsigned int width,
                                unsigned int height )
{
    sgui_canvas_xlib* this = (sgui_canvas_xlib*)super;
    Pixmap newpm;
    GC newgc;

    sgui_internal_lock_mutex( );
    newpm = XCreatePixmap( dpy, ((sgui_canvas_x11*)this)->wnd,
                           width, height, 24 );

    if( !newpm )
    {
        sgui_internal_unlock_mutex( );
        return;
    }

    newgc = XCreateGC( dpy, this->pixmap, 0, NULL );

    if( !newgc )
    {
        XFreePixmap( dpy, newpm );
        sgui_internal_unlock_mutex( );
        return;
    }

    XFreeGC( dpy, this->gc );
    XFreePixmap( dpy, this->pixmap );
    this->pixmap = newpm;
    this->gc = newgc;
    sgui_internal_unlock_mutex( );
}

static void canvas_xlib_clear( sgui_canvas* super, sgui_rect* r )
{
    sgui_canvas_xlib* this = (sgui_canvas_xlib*)super;
    unsigned long color;

    color = (this->bg[0]<<16) | (this->bg[1]<<8) | this->bg[2];

    sgui_internal_lock_mutex( );
    XSetForeground( dpy, this->gc, color );
    XFillRectangle( dpy, this->pixmap, this->gc, r->left, r->top,
                    SGUI_RECT_WIDTH_V(r), SGUI_RECT_HEIGHT_V(r) );
    sgui_internal_unlock_mutex( );
}

static void canvas_xlib_draw_box( sgui_canvas* super, sgui_rect* r,
                                  const unsigned char* color, int format )
{
    sgui_canvas_xlib* this = (sgui_canvas_xlib*)super;
    unsigned long R, G, B, A, iA;

    if( format==SGUI_RGB8 )
    {
        R = color[0];
        G = color[1];
        B = color[2];
    }
    else if( format==SGUI_RGBA8 )
    {
        A = color[3];
        iA = 0xFF - A;

        R = ((color[0]*A + this->bg[0]*iA) >> 8) & 0x00FF;
        G = ((color[1]*A + this->bg[1]*iA) >> 8) & 0x00FF;
        B = ((color[2]*A + this->bg[2]*iA) >> 8) & 0x00FF;
    }
    else
    {
        R = G = B = color[0];
    }

    sgui_internal_lock_mutex( );
    XSetForeground( dpy, this->gc, (R<<16) | (G<<8) | B );
    XFillRectangle( dpy, this->pixmap, this->gc, r->left, r->top,
                    SGUI_RECT_WIDTH_V(r), SGUI_RECT_HEIGHT_V(r) );
    sgui_internal_unlock_mutex( );
}

static void canvas_xlib_blit( sgui_canvas* super, int x, int y,
                              sgui_pixmap* pixmap, sgui_rect* srcrect )
{
    sgui_canvas_xlib* this = (sgui_canvas_xlib*)super;
    xlib_pixmap* pix = (xlib_pixmap*)pixmap;

    if( pix->is_stencil )
        return;

    sgui_internal_lock_mutex( );
    XCopyArea( dpy, pix->data.xpm, this->pixmap, this->gc,
               srcrect->left, srcrect->top,
               SGUI_RECT_WIDTH_V(srcrect), SGUI_RECT_HEIGHT_V(srcrect), x, y );
    sgui_internal_unlock_mutex( );
}

static void canvas_xlib_blend_glyph( sgui_canvas* super, int x, int y,
                                     sgui_pixmap* pixmap, sgui_rect* r,
                                     const unsigned char* color )

{
    sgui_canvas_xlib* this = (sgui_canvas_xlib*)super;
    xlib_pixmap* pix = (xlib_pixmap*)pixmap;
    unsigned char *src, *src_row;
    int X, Y, C;

    src = pix->data.pixels + (r->top*pixmap->width + r->left);
    C = (color[0]<<16) | (color[1]<<8) | color[2];

    sgui_internal_lock_mutex( );
    for( Y=r->top; Y<=r->bottom; ++Y, src+=pixmap->width )
    {
        for( src_row=src, X=r->left; X<=r->right; ++X, ++src_row )
        {
            if( (*src_row)>=0x80 )
            {
                XSetForeground( dpy, this->gc, C );
                XDrawPoint( dpy, this->pixmap, this->gc,
                            x+X-r->left, y+Y-r->top );
            }
        }
    }
    sgui_internal_unlock_mutex( );
}

static void canvas_xlib_display( sgui_canvas_x11* super, int x, int y,
                                 unsigned int width, unsigned int height )
{
    sgui_canvas_xlib* this = (sgui_canvas_xlib*)super;

    sgui_internal_lock_mutex( );
    XCopyArea( dpy, this->pixmap, super->wnd, this->gc,
               x, y, width, height, x, y );
    sgui_internal_unlock_mutex( );
}

/*********************** xrender based implementation ***********************/
static void canvas_xrender_destroy( sgui_canvas* super )
{
    sgui_canvas_xrender* this = (sgui_canvas_xrender*)super;

    sgui_internal_lock_mutex( );

    if( ((sgui_canvas_x11*)this)->cache )
        sgui_icon_cache_destroy( ((sgui_canvas_x11*)this)->cache );

    if( this->pic ) XRenderFreePicture( dpy, this->pic );
    if( this->pen ) XRenderFreePicture( dpy, this->pen );
    if( this->wndpic ) XRenderFreePicture( dpy, this->wndpic );

    if( this->pixmap ) XFreePixmap( dpy, this->pixmap );
    if( this->penmap ) XFreePixmap( dpy, this->penmap );

    sgui_internal_unlock_mutex( );

    free( this );
}

static void canvas_xrender_set_clip_rect( sgui_canvas_x11* super,
                                          int left, int top,
                                          int width, int height )
{
    sgui_canvas_xrender* this = (sgui_canvas_xrender*)super;
    XRectangle r;
    r.x      = left;
    r.y      = top;
    r.width  = width;
    r.height = height;
    sgui_internal_lock_mutex( );
    XRenderSetPictureClipRectangles( dpy, this->pic, 0, 0, &r, 1 );
    sgui_internal_unlock_mutex( );
}

static void canvas_xrender_resize( sgui_canvas* super, unsigned int width,
                                   unsigned int height )
{
    sgui_canvas_xrender* this = (sgui_canvas_xrender*)super;
    XRenderPictFormat* fmt;

    sgui_internal_lock_mutex( );

    /* destroy the pixmap */
    XRenderFreePicture( dpy, this->pic );
    XFreePixmap( dpy, this->pixmap );

    /* create a new pixmap */
    this->pixmap = XCreatePixmap( dpy, ((sgui_canvas_x11*)this)->wnd,
                                  width, height, 32 );

    fmt = XRenderFindStandardFormat( dpy, PictStandardARGB32 );
    this->pic = XRenderCreatePicture( dpy, this->pixmap, fmt, 0, NULL );

    sgui_internal_unlock_mutex( );
}

static void canvas_xrender_clear( sgui_canvas* super, sgui_rect* r )
{
    sgui_canvas_xrender* this = (sgui_canvas_xrender*)super;
    XRenderColor c;

    c.red = c.green = c.blue = c.alpha = 0;

    sgui_internal_lock_mutex( );
    XRenderFillRectangle( dpy, PictOpSrc, this->pic, &c, r->left, r->top,
                          SGUI_RECT_WIDTH_V( r ), SGUI_RECT_HEIGHT_V( r ) );

    sgui_internal_unlock_mutex( );
}

static void canvas_xrender_draw_box( sgui_canvas* super, sgui_rect* r,
                                     const unsigned char* color, int format )
{
    sgui_canvas_xrender* this = (sgui_canvas_xrender*)super;
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

    sgui_internal_lock_mutex( );
    XRenderFillRectangle( dpy, PictOpOver, this->pic, &c, r->left, r->top,
                          SGUI_RECT_WIDTH_V( r ), SGUI_RECT_HEIGHT_V( r ) );
    sgui_internal_unlock_mutex( );
}

static void canvas_xrender_blit( sgui_canvas* super, int x, int y,
                                 sgui_pixmap* pixmap, sgui_rect* srcrect )
{
    sgui_canvas_xrender* this = (sgui_canvas_xrender*)super;
    unsigned int w = SGUI_RECT_WIDTH_V(srcrect);
    unsigned int h = SGUI_RECT_HEIGHT_V(srcrect);
    XRenderColor c;

    c.red = c.green = c.blue = 0;
    c.alpha = 0xFFFF;

    sgui_internal_lock_mutex( );
    XRenderFillRectangle( dpy, PictOpSrc, this->pic, &c, x, y, w, h );
    XRenderComposite( dpy, PictOpOver, ((xrender_pixmap*)pixmap)->pic, 0,
                      this->pic, srcrect->left, srcrect->top, 0, 0,
                      x, y, w, h );
    sgui_internal_unlock_mutex( );
}

static void canvas_xrender_blend( sgui_canvas* super, int x, int y,
                                  sgui_pixmap* pixmap, sgui_rect* srcrect )
{
    sgui_canvas_xrender* this = (sgui_canvas_xrender*)super;

    sgui_internal_lock_mutex( );
    XRenderComposite( dpy, PictOpOver, ((xrender_pixmap*)pixmap)->pic, 0,
                      this->pic, srcrect->left, srcrect->top, 0, 0, x, y,
                      SGUI_RECT_WIDTH_V(srcrect),
                      SGUI_RECT_HEIGHT_V(srcrect) );
    sgui_internal_unlock_mutex( );
}

static void canvas_xrender_blend_glyph( sgui_canvas* super, int x, int y,
                                        sgui_pixmap* pixmap, sgui_rect* r,
                                        const unsigned char* color )
{
    sgui_canvas_xrender* this = (sgui_canvas_xrender*)super;
    XRenderColor c;

    c.red   = color[0]<<8;
    c.green = color[1]<<8;
    c.blue  = color[2]<<8;
    c.alpha = 0xFFFF;

    sgui_internal_lock_mutex( );
    XRenderFillRectangle( dpy, PictOpSrc, this->pen, &c, 0, 0, 1, 1 );

    XRenderComposite( dpy, PictOpOver, this->pen,
                      ((xrender_pixmap*)pixmap)->pic,
                      this->pic, 0, 0, r->left, r->top, x, y,
                      SGUI_RECT_WIDTH_V(r), SGUI_RECT_HEIGHT_V(r) );
    sgui_internal_unlock_mutex( );
}

static void canvas_xrender_display( sgui_canvas_x11* super, int x, int y,
                                    unsigned int width, unsigned int height )
{
    sgui_canvas_xrender* this = (sgui_canvas_xrender*)super;

    sgui_internal_lock_mutex( );
    XClearArea( dpy, super->wnd, x, y, width, height, False );
    XRenderComposite( dpy, PictOpOver, this->pic, 0, this->wndpic, x, y,
                      0, 0, x, y, width, height );
    sgui_internal_unlock_mutex( );
}

/************************ internal canvas functions ************************/
sgui_canvas* canvas_xrender_create( Window wnd, unsigned int width,
                                    unsigned int height )
{
    sgui_canvas_xrender* this;
    sgui_canvas* super;
    XRenderPictFormat* fmt;
    XRenderPictureAttributes attr;
    int base, error;

    sgui_internal_lock_mutex( );

    /* make sure that the XRender extension is present */
    if( !XRenderQueryExtension( dpy, &base, &error ) )
    {
        sgui_internal_unlock_mutex( );
        return NULL;
    }

    /* allocate xlib canvas */
    this = malloc( sizeof(sgui_canvas_xrender) );
    super = (sgui_canvas*)this;

    if( !this )
    {
        sgui_internal_unlock_mutex( );
        return NULL;
    }

    /* create pixmaps */
    if( !(this->pixmap = XCreatePixmap( dpy, wnd, width, height, 32 )) )
        goto failure;

    if( !(this->penmap = XCreatePixmap( dpy, wnd, 1, 1, 24 )) )
        goto failure;

    /* crate Xrender pictures */
    fmt = XRenderFindStandardFormat( dpy, PictStandardARGB32 );
    this->pic = XRenderCreatePicture( dpy, this->pixmap, fmt, 0, NULL );

    if( !this->pic )
        goto failure;

    fmt = XRenderFindStandardFormat( dpy, PictStandardRGB24 );
    this->wndpic = XRenderCreatePicture( dpy, wnd, fmt, 0, NULL );

    if( !this->wndpic )
        goto failure;

    attr.repeat = RepeatNormal;
    this->pen = XRenderCreatePicture(dpy, this->penmap, fmt, CPRepeat, &attr);

    if( !this->pen )
        goto failure;

    /* finish initialisation */
    sgui_canvas_init( super, width, height );
    sgui_internal_unlock_mutex( );

    super->destroy       = canvas_xrender_destroy;
    super->resize        = canvas_xrender_resize;
    super->blit          = canvas_xrender_blit;
    super->blend         = canvas_xrender_blend;
    super->blend_glyph   = canvas_xrender_blend_glyph;
    super->clear         = canvas_xrender_clear;
    super->draw_string   = canvas_x11_draw_string;
    super->create_pixmap = xrender_pixmap_create;
    super->draw_box      = canvas_xrender_draw_box;

    ((sgui_canvas_x11*)this)->cache         = NULL;
    ((sgui_canvas_x11*)this)->wnd           = wnd;
    ((sgui_canvas_x11*)this)->display       = canvas_xrender_display;
    ((sgui_canvas_x11*)this)->set_clip_rect = canvas_xrender_set_clip_rect;

    return (sgui_canvas*)this;
failure:
    sgui_internal_unlock_mutex( );
    canvas_xrender_destroy( super );
    return NULL;
}

sgui_canvas* canvas_xlib_create( Window wnd, unsigned int width,
                                 unsigned int height )
{
    sgui_canvas_xlib* this;
    sgui_canvas* super;


    /* allocate xlib canvas */
    this = malloc( sizeof(sgui_canvas_xlib) );
    super = (sgui_canvas*)this;

    if( !this )
        return NULL;

    sgui_internal_lock_mutex( );

    /* create pixmap */
    if( !(this->pixmap = XCreatePixmap( dpy, wnd, width, height, 24 )) )
    {
        sgui_internal_unlock_mutex( );
        free( this );
        return NULL;
    }

    if( !(this->gc = XCreateGC( dpy, this->pixmap, 0, NULL )) )
    {
        XFreePixmap( dpy, this->pixmap );
        sgui_internal_unlock_mutex( );
        free( this );
        return NULL;
    }

    /* finish initialisation */
    sgui_canvas_init( super, width, height );
    memcpy( this->bg, sgui_skin_get( )->window_color, 4 );

    sgui_internal_unlock_mutex( );

    super->destroy       = canvas_xlib_destroy;
    super->resize        = canvas_xlib_resize;
    super->blit          = canvas_xlib_blit;
    super->blend         = canvas_xlib_blit;
    super->blend_glyph   = canvas_xlib_blend_glyph;
    super->clear         = canvas_xlib_clear;
    super->draw_string   = canvas_x11_draw_string;
    super->create_pixmap = xlib_pixmap_create;
    super->draw_box      = canvas_xlib_draw_box;

    ((sgui_canvas_x11*)this)->cache         = NULL;
    ((sgui_canvas_x11*)this)->wnd           = wnd;
    ((sgui_canvas_x11*)this)->display       = canvas_xlib_display;
    ((sgui_canvas_x11*)this)->set_clip_rect = canvas_xlib_set_clip_rect;

    return (sgui_canvas*)this;
}

void canvas_x11_display( sgui_canvas* super, int x, int y,
                         unsigned int width, unsigned int height )
{
    sgui_canvas_x11* this = (sgui_canvas_x11*)super;

    if( this )
        this->display( this, x, y, width, height );
}

