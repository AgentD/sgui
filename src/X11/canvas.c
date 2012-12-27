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


#define COLOR_COPY_INV( a, b ) (a)[0]=(b)[2]; (a)[1]=(b)[1]; (a)[2]=(b)[0]


typedef struct
{
    sgui_canvas canvas;

    sgui_rect locked;

    XImage* img;
}
sgui_canvas_xlib;



/************************* public canvas functions *************************/
void canvas_xlib_download( sgui_canvas* canvas, sgui_rect* r )
{
    int i, j, delta;
    unsigned char temp, *ptr, *row;

    canvas->buffer = (unsigned char*)((sgui_canvas_xlib*)canvas)->img->data;
    canvas->buffer_x = 0;
    canvas->buffer_y = 0;
    canvas->buffer_w = canvas->width;
    canvas->buffer_h = canvas->height;

    sgui_rect_copy( &((sgui_canvas_xlib*)canvas)->locked, r );

    /* switch colors of "downloaded" region from BGR to RGB */
    ptr = canvas->buffer + (r->top * canvas->width + r->left) * 4;
    delta = canvas->width * 4;

    for( j=r->top; j<=r->bottom; ++j, ptr+=delta )
    {
        for( row=ptr, i=r->left; i<=r->right; ++i, row+=4 )
        {
            temp = row[0];
            row[0] = row[2];
            row[2] = temp;
        }
    }
}

void canvas_xlib_upload( sgui_canvas* canvas )
{
    sgui_canvas_xlib* cv = (sgui_canvas_xlib*)canvas;
    int i, j, delta;
    unsigned char temp, *ptr, *row;

    /* switch colors of "downloaded" region from RGB to BGR */
    ptr = canvas->buffer + (cv->locked.top*canvas->width + cv->locked.left)*4;
    delta = canvas->width * 4;

    for( j=cv->locked.top; j<=cv->locked.bottom; ++j, ptr+=delta )
    {
        for( row=ptr, i=cv->locked.left; i<=cv->locked.right; ++i, row+=4 )
        {
            temp = row[0];
            row[0] = row[2];
            row[2] = temp;
        }
    }
}

void canvas_xlib_clear( sgui_canvas* canvas, sgui_rect* r )
{
    sgui_canvas_xlib* cv = (sgui_canvas_xlib*)canvas;
    unsigned char *dst, *row;
    int i, j;

    dst = (unsigned char*)cv->img->data + (r->top*canvas->width + r->left)*4;

    /* clear */
    for( j=r->top; j<=r->bottom; ++j, dst+=canvas->width*4 )
    {
        for( row=dst, i=r->left; i<=r->right; ++i, row+=4 )
        {
            COLOR_COPY_INV( row, canvas->bg_color );
        }
    }
}

/****************************************************************************/
sgui_canvas* sgui_canvas_create( unsigned int width, unsigned int height )
{
    sgui_canvas_xlib* cv = malloc( sizeof(sgui_canvas_xlib) );
    char* buffer;

    if( !cv )
        return NULL;

    buffer = malloc( width * height * 4 );

    if( !buffer )
    {
        free( cv );
        return NULL;
    }

    cv->img = XCreateImage( dpy, CopyFromParent, 24, ZPixmap, 0,
                            buffer, width, height, 32, 0 );

    if( !cv->img )
    {
        free( buffer );
        free( cv );
        return NULL;
    }

    sgui_internal_canvas_init( (sgui_canvas*)cv, width, height );

    cv->canvas.download = canvas_xlib_download;
    cv->canvas.upload = canvas_xlib_upload;
    cv->canvas.clear = canvas_xlib_clear;

    return (sgui_canvas*)cv;
}

void sgui_canvas_destroy( sgui_canvas* canvas )
{
    if( ((sgui_canvas_xlib*)canvas)->img )
    {
        XDestroyImage( ((sgui_canvas_xlib*)canvas)->img );
    }

    free( canvas );
}

void sgui_canvas_resize( sgui_canvas* canvas, unsigned int width,
                         unsigned int height )
{
    unsigned int new_mem, old_mem;
    char* buffer;

    if( !canvas || !width || !height )
        return;

    new_mem = width * height;
    old_mem = canvas->width * canvas->height;

    if( new_mem > old_mem )
    {
        buffer = malloc( new_mem * 4 );
    }
    else
    {
        buffer = ((sgui_canvas_xlib*)canvas)->img->data;
        ((sgui_canvas_xlib*)canvas)->img->data = NULL;
    }

    XDestroyImage( ((sgui_canvas_xlib*)canvas)->img );

    ((sgui_canvas_xlib*)canvas)->img =
    XCreateImage( dpy, CopyFromParent, 24, ZPixmap, 0,
                  buffer, width, height, 32, 0 );

    canvas->width = width;
    canvas->height = height;
}

/************************ internal canvas functions ************************/
void display_canvas( Window wnd, GC gc, sgui_canvas* cv, int x, int y,
                     unsigned int width, unsigned int height )
{
    if( !cv || !gc )
        return;

    XPutImage( dpy, wnd, gc, ((sgui_canvas_xlib*)cv)->img,
               x, y, x, y, width, height );
}

