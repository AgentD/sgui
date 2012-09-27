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
#include "internal.h"



#define COLOR_COPY( a, b ) (a)[0]=(b)[0]; (a)[1]=(b)[1]; (a)[2]=(b)[2]
#define COLOR_COPY_INV( a, b ) (a)[0]=(b)[2]; (a)[1]=(b)[1]; (a)[2]=(b)[0]

#define COLOR_BLEND( a, b, A, iA )\
        (a)[0] = ((a)[0]*iA + (b)[0]*A)>>8;\
        (a)[1] = ((a)[1]*iA + (b)[1]*A)>>8;\
        (a)[2] = ((a)[2]*iA + (b)[2]*A)>>8;

#define COLOR_BLEND_INV( a, b, A, iA )\
        (a)[0] = ((a)[0]*iA + (b)[2]*A)>>8;\
        (a)[1] = ((a)[1]*iA + (b)[1]*A)>>8;\
        (a)[2] = ((a)[2]*iA + (b)[0]*A)>>8;

/************************* public canvas functions *************************/
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
    unsigned char *dst, *row;
    int i, j;

    dst = (unsigned char*)cv->data + (r->top*canvas->width+r->left)*4;

    /* clear */
    for( j=r->top; j<=r->bottom; ++j, dst+=canvas->width*4 )
    {
        for( row=dst, i=r->left; i<=r->right; ++i, row+=4 )
        {
            COLOR_COPY( row, canvas->bg_color );
        }
    }
}

void canvas_xlib_blit( sgui_canvas* canvas, int x, int y, unsigned int width,
                       unsigned int height, unsigned int scanline_length,
                       SGUI_COLOR_FORMAT format, const void* data )
{
    sgui_canvas_xlib* cv = (sgui_canvas_xlib*)canvas;
    unsigned char *drow, *srow, *src, *dst;
    unsigned int i, j, ds, dt, src_bpp = (format==SCF_RGBA8 ? 4 : 3);

    dst = ((unsigned char*)cv->data) + (y*canvas->width + x)*4;
    src = (unsigned char*)data;

    ds = scanline_length * (format==SCF_RGBA8 ? 4 : 3);
    dt = canvas->width * 4;

    for( j=0; j<height; ++j, src+=ds, dst+=dt )
    {
        for( drow=dst, srow=src, i=0; i<width; ++i, drow+=4, srow+=src_bpp )
        {
            COLOR_COPY_INV( drow, srow );
        }
    }
}

void canvas_xlib_blend( sgui_canvas* canvas, int x, int y, unsigned int width,
                        unsigned int height, unsigned int scanline_length,
                        const void* data )
{
    sgui_canvas_xlib* cv = (sgui_canvas_xlib*)canvas;
    unsigned char *dst, *src, *drow, *srow, A, iA;
    unsigned int ds, dt, i, j;

    dst = (unsigned char*)cv->data + (y*canvas->width + x)*4;
    src = (unsigned char*)data;

    ds = scanline_length * 4;
    dt = canvas->width*4;

    for( j=0; j<height; ++j, src+=ds, dst+=dt )
    {
        for( drow=dst, srow=src, i=0; i<width; ++i, drow+=4, srow+=4 )
        {
            A = srow[3];
            iA = 0xFF-A;

            COLOR_BLEND_INV( drow, srow, A, iA );
        }
    }
}

void canvas_xlib_draw_box( sgui_canvas* canvas, sgui_rect* r,
                           unsigned char* color, SGUI_COLOR_FORMAT format )
{
    sgui_canvas_xlib* cv = (sgui_canvas_xlib*)canvas;
    unsigned char c[3], A, iA;
    unsigned char *dst, *row;
    int i, j;

    COLOR_COPY_INV( c, color );

    dst = (unsigned char*)cv->data + (r->top*canvas->width + r->left)*4;

    if( format==SCF_RGBA8 )
    {
        A = color[3];
        iA = 255 - A;

        for( j=r->top; j<=r->bottom; ++j, dst+=canvas->width*4 )
        {
            for( row=dst, i=r->left; i<=r->right; ++i, row+=4 )
            {
                COLOR_BLEND( row, c, A, iA );
            }
        }
    }
    else
    {
        for( j=r->top; j<=r->bottom; ++j, dst+=canvas->width*4 )
        {
            for( row=dst, i=r->left; i<=r->right; ++i, row+=4 )
            {
                COLOR_COPY( row, c );
            }
        }
    }
}

void canvas_xlib_draw_line( sgui_canvas* canvas, int x, int y,
                            unsigned int length, int horizontal,
                            unsigned char* color, SGUI_COLOR_FORMAT format )
{
    sgui_canvas_xlib* cv = (sgui_canvas_xlib*)canvas;
    unsigned char* dst;
    unsigned char c[3], A, iA;
    unsigned int i, delta;

    COLOR_COPY_INV( c, color );

    dst = (unsigned char*)cv->data + (y*canvas->width + x)*4;
    delta = horizontal ? 4 : canvas->width*4;

    if( format==SCF_RGBA8 )
    {
        A = color[3];
        iA = 255 - A;

        for( i=0; i<length; ++i, dst+=delta )
        {
            COLOR_BLEND( dst, c, A, iA );
        }
    }
    else
    {
        for( i=0; i<length; ++i, dst+=delta )
        {
            COLOR_COPY( dst, c );
        }
    }
}

void canvas_xlib_blend_stencil( sgui_canvas* canvas, unsigned char* buffer,
                                int x, int y, unsigned int w, unsigned int h,
                                unsigned int scan, unsigned char* color )
{
    sgui_canvas_xlib* cv = (sgui_canvas_xlib*)canvas;
    unsigned char A, iA, *src, *dst, *row;
    unsigned int i, j;

    dst = (unsigned char*)cv->data + (y*canvas->width + x)*4;

    for( j=0; j<h; ++j, buffer+=scan, dst+=canvas->width*4 )
    {
        for( src=buffer, row=dst, i=0; i<w; ++i, row+=4, ++src )
        {
            A = *src;
            iA = 255-A;

            COLOR_BLEND_INV( row, color, A, iA );
        }
    }
}

/************************ internal canvas functions ************************/
sgui_canvas_xlib* sgui_canvas_create( unsigned int width, unsigned int height,
                                      Display* dpy )
{
    sgui_canvas_xlib* cv = malloc( sizeof(sgui_canvas_xlib) );

    if( !cv )
        return NULL;

    cv->data = malloc( width * height * 4 );

    if( !cv->data )
    {
        free( cv );
        return NULL;
    }

    cv->img = XCreateImage( dpy, CopyFromParent, 24, ZPixmap, 0,
                            cv->data, width, height, 32, 0 );

    if( !cv->img )
    {
        free( cv->data );
        free( cv );
        return NULL;
    }

    sgui_internal_canvas_init( (sgui_canvas*)cv, width, height );

    cv->canvas.begin = canvas_xlib_begin;
    cv->canvas.end = canvas_xlib_end;
    cv->canvas.clear = canvas_xlib_clear;
    cv->canvas.blit = canvas_xlib_blit;
    cv->canvas.blend = canvas_xlib_blend;
    cv->canvas.draw_box = canvas_xlib_draw_box;
    cv->canvas.draw_line = canvas_xlib_draw_line;
    cv->canvas.blend_stencil = canvas_xlib_blend_stencil;

    return cv;
}

void sgui_canvas_destroy( sgui_canvas_xlib* canvas )
{
    if( canvas->img )
    {
        canvas->img->data = NULL;
        XDestroyImage( canvas->img );
    }

    free( canvas->data );
    free( canvas );
}

void sgui_canvas_resize( sgui_canvas_xlib* canvas, unsigned int width,
                         unsigned int height, Display* dpy )
{
    unsigned int new_mem, old_mem;

    if( !canvas || !width || !height )
        return;

    new_mem = width * height;
    old_mem = canvas->canvas.width * canvas->canvas.height;

    if( new_mem != old_mem )
    {
        free( canvas->data );
        canvas->data = malloc( new_mem * 4 );
    }

    canvas->img->data = NULL;
    XDestroyImage( canvas->img );

    canvas->img = XCreateImage( dpy, CopyFromParent, 24, ZPixmap, 0,
                                (char*)canvas->data, width, height, 32, 0 );

    canvas->canvas.width = width;
    canvas->canvas.height = height;
}

