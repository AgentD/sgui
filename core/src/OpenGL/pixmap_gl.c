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



unsigned int sgui_opengl_pixmap_create( unsigned int width,
                                        unsigned int height,
                                        int format )
{
    GLuint tex;
    GLint current;

    /* try to create an OpenGL texture */
    glGenTextures( 1, &tex );

    if( !tex )
        return 0;

    /* get current texture and bind ours */
    glGetIntegerv( GL_TEXTURE_BINDING_2D, &current );
    glBindTexture( GL_TEXTURE_2D, tex );

    /* reserve texture memory */
    if( format==SGUI_RGBA8 )
    {
        glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0,
                      GL_RGBA, GL_UNSIGNED_BYTE, NULL );
    }
    else
    {
        glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0,
                      GL_RGB, GL_UNSIGNED_BYTE, NULL );
    }

    /* disable mipmapping */
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

    /* rebind old texture */
    glBindTexture( GL_TEXTURE_2D, current );

    return tex;
}

void sgui_opengl_pixmap_load( unsigned int pixmap, sgui_rect* dstrect,
                              const unsigned char* data, int srcx, int srcy,
                              unsigned int width, unsigned int height,
                              int format )
{
    GLint current;
    int dstx, dsty, subw, subh;

    if( !pixmap || !data || !width || !height )
        return;

    /* adjust the data pointer and get sub width/height */
    dstx = dstrect ? dstrect->left : 0;
    dsty = dstrect ? dstrect->top  : 0;
    subw = dstrect ? SGUI_RECT_WIDTH_V( dstrect ) : (int)width;
    subh = dstrect ? SGUI_RECT_HEIGHT_V( dstrect ) : (int)height;

    data += (srcy*width + srcx)*(format==SGUI_RGB8 ? 3 : 4);

    /* get current texture and bind ours */
    glGetIntegerv( GL_TEXTURE_BINDING_2D, &current );
    glBindTexture( GL_TEXTURE_2D, pixmap );

    /* upload the image */
    glPixelStorei( GL_UNPACK_ROW_LENGTH, width );

    glTexSubImage2D( GL_TEXTURE_2D, 0, dstx, dsty, subw, subh,
                     format==SGUI_RGB8 ? GL_RGB : GL_RGBA,
                     GL_UNSIGNED_BYTE, data );

    glPixelStorei( GL_UNPACK_ROW_LENGTH, 0 );

    /* rebind the previous texture */
    glBindTexture( GL_TEXTURE_2D, current );
}

void sgui_opengl_pixmap_destroy( unsigned int pixmap )
{
    if( pixmap )
        glDeleteTextures( 1, &pixmap );
}
#else
unsigned int sgui_opengl_pixmap_create( unsigned int width,
                                        unsigned int height,
                                        int format )
{
    (void)width; (void)height; (void)format;

    return 0;
}

void sgui_opengl_pixmap_load( unsigned int pixmap, sgui_rect* dstrect,
                              const unsigned char* data, int srcx, int srcy,
                              unsigned int width, unsigned int height,
                              int format )
{
    (void)pixmap; (void)dstrect; (void)data;  (void)srcx; (void)srcy;
    (void)width;  (void)height;  (void)format;
}

void sgui_opengl_pixmap_destroy( unsigned int pixmap )
{
    (void)pixmap;
}
#endif

