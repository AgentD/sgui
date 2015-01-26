/*
 * sgui_frame.h
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
 * \file sgui_frame.h
 *
 * \brief Contains the declarations of the sgui_frame widget
 */
#ifndef SGUI_FRAME_H
#define SGUI_FRAME_H



#include "sgui_predef.h"



/**
 * \struct sgui_frame
 *
 * \extends sgui_widget
 *
 * \brief A scrollable container widget
 *
 * \image html frame.png "A frame with multiple children inside"
 */



#ifdef __cplusplus
extern "C"
{
#endif

/**
 * \brief Create a frame widget
 *
 * \memberof sgui_frame
 *
 * A frame is a simple widget that can contain other widgets. A frame uses a
 * scroll bar in case there are more widgets than it can display at once.
 *
 * \param x      The horizontal component of the frames position
 * \param y      The vertical component of the frames position
 * \param width  The width of the frame
 * \param height The height of the frame
 *
 * \return A pointer to a new frame widget
 */
SGUI_DLL sgui_widget* sgui_frame_create( int x, int y, unsigned int width,
                                         unsigned int height );

/**
 * \brief Override dynamic internal scrollbar enabling/disabling of a frame
 *
 * \memberof sgui_frame
 *
 * \param frame       A pointer to a frame widget
 * \param always_draw If non-zero, the scroll bars of a frame are always
 *                    drawn, even if they aren't required
 */
SGUI_DLL void sgui_frame_override_scrollbars( sgui_widget* frame,
                                              int always_draw );

#ifdef __cplusplus
}
#endif

#endif /* SGUI_FRAME_H */

