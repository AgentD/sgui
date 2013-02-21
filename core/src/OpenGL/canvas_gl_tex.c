/*
 * canvas_gl_tex.c
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
#include "sgui_opengl.h"
#include "sgui_internal.h"
#include "sgui_font.h"
#include "sgui_utf8.h"

#include <stdlib.h>

#ifndef SGUI_NO_OPENGL

/* Operating system check */
#ifndef MACHINE_OS_WINDOWS
    #if defined(_WIN16) || defined(_WIN32) || defined(_WIN64)
        #define MACHINE_OS_WINDOWS
    #elif defined(__TOS_WIN__) || defined(__WINDOWS__) || defined(__WIN32__)
        #define MACHINE_OS_WINDOWS
    #endif
#endif

#ifndef MACHINE_OS_X
    #if defined(__APPLE__) && defined(__MACH__)
        #define MACHINE_OS_X
    #endif
#endif

/* include required system headers */
#ifdef MACHINE_OS_WINDOWS
    #include <windows.h>
#endif

#ifdef MACHINE_OS_X
    #include <OpenGL/gl.h>
#else
    #include <GL/gl.h>
#endif



typedef struct
{
    sgui_canvas canvas;

    sgui_rect locked;
    unsigned char* data;
    GLuint texture;
}
sgui_canvas_gl;



/* binds a canvas texture and returns the previously bound texture */
GLuint canvas_gl_bind_tex( sgui_canvas_gl* cv )
{
    GLint current;

    glGetIntegerv( GL_TEXTURE_BINDING_2D, &current );
    glBindTexture( GL_TEXTURE_2D, cv->texture );

    return current;
}



void canvas_gl_destroy( sgui_canvas* canvas )
{
    glDeleteTextures( 1, &((sgui_canvas_gl*)canvas)->texture );
    free( ((sgui_canvas_gl*)canvas)->data );
    free( canvas );
}

void canvas_gl_resize( sgui_canvas* canvas, unsigned int width,
                       unsigned int height )
{
    (void)canvas;
    (void)width;
    (void)height;
}


void canvas_gl_begin( sgui_canvas* canvas, sgui_rect* r )
{
    sgui_rect_copy( &((sgui_canvas_gl*)canvas)->locked, r );
}

void canvas_gl_end( sgui_canvas* canvas )
{
    sgui_canvas_gl* cv = (sgui_canvas_gl*)canvas;
    GLuint current = canvas_gl_bind_tex( cv );
    int x = cv->locked.left, y = cv->locked.top;
    unsigned int w = cv->locked.right - cv->locked.left + 1;
    unsigned int h = cv->locked.bottom - cv->locked.top + 1;

    glPixelStorei( GL_UNPACK_ROW_LENGTH, canvas->width );

    glTexSubImage2D( GL_TEXTURE_2D, 0, x, y, w, h, GL_RGBA, GL_UNSIGNED_BYTE,
                     cv->data + (y*canvas->width + x)*4 );

    glPixelStorei( GL_UNPACK_ROW_LENGTH, 0 );

    glBindTexture( GL_TEXTURE_2D, current );
}

void canvas_gl_clear( sgui_canvas* canvas, sgui_rect* r )
{
    sgui_canvas_gl* cv = (sgui_canvas_gl*)canvas;
    unsigned char *dst = cv->data + (r->top*canvas->width + r->left)*4, *row;
    int i, j, x, y, w, h;
    GLuint current;

    /* clear */
    for( j=r->top; j<=r->bottom; ++j, dst+=canvas->width*4 )
    {
        for( row=dst, i=r->left; i<=r->right; ++i )
        {
            *(row++) = canvas->bg_color[0];
            *(row++) = canvas->bg_color[1];
            *(row++) = canvas->bg_color[2];
            *(row++) = 0x00;
        }
    }

    /* upload */
    x = r->left;
    y = r->top;
    w = r->right - r->left + 1;
    h = r->bottom - r->top + 1;

    current = canvas_gl_bind_tex( cv );

    glPixelStorei( GL_UNPACK_ROW_LENGTH, canvas->width );

    glTexSubImage2D( GL_TEXTURE_2D, 0, x, y, w, h, GL_RGBA, GL_UNSIGNED_BYTE,
                     cv->data + (y*canvas->width + x)*4 );

    glPixelStorei( GL_UNPACK_ROW_LENGTH, 0 );

    glBindTexture( GL_TEXTURE_2D, current );
}

void canvas_gl_blit( sgui_canvas* canvas, int x, int y, unsigned int width,
                     unsigned int height, unsigned int scanline_length,
                     int format, const void* data )
{
    sgui_canvas_gl* cv = (sgui_canvas_gl*)canvas;
    unsigned char *drow, *srow, *src, *dst;
    unsigned int i, j, ds, dt, src_bpp = (format==SGUI_RGBA8 ? 4 : 3);

    dst = cv->data + (y*canvas->width + x)*4;
    src = (unsigned char*)data;

    ds = scanline_length * (format==SGUI_RGBA8 ? 4 : 3);
    dt = canvas->width * 4;

    for( j=0; j<height; ++j, src+=ds, dst+=dt )
    {
        for( drow=dst, srow=src, i=0; i<width; ++i, srow+=src_bpp )
        {
            *(drow++) = srow[0];
            *(drow++) = srow[1];
            *(drow++) = srow[2];
            *(drow++) = 0x00;
        }
    }
}

void canvas_gl_blend( sgui_canvas* canvas, int x, int y, unsigned int width,
                      unsigned int height, unsigned int scanline_length,
                      const void* data )
{
    sgui_canvas_gl* cv = (sgui_canvas_gl*)canvas;
    unsigned char *dst, *src, *drow, *srow, A, iA;
    unsigned int ds, dt, i, j;

    dst = cv->data + (y*canvas->width + x)*4;
    src = (unsigned char*)data;

    ds = scanline_length * 4;
    dt = canvas->width*4;

    for( j=0; j<height; ++j, src+=ds, dst+=dt )
    {
        for( drow=dst, srow=src, i=0; i<width; ++i, srow+=4, drow+=4 )
        {
            A = srow[3];
            iA = 0xFF-A;

            drow[0] = (drow[0] * iA + srow[0] * A)>>8;
            drow[1] = (drow[1] * iA + srow[1] * A)>>8;
            drow[2] = (drow[2] * iA + srow[2] * A)>>8;
            drow[3] = 0x00;
        }
    }
}

void canvas_gl_draw_box( sgui_canvas* canvas, sgui_rect* r,
                         unsigned char* color, int format )
{
    sgui_canvas_gl* cv = (sgui_canvas_gl*)canvas;
    unsigned char A, iA;
    unsigned char *dst, *row;
    int i, j;

    dst = cv->data + (r->top*canvas->width + r->left)*4;

    if( format==SGUI_RGBA8 )
    {
        A = color[3];
        iA = 0xFF - A;

        for( j=r->top; j<=r->bottom; ++j, dst+=canvas->width*4 )
        {
            for( row=dst, i=r->left; i<=r->right; ++i, row+=4 )
            {
                row[0] = (row[0] * iA + color[0] * A)>>8;
                row[1] = (row[1] * iA + color[1] * A)>>8;
                row[2] = (row[2] * iA + color[2] * A)>>8;
                row[3] = 0x00;
            }
        }
    }
    else
    {
        for( j=r->top; j<=r->bottom; ++j, dst+=canvas->width*4 )
        {
            for( row=dst, i=r->left; i<=r->right; ++i, row+=4 )
            {
                row[0] = color[0];
                row[1] = color[1];
                row[2] = color[2];
                row[3] = 0x00;
            }
        }
    }
}

void canvas_gl_blend_stencil( sgui_canvas* canvas, unsigned char* buffer,
                              int x, int y, unsigned int w, unsigned int h,
                              unsigned int scan, unsigned char* color )
{
    sgui_canvas_gl* cv = (sgui_canvas_gl*)canvas;
    unsigned char A, iA, *src, *dst, *row;
    unsigned int i, j;

    dst = cv->data + (y*canvas->width + x)*4;

    for( j=0; j<h; ++j, buffer+=scan, dst+=canvas->width*4 )
    {
        for( src=buffer, row=dst, i=0; i<w; ++i, row+=4, ++src )
        {
            A = *src;
            iA = 0xFF-A;

            row[0] = (row[0] * iA + color[0] * A)>>8;
            row[1] = (row[1] * iA + color[1] * A)>>8;
            row[2] = (row[2] * iA + color[2] * A)>>8;
            row[3] = 0x00;
        }
    }
}



int canvas_gl_draw_string( sgui_canvas* canvas, int x, int y,
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

            canvas_gl_blend_stencil( canvas, buffer, r.left, r.top,
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

/****************************************************************************/
sgui_canvas* sgui_opengl_canvas_create( unsigned int width,
                                        unsigned int height )
{
    sgui_canvas_gl* cv = malloc( sizeof(sgui_canvas_gl) );
    GLint current;

    if( !cv )
        return NULL;

    cv->data = malloc( width * height * 4 );

    if( !cv->data )
    {
        free( cv );
        return NULL;
    }

    /* create OpenGL texture */
    glGenTextures( 1, &cv->texture );

    if( !cv->texture )
    {
        free( cv->data );
        free( cv );
        return NULL;
    }

    current = canvas_gl_bind_tex( cv );

    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA,
                  GL_UNSIGNED_BYTE, NULL );

    /* no mipmapping, we would have to recreate them all the time */
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

    glBindTexture( GL_TEXTURE_2D, current );

    /* finish initialisation */
    sgui_internal_canvas_init( (sgui_canvas*)cv, width, height );

    cv->canvas.destroy = canvas_gl_destroy;
    cv->canvas.resize = canvas_gl_resize;
    cv->canvas.begin = canvas_gl_begin;
    cv->canvas.end = canvas_gl_end;
    cv->canvas.clear = canvas_gl_clear;
    cv->canvas.blit = canvas_gl_blit;
    cv->canvas.blend = canvas_gl_blend;
    cv->canvas.draw_box = canvas_gl_draw_box;
    cv->canvas.draw_string = canvas_gl_draw_string;

    return (sgui_canvas*)cv;
}

unsigned int sgui_opengl_canvas_get_texture( sgui_canvas* canvas )
{
    return canvas ? ((sgui_canvas_gl*)canvas)->texture : 0;
}
#else
sgui_canvas* sgui_opengl_canvas_create( unsigned int width,
                                        unsigned int height )
{
    (void)width;
    (void)height;
    return NULL;
}

unsigned int sgui_opengl_canvas_get_texture( sgui_canvas* canvas )
{
    (void)canvas;
    return 0;
}
#endif

