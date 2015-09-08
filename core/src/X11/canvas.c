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
#include "sgui_config.h"



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
            goto fail;

        if( !(this->cache = sgui_font_cache_create( font_map )) )
        {
            sgui_pixmap_destroy( font_map );
            goto fail;
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
        x += font->get_kerning_distance( font, previous, character );

        /* blend onto destination buffer */
        x += sgui_font_cache_draw_glyph( this->cache, font, character,
                                         x, y, super, color ) + 1;

        /* store previous glyph index for kerning */
        previous = character;
    }

    this->set_clip_rect( this, 0, 0, super->width, super->height );
    sgui_internal_unlock_mutex( );
    return x - oldx;
fail:
    sgui_internal_unlock_mutex( );
    return super->width;
}

/************************ xlib based implementation ************************/
static void canvas_xlib_destroy( sgui_canvas* super )
{
    sgui_canvas_xlib* this = (sgui_canvas_xlib*)super;

    sgui_internal_lock_mutex( );
    if( ((sgui_canvas_x11*)this)->cache )
        sgui_icon_cache_destroy( ((sgui_canvas_x11*)this)->cache );

    XFreeGC( x11.dpy, this->gc );
    XFreePixmap( x11.dpy, this->pixmap );
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
    XSetClipRectangles( x11.dpy, this->gc, 0, 0, &r, 1, Unsorted );
    sgui_internal_unlock_mutex( );
}

static void canvas_xlib_resize( sgui_canvas* super, unsigned int width,
                                unsigned int height )
{
    sgui_canvas_xlib* this = (sgui_canvas_xlib*)super;
    Pixmap newpm;
    GC newgc;

    sgui_internal_lock_mutex( );
    newpm = XCreatePixmap( x11.dpy, ((sgui_canvas_x11*)this)->wnd,
                           width, height, 24 );

    if( !newpm )
        goto done;

    newgc = XCreateGC( x11.dpy, this->pixmap, 0, NULL );

    if( !newgc )
    {
        XFreePixmap( x11.dpy, newpm );
        goto done;
    }

    XFreeGC( x11.dpy, this->gc );
    XFreePixmap( x11.dpy, this->pixmap );
    this->pixmap = newpm;
    this->gc = newgc;
done:
    sgui_internal_unlock_mutex( );
}

static void canvas_xlib_clear( sgui_canvas* super, sgui_rect* r )
{
    sgui_canvas_xlib* this = (sgui_canvas_xlib*)super;
    unsigned long color;

    color = (this->bg[0]<<16) | (this->bg[1]<<8) | this->bg[2];

    sgui_internal_lock_mutex( );
    XSetForeground( x11.dpy, this->gc, color );
    XFillRectangle( x11.dpy, this->pixmap, this->gc, r->left, r->top,
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
    XSetForeground( x11.dpy, this->gc, (R<<16) | (G<<8) | B );
    XFillRectangle( x11.dpy, this->pixmap, this->gc, r->left, r->top,
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
    XCopyArea( x11.dpy, pix->data.xpm, this->pixmap, this->gc,
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
    int X, Y, C[4], sc;


    src = pix->data.pixels + (r->top*pixmap->width + r->left);
    C[0] = (color[0]<<16) | (color[1]<<8) | color[2];
    C[1] = ((3*color[0]/4 + this->bg[0]/4)<<16) |
           ((3*color[1]/4 + this->bg[1]/4)<<8) |
            (3*color[2]/4 + this->bg[2]/4);
    C[2] = ((color[0]/2 + this->bg[0]/2)<<16) |
           ((color[1]/2 + this->bg[1]/2)<<8) |
            (color[2]/2 + this->bg[2]/2);
    C[3] = ((color[0]/4 + 3*this->bg[0]/4)<<16) |
           ((color[1]/4 + 3*this->bg[1]/4)<<8) |
            (color[2]/4 + 3*this->bg[2]/4);

    sgui_internal_lock_mutex( );
    for( Y=r->top; Y<=r->bottom; ++Y, src+=pixmap->width )
    {
        for( src_row=src, X=r->left; X<=r->right; ++X, ++src_row )
        {
            if( (*src_row)<=0x20 )
                continue;

            sc = (*src_row)<=0x40 ? C[3] :
                 (*src_row)<=0x80 ? C[2] :
                 (*src_row)<=0xC0 ? C[1] :
                 C[0];

            XSetForeground( x11.dpy, this->gc, sc );
            XDrawPoint( x11.dpy, this->pixmap, this->gc,
                        x+X-r->left, y+Y-r->top );
        }
    }
    sgui_internal_unlock_mutex( );
}

static void canvas_xlib_display( sgui_canvas_x11* super, int x, int y,
                                 unsigned int width, unsigned int height )
{
    sgui_canvas_xlib* this = (sgui_canvas_xlib*)super;

    sgui_internal_lock_mutex( );
    XCopyArea( x11.dpy, this->pixmap, super->wnd, this->gc,
               x, y, width, height, x, y );
    sgui_internal_unlock_mutex( );
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

    if( !(this->pixmap = XCreatePixmap( x11.dpy, wnd, width, height, 24 )) )
        goto fail;

    if( !(this->gc = XCreateGC( x11.dpy, this->pixmap, 0, NULL )) )
        goto fail;

    if( !sgui_canvas_init( super, width, height ) )
        goto fail;

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
fail:
    if( this->gc     ) XFreeGC( x11.dpy, this->gc );
    if( this->pixmap ) XFreePixmap( x11.dpy, this->pixmap );
    sgui_internal_unlock_mutex( );
    free( this );
    return NULL;
}

/*********************** xrender based implementation ***********************/
#ifndef SGUI_NO_XRENDER
static void canvas_xrender_destroy( sgui_canvas* super )
{
    sgui_canvas_xrender* this = (sgui_canvas_xrender*)super;

    sgui_internal_lock_mutex( );

    if( ((sgui_canvas_x11*)this)->cache )
        sgui_icon_cache_destroy( ((sgui_canvas_x11*)this)->cache );

    if( this->pic ) XRenderFreePicture( x11.dpy, this->pic );
    if( this->pen ) XRenderFreePicture( x11.dpy, this->pen );
    if( this->wndpic ) XRenderFreePicture( x11.dpy, this->wndpic );

    if( this->pixmap ) XFreePixmap( x11.dpy, this->pixmap );
    if( this->penmap ) XFreePixmap( x11.dpy, this->penmap );

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
    XRenderSetPictureClipRectangles( x11.dpy, this->pic, 0, 0, &r, 1 );
    sgui_internal_unlock_mutex( );
}

static void canvas_xrender_resize( sgui_canvas* super, unsigned int width,
                                   unsigned int height )
{
    sgui_canvas_xrender* this = (sgui_canvas_xrender*)super;
    XRenderPictFormat* fmt;

    sgui_internal_lock_mutex( );

    /* destroy the pixmap */
    XRenderFreePicture( x11.dpy, this->pic );
    XFreePixmap( x11.dpy, this->pixmap );

    /* create a new pixmap */
    this->pixmap = XCreatePixmap( x11.dpy, ((sgui_canvas_x11*)this)->wnd,
                                  width, height, 32 );

    fmt = XRenderFindStandardFormat( x11.dpy, PictStandardARGB32 );
    this->pic = XRenderCreatePicture( x11.dpy, this->pixmap, fmt, 0, NULL );

    sgui_internal_unlock_mutex( );
}

static void canvas_xrender_clear( sgui_canvas* super, sgui_rect* r )
{
    sgui_canvas_xrender* this = (sgui_canvas_xrender*)super;
    XRenderColor c;

    c.red = c.green = c.blue = c.alpha = 0;

    sgui_internal_lock_mutex( );
    XRenderFillRectangle( x11.dpy, PictOpSrc, this->pic, &c, r->left, r->top,
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
    XRenderFillRectangle( x11.dpy, PictOpOver, this->pic, &c, r->left, r->top,
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
    XRenderFillRectangle( x11.dpy, PictOpSrc, this->pic, &c, x, y, w, h );
    XRenderComposite( x11.dpy, PictOpOver, ((xrender_pixmap*)pixmap)->pic, 0,
                      this->pic, srcrect->left, srcrect->top, 0, 0,
                      x, y, w, h );
    sgui_internal_unlock_mutex( );
}

static void canvas_xrender_blend( sgui_canvas* super, int x, int y,
                                  sgui_pixmap* pixmap, sgui_rect* srcrect )
{
    sgui_canvas_xrender* this = (sgui_canvas_xrender*)super;

    sgui_internal_lock_mutex( );
    XRenderComposite( x11.dpy, PictOpOver, ((xrender_pixmap*)pixmap)->pic, 0,
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
    XRenderFillRectangle( x11.dpy, PictOpSrc, this->pen, &c, 0, 0, 1, 1 );

    XRenderComposite( x11.dpy, PictOpOver, this->pen,
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
    XClearArea( x11.dpy, super->wnd, x, y, width, height, False );
    XRenderComposite( x11.dpy, PictOpOver, this->pic, 0, this->wndpic, x, y,
                      0, 0, x, y, width, height );
    sgui_internal_unlock_mutex( );
}

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
    if( !XRenderQueryExtension( x11.dpy, &base, &error ) )
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
    if( !(this->pixmap = XCreatePixmap( x11.dpy, wnd, width, height, 32 )) )
        goto fail;

    if( !(this->penmap = XCreatePixmap( x11.dpy, wnd, 1, 1, 24 )) )
        goto fail;

    /* crate Xrender pictures */
    fmt = XRenderFindStandardFormat( x11.dpy, PictStandardARGB32 );
    this->pic = XRenderCreatePicture( x11.dpy, this->pixmap, fmt, 0, NULL );

    if( !this->pic )
        goto fail;

    fmt = XRenderFindStandardFormat( x11.dpy, PictStandardRGB24 );
    this->wndpic = XRenderCreatePicture( x11.dpy, wnd, fmt, 0, NULL );

    if( !this->wndpic )
        goto fail;

    attr.repeat = RepeatNormal;
    this->pen = XRenderCreatePicture(x11.dpy,this->penmap,fmt,CPRepeat,&attr);

    if( !this->pen )
        goto fail;

    /* finish initialisation */
    if( !sgui_canvas_init( super, width, height ) )
        goto fail;
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
fail:
    sgui_internal_unlock_mutex( );
    canvas_xrender_destroy( super );
    return NULL;
}
#endif /* !SGUI_NO_XRENDER */

