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
 


#define IS_OUTSIDE_SCISSOR_RECT( x, y, w, h, canvas ) \
           ( (x)>(canvas)->sc.right || (y)>(canvas)->sc.bottom ||\
            ((x)+(int)(w))<(canvas)->sc.left || ((y)+(int)(h))<(canvas)->sc.top )

#define NEED_CHANNEL_SWAP( f ) ((f)==SCF_RGBA8 || (f)==SCF_RGB8)

#define HAS_ALPHA_CHANNEL( f ) ((f)==SCF_RGBA8)

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

/***************************** Canvas functions *****************************/
sgui_canvas* sgui_canvas_create( unsigned int width, unsigned int height )
{
    sgui_canvas* cv = malloc( sizeof(sgui_canvas) );

    if( !cv )
        return NULL;

    memset( cv, 0, sizeof(sgui_canvas) );

    cv->dc = CreateCompatibleDC( NULL );

    if( !cv->dc )
    {
        free( cv );
        return NULL;
    }

    cv->info.bmiHeader.biSize        = sizeof(cv->info.bmiHeader);
    cv->info.bmiHeader.biBitCount    = 32;
    cv->info.bmiHeader.biCompression = BI_RGB;
    cv->info.bmiHeader.biPlanes      = 1;
    cv->info.bmiHeader.biWidth       = width;
    cv->info.bmiHeader.biHeight      = -((int)height);

    cv->bitmap = CreateDIBSection( cv->dc, &cv->info, DIB_RGB_COLORS,
                                   &cv->data, 0, 0 );

    if( !cv->bitmap )
    {
        DeleteDC( cv->dc );
        free( cv );
        return NULL;
    }

    SelectObject( cv->dc, cv->bitmap );

    cv->width  = width;
    cv->height = height;
    cv->clear  = 1;

    SetRect( &cv->sc, 0, 0, width, height );

    return cv;
}

void sgui_canvas_destroy( sgui_canvas* canvas )
{
    if( canvas )
    {
        if( canvas->bg_brush )
            DeleteObject( (HGDIOBJ)canvas->bg_brush );

        if( canvas->dc )
        {
            SelectObject( canvas->dc, 0 );
            DeleteObject( canvas->bitmap );
            DeleteDC( canvas->dc );
        }

        free( canvas );
    }
}

void sgui_canvas_resize( sgui_canvas* canvas, unsigned int width,
                         unsigned int height )
{
    if( canvas && width && height )
    {
        canvas->info.bmiHeader.biWidth  = width;
        canvas->info.bmiHeader.biHeight = -((int)height);

        SelectObject( canvas->dc, 0 );
        DeleteObject( canvas->bitmap );

        canvas->bitmap = CreateDIBSection( canvas->dc, &canvas->info,
                                           DIB_RGB_COLORS,
                                           &canvas->data, 0, 0 );

        SelectObject( canvas->dc, canvas->bitmap );

        canvas->width  = width;
        canvas->height = height;

        SetRect( &canvas->sc, 0, 0, width, height );
    }
}




void sgui_canvas_get_size( sgui_canvas* canvas, unsigned int* width,
                           unsigned int* height )
{
    if( width  ) *width  = canvas ? canvas->width  : 0;
    if( height ) *height = canvas ? canvas->height : 0;
}

void sgui_canvas_set_background_color( sgui_canvas* canvas,
                                       unsigned char* color,
                                       SGUI_COLOR_FORMAT format )
{
    COLORREF c;

    if( canvas && color )
    {
        if( canvas->bg_brush )
            DeleteObject( (HGDIOBJ)canvas->bg_brush );

        if( format==SCF_RGB8 || format==SCF_RGBA8 )
            c = RGB( color[0], color[1], color[2] );
        else
            c = RGB( color[2], color[1], color[0] );

        canvas->bg_brush = CreateSolidBrush( c );
    }
}

void sgui_canvas_clear( sgui_canvas* canvas, int x, int y,
                        unsigned int width, unsigned int height )
{
    RECT r, r0;

    if( !canvas || !canvas->clear )
        return;

    x += canvas->ox;
    y += canvas->oy;

    SetRect( &r0, x, y, x+width, y+height );

    if( IntersectRect( &r, &canvas->sc, &r0 ) )
        FillRect( canvas->dc, &r, canvas->bg_brush );
}



void sgui_canvas_set_scissor_rect( sgui_canvas* canvas, int x, int y,
                                   unsigned int width, unsigned int height )
{
    RECT r, r0;

    if( canvas )
    {
        x += canvas->ox;
        y += canvas->oy;

        if( width && height )
        {
            if( canvas->scissor_stack_pointer == SGUI_CANVAS_STACK_DEPTH )
                return;

            /* push current scissor rect */
            CopyRect( canvas->sc_stack+canvas->scissor_stack_pointer,
                      &canvas->sc );

            ++(canvas->scissor_stack_pointer);

            /* merge rectangles */
            SetRect( &r0, x, y, x+width, y+height );
            IntersectRect( &r, &canvas->sc, &r0 );
            CopyRect( &canvas->sc, &r );
        }
        else
        {
            if( canvas->scissor_stack_pointer )
            {
                /* pop old scissor rect from stack */
                --(canvas->scissor_stack_pointer);

                CopyRect( &canvas->sc,
                          canvas->sc_stack+canvas->scissor_stack_pointer );
            }
            else
            {
                SetRect( &canvas->sc, 0, 0, canvas->width, canvas->height );
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
    RECT r, r0;

    if( !canvas || !width || !height || !data )
        return;

    x += canvas->ox;
    y += canvas->oy;

    SetRect( &r0, x, y, x+width, y+height );

    /* don't blit if outside the drawing area */
    if( !IntersectRect( &r, &canvas->sc, &r0 ) )
        return;

    /* color format checks */
    if( HAS_ALPHA_CHANNEL( format ) )
        src_bpp = 4;

    /* compute source and destination pointers */
    dst = ((unsigned char*)canvas->data) + (r.top*canvas->width + r.left)*4;

    src = (unsigned char*)data;
    src += ((r.top-r0.top)*canvas->width + r.left-r0.left)*4;

    ds = width * src_bpp;
    dt = canvas->width * 4;

    /* do the blit */
    if( NEED_CHANNEL_SWAP( format ) )
    {
        for( j=r.top; j!=r.bottom; ++j, src+=ds, dst+=dt )
        {
            for( drow=dst, srow=src, i=r.left; i!=r.right; ++i, drow+=4,
                                                           srow+=src_bpp )
            {
                COLOR_COPY_INV( drow, srow );
            }
        }
    }
    else
    {
        for( j=r.top; j!=r.bottom; ++j, src+=ds, dst+=dt )
        {
            for( drow=dst, srow=src, i=r.left; i!=r.right; ++i, drow+=4,
                                                           srow+=src_bpp )
            {
                COLOR_COPY( drow, srow );
            }
        }
    }
}

void sgui_canvas_blend( sgui_canvas* canvas, int x, int y, unsigned int width,
                        unsigned int height, SGUI_COLOR_FORMAT format,
                        const void* data )
{
    unsigned char *dst, *src, *drow, *srow, A, iA;
    unsigned int i, j, ds, dt;
    RECT r, r0;

    if( !canvas || !width || !height || !data )
        return;

    if( !HAS_ALPHA_CHANNEL( format ) )
        return;

    x += canvas->ox;
    y += canvas->oy;

    SetRect( &r0, x, y, x+width, y+height );

    /* don't blend outside the drawing area */
    if( !IntersectRect( &r, &canvas->sc, &r0 ) )
        return;

    /* compute source and destination pointers */
    dst = ((unsigned char*)canvas->data) + (r.top*canvas->width + r.left)*4;

    src = (unsigned char*)data;
    src += ((r.top-r0.top)*canvas->width + r.left-r0.left)*4;

    ds = width * 4;
    dt = canvas->width * 4;

    /* do the blend */
    if( NEED_CHANNEL_SWAP( format ) )
    {
        for( j=0; j<height; ++j, src+=ds, dst+=dt )
        {
            for( drow=dst, srow=src, i=0; i<width; ++i, drow+=4, srow+=4 )
            {
                A = srow[3], iA = 255-A;

                COLOR_BLEND_INV( drow, srow, A, iA );
            }
        }
    }
    else
    {
        for( j=0; j<height; ++j, src+=ds, dst+=dt )
        {
            for( drow=dst, srow=src, i=0; i<width; ++i, drow+=4, srow+=4 )
            {
                A = srow[3], iA = 255-A;

                COLOR_BLEND( drow, srow, A, iA );
            }
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
    COLORREF ref;
    HBRUSH brush;
    RECT r0, r;

    if( !canvas || !color )
        return;

    /* Determine rect to draw to */
    x += canvas->ox;
    y += canvas->oy;

    SetRect( &r0, x, y, x+width, y+height );

    if( !IntersectRect( &r, &canvas->sc, &r0 ) )
        return;

    /* draw */
    if( HAS_ALPHA_CHANNEL( format ) )
    {
        if( NEED_CHANNEL_SWAP( format ) )
        {
            COLOR_COPY_INV( c, color );
        }
        else
        {
            COLOR_COPY( c, color );
        }

        dst = ((unsigned char*)canvas->data) + (r.top*canvas->width+r.left)*4;

        A = color[3];
        iA = 255 - A;

        for( j=r.top; j!=r.bottom; ++j, dst+=canvas->width*4 )
        {
            for( row=dst, i=r.left; i!=r.right; ++i, row+=4 )
            {
                COLOR_BLEND( row, c, A, iA );
            }
        }
    }
    else
    {
        ref = format==SCF_RGB8 ? RGB( color[0], color[1], color[2] ) :
                                 RGB( color[2], color[1], color[0] );

        brush = CreateSolidBrush( ref );
        FillRect( canvas->dc, &r, brush );
        DeleteObject( brush );
    }
}

void sgui_canvas_draw_line( sgui_canvas* canvas, int x, int y,
                            unsigned int length, int horizontal,
                            unsigned char* color, SGUI_COLOR_FORMAT format )
{
    unsigned char* dst;
    unsigned char c[3], A, iA;
    int i;
    POINT pt[2];

    if( !canvas )
        return;

    pt[0].x = x + canvas->ox;
    pt[0].y = y + canvas->oy;
    pt[1].x = horizontal ? (pt[0].x+(int)length) :  pt[0].x;
    pt[1].y = horizontal ?  pt[0].y              : (pt[0].y+(int)length);

    if( !PtInRect( &canvas->sc, pt[0] ) && !PtInRect( &canvas->sc, pt[1] ) )
        return;

    if( HAS_ALPHA_CHANNEL( format ) )
    {
        if( NEED_CHANNEL_SWAP( format ) )
        {
            COLOR_COPY_INV( c, color );
        }
        else
        {
            COLOR_COPY( c, color );
        }

        A = color[3];
        iA = 255 - A;

        if( pt[0].x < canvas->sc.left   ) pt[0].x = canvas->sc.left;
        if( pt[0].y < canvas->sc.top    ) pt[0].y = canvas->sc.top;
        if( pt[1].x > canvas->sc.right  ) pt[1].x = canvas->sc.right;
        if( pt[1].y > canvas->sc.bottom ) pt[1].y = canvas->sc.bottom;

        dst = (unsigned char*)canvas->data +
              (pt[0].y*canvas->width+pt[0].x)*4;

        if( horizontal )
        {
            for( i=pt[0].x; i!=pt[1].x; ++i, dst+=4 )
            {
                COLOR_BLEND( dst, c, A, iA );
            }
        }
        else
        {
            for( i=pt[0].y; i!=pt[1].y; ++i, dst+=canvas->width*4 )
            {
                COLOR_BLEND( dst, c, A, iA );
            }
        }
    }
    else
    {
        COLORREF ref = format==SCF_RGB8 ? RGB(color[0], color[1], color[2]) :
                                          RGB(color[2], color[1], color[0]);

        HPEN pen = CreatePen( PS_SOLID, 1, ref );
        HGDIOBJ old = SelectObject( canvas->dc, pen );

        Polyline( canvas->dc, pt, 2 );
        SelectObject( canvas->dc, old );
        DeleteObject( pen );
    }
}

int sgui_canvas_blend_stencil( sgui_canvas* canvas, unsigned char* buffer,
                               int x, int y, unsigned int w, unsigned int h,
                               unsigned char* color )
{
    unsigned char A, iA, *src, *dst, *row;
    unsigned int i, j, ds, dt, delta;

    x += canvas->ox;
    y += canvas->oy;

    if( (x+(int)w)<(int)canvas->sc.left || (y+(int)h)<(int)canvas->sc.top )
        return -1;

    if( x>=canvas->sc.right || y>=canvas->sc.bottom )
        return 1;

    /* adjust parameters to only blend visible portion */
    dst = ((unsigned char*)canvas->data) + (y*canvas->width + x)*4;

    if( y<canvas->sc.top )
    {
        delta = canvas->sc.top-y;

        dst += delta*canvas->width*4;
        buffer += delta*w;
        h -= delta;
        y = canvas->sc.top;
    }

    if( (y+(int)h) > canvas->sc.bottom )
        h = canvas->sc.bottom - y;

    ds = w;
    dt = canvas->width*4;

    if( x < canvas->sc.left )
    {
        delta = canvas->sc.left - x;

        w -= delta;
        buffer += delta;
        x = canvas->sc.left;
    }

    if( (x+(int)w) >= canvas->sc.right )
        w = canvas->sc.right - x;

    /* do the blend */
    for( j=0; j<h; ++j, buffer+=ds, dst+=dt )
    {
        for( src=buffer, row=dst, i=0; i<w; ++i, row+=4, ++src )
        {
            A = *src;
            iA = 255-A;

            COLOR_BLEND_INV( row, color, A, iA );
        }
    }

    return 0;
}

