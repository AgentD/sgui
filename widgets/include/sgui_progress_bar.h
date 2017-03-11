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

/**
 * \file sgui_progress_bar.h
 *
 * \brief Contains the declarations of the sgui_progress_bar widget
 */
#ifndef SGUI_PROGRESS_BAR_H
#define SGUI_PROGRESS_BAR_H



#include "sgui_predef.h"



/**
 * \struct sgui_progress_bar
 *
 * \extends sgui_widget
 *
 * \brief A progress bar widget
 *
 * \image html progress.png "Two types of horizontal progress bars"
 */


#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Create a progress bar widget
 *
 * \memberof sgui_progress_bar
 *
 * \param x        Distance from the left of the window.
 * \param y        Distance from the top of the window.
 * \param flags    SGUI_PROGRESS_BAR_CONTINUOUS for a continous bar,
 *                 SGUI_PROGRESS_BAR_DISCRETE (default) for a bar out of
 *                 discrete slices. SGUI_PROGRESS_BAR_HORIZONTAL to draw a
 *                 horizontal bar (default) or SGUI_PROGRESS_BAR_VERTICAL
 *                 to draw a vertical bar.
 * \param progress Initial progress for the bar (value between 0 and 100)
 * \param length   The length of the bar in pixels.
 */
SGUI_DLL sgui_widget* sgui_progress_bar_create( int x, int y, int flags,
                                                unsigned int progress,
                                                unsigned int length );

/**
 * \brief Set the progress on a progress bar
 *
 * \memberof sgui_progress_bar
 *
 * \param bar      The progress bar to alter.
 * \param progress Value between 0 and 100 for the bar to display.
 */
SGUI_DLL void sgui_progress_bar_set_progress( sgui_widget* bar,
                                              unsigned int progress );

/**
 * \brief Get the progress on a progress bar
 *
 * \memberof sgui_progress_bar
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

