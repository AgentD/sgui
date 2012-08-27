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
#include "sgui_canvas.h"
#include "sgui_font_manager.h"

#include <stdlib.h>
#include <string.h>



struct sgui_canvas
{
    unsigned int width, height;
    void* data;
    SGUI_COLOR_FORMAT format;
    unsigned int bpp;
    int own_buffer;
};



sgui_canvas* sgui_canvas_create( unsigned int width, unsigned int height,
                                 SGUI_COLOR_FORMAT format )
{
    sgui_canvas* cv = malloc( sizeof(sgui_canvas) );
    unsigned int bpp = 3;

    if( !cv )
        return NULL;

    if( format==SCF_RGBA8 || format==SCF_BGRA8 )
        bpp = 4;

    cv->width      = width;
    cv->height     = height;
    cv->data       = malloc( width * height * bpp );
    cv->format     = format;
    cv->bpp        = bpp;
    cv->own_buffer = 1;

    if( !cv->data )
    {
        free( cv );
        return NULL;
    }

    return cv;
}

sgui_canvas* sgui_canvas_create_use_buffer( void* buffer,
                                            unsigned int width,
                                            unsigned int height,
                                            SGUI_COLOR_FORMAT format )
{
    sgui_canvas* cv = malloc( sizeof(sgui_canvas) );
    unsigned int bpp = 3;

    if( !cv )
        return NULL;

    if( format==SCF_RGBA8 || format==SCF_BGRA8 )
        bpp = 4;

    cv->width      = width;
    cv->height     = height;
    cv->data       = buffer;
    cv->format     = format;
    cv->bpp        = bpp;
    cv->own_buffer = 0;

    return cv;
}

void sgui_canvas_destroy( sgui_canvas* canvas )
{
    if( canvas )
    {
        if( canvas->own_buffer )
            free( canvas->data );

        free( canvas );
    }
}

void sgui_canvas_get_size( sgui_canvas* canvas, unsigned int* width,
                           unsigned int* height )
{
    if( canvas )
    {
        if( width  ) *width  = canvas->width;
        if( height ) *height = canvas->height;
    }
    else
    {
        if( width  ) *width  = 0;
        if( height ) *height = 0;
    }
}

void sgui_canvas_resize( sgui_canvas* canvas, unsigned int width,
                         unsigned int height )
{
    unsigned int new_mem, old_mem;

    if( !canvas || (canvas->width==width && height==canvas->height) )
        return;

    if( !canvas->own_buffer )
        return;

    new_mem = width*height;
    old_mem = canvas->width*canvas->height;

    if( new_mem != old_mem )
        canvas->data = realloc( canvas->data, new_mem * canvas->bpp );

    canvas->width = width;
    canvas->height = height;
}

void sgui_canvas_blit( sgui_canvas* canvas, int x, int y, unsigned int width,
                       unsigned int height, SGUI_COLOR_FORMAT format,
                       const void* data )
{
    unsigned char* dst;
    unsigned char* src;
    unsigned int i, j, src_bpp = 3, R = 0, G = 1, B = 2;

    if( !canvas || !width || !height || !data )
        return;

    if( format==SCF_RGBA8 || format==SCF_BGRA8 )
        src_bpp = 4;

    if( ((format==SCF_RGB8 || format==SCF_RGBA8) &&
         (canvas->format==SCF_BGR8 || canvas->format==SCF_BGRA8)) ||
        ((format==SCF_BGR8 || format==SCF_BGRA8) &&
         (canvas->format==SCF_RGB8 || canvas->format==SCF_RGBA8)) )
    {
        R = 2;
        G = 1;
        B = 0;
    }

    dst = ((unsigned char*)canvas->data) + (y*canvas->width + x)*canvas->bpp;
    src = (unsigned char*)data;

    for( j=0; j<height; ++j, dst+=canvas->width*canvas->bpp,
                             src+=width*src_bpp )
    {
        if( (y+(int)j)<0 )
            continue;

        if( (y+(int)j)>=(int)canvas->height )
            break;

        for( i=0; i<width; ++i )
        {
            if( (x+(int)i)>0 && (x+(int)i)<(int)canvas->width )
            {
                dst[ i*canvas->bpp     ] = src[ i*src_bpp + R ];
                dst[ i*canvas->bpp + 1 ] = src[ i*src_bpp + G ];
                dst[ i*canvas->bpp + 2 ] = src[ i*src_bpp + B ];
            }
        }
    }
}

void sgui_canvas_blend( sgui_canvas* canvas, int x, int y, unsigned int width,
                        unsigned int height, SGUI_COLOR_FORMAT format,
                        const void* data )
{
    unsigned char* dst;
    unsigned char* src;
    unsigned int i, j, R = 0, G = 1, B = 2;
    float A;

    if( !canvas || !width || !height || !data )
        return;

    if( format!=SCF_RGBA8 && format!=SCF_BGRA8 )
        return;

    if( (format==SCF_RGBA8 &&
         (canvas->format==SCF_BGR8 || canvas->format==SCF_BGRA8)) ||
        (format==SCF_BGRA8 &&
         (canvas->format==SCF_RGB8 || canvas->format==SCF_RGBA8)) )
    {
        R = 2;
        G = 1;
        B = 0;
    }

    dst = ((unsigned char*)canvas->data) + (y*canvas->width + x)*canvas->bpp;
    src = (unsigned char*)data;

    for( j=0; j<height; ++j, dst+=canvas->width*canvas->bpp,
                             src+=width*4 )
    {
        if( (y+(int)j)<0 )
            continue;

        if( (y+(int)j)>=(int)canvas->height )
            break;

        for( i=0; i<width; ++i )
        {
            if( (x+(int)i)>0 && (x+(int)i)<(int)canvas->width )
            {
                A = ((float)src[ i*4 + 3 ]) / 255.0f;

                dst[ i*canvas->bpp     ] *= (1.0f-A);
                dst[ i*canvas->bpp + 1 ] *= (1.0f-A);
                dst[ i*canvas->bpp + 2 ] *= (1.0f-A);

                dst[ i*canvas->bpp     ] += A * src[ i*4 + R ];
                dst[ i*canvas->bpp + 1 ] += A * src[ i*4 + G ];
                dst[ i*canvas->bpp + 2 ] += A * src[ i*4 + B ];
            }
        }
    }
}

void sgui_canvas_draw_box( sgui_canvas* canvas, int x, int y,
                           unsigned int width, unsigned int height,
                           unsigned char* color, SGUI_COLOR_FORMAT format )
{
    unsigned char R=0, G=1, B=2;
    float A;
    unsigned int i, j;
    unsigned char* dst;
    unsigned char* row_ptr;

    if( canvas && color )
    {
        if( (y+(int)height) < 0 || (x+(int)width) < 0 )
            return;

        if( y >= (int)canvas->height || x >= (int)canvas->width )
            return;

        if( ((format==SCF_RGBA8 || format==SCF_RGB8) &&
             (canvas->format==SCF_BGR8 || canvas->format==SCF_BGRA8)) ||
            ((format==SCF_BGRA8 && format==SCF_BGR8) &&
             (canvas->format==SCF_RGB8 || canvas->format==SCF_RGBA8)) )
        {
            R = color[2];
            G = color[1];
            B = color[0];
        }
        else
        {
            R = color[0];
            G = color[1];
            B = color[2];
        }

        if( format==SCF_RGBA8 || format==SCF_BGRA8 )
            A = ((float)color[3]) / 255.0f;

        if( y<0 ) { height -= (unsigned int)(-y); y = 0; }
        if( x<0 ) { width  -= (unsigned int)(-x); x = 0; }

        if( (y+((int)height-1)) >= (int)canvas->height )
            height = canvas->height - (unsigned int)y;

        if( (x+((int)width-1)) >= (int)canvas->width )
            width = canvas->width - (unsigned int)x;

        dst = (unsigned char*)canvas->data + (y*canvas->width+x)*canvas->bpp;

        if( format==SCF_RGBA8 || format==SCF_BGRA8 )
        {
            for( j=0; j<height; ++j, dst+=canvas->width*canvas->bpp )
            {
                row_ptr = dst;

                for( i=0; i<width; ++i, row_ptr+=canvas->bpp )
                {
                    row_ptr[0] *= 1.0f-A;
                    row_ptr[1] *= 1.0f-A;
                    row_ptr[2] *= 1.0f-A;

                    row_ptr[0] += R * A;
                    row_ptr[1] += G * A;
                    row_ptr[2] += B * A;
                }
            }
        }
        else
        {
            for( j=0; j<height; ++j, dst+=canvas->width*canvas->bpp )
            {
                row_ptr = dst;

                for( i=0; i<width; ++i, row_ptr+=canvas->bpp )
                {
                    row_ptr[0] = R;
                    row_ptr[1] = G;
                    row_ptr[2] = B;
                }
            }
        }
    }
}

void sgui_canvas_draw_line( sgui_canvas* canvas, int x, int y,
                            unsigned int length, int horizontal,
                            unsigned char* color, SGUI_COLOR_FORMAT format )
{
    unsigned char* dst;
    unsigned char R, G, B;
    float A;
    unsigned int i;

    if( canvas )
    {
        if( ((format==SCF_RGBA8 || format==SCF_RGB8) &&
             (canvas->format==SCF_BGR8 || canvas->format==SCF_BGRA8)) ||
            ((format==SCF_BGRA8 && format==SCF_BGR8) &&
             (canvas->format==SCF_RGB8 || canvas->format==SCF_RGBA8)) )
        {
            R = color[2];
            G = color[1];
            B = color[0];
        }
        else
        {
            R = color[0];
            G = color[1];
            B = color[2];
        }

        if( format==SCF_RGBA8 || format==SCF_BGRA8 )
            A = ((float)color[3]) / 255.0f;

        if( horizontal )
        {
            if( (y<0) || (((unsigned int)y)>=canvas->height) )
                return;

            if( (x+(int)length)<0 || ((unsigned int)x)>canvas->width )
                return;

            if( x<0 )
            {
                length -= (unsigned int)(-x);
                x = 0;
            }

            if( ((unsigned int)x+length) > canvas->width )
                length = canvas->width - x;

            dst = (unsigned char*)canvas->data +
                  (y*canvas->width+x)*canvas->bpp;

            if( format==SCF_RGBA8 || format==SCF_BGRA8 )
            {
                for( i=0; i<length; ++i, dst+=canvas->bpp )
                {
                    dst[0] = R*A + (1.0f-A)*dst[0];
                    dst[1] = G*A + (1.0f-A)*dst[1];
                    dst[2] = B*A + (1.0f-A)*dst[2];
                }
            }
            else
            {
                for( i=0; i<length; ++i, dst+=canvas->bpp )
                {
                    dst[0] = R;
                    dst[1] = G;
                    dst[2] = B;
                }
            }
        }
        else
        {
            if( (x<0) || (((unsigned int)x)>=canvas->width) )
                return;

            if( (y+(int)length)<0 || ((unsigned int)y)>canvas->height )
                return;

            if( y<0 )
            {
                length -= (unsigned int)(-y);
                y = 0;
            }

            dst = (unsigned char*)canvas->data +
                  (y*canvas->width+x)*canvas->bpp;

            if( ((unsigned int)y+length) > canvas->height )
                length = canvas->height - y;

            if( format==SCF_RGBA8 || format==SCF_BGRA8 )
            {

                for( i=0; i<length; ++i, dst+=canvas->width*canvas->bpp )
                {
                    dst[0] = R*A + (1.0f-A)*dst[0];
                    dst[1] = G*A + (1.0f-A)*dst[1];
                    dst[2] = B*A + (1.0f-A)*dst[2];
                }
            }
            else
            {
                for( i=0; i<length; ++i, dst+=canvas->width*canvas->bpp )
                {
                    dst[0] = R;
                    dst[1] = G;
                    dst[2] = B;
                }
            }
        }
    }
}

void sgui_canvas_draw_text_plain( sgui_canvas* canvas, int x, int y,
                                  sgui_font* font_face, unsigned int height,
                                  unsigned char* color,
                                  SGUI_COLOR_FORMAT format,
                                  const unsigned char* text,
                                  unsigned int length,
                                  unsigned int max_width )
{
    unsigned char rgb[3];
    int alpha = canvas->format==SCF_RGBA8 || canvas->format==SCF_BGRA8;

    if( !canvas )
        return;

    if( format==SCF_BGR8 || format==SCF_BGRA8 )
    {
        rgb[0] = color[2];
        rgb[1] = color[1];
        rgb[2] = color[0];

        sgui_font_print( text, font_face, height, canvas->data, x, y,
                         canvas->width, canvas->height, x+max_width,
                         rgb, length, alpha );
    }
    else
    {
        sgui_font_print( text, font_face, height, canvas->data, x, y,
                         canvas->width, canvas->height, x+max_width,
                         color, length, alpha );
    }
}

void sgui_canvas_draw_text( sgui_canvas* canvas, int x, int y,
                            sgui_font* font_norm, sgui_font* font_bold,
                            sgui_font* font_ital, sgui_font* font_boit,
                            unsigned int font_height, unsigned char* color,
                            SGUI_COLOR_FORMAT format,
                            const unsigned char* text )
{
    int i = 0, X = 0, font_stack_index = 0;
    sgui_font* f = font_norm;
    sgui_font* font_stack[10];
    unsigned char col[3];
    long c;

    if( !canvas || !font_norm || !font_height || !color || !text )
        return;

    col[0] = color[0];
    col[1] = color[1];
    col[2] = color[2];

    for( ; text && text[ i ]; ++i )
    {
        if( text[ i ] == '<' )
        {
            sgui_canvas_draw_text_plain( canvas, x+X, y, f, font_height, col,
                                         format, text, i, 0xFFFF );

            X += sgui_font_extents( text, f, font_height, i );

            if( !strncmp( (const char*)text+i+1, "color=", 6 ) )
            {
                if( !strncmp( (const char*)text+i+9, "default", 7 ) )
                {
                    col[0] = color[0];
                    col[1] = color[1];
                    col[2] = color[2];
                }
                else
                {
                    c = strtol( (const char*)text+i+9, NULL, 16 );

                    if( format==SCF_RGBA8 || format==SCF_RGB8 )
                    {
                        col[0] = (c>>16) & 0xFF;
                        col[1] = (c>>8 ) & 0xFF;
                        col[2] =  c      & 0xFF;
                    }
                    else
                    {
                        col[2] = (c>>16) & 0xFF;
                        col[1] = (c>>8 ) & 0xFF;
                        col[0] =  c      & 0xFF;
                    }
                }
            }
            else if( text[ i+1 ] == 'b' )
            {
                font_stack[ font_stack_index++ ] = f;
                f = f==font_ital ? font_boit : font_bold;
            }
            else if( text[ i+1 ] == 'i' )
            {
                font_stack[ font_stack_index++ ] = f;
                f = f==font_bold ? font_boit : font_ital;
            }
            else if( text[ i+1 ] == '/' && font_stack_index )
            {
                f = font_stack[ --font_stack_index ];
            }

            text = (const unsigned char*)strchr( (const char*)text+i, '>' );

            if( text )
                ++text;

            i = -1;
        }
        else if( text[ i ] == '\n' )
        {
            sgui_canvas_draw_text_plain( canvas, x+X, y, f, font_height, col,
                                         format, text, i, 0xFFFF );

            text += i + 1;
            i = -1;
            X = 0;
            y += font_height+font_height/4;
        }
    }

    sgui_canvas_draw_text_plain( canvas, x+X, y, f, font_height, col,
                                 format, text, i, 0xFFFF );
}

void* sgui_canvas_get_raw_data( sgui_canvas* canvas )
{
    return canvas ? canvas->data : NULL;
}

void sgui_canvas_set_raw_data( sgui_canvas* canvas, SGUI_COLOR_FORMAT format,
                               unsigned int width, unsigned int height,
                               void* data )
{
    if( canvas )
    {
        canvas->width  = width;
        canvas->height = height;
        canvas->format = format;
        canvas->bpp    = (format==SCF_RGBA8 || format==SCF_BGRA8) ? 4 : 3;

        if( canvas->own_buffer && canvas->data )
            free( canvas->data );

        if( data )
        {
            canvas->data       = data;
            canvas->own_buffer = 0;
        }
        else
        {
            canvas->data       = malloc( width * height * canvas->bpp );
            canvas->own_buffer = 1;
        }
    }
}

