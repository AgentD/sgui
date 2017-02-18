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
#include "sgui_pixmap.h"



static int canvas_x11_dirty_hook( sgui_canvas* super, const sgui_rect* r )
{
    sgui_canvas_x11* this = (sgui_canvas_x11*)super;
    XExposeEvent exp;

    memset( &exp, 0, sizeof(exp) );
    exp.type       = Expose;
    exp.send_event = 1;
    exp.display    = x11.dpy;
    exp.window     = (Window)this->wnd;
    exp.x          = r->left;
    exp.y          = r->top;
    exp.width      = SGUI_RECT_WIDTH_V(r);
    exp.height     = SGUI_RECT_HEIGHT_V(r);

    sgui_internal_lock_mutex( );
    XSendEvent(x11.dpy,(Window)this->wnd,False,ExposureMask,(XEvent*)&exp);
    sgui_internal_unlock_mutex( );
    return 0;
}

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

static void canvas_x11_resize( sgui_canvas* this, unsigned int width,
                               unsigned int height )
{
    (void)this; (void)width; (void)height;
}

static void canvas_x11_clear( sgui_canvas* super, sgui_rect* r )
{
    sgui_canvas_x11* this = (sgui_canvas_x11*)super;

    sgui_internal_lock_mutex( );
    XClearArea( x11. dpy, this->wnd, r->left, r->top,
                SGUI_RECT_WIDTH_V( r ), SGUI_RECT_HEIGHT_V( r ), False );
    sgui_internal_unlock_mutex( );
}
/****************************************************************************/
sgui_canvas* canvas_xrender_create( Drawable wnd, unsigned int width,
                                    unsigned int height, int sendexpose );

sgui_canvas* canvas_xlib_create( Drawable wnd, unsigned int width,
                                 unsigned int height, int sendexpose );


static struct {
    sgui_canvas *(*create)(Drawable wnd, unsigned int width,
                           unsigned int height, int sendexpose);
} x11_drivers[] = {
#ifndef SGUI_NO_XRENDER
    { canvas_xrender_create },
#endif
    { canvas_xlib_create },
};

sgui_canvas* canvas_x11_create( Drawable wnd, unsigned int width,
                                unsigned int height, int sendexpose )
{
    sgui_canvas* cv = NULL;
    unsigned int i;

    for( i = 0; i < sizeof(x11_drivers) / sizeof(x11_drivers[0]); ++i ) {
        cv = x11_drivers[i].create(wnd, width, height, sendexpose);

        if( cv )
            break;
    }
    return cv;
}

void canvas_x11_init( sgui_canvas* super, Drawable wnd,
                      sgui_funptr clip, int sendexpose )
{
    sgui_canvas_x11* this = (sgui_canvas_x11*)super;
    super->resize       = canvas_x11_resize;
    super->clear        = canvas_x11_clear;
    super->draw_string  = canvas_x11_draw_string;
    this->wnd           = wnd;
    this->set_clip_rect = clip;
    super->dirty_rect_hook = sendexpose ? canvas_x11_dirty_hook : NULL;
}
