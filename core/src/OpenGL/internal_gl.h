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
#include <stdio.h>
#include <stddef.h>
#include <string.h>

#include "sgui_internal.h"
#include "sgui_opengl.h"
#include "sgui_internal.h"
#include "sgui_font.h"
#include "sgui_utf8.h"
#include "sgui_pixmap.h"
#include "sgui_font_cache.h"
#include "sgui_skin.h"



/* Operating system check */
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
#ifndef GL_VERSION_1_5
    typedef ptrdiff_t GLintptr;
    typedef ptrdiff_t GLsizeiptr;

    #define GL_ARRAY_BUFFER 0x8892
    #define GL_ARRAY_BUFFER_BINDING 0x8894

    #define GL_STREAM_DRAW 0x88E0
    #define GL_STREAM_READ 0x88E1
    #define GL_STREAM_COPY 0x88E2
    #define GL_STATIC_DRAW 0x88E4
    #define GL_STATIC_READ 0x88E5
    #define GL_STATIC_COPY 0x88E6
    #define GL_DYNAMIC_DRAW 0x88E8
    #define GL_DYNAMIC_READ 0x88E9
    #define GL_DYNAMIC_COPY 0x88EA
#endif

#ifndef GL_VERSION_2_0
    #define GL_FRAGMENT_SHADER 0x8B30
    #define GL_VERTEX_SHADER 0x8B31

    typedef char GLchar;
#endif

#ifndef GL_MULTISAMPLE
    #define GL_MULTISAMPLE 0x809D
#endif

typedef void(* GLGENBUFFERSPROC )( GLsizei, GLuint* );
typedef void(* GLDELETEBUFFERSPROC )( GLsizei, GLuint* );
typedef void(* GLBINDBUFFERPROC )( GLenum, GLuint );
typedef void(* GLBUFFERDATAPROC )(GLenum, GLsizeiptr, const GLvoid*, GLenum);
typedef void(* GLDRAWARRAYSPROC )( GLenum, GLint, GLsizei );
typedef void(* GLBUFFERSUBDATAPROC )( GLenum, GLintptr, GLsizeiptr,
                                      const GLvoid* );

typedef GLuint (* GLCREATESHADERPROC )( GLenum );
typedef void (* GLDELETESHADERPROC )( GLuint );
typedef void (* GLSHADERSOURCEPROC )( GLuint, GLsizei,
                                      const GLchar**, const GLint* );
typedef void (* GLCOMPILESHADERPROC )( GLuint );
typedef void (* GLATTACHSHADERPROC )( GLuint, GLuint );
typedef void (* GLDETACHSHADERPROC )( GLuint, GLuint );
typedef GLuint (* GLCREATEPROGRAMPROC )( void );
typedef void (* GLDELETEPROGRAMPROC )( GLuint );
typedef void (* GLLINKPROGRAMPROC )( GLuint );
typedef void (* GLUSEPROGRAMPROC )( GLuint );

typedef void (* GLBINDATTRIBLOCATION )( GLuint, GLuint, const GLchar* );
typedef void (* GLBINDFRAGDATALOCATION )( GLuint, GLuint, const GLchar* );

typedef void (* GLGENVERTEXARRAYSPROC )( GLsizei, GLuint* );
typedef void (* GLDELETEVERTEXARRAYSPROC )( GLsizei, const GLuint* );
typedef void (* GLBINDVERTEXARRAYPROC )( GLuint );
typedef void (* GLENABLEVERTEXATTRIBARRAYPROC )( GLuint );
typedef void (* GLVERTEXATTRIBPOINTERPROC )( GLuint, GLint, GLenum, GLboolean,
                                             GLsizei, const GLvoid* );
typedef GLint (* GLGETUNIFORMLOCATIONPROC )( GLuint, const GLchar* );
typedef void (* GLUNIFORMMATRIX4FVPROC )( GLint, GLsizei, GLboolean,
                                          const GLfloat* );

typedef void(* GLGETSHADERINFOLOG )( GLuint, GLsizei, GLsizei*, GLchar* );
typedef void(* GLGETPROGRAMINFOLOG )( GLuint, GLsizei, GLsizei*, GLchar* );

/* font cache texture size */
#define FONT_MAP_WIDTH  256
#define FONT_MAP_HEIGHT 256

/* vertex buffer details */
#define VERTEX_SIZE 8
#define MAX_VERTICES 2048

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

