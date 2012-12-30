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




void canvas_gl_download( sgui_canvas* canvas, sgui_rect* r )
{
    canvas->buffer = ((sgui_canvas_gl*)canvas)->data;
    canvas->buffer_x = 0;
    canvas->buffer_y = 0;
    canvas->buffer_w = canvas->width;
    canvas->buffer_h = canvas->height;

    sgui_rect_copy( &((sgui_canvas_gl*)canvas)->locked, r );
}

void canvas_gl_upload( sgui_canvas* canvas )
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

    cv->canvas.download = canvas_gl_download;
    cv->canvas.upload = canvas_gl_upload;
    cv->canvas.clear = canvas_gl_clear;

    return (sgui_canvas*)cv;
}

void sgui_opengl_canvas_resize( sgui_canvas* canvas, unsigned int width,
                                unsigned int height )
{
    (void)canvas;
    (void)width;
    (void)height;
}

void sgui_opengl_canvas_destroy( sgui_canvas* canvas )
{
    if( canvas )
    {
        glDeleteTextures( 1, &((sgui_canvas_gl*)canvas)->texture );
        free( ((sgui_canvas_gl*)canvas)->data );
        free( canvas );
    }
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

void sgui_opengl_canvas_resize( sgui_canvas* canvas, unsigned int width,
                                unsigned int height )
{
    (void)canvas;
    (void)width;
    (void)height;
}

void sgui_opengl_canvas_destroy( sgui_canvas* canvas )
{
    (void)canvas;
}

unsigned int sgui_opengl_canvas_get_texture( sgui_canvas* canvas )
{
    (void)canvas;
    return 0;
}
#endif

