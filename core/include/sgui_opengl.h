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

/** \brief Get the OpenGL texture handle from an OpenGL canvas */
SGUI_DLL unsigned int sgui_opengl_canvas_get_texture( sgui_canvas* canvas );



/**
 * \brief Create an OpenGL pixmap
 *
 * This function is an OpenGL specific variation of sgui_pixmap_create.
 *
 * \note This function requires an OpenGL context to be made current.
 *
 * \param width   The width of the pixmap in pixels
 * \param height  The height of the pixmap in pixels
 * \param format  The color format used by the pixmap (SGUI_RGB8, SGUI_RGBA8,
 *                etc...)
 *
 * \return An OpenGL texture handle
 */

SGUI_DLL unsigned int sgui_opengl_pixmap_create( unsigned int width,
                                                 unsigned int height,
                                                 int format );

/**
 * \brief Upload data to an OpenGL pixmap
 *
 * This s an OpenGL specific variant of sgui_pixmap_load.
 *
 * \note This function requires an OpenGL context to be made current.
 *
 * \param pixmap   An OpenGL texture handle.
 * \param dstrect  A subrect within the texture to update.
 * \param data     A pointer to the data buffer of which to upload a portion.
 * \param srcx     Offset from the left of the source buffer to start reading.
 * \param srcy     Offset from the top of the source buffer to start reading.
 * \param width    The width of the source buffer.
 * \param height   The height of the source buffer.
 * \param format   The color format of the source data (SGUI_RGB8, etc...)
 */
SGUI_DLL void sgui_opengl_pixmap_load( unsigned int pixmap,
                                       sgui_rect* dstrect,
                                       const unsigned char* data,
                                       int srcx, int srcy,
                                       unsigned int width,
                                       unsigned int height,
                                       int format );

/** \brief Destroy an OpenGL pixmap */
SGUI_DLL void sgui_opengl_pixmap_destroy( unsigned int pixmap );

#ifdef __cplusplus
}
#endif

#endif /* SGUI_OPENGL_H */

