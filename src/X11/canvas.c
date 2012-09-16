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

/************************ internal canvas functions ************************/
sgui_canvas* sgui_canvas_create( unsigned int width, unsigned int height,
                                 Display* dpy )
{
    sgui_canvas* cv = malloc( sizeof(sgui_canvas) );

    if( !cv )
        return NULL;

    memset( cv, 0, sizeof(sgui_canvas) );

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

    cv->clear = 1;

    sgui_rect_set_size( &cv->sc, 0, 0, width, height );

    return cv;
}

void sgui_canvas_destroy( sgui_canvas* canvas )
{
    if( canvas )
    {
        if( canvas->img )
        {
            canvas->img->data = NULL;
            XDestroyImage( canvas->img );
        }

        free( canvas->data );
        free( canvas );
    }
}

void sgui_canvas_resize( sgui_canvas* canvas, unsigned int width,
                         unsigned int height, Display* dpy )
{
    unsigned int new_mem, old_mem;

    if( !canvas || !width || !height )
        return;

    new_mem = width*height;
    old_mem = canvas->img->width*canvas->img->height;

    if( new_mem != old_mem )
    {
        free( canvas->data );
        canvas->data = malloc( new_mem * 4 );
    }

    canvas->img->data = NULL;
    XDestroyImage( canvas->img );

    canvas->img = XCreateImage( dpy, CopyFromParent, 24, ZPixmap, 0,
                                (char*)canvas->data, width, height, 32, 0 );

    sgui_rect_set_size( &canvas->sc, 0, 0, width, height );
}

/************************* public canvas functions *************************/
void sgui_canvas_get_size( sgui_canvas* canvas, unsigned int* width,
                           unsigned int* height )
{
    if( width  ) *width  = canvas ? canvas->img->width  : 0;
    if( height ) *height = canvas ? canvas->img->height : 0;
}

void sgui_canvas_set_background_color( sgui_canvas* canvas,
                                       unsigned char* color )
{
    if( canvas && color )
    {
        COLOR_COPY_INV( canvas->bg_color, color );
    }
}

void sgui_canvas_clear( sgui_canvas* canvas, int x, int y,
                        unsigned int width, unsigned int height )
{
    unsigned char *dst, *row;
    int i, j;
    sgui_rect r;

    if( !canvas || !canvas->clear )
        return;

    sgui_rect_set_size( &r, x+canvas->ox, y+canvas->oy, width, height );

    if( !sgui_rect_get_intersection( &r, &canvas->sc, &r ) )
        return;

    dst = (unsigned char*)canvas->data + (r.top*canvas->img->width+r.left)*4;

    /* clear */
    for( j=r.top; j<=r.bottom; ++j, dst+=canvas->img->width*4 )
    {
        for( row=dst, i=r.left; i<=r.right; ++i, row+=4 )
        {
            COLOR_COPY( row, canvas->bg_color );
        }
    }
}

void sgui_canvas_set_scissor_rect( sgui_canvas* canvas, int x, int y,
                                   unsigned int width, unsigned int height )
{
    sgui_rect r;

    if( canvas )
    {
        if( width && height )
        {
            if( canvas->scissor_stack_pointer == SGUI_CANVAS_STACK_DEPTH )
                return;

            /* push current scissor rect */
            sgui_rect_copy( canvas->sc_stack + canvas->scissor_stack_pointer,
                            &canvas->sc );

            ++(canvas->scissor_stack_pointer);

            /* merge rectangles */
            sgui_rect_set_size( &r, x+canvas->ox, y+canvas->oy,
                                    width, height );

            sgui_rect_get_intersection( &r, &canvas->sc, &r );

            /* set scissor rect */
            sgui_rect_copy( &canvas->sc, &r );
        }
        else
        {
            if( canvas->scissor_stack_pointer )
            {
                /* pop old scissor rect from stack */
                --(canvas->scissor_stack_pointer);

                sgui_rect_copy( &canvas->sc,
                                canvas->sc_stack+
                                canvas->scissor_stack_pointer );
            }
            else
            {
                sgui_rect_set_size( &canvas->sc, 0, 0, width, height );
            }
        }
    }
}

void sgui_canvas_allow_clear( sgui_canvas* canvas, int clear )
{
    if( canvas )
        canvas->clear = clear;
}

int sgui_canvas_is_clear_allowed( sgui_canvas* canvas )
{
    return canvas ? canvas->clear : 0;
}

void sgui_canvas_set_offset( sgui_canvas* canvas, int x, int y )
{
    if( canvas )
    {
        if( canvas->offset_stack_pointer == SGUI_CANVAS_STACK_DEPTH )
            return;

        canvas->offset_stack_x[ canvas->offset_stack_pointer ] = canvas->ox;
        canvas->offset_stack_y[ canvas->offset_stack_pointer ] = canvas->oy;

        canvas->ox += x;
        canvas->oy += y;

        ++(canvas->offset_stack_pointer);
    }
}

void sgui_canvas_restore_offset( sgui_canvas* canvas )
{
    if( canvas )
    {
        if( canvas->offset_stack_pointer )
        {
            --(canvas->offset_stack_pointer);

            canvas->ox = canvas->offset_stack_x[canvas->offset_stack_pointer];
            canvas->oy = canvas->offset_stack_y[canvas->offset_stack_pointer];
        }
        else
        {
            canvas->ox = 0;
            canvas->oy = 0;
        }
    }
}

void sgui_canvas_blit( sgui_canvas* canvas, int x, int y, unsigned int width,
                       unsigned int height, SGUI_COLOR_FORMAT format,
                       const void* data )
{
    unsigned char *dst, *src, *drow, *srow;
    unsigned int ds, dt;
    int i, j, src_bpp = 3;
    sgui_rect r, r0;

    if( !canvas || !width || !height || !data )
        return;

    sgui_rect_set_size( &r0, x+canvas->ox, y+canvas->oy, width, height );

    if( !sgui_rect_get_intersection( &r, &canvas->sc, &r0 ) )
        return;

    /* color format checks */
    if( format==SCF_RGBA8 )
        src_bpp = 4;

    /* compute source and destination pointers */
    dst = ((unsigned char*)canvas->data)+(r.top*canvas->img->width+r.left)*4;
    src = (unsigned char*)data +
          ((r.top-r0.top)*width + r.left-r0.left)*src_bpp;

    ds = width*src_bpp;
    dt = canvas->img->width*4;

    /* do the blit */
    for( j=r.top; j<=r.bottom; ++j, src+=ds, dst+=dt )
    {
        for( drow=dst, srow=src, i=r.left; i<=r.right; ++i, drow+=4,
                                                       srow+=src_bpp )
        {
            COLOR_COPY_INV( drow, srow );
        }
    }
}

void sgui_canvas_blend( sgui_canvas* canvas, int x, int y, unsigned int width,
                        unsigned int height, SGUI_COLOR_FORMAT format,
                        const void* data )
{
    unsigned char *dst, *src, *drow, *srow, A, iA;
    unsigned int ds, dt;
    int i, j;
    sgui_rect r, r0;

    if( !canvas || !width || !height || !data || format!=SCF_RGBA8 )
        return;

    sgui_rect_set_size( &r0, x+canvas->ox, y+canvas->oy, width, height );

    if( !sgui_rect_get_intersection( &r, &canvas->sc, &r0 ) )
        return;

    /* compute source and destination pointers */
    dst = ((unsigned char*)canvas->data)+(r.top*canvas->img->width+r.left)*4;
    src = (unsigned char*)data + ((r.top-r0.top)*width + r.left-r0.left)*4;

    ds = width*4;
    dt = canvas->img->width*4;

    /* do the blit */
    for( j=r.top; j<=r.bottom; ++j, src+=ds, dst+=dt )
    {
        for( drow=dst, srow=src, i=r.left; i<=r.right; ++i, drow+=4, srow+=4 )
        {
            A = srow[3];
            iA = 0xFF-A;

            COLOR_BLEND_INV( drow, srow, A, iA );
        }
    }
}

void sgui_canvas_draw_box( sgui_canvas* canvas, int x, int y,
                           unsigned int width, unsigned int height,
                           unsigned char* color, SGUI_COLOR_FORMAT format )
{
    unsigned char c[3], A, iA;
    unsigned char *dst, *row;
    int i, j;
    sgui_rect r;

    if( !canvas || !color )
        return;

    sgui_rect_set_size( &r, x+canvas->ox, y+canvas->oy, width, height );

    if( !sgui_rect_get_intersection( &r, &canvas->sc, &r ) )
        return;

    COLOR_COPY_INV( c, color );

    dst = (unsigned char*)canvas->data + (r.top*canvas->img->width+r.left)*4;

    if( format==SCF_RGBA8 )
    {
        A = color[3];
        iA = 255 - A;

        for( j=r.top; j<=r.bottom; ++j, dst+=canvas->img->width*4 )
        {
            for( row=dst, i=r.left; i<=r.right; ++i, row+=4 )
            {
                COLOR_BLEND( row, c, A, iA );
            }
        }
    }
    else
    {
        for( j=r.top; j<=r.bottom; ++j, dst+=canvas->img->width*4 )
        {
            for( row=dst, i=r.left; i<=r.right; ++i, row+=4 )
            {
                COLOR_COPY( row, c );
            }
        }
    }
}

void sgui_canvas_draw_line( sgui_canvas* canvas, int x, int y,
                            unsigned int length, int horizontal,
                            unsigned char* color, SGUI_COLOR_FORMAT format )
{
    unsigned char* dst;
    unsigned char c[3], A, iA;
    unsigned int i, delta;

    if( !canvas )
        return;

    x += canvas->ox;
    y += canvas->oy;

    if( !sgui_rect_clip_line( &canvas->sc, horizontal, &x, &y, &length ) )
        return;

    COLOR_COPY_INV( c, color );

    dst = (unsigned char*)canvas->data + (y*canvas->img->width + x)*4;
    delta = horizontal ? 4 : canvas->img->width*4;

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

int sgui_canvas_blend_stencil( sgui_canvas* canvas, unsigned char* buffer,
                               int x, int y, unsigned int w, unsigned int h,
                               unsigned char* color )
{
    unsigned char A, iA, *src, *dst, *row;
    int i, j;
    sgui_rect r;

    if( !canvas || !buffer || !color || !w || !h )
        return 0;

    x+=canvas->ox;
    y+=canvas->oy;

    if( (x+(int)w-1)<canvas->sc.left || (y+(int)h-1)<canvas->sc.top )
        return -1;

    if( x>canvas->sc.right || y>canvas->sc.bottom )
        return 1;

    sgui_rect_set_size( &r, x, y, w, h );
    sgui_rect_get_intersection( &r, &canvas->sc, &r );

    /* compute source and destination buffer pointer */
    dst = ((unsigned char*)canvas->data)+(r.top*canvas->img->width+r.left)*4;

    dst    += (r.top - y) * canvas->img->width*4;
    buffer += (r.top - y) * w + (r.left - x);

    /* do the blend */
    for( j=r.top; j<=r.bottom; ++j, buffer+=w, dst+=canvas->img->width*4 )
    {
        for( src=buffer, row=dst, i=r.left; i<=r.right; ++i, row+=4, ++src )
        {
            A = *src;
            iA = 255-A;

            COLOR_BLEND_INV( row, color, A, iA );
        }
    }

    return 0;
}

