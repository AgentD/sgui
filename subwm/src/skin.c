/*
 * skin.c
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
#include "sgui_subwm_skin.h"
#include "sgui_canvas.h"
#include "sgui_skin.h"

#include <stdlib.h>
#include <string.h>



static sgui_subwm_skin* wmskin = NULL;



static sgui_subwm_skin defaultskin;
static unsigned char defaultctxtex[ 16*16*4 ];



static void default_get_ctx_skin_texture_size( sgui_subwm_skin* skin,
                                               unsigned int* width,
                                               unsigned int* height )
{
    (void)skin;

    if( width )
        *width = 16;

    if( height )
        *height = 16;
}

static const unsigned char*
default_get_ctx_skin_texture( sgui_subwm_skin* skin )
{
    (void)skin;
    return defaultctxtex;
}

static void default_get_ctx_window_corner( sgui_subwm_skin* skin,
                                           sgui_rect* rect, int type )
{
    (void)skin;
    switch( type )
    {
    case SGUI_WINDOW_TOP_LEFT:     sgui_rect_set_size(rect,0,0,8,8); break;
    case SGUI_WINDOW_TOP_RIGHT:    sgui_rect_set_size(rect,8,0,8,8); break;
    case SGUI_WINDOW_BOTTOM_LEFT:  sgui_rect_set_size(rect,0,8,8,8); break;
    case SGUI_WINDOW_BOTTOM_RIGHT: sgui_rect_set_size(rect,8,8,8,8); break;
    }
}

static int default_get_window_transparency( sgui_subwm_skin* skin, int type )
{
    (void)skin;
    switch( type )
    {
    case SGUI_WINDOW_BACKGROUND: return 0x80;
    case SGUI_WINDOW_TOPMOST:    return 0xF0;
    case SGUI_WINDOW_DRAGING:    return 0xC0;
    }
    return 0x00;
}

static void default_draw_title_bar( sgui_subwm_skin* skin, sgui_canvas* cv,
                                    const char* title )
{
    const unsigned char color[4] = { 0x00, 0x00, 0x00, 0xFF };
    unsigned int w, h;
    sgui_rect r;
    (void)skin;

    sgui_canvas_get_size( cv, &w, &h );
    sgui_rect_set_size( &r, 0, 0, w, 20 );
    sgui_canvas_clear( cv, &r );
    sgui_canvas_draw_text_plain( cv, 15, 0, 1, 0,
                                 sgui_skin_get( )->font_color, title, -1 );

    sgui_canvas_draw_line( cv, 0, 19, w, 1, color, SGUI_RGBA8 );
}

static unsigned int default_get_titlebar_height( sgui_subwm_skin* skin )
{
    (void)skin;
    return 20;
}

static sgui_subwm_skin* suwbm_skin_default( void )
{
    unsigned char color[4];
    int x, y, a;

    defaultskin.get_ctx_skin_texture_size = default_get_ctx_skin_texture_size;
    defaultskin.get_ctx_skin_texture = default_get_ctx_skin_texture;
    defaultskin.get_window_transparency = default_get_window_transparency;
    defaultskin.get_ctx_window_corner = default_get_ctx_window_corner;
    defaultskin.draw_title_bar = default_draw_title_bar;
    defaultskin.get_titlebar_height = default_get_titlebar_height;

    memset( defaultctxtex, 0, sizeof(defaultctxtex) );
    memcpy( color, sgui_skin_get( )->window_color, 4 );

    for( y=0; y<16; ++y )
    {
        for( x=0; x<16; ++x )
        {
            a = 0xFF;
            if( ((y==0||y==15)&&(x<5||x>10)) || ((x==0||x==15)&&(y<5||y>10)) )
                a = 0;
            if( ((y==1||y==14)&&(x<3||x>12)) || ((x==1||x==14)&&(y<3||y>12)) )
                a = 0;

            defaultctxtex[ (y*16 + x)*4   ] = color[0];
            defaultctxtex[ (y*16 + x)*4+1 ] = color[1];
            defaultctxtex[ (y*16 + x)*4+2 ] = color[2];
            defaultctxtex[ (y*16 + x)*4+3 ] = a;
        }
    }

    return &defaultskin;
}

/****************************************************************************/

sgui_subwm_skin* sgui_subwm_skin_get( void )
{
    if( !wmskin )
        wmskin = suwbm_skin_default( );

    return wmskin;
}

void sgui_subwm_skin_set( sgui_subwm_skin* skin )
{
    wmskin = skin ? skin : suwbm_skin_default( );
}

