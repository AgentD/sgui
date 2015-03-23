/*
 * sgui_image.h
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

/**
 * \file sgui_image.h
 *
 * \brief Contains the declarations of the sgui_image widget
 */
#ifndef SGUI_IMAGE_H
#define SGUI_IMAGE_H



#include "sgui_predef.h"



/**
 * \struct sgui_image
 *
 * \extends sgui_widget
 *
 * \brief A widget that displays a staticially assigned bitmap image
 *
 * \image html static.png "Two image widgets, with and without transparency"
 */



#ifdef __cplusplus
extern "C"
{
#endif

/**
 * \brief Create an image widget
 *
 * \memberof sgui_image
 *
 * \param x      Distance from the left of the window to the left of the image
 * \param y      Distance from the top of the window to the top of the image
 * \param width  Width of the image
 * \param height Height of the image
 * \param data   The RGB or RGBA image data
 * \param format The color format used by the image
 * \param blend  Non-zero if the image should be blended instead of blitted
 * \param useptr Non-zero if the given pointer should be used instead of
 *               creating an internal buffer to safe memory
 */
SGUI_DLL sgui_widget* sgui_image_create( int x, int y,
                                         unsigned int width,
                                         unsigned int height,
                                         const void* data, int format,
                                         int blend, int useptr );

/**
 * \brief Reload a portion of an image
 *
 * \memberof sgui_image
 *
 * If a portion of an image has changed, reupload the data to the underlying
 * pixmap and ask the canvas to redraw the image. This way, dynamic, animated
 * images can be displayed based on a simple sgui_image.
 *
 * \param image  A pointer to an image widget
 * \param x      A distance from the left of the image
 * \param y      A distance from the top of the image
 * \param width  The width of the altered are in pixels
 * \param height The height of the altered are in pixels
 */
SGUI_DLL void sgui_image_reload( sgui_widget* image,
                                 unsigned int x, unsigned int y,
                                 unsigned int width, unsigned int height );

#ifdef __cplusplus
}
#endif

#endif /* SGUI_IMAGE_H */

