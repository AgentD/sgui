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



#ifndef SGUI_NO_OPENGL



#include <stdlib.h>

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

#include "sgui_internal.h"



typedef struct
{
    sgui_pixmap pm;

    GLuint texture;
}
pixmap_gl;



sgui_pixmap* gl_pixmap_create( unsigned int width, unsigned int height,
                               int format );

#endif /* !SGUI_NO_OPENGL */

#endif /* INTERNAL_GL_H */

