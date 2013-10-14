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


#define COLOR_COPY_INV( a, b ) (a)[0]=(b)[2]; (a)[1]=(b)[1]; (a)[2]=(b)[0]
#define COLOR_COPY( a, b ) (a)[0]=(b)[0]; (a)[1]=(b)[1]; (a)[2]=(b)[2]


typedef struct
{
    sgui_canvas super;

    void* data;
    HDC dc;
    BITMAPINFO info;
    HBITMAP bitmap;
}
sgui_canvas_gdi;

/************************* public canvas functions *************************/
static void canvas_gdi_destroy( sgui_canvas* this )
{
    /* destroy offscreen context and dib section */
    if( ((sgui_canvas_gdi*)this)->dc )
    {
        SelectObject( ((sgui_canvas_gdi*)this)->dc, 0 );
        DeleteObject( ((sgui_canvas_gdi*)this)->bitmap );
        DeleteDC( ((sgui_canvas_gdi*)this)->dc );
    }

    free( this );
}

static void canvas_gdi_resize( sgui_canvas* super, unsigned int width,
                               unsigned int height )
{
    sgui_canvas_gdi* this = (sgui_canvas_gdi*)super;

    /* adjust size in the header */
    this->info.bmiHeader.biWidth  = width;
    this->info.bmiHeader.biHeight = -((int)height);

    /* unbind the the dib section and delete it */
    SelectObject( this->dc, 0 );
    DeleteObject( this->bitmap );

    /* create a new dib section */
    this->bitmap = CreateDIBSection( this->dc, &this->info, DIB_RGB_COLORS,
                                     &this->data, 0, 0 );

    /* bind it */
    SelectObject( this->dc, this->bitmap );
}

static void canvas_gdi_clear( sgui_canvas* super, sgui_rect* r )
{
    sgui_canvas_gdi* this = (sgui_canvas_gdi*)super;
    unsigned char *dst, *row;
    int i, j;

    dst = (unsigned char*)this->data + (r->top*super->width + r->left)*4;

    /* clear */
    for( j=r->top; j<=r->bottom; ++j, dst+=super->width*4 )
    {
        for( row=dst, i=r->left; i<=r->right; ++i, row+=4 )
        {
            COLOR_COPY_INV( row, super->bg_color );
        }
    }
}

static void canvas_gdi_blit( sgui_canvas* super, int x, int y,
                             sgui_pixmap* pixmap, sgui_rect* srcrect )
{
    sgui_canvas_gdi* this = (sgui_canvas_gdi*)super;
    unsigned int w = SGUI_RECT_WIDTH_V( srcrect );
    unsigned int h = SGUI_RECT_HEIGHT_V( srcrect );

    BitBlt( this->dc, x, y, w, h, ((gdi_pixmap*)pixmap)->hDC,
            srcrect->left, srcrect->top, SRCCOPY );
}

static void canvas_gdi_blend( sgui_canvas* super, int x, int y,
                              sgui_pixmap* pixmap, sgui_rect* srcrect )
{
    sgui_canvas_gdi* this = (sgui_canvas_gdi*)super;
    unsigned int w = SGUI_RECT_WIDTH_V( srcrect );
    unsigned int h = SGUI_RECT_HEIGHT_V( srcrect );
    BLENDFUNCTION ftn;

    ftn.BlendOp             = AC_SRC_OVER;
    ftn.BlendFlags          = 0;
    ftn.SourceConstantAlpha = 0xFF;
    ftn.AlphaFormat         = AC_SRC_ALPHA;

    AlphaBlend( this->dc, x, y, w, h, ((gdi_pixmap*)pixmap)->hDC,
                srcrect->left, srcrect->top, w, h, ftn );
}

static void canvas_gdi_blend_stencil( sgui_canvas* super,
                                      unsigned char* buffer,
                                      int x, int y,
                                      unsigned int w, unsigned int h,
                                      unsigned int scan,
                                      unsigned char* color )
{
    sgui_canvas_gdi* this = (sgui_canvas_gdi*)super;
    unsigned char A, iA, *src, *dst, *row;
    unsigned int i, j;

    dst = (unsigned char*)this->data + (y*super->width + x)*4;

    for( j=0; j<h; ++j, buffer+=scan, dst+=super->width*4 )
    {
        for( src=buffer, row=dst, i=0; i<w; ++i, row+=4, ++src )
        {
            A = *src;
            iA = 0xFF-A;

            row[0] = (row[0] * iA + color[2] * A)>>8;
            row[1] = (row[1] * iA + color[1] * A)>>8;
            row[2] = (row[2] * iA + color[0] * A)>>8;
        }
    }
}

static int canvas_gdi_draw_string( sgui_canvas* super, int x, int y,
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

        if( buffer && sgui_rect_get_intersection( &r, &super->sc, &r ) )
        {
            buffer += (r.top - (y + bearing)) * w + (r.left - x);

            canvas_gdi_blend_stencil( super, buffer, r.left, r.top,
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

static sgui_pixmap* canvas_gdi_create_pixmap( sgui_canvas* super,
                                              unsigned int width,
                                              unsigned int height,
                                              int format )
{
    (void)super;

    return gdi_pixmap_create( width, height, format );
}

/************************ internal canvas functions ************************/
sgui_canvas* canvas_gdi_create( unsigned int width, unsigned int height )
{
    sgui_canvas_gdi* this = malloc( sizeof(sgui_canvas_gdi) );
    sgui_canvas* super = (sgui_canvas*)this;

    if( !this )
        return NULL;

    /* create an offscreen Device Context */
    this->dc = CreateCompatibleDC( NULL );

    if( !this->dc )
    {
        free( this );
        return NULL;
    }

    /*fill the bitmap header */
    this->info.bmiHeader.biSize        = sizeof(this->info.bmiHeader);
    this->info.bmiHeader.biBitCount    = 32;
    this->info.bmiHeader.biCompression = BI_RGB;
    this->info.bmiHeader.biPlanes      = 1;
    this->info.bmiHeader.biWidth       = width;
    this->info.bmiHeader.biHeight      = -((int)height);

    /* create a DIB section = bitmap with accessable data pointer */
    this->bitmap = CreateDIBSection( this->dc, &this->info, DIB_RGB_COLORS,
                                     &this->data, 0, 0 );

    if( !this->bitmap )
    {
        DeleteDC( this->dc );
        free( this );
        return NULL;
    }

    /* bind the dib section to the offscreen context */
    SelectObject( this->dc, this->bitmap );

    /* finish base initialisation */
    sgui_internal_canvas_init( super, width, height );

    super->destroy = canvas_gdi_destroy;
    super->resize = canvas_gdi_resize;
    super->clear = canvas_gdi_clear;
    super->blit = canvas_gdi_blit;
    super->blend = canvas_gdi_blend;
    super->stretch_blit = NULL;
    super->stretch_blend = NULL;
    super->draw_string = canvas_gdi_draw_string;
    super->create_pixmap = canvas_gdi_create_pixmap;
    super->skin_pixmap = get_skin_pixmap( );

    return (sgui_canvas*)this;
}

void canvas_gdi_display( HDC dc, sgui_canvas* this, int x, int y,
                         unsigned int width, unsigned int height )
{
    BitBlt( dc, x, y, width, height, ((sgui_canvas_gdi*)this)->dc,
            x, y, SRCCOPY );
}

