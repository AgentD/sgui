/*
 * sgui_opengl.h
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
#ifndef SGUI_OPENGL_H
#define SGUI_OPENGL_H



#include "sgui_predef.h"



#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Creates a canvas object that renders widgets using OpenGL functions
 *
 * Note, that an OpenGL context has to be made current, before the creating,
 * destroying, calling begin or end on an OpenGL canvas.
 *
 * \note The OpenGL canvas works a little bit different from other canvases.
 *       When using the OpenGL canvas in a backend, never do a partial redraw
 *       of the widgets! Always do a full redraw. The OpenGL canvas internally
 *       uses retained mode rendering and needs to build up a vertex buffer.
 *       The vertex buffer is reset when calling begin and uploaded & rendered
 *       when calling end.
 *
 * \param width          The width of the canvas
 * \param height         The height of the canvas
 * \param compatibillity Non-zero for compatibillity profile, zero for core
 *                       profile.
 *
 * \return A pointer to an canvas that uses an OpenGL texture back end.
 */
SGUI_DLL sgui_canvas* sgui_opengl_canvas_create( unsigned int width,
                                                 unsigned int height,
                                                 int compatibillity );

#ifdef __cplusplus
}
#endif

#endif /* SGUI_OPENGL_H */

