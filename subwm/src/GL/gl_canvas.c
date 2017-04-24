/*
 * gl_canvas.c
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
#include "gl_canvas.h"

#include <stdlib.h>
#include <string.h>



#ifndef SGUI_NO_OPENGL
static void gl_canvas_destroy( sgui_canvas* super )
{
    sgui_gl_canvas* this = (sgui_gl_canvas*)super;

    glDeleteTextures( 1, &this->tex );

    free( this->buffer );
    free( this );
}

static int gl_canvas_begin( sgui_canvas* this, const sgui_rect* r )
{
    ((sgui_gl_canvas*)this)->locked = *r;
    return 1;
}

static void gl_canvas_end( sgui_canvas* super )
{
    sgui_gl_canvas* this = (sgui_gl_canvas*)super;
    unsigned int x, y, w, h;
    GLint len, allign;
    GLuint old;

    x = this->locked.left;
    y = this->locked.top;
    w = SGUI_RECT_WIDTH(this->locked);
    h = SGUI_RECT_HEIGHT(this->locked);

    /* make current & get state */
    glGetIntegerv( GL_TEXTURE_BINDING_2D, (GLint*)&old );
    glGetIntegerv( GL_UNPACK_ROW_LENGTH, &len );
    glGetIntegerv( GL_UNPACK_ALIGNMENT, &allign );

    /* bind & upload */
    glBindTexture( GL_TEXTURE_2D, this->tex );
    glPixelStorei( GL_UNPACK_ROW_LENGTH, super->width );
    glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );

    glTexSubImage2D( GL_TEXTURE_2D, 0, x, y, w, h, GL_RGBA, GL_UNSIGNED_BYTE,
                     this->buffer + (y*super->width + x)*4 );

    /* restore state & release */
    glBindTexture( GL_TEXTURE_2D, old );
    glPixelStorei( GL_UNPACK_ROW_LENGTH, len );
    glPixelStorei( GL_UNPACK_ALIGNMENT, allign );
}

void* gl_canvas_get_texture( sgui_canvas* this )
{
    return &( ((sgui_gl_canvas*)this)->tex );
}



sgui_canvas* sgui_gl_canvas_create(sgui_lib *lib, unsigned width,
					unsigned int height)
{
    sgui_gl_canvas* this;
    GLuint old;

    this = calloc( 1, sizeof(sgui_gl_canvas) );

    if( !this )
        return NULL;

    /* create in-memory drawing buffer */
    this->buffer = malloc( width*height*4 );

    if( !this->buffer )
    {
        free( this );
        return NULL;
    }

    /* initialize base structure */
    if( !sgui_memory_canvas_init( (sgui_canvas*)this, lib, this->buffer,
                                  width, height, SGUI_RGBA8, 0 ) )
    {
        free( this->buffer );
        free( this );
        return NULL;
    }

    /* create and initialize texture */
    glGetIntegerv( GL_TEXTURE_BINDING_2D, (GLint*)&old );
    glGenTextures( 1, &this->tex );

    glBindTexture( GL_TEXTURE_2D, this->tex );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA,
                  GL_UNSIGNED_BYTE, NULL );

    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );

    glBindTexture( GL_TEXTURE_2D, old );

    /* hook callbacks */
    ((sgui_tex_canvas*)this)->get_texture = gl_canvas_get_texture;
    ((sgui_canvas*)this)->destroy = gl_canvas_destroy;
    ((sgui_canvas*)this)->begin = gl_canvas_begin;
    ((sgui_canvas*)this)->end = gl_canvas_end;

    return (sgui_canvas*)this;
}
#endif

