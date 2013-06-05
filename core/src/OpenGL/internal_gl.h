/*
 * internal_gl.h
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
#ifndef INTERNAL_GL_H
#define INTERNAL_GL_H



#include <stdlib.h>

#include "sgui_internal.h"
#include "sgui_opengl.h"
#include "sgui_internal.h"
#include "sgui_font.h"
#include "sgui_utf8.h"
#include "sgui_pixmap.h"
#include "sgui_font_cache.h"



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

#if !defined(MACHINE_OS_WINDOWS) && !defined(MACHINE_OS_X)
    #define MACHINE_OS_UNIX
#endif



#ifndef SGUI_NO_OPENGL

/* include required system headers */
#if defined(MACHINE_OS_WINDOWS)
    #include <windows.h>
    #include <GL/gl.h>

    #define GL_LOAD_FUN( f ) wglGetProcAddress( f )
#elif defined(MACHINE_OS_X)
    #include <OpenGL/gl.h>
#elif defined(MACHINE_OS_UNIX)
    #include <GL/gl.h>
    #include <GL/glx.h>

    #define GL_LOAD_FUN( f ) glXGetProcAddress( (const GLubyte*)(f) )
#endif

/* function pointer types and defines for OpenGL version>1.1 or extensions */
typedef void(* GLGENBUFFERSPROC )( GLsizei, GLuint* );
typedef void(* GLDELETEBUFFERSPROC )( GLsizei, GLuint* );
typedef void(* GLBINDBUFFERPROC )( GLenum, GLuint );

#ifndef GL_MULTISAMPLE
    #define GL_MULTISAMPLE 0x809D
#endif

/* font cache texture size */
#define FONT_MAP_WIDTH  256
#define FONT_MAP_HEIGHT 256

/* OpenGL state flags */
#define TEX_ENABLE     0x01
#define DEPTH_ENABLE   0x02
#define DEPTH_WRITE    0x04
#define BLEND_ENABLE   0x08
#define MS_ENABLE      0x10
#define SCISSOR_ENABLE 0x20
#define CULL_ENABLE    0x40


/* an OpenGL pixmap */
typedef struct
{
    sgui_pixmap pm;

    GLuint texture;
}
pixmap_gl;

/* in canvas_gl_core.c: create OpenGL canvas for core profile rendering */
sgui_canvas* gl_canvas_create_core( unsigned int width, unsigned int height );

/* in canvas_gl_compat.c: create OpenGL canvas for compatibillity profile */
sgui_canvas* gl_canvas_create_compat( unsigned int width,
                                      unsigned int height );

/* in pixmap_gl.c: create OpenGL pixmap */
sgui_pixmap* gl_pixmap_create( unsigned int width, unsigned int height,
                               int format );

#endif /* !SGUI_NO_OPENGL */

#endif /* INTERNAL_GL_H */

