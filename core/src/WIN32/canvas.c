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

    void* data;
    HDC dc;
    BITMAPINFO info;
    HBITMAP bitmap;
}
sgui_canvas_gdi;

/************************* public canvas functions *************************/
void canvas_gdi_download( sgui_canvas* canvas, sgui_rect* r )
{
    int i, j, delta;
    unsigned char temp, *ptr, *row;

    canvas->buffer = ((sgui_canvas_gdi*)canvas)->data;
    canvas->buffer_x = 0;
    canvas->buffer_y = 0;
    canvas->buffer_w = canvas->width;
    canvas->buffer_h = canvas->height;

    sgui_rect_copy( &((sgui_canvas_gdi*)canvas)->locked, r );

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

void canvas_gdi_upload( sgui_canvas* canvas )
{
    sgui_canvas_gdi* cv = (sgui_canvas_gdi*)canvas;
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

    cv->canvas.upload = canvas_gdi_upload;
    cv->canvas.download = canvas_gdi_download;
    cv->canvas.clear = canvas_gdi_clear;

    return (sgui_canvas*)cv;
}

void canvas_gdi_destroy( sgui_canvas* canvas )
{
    if( canvas )
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
}

void canvas_gdi_resize( sgui_canvas* canvas, unsigned int width,
                        unsigned int height )
{
    if( canvas && width && height )
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

        /* store adjusted parameters */
        canvas->width  = width;
        canvas->height = height;
    }
}

void canvas_gdi_display( HDC dc, sgui_canvas* cv, int x, int y,
                         unsigned int width, unsigned int height )
{
    BitBlt( dc, x, y, width, height, ((sgui_canvas_gdi*)cv)->dc,
            x, y, SRCCOPY );
}

