/*
 * sgui_progress_bar.h
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
#ifndef SGUI_PROGRESS_BAR_H
#define SGUI_PROGRESS_BAR_H



#include "sgui_widget.h"



#define SGUI_PROGRESS_BAR_CONTINUOUS  1
#define SGUI_PROGRESS_BAR_STIPPLED    0
#define SGUI_PROGRESS_BAR_OFFSET      1
#define SGUI_PROGRESS_BAR_NO_OFFSET   0
#define SGUI_PROGRESS_BAR_HORIZONTAL  0
#define SGUI_PROGRESS_BAR_VERTICAL    1


#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Create a progress bar widget
 *
 * \param x        The x component of the position of the progress bar.
 * \param y        The y component of the position of the progress bar.
 * \param width    Width of the progress bar in pixels.
 * \param height   Height of the progress bar in pixels.
 * \param progress Initial progress for the bar (value between 0.0 and 1.0)
 */
sgui_widget* sgui_progress_bar_create( int x, int y, unsigned int width,
                                       unsigned int height, float progress );

/**
 * \brief Set the progress on a progress bar
 *
 * \param bar      The progress bar to alter.
 * \param progress Value between 0.0 and 1.0 for the bar to display.
 */
void sgui_progress_bar_set_progress( sgui_widget* bar, float progress );

/**
 * \brief Get the progress on a progress bar
 *
 * \param bar The progress bar to alter.
 */
float sgui_progress_bar_get_progress( sgui_widget* bar );

/**
 * \brief Set the style of a progress bar
 *
 * \param bar        The progress bar to set the style of.
 * \param continuous Whether the progress bar should be one continuous stripe
 *                   or stippled. Default is stippled.
 * \param offset     Whether the progress bar should touch the borders around
 *                   it or keep some distance. Default is offset.
 */
void sgui_progress_bar_set_style( sgui_widget* bar, int continuous,
                                  int offset );

/**
 * \brief Set the color of a progress bar
 *
 * \param bar   The progress bar to set the color of.
 * \param color The color to use for the progress bar. Default is white.
 */
void sgui_progress_bar_set_color( sgui_widget* bar, unsigned long color );

/**
 * \brief Set the direction of a progress bar
 *
 * \param bar      The progress bar to set the direction of.
 * \param vertical Whether the bar should be vertical (default: horizontal).
 */
void sgui_progress_bar_set_direction( sgui_widget* bar, int vertical );

/**
 * \brief Delete a progress bar widget
 *
 * \param par The progress bar widget to delete
 */
void sgui_progress_bar_delete( sgui_widget* bar );



#ifdef __cplusplus
}
#endif

#endif /* SGUI_PROGRESS_BAR_H */

