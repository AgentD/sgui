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



#include "sgui_predef.h"



#define SGUI_PROGRESS_BAR_CONTINUOUS  1
#define SGUI_PROGRESS_BAR_STIPPLED    0
#define SGUI_PROGRESS_BAR_HORIZONTAL  0
#define SGUI_PROGRESS_BAR_VERTICAL    1


#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Create a progress bar widget
 *
 * \param x        Distance from the left of the window.
 * \param y        Distance from the top of the window.
 * \param style    SGUI_PROGRESS_BAR_CONTINUOUS for a continous bar,
 *                 SGUI_PROGRESS_BAR_STIPPLED for a bar out of discrete
 *                 slices.
 * \param vertical Non-zero for a vertical bar, zero for a horizontal bar.
 * \param progress Initial progress for the bar (value between 0 and 100)
 * \param length   The length of the bar in pixels.
 */
SGUI_DLL sgui_widget* sgui_progress_bar_create( int x, int y, int style,
                                                int vertical,
                                                unsigned int progress,
                                                unsigned int length );

/**
 * \brief Set the progress on a progress bar
 *
 * \param bar      The progress bar to alter.
 * \param progress Value between 0 and 100 for the bar to display.
 */
SGUI_DLL void sgui_progress_bar_set_progress( sgui_widget* bar,
                                              unsigned int progress );

/**
 * \brief Get the progress on a progress bar
 *
 * \param bar The progress bar.
 *
 + \return Progress value (percentage, value between 0 and 100)
 */
SGUI_DLL unsigned int sgui_progress_bar_get_progress( sgui_widget* bar );

#ifdef __cplusplus
}
#endif

#endif /* SGUI_PROGRESS_BAR_H */

