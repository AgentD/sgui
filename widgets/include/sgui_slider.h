/*
 * sgui_slider.h
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
#ifndef SGUI_SLIDER_H
#define SGUI_SLIDER_H



#include "sgui_predef.h"



#define SGUI_SLIDER_VERTICAL 1
#define SGUI_SLIDER_HORIZONTAL 0
#define SGUI_SLIDER_CONTINUOUS 0



/**
 * \struct sgui_slider
 *
 * \extends sgui_widget
 *
 * \brief A widget with an object that can be draged around to enter values
 *
 * \image html slider.png "Two types of vertical sliders"
 *
 * An SGUI_SLIDER_CHANGED_EVENT is fired (with integer argument set to the
 * current value) when the value changes.
 */



#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Create a slider widget
 *
 * \memberof sgui_slider
 *
 * \param x        The distance from the left of the parent
 * \param y        The distance from the top of the parent
 * \param length   The length of the widget in pixels
 * \param vertical Zero (or SGUI_SLIDER_HORIZONTAL) for a horizontal dragging
 *                 area, non-zero (or SGUI_SLIDER_VERTICAL) for a vertical
 *                 dragging area
 * \param min      The minimum value selectable
 * \param max      Tha maximum value selectable
 * \param steps    The number of discrete steps in the dragging area, or 0
 *                 (aka SGUI_SLIDER_CONTINUOUS) for a continuous slider
 *
 * \return A pointer to a slider widget on success, NULL on failure
 */
SGUI_DLL sgui_widget* sgui_slider_create( int x, int y, unsigned int length,
                                          int vertical,
                                          int min, int max,
                                          unsigned int steps );

/**
 * \brief Get the currently set value of a slider widget
 *
 * \memberof sgui_slider
 *
 * \param slider A pointer to a slider widget
 */
SGUI_DLL int sgui_slider_get_value( sgui_widget* slider );

/**
 * \brief Change the currently set value of a slider widget
 *
 * \memberof sgui_slider
 *
 * \note Calling this function does not trigger an SGUI_SLIDER_CHANGED_EVENT
 *
 * \param slider A pointer to a slider widget
 * \param value  The new value to set
 */
SGUI_DLL void sgui_slider_set_value( sgui_widget* slider, int value );

#ifdef __cplusplus
}
#endif

#endif /* SGUI_SLIDER_H */

