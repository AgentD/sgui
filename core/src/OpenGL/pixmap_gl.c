/*
 * pixmap_gl.h
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
#include "sgui_canvas.h"
#include "sgui_rect.h"

#include "internal_gl.h"

#include <string.h>
#include <stdio.h>



#ifndef SGUI_NO_OPENGL
void gl_pixmap_load( sgui_pixmap* pixmap, int dstx, int dsty,
                     const unsigned char* data, unsigned int scan,
                     unsigned int width, unsigned int height, int format )
{
    GLint current;

    /* get current texture and bind ours */
    glGetIntegerv( GL_TEXTURE_BINDING_2D, &current );
    glBindTexture( GL_TEXTURE_2D, ((pixmap_gl*)pixmap)->texture );

    /* upload the image */
    glPixelStorei( GL_UNPACK_ROW_LENGTH, scan );
    glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );

    glTexSubImage2D( GL_TEXTURE_2D, 0, dstx, dsty, width, height,
                      format==SGUI_RGB8 ? GL_RGB :
                     (format==SGUI_RGBA8 ? GL_RGBA : GL_ALPHA),
                     GL_UNSIGNED_BYTE, data );

    glPixelStorei( GL_UNPACK_ROW_LENGTH, 0 );

    if( format == SGUI_A8 )
    {
        unsigned char* buffer = malloc( 256*256 );
        FILE* file;

        glGetTexImage( GL_TEXTURE_2D, 0, GL_ALPHA, GL_UNSIGNED_BYTE, buffer );

        file = fopen( "test.pbm", "wb" );
        fprintf( file, "P5\n256 256\n255\n" );
        fwrite( buffer, 1, 256*256, file );
        fclose( file );

        free( buffer );
    }

    /* rebind the previous texture */
    glBindTexture( GL_TEXTURE_2D, current );
}

void gl_pixmap_destroy( sgui_pixmap* pm )
{
    glDeleteTextures( 1, &((pixmap_gl*)pm)->texture );
    free( pm );
}



sgui_pixmap* gl_pixmap_create( unsigned int width, unsigned int height,
                               int format )
{
    GLint current;
    pixmap_gl* pixmap = malloc( sizeof(pixmap_gl) );

    if( !pixmap )
        return NULL;

    pixmap->pm.width   = width;
    pixmap->pm.height  = height;
    pixmap->pm.destroy = gl_pixmap_destroy;
    pixmap->pm.load    = gl_pixmap_load;

    /* try to create an OpenGL texture */
    glGenTextures( 1, &pixmap->texture );

    if( !pixmap->texture )
    {
        free( pixmap );
        return NULL;
    }

    /* get current texture and bind ours */
    glGetIntegerv( GL_TEXTURE_BINDING_2D, &current );
    glBindTexture( GL_TEXTURE_2D, pixmap->texture );

    /* reserve texture memory */
    if( format==SGUI_RGBA8 )
    {
        glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0,
                      GL_RGBA, GL_UNSIGNED_BYTE, NULL );
    }
    else if( format==SGUI_RGB8 )
    {
        glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0,
                      GL_RGB, GL_UNSIGNED_BYTE, NULL );
    }
    else
    {
        glTexImage2D( GL_TEXTURE_2D, 0, GL_ALPHA8, width, height, 0,
                      GL_ALPHA, GL_UNSIGNED_BYTE, NULL );
    }

    /* disable mipmapping */
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );

    /* disable wrapping */
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );

    /* rebind old texture */
    glBindTexture( GL_TEXTURE_2D, current );

    return (sgui_pixmap*)pixmap;
}
#endif

