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
#ifndef SGUI_IMAGE_H
#define SGUI_IMAGE_H



#include "sgui_predef.h"



#ifdef __cplusplus
extern "C"
{
#endif

/**
 * \brief Create an image widget
 *
 * \param x      Distance from the left of the window to the left of the image
 * \param y      Distance from the top of the window to the top of the image
 * \param width  Width of the image
 * \param height Height of the image
 * \param data   The RGB or RGBA image data
 * \param format The color format used by the image
 * \param blend  Non-zero if the image should be blended instead of blitted
 * \param backend The backend for wich to create a pixmap
 */
SGUI_DLL sgui_widget* sgui_image_create( int x, int y,
                                         unsigned int width,
                                         unsigned int height,
                                         const void* data, int format,
                                         int blend, int backend );

#ifdef __cplusplus
}
#endif

#endif /* SGUI_IMAGE_H */

