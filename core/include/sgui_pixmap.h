/*
 * sgui_pixmap.h
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
#ifndef SGUI_PIXMAP_H
#define SGUI_PIXMAP_H



#include "sgui_predef.h"
#include "sgui_window.h"



/**
 * \brief Create a pixmap
 *
 * This function creates a pixmap. A pixmap is a server side memory area that
 * can hold a picture wich can be displayed on a canvas.
 * A pixmap created for a certain backend is automatically available to all
 * canvases and windows of that backend, so even if you create an OpenGL
 * pixmap with one context, it can be used with every other context created
 * through SGUI.
 *
 * \note Creating a pixmap for an OpenGL backend requires that a context is
 *       made current.
 *
 * \param width   The width of the pixmap in pixels
 * \param height  The height of the pixmap in pixels
 * \param format  The color format used by the pixmap (SGUI_RGB8, SGUI_RGBA8,
 *                etc...)
 * \param backend The backend for wich to create the pixmap (SGUI_NATIVE,
 *                SGUI_OPENGL_CORE, etc...)
 *
 * \return A pointer to a new pixmap object on success, NULL on error.
 */
sgui_pixmap* sgui_pixmap_create( unsigned int width, unsigned int height,
                                 int format, int backend );

/**
 * \brief Upload data to a pixmap
 *
 * \param pixmap   The pixmap to upload data to.
 * \param dstrect  A subrect within the pixmap to draw to.
 * \param data     A pointer to the data buffer of which to upload a portion.
 * \param srcx     Offset from the left of the source buffer to start reading.
 * \param srcy     Offset from the top of the source buffer to start reading.
 * \param width    The width of the source buffer.
 * \param height   The height of the source buffer.
 * \param format   The color format of the source data (SGUI_RGB8, etc...)
 */
void sgui_pixmap_load( sgui_pixmap* pixmap, sgui_rect* dstrect,
                       const unsigned char* data, int srcx, int srcy,
                       unsigned int width, unsigned int height,
                       int format );

/**
 * \brief Get the size of a pixmap
 *
 * \param pixmap The pixmap from wich to get the size
 * \param width  Returns the width of the pixmap
 * \param height Returns the height of the pixmap
 */
void sgui_pixmap_get_size( sgui_pixmap* pixmap, unsigned int* width,
                           unsigned int* height );

/** \brief Destroy a pixmap */
void sgui_pixmap_destroy( sgui_pixmap* pixmap );



#endif /* SGUI_PIXMAP_H */

