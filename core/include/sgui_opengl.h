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
 * \brief Creates a canvas object that renders to an OpenGL texture
 *
 * Note, that an OpenGL context has to be made current, before the creating,
 * destroying, calling begin or end on an OpenGL canvas.
 *
 * \param width  The width of the canvas
 * \param height The height of the canvas
 *
 * \return A pointer to an canvas that uses an OpenGL texture back end.
 */
SGUI_DLL sgui_canvas* sgui_opengl_canvas_create( unsigned int width,
                                                 unsigned int height );

/**
 * \brief Resize an OpenGL canvas
 *
 * \param canvas The OpenGL canvas to resize
 * \param width  The new width of the canvas
 * \param height The new height of the canvas
 */
SGUI_DLL void sgui_opengl_canvas_resize( sgui_canvas* canvas,
                                         unsigned int width,
                                         unsigned int height );

/** \brief Destroy an OpenGL canvas */
SGUI_DLL void sgui_opengl_canvas_destroy( sgui_canvas* canvas );

/** \brief Get the OpenGL texture handle from an OpenGL canvas */
SGUI_DLL unsigned int sgui_opengl_canvas_get_texture( sgui_canvas* canvas );

#ifdef __cplusplus
}
#endif

#endif /* SGUI_OPENGL_H */

