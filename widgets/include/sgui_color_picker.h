/*
 * sgui_color_picker.h
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
 * \file sgui_color_picker.h
 *
 * \brief Contains the declarations of the sgui_color_picker widget
 */
#ifndef SGUI_COLOR_PICKER_H
#define SGUI_COLOR_PICKER_H



#include "sgui_predef.h"



/**
 * \struct sgui_color_picker
 *
 * \extends sgui_widget
 *
 * \brief A widget for selecting a color, designed for the sgui_color_dialog
 *
 * \image html colorpicker.png "A color picker widget used in a color dialog"
 */



#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Create a color picker widget
 *
 * \memberof sgui_color_picker
 *
 * A color picker widget is a widget that allows a user to click-select a HSV
 * color. It was created specifically for the sgui_color_dialog but may as
 * well be used otherwise. When a color is selected, an
 * SGUI_HSVA_CHANGED_EVENT is generated with the new HSVA color as argument.
 * An SGUI_RGBA_CHANGED_EVENT is also generated, holding the equivalent RGBA
 * color.
 *
 * \param x Distance from the left of the parent widget
 * \param y Distance from the left of the parent widget
 *
 * \return A pointer to a color picker widget on success, NULL on failure
 */
SGUI_DLL sgui_widget* sgui_color_picker_create( int x, int y );

/**
 * \brief Change the currently selected HSVA color of an sgui_color_picker
 *
 * \memberof sgui_color_picker
 *
 * \param picker A pointer to an sgui_color_picker
 * \param hsva   A pointer to a 4 component hsva color vector
 */
SGUI_DLL void sgui_color_picker_set_hsv( sgui_widget* picker,
                                         const unsigned char* hsva );

/**
 * \brief Change the currently selected RGBA color of an sgui_color_picker
 *
 * \memberof sgui_color_picker
 *
 * \param picker A pointer to an sgui_color_picker
 * \param rgba   A pointer to a 4 component rgba color vector
 */
SGUI_DLL void sgui_color_picker_set_rgb( sgui_widget* picker,
                                         const unsigned char* rgba );

/**
 * \brief Get the currently selected HSVA color of an sgui_color_picker
 *
 * \memberof sgui_color_picker
 *
 * \param picker A pointer to an sgui_color_picker
 * \param hsva   A pointer to a 4 component hsva color vector
 */
SGUI_DLL void sgui_color_picker_get_hsv( const sgui_widget* picker,
                                         unsigned char* hsva );

/**
 * \brief Get the currently selected RGBA color of an sgui_color_picker
 *
 * \memberof sgui_color_picker
 *
 * \param picker A pointer to an sgui_color_picker
 * \param rgba   A pointer to a 4 component rgba color vector
 */
SGUI_DLL void sgui_color_picker_get_rgb( const sgui_widget* picker,
                                         unsigned char* rgba );

#ifdef __cplusplus
}
#endif

#endif /* SGUI_COLOR_PICKER_H */

