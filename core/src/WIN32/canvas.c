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
    sgui_canvas canvas;

    sgui_rect locked;

    void* data;
    HDC dc;
    BITMAPINFO info;
    HBITMAP bitmap;
}
sgui_canvas_gdi;

/************************* public canvas functions *************************/
void canvas_gdi_destroy( sgui_canvas* canvas )
{
    /* destroy offscreen context and dib section */
    if( ((sgui_canvas_gdi*)canvas)->dc )
    {
        SelectObject( ((sgui_canvas_gdi*)canvas)->dc, 0 );
        DeleteObject( ((sgui_canvas_gdi*)canvas)->bitmap );
        DeleteDC( ((sgui_canvas_gdi*)canvas)->dc );
    }

    free( canvas );
}

void canvas_gdi_resize( sgui_canvas* canvas, unsigned int width,
                        unsigned int height )
{
    /* adjust size in the header */
    ((sgui_canvas_gdi*)canvas)->info.bmiHeader.biWidth  = width;
    ((sgui_canvas_gdi*)canvas)->info.bmiHeader.biHeight = -((int)height);

    /* unbind the the dib section and delete it */
    SelectObject( ((sgui_canvas_gdi*)canvas)->dc, 0 );
    DeleteObject( ((sgui_canvas_gdi*)canvas)->bitmap );

    /* create a new dib section */
    ((sgui_canvas_gdi*)canvas)->bitmap =
    CreateDIBSection( ((sgui_canvas_gdi*)canvas)->dc,
                      &((sgui_canvas_gdi*)canvas)->info, DIB_RGB_COLORS,
                      &((sgui_canvas_gdi*)canvas)->data, 0, 0 );

    /* bind it */
    SelectObject( ((sgui_canvas_gdi*)canvas)->dc,
                  ((sgui_canvas_gdi*)canvas)->bitmap );
}



void canvas_gdi_begin( sgui_canvas* canvas, sgui_rect* r )
{
    (void)canvas;
    (void)r;
}

void canvas_gdi_end( sgui_canvas* canvas )
{
    (void)canvas;
}

void canvas_gdi_clear( sgui_canvas* canvas, sgui_rect* r )
{
    sgui_canvas_gdi* cv = (sgui_canvas_gdi*)canvas;
    unsigned char *dst, *row;
    int i, j;

    dst = (unsigned char*)cv->data + (r->top*canvas->width+r->left)*4;

    /* clear */
    for( j=r->top; j<=r->bottom; ++j, dst+=canvas->width*4 )
    {
        for( row=dst, i=r->left; i<=r->right; ++i, row+=4 )
        {
            COLOR_COPY_INV( row, canvas->bg_color );
        }
    }
}

void canvas_gdi_blit( sgui_canvas* canvas, int x, int y, sgui_pixmap* pixmap,
                      sgui_rect* srcrect )
{
    sgui_canvas_gdi* cv = (sgui_canvas_gdi*)canvas;
    unsigned char *drow, *srow, *src, *dst;
    unsigned int i, j, ds, dt, src_bpp, w, h;
    sgui_rect r;

    sgui_pixmap_get_size( pixmap, &w, &h );
    sgui_rect_set_size( &r, 0, 0, w, h );
    sgui_rect_get_intersection( &r, &r, srcrect );

    src_bpp = pixmap_get_bpp( pixmap );
    dst = (unsigned char*)cv->data + (y*canvas->width + x)*4;
    src = pixmap_get_data( pixmap );

    if( !src )
        return;

    src += (r.top*w + r.left)*src_bpp;

    ds = w * src_bpp;
    dt = canvas->width * 4;

    w = SGUI_RECT_WIDTH( r );
    h = SGUI_RECT_HEIGHT( r );

    for( j=0; j<h; ++j, src+=ds, dst+=dt )
    {
        for( drow=dst, srow=src, i=0; i<w; ++i, srow+=src_bpp, drow+=4 )
        {
            COLOR_COPY( drow, srow );
        }
    }
}

void canvas_gdi_blend( sgui_canvas* canvas, int x, int y, sgui_pixmap* pixmap,
                       sgui_rect* srcrect )
{
    sgui_canvas_gdi* cv = (sgui_canvas_gdi*)canvas;
    unsigned char *drow, *srow, *src, *dst;
    unsigned int i, j, ds, dt, src_bpp, w, h;
    sgui_rect r;

    src_bpp = pixmap_get_bpp( pixmap );

    if( src_bpp != 4 )
    {
        canvas_gdi_blit( canvas, x, y, pixmap, srcrect );
        return;
    }

    sgui_pixmap_get_size( pixmap, &w, &h );
    sgui_rect_set_size( &r, 0, 0, w, h );
    sgui_rect_get_intersection( &r, &r, srcrect );

    dst = (unsigned char*)cv->data + (y*canvas->width + x)*4;
    src = pixmap_get_data( pixmap );

    if( !src )
        return;

    src += (r.top*w + r.left)*4;

    ds = w * 4;
    dt = canvas->width * 4;

    w = SGUI_RECT_WIDTH( r );
    h = SGUI_RECT_HEIGHT( r );

    for( j=0; j<h; ++j, src+=ds, dst+=dt )
    {
        for( drow=dst, srow=src, i=0; i<w; ++i, srow+=src_bpp, drow+=4 )
        {
            drow[0] = srow[0] + ((drow[0]*srow[3])>>8);
            drow[1] = srow[1] + ((drow[1]*srow[3])>>8);
            drow[2] = srow[2] + ((drow[2]*srow[3])>>8);
        }
    }
}

void canvas_gdi_draw_box( sgui_canvas* canvas, sgui_rect* r,
                          unsigned char* color, int format )
{
    sgui_canvas_gdi* cv = (sgui_canvas_gdi*)canvas;
    unsigned char A, iA;
    unsigned char *dst, *row;
    int i, j;

    dst = (unsigned char*)cv->data + (r->top*canvas->width + r->left)*4;

    if( format==SGUI_RGBA8 )
    {
        A = color[3];
        iA = 0xFF - A;

        for( j=r->top; j<=r->bottom; ++j, dst+=canvas->width*4 )
        {
            for( row=dst, i=r->left; i<=r->right; ++i, row+=4 )
            {
                row[0] = (row[0] * iA + color[2] * A)>>8;
                row[1] = (row[1] * iA + color[1] * A)>>8;
                row[2] = (row[2] * iA + color[0] * A)>>8;
            }
        }
    }
    else
    {
        for( j=r->top; j<=r->bottom; ++j, dst+=canvas->width*4 )
        {
            for( row=dst, i=r->left; i<=r->right; ++i, row+=4 )
            {
                row[0] = color[2];
                row[1] = color[1];
                row[2] = color[0];
            }
        }
    }
}

void canvas_gdi_blend_stencil( sgui_canvas* canvas, unsigned char* buffer,
                               int x, int y, unsigned int w, unsigned int h,
                               unsigned int scan, unsigned char* color )
{
    sgui_canvas_gdi* cv = (sgui_canvas_gdi*)canvas;
    unsigned char A, iA, *src, *dst, *row;
    unsigned int i, j;

    dst = (unsigned char*)cv->data + (y*canvas->width + x)*4;

    for( j=0; j<h; ++j, buffer+=scan, dst+=canvas->width*4 )
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

int canvas_gdi_draw_string( sgui_canvas* canvas, int x, int y,
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

            canvas_gdi_blend_stencil( canvas, buffer, r.left, r.top,
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
sgui_canvas* canvas_gdi_create( unsigned int width, unsigned int height )
{
    sgui_canvas_gdi* cv = malloc( sizeof(sgui_canvas_gdi) );

    if( !cv )
        return NULL;

    /* create an offscreen Device Context */
    cv->dc = CreateCompatibleDC( NULL );

    if( !cv->dc )
    {
        free( cv );
        return NULL;
    }

    /*fill the bitmap header */
    cv->info.bmiHeader.biSize        = sizeof(cv->info.bmiHeader);
    cv->info.bmiHeader.biBitCount    = 32;
    cv->info.bmiHeader.biCompression = BI_RGB;
    cv->info.bmiHeader.biPlanes      = 1;
    cv->info.bmiHeader.biWidth       = width;
    cv->info.bmiHeader.biHeight      = -((int)height);

    /* create a DIB section = bitmap with accessable data pointer */
    cv->bitmap = CreateDIBSection( cv->dc, &cv->info, DIB_RGB_COLORS,
                                   &cv->data, 0, 0 );

    if( !cv->bitmap )
    {
        DeleteDC( cv->dc );
        free( cv );
        return NULL;
    }

    /* bind the dib section to the offscreen context */
    SelectObject( cv->dc, cv->bitmap );

    /* finish base initialisation */
    sgui_internal_canvas_init( (sgui_canvas*)cv, width, height );

    cv->canvas.destroy = canvas_gdi_destroy;
    cv->canvas.resize = canvas_gdi_resize;
    cv->canvas.begin = canvas_gdi_begin;
    cv->canvas.end = canvas_gdi_end;
    cv->canvas.clear = canvas_gdi_clear;
    cv->canvas.blit = canvas_gdi_blit;
    cv->canvas.blend = canvas_gdi_blend;
    cv->canvas.draw_box = canvas_gdi_draw_box;
    cv->canvas.draw_string = canvas_gdi_draw_string;

    return (sgui_canvas*)cv;
}

void canvas_gdi_display( HDC dc, sgui_canvas* cv, int x, int y,
                         unsigned int width, unsigned int height )
{
    BitBlt( dc, x, y, width, height, ((sgui_canvas_gdi*)cv)->dc,
            x, y, SRCCOPY );
}

