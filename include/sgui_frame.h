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
#ifndef SGUI_FRAME_H
#define SGUI_FRAME_H



#include "sgui_predef.h"



#ifdef __cplusplus
extern "C"
{
#endif

/**
 * \brief Create a frame widget
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
sgui_widget* sgui_frame_create( int x, int y, unsigned int width,
                                unsigned int height );

/**
 * \brief Destroy a frame widget
 *
 * \param frame The frame widget to destroy
 */
void sgui_frame_destroy( sgui_widget* frame );

/**
 * \brief Add a widget to a frame
 *
 * \param frame The frame to add the widget to
 * \param w     The widget to add
 */
void sgui_frame_add_widget( sgui_widget* frame, sgui_widget* w );

/**
 * \brief Remove a widget from a frame
 *
 * \param frame The frame to remove the widget from
 * \param w     The widget to remove
 */
void sgui_frame_remove_widget( sgui_widget* frame, sgui_widget* w );

#ifdef __cplusplus
}
#endif

#endif /* SGUI_FRAME_H */

