/*
 * gl_canvas.h
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
#ifndef GL_CANVAS_H
#define GL_CANVAS_H



#include "sgui_tex_canvas.h"
#include "sgui_canvas.h"



#ifndef SGUI_NO_OPENGL
#ifdef SGUI_WINDOWS
    #include <windows.h>
#endif

#ifdef SGUI_OS_X
    #include <OpenGL/gl.h>
#else
    #include <GL/gl.h>
#endif



typedef struct
{
    sgui_tex_canvas super;

    sgui_rect locked;
    unsigned char* buffer;
    GLuint tex;
}
sgui_gl_canvas;



#ifdef __cplusplus
extern "C" {
#endif

sgui_canvas* sgui_gl_canvas_create(sgui_lib *lib, unsigned width,
					unsigned int height);

#ifdef __cplusplus
}
#endif

#endif /* SGUI_NO_OPENGL */

#endif /* GL_CANVAS_H */

