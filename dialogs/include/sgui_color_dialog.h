/*
 * sgui_color_dialog.h
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
 * \file sgui_color_dialog.h
 *
 * \brief Contains the declarations of the color dialog implementation
 */
#ifndef SGUI_COLOR_DIALOG_H
#define SGUI_COLOR_DIALOG_H



#include "sgui_predef.h"



/**
 * \struct sgui_color_dialog
 *
 * \extends sgui_dialog
 *
 * \brief A color selector dialog window
 *
 * \image html colorpicker.png "A color picker dialog"
 */



#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Create a color dialog window
 *
 * \memberof sgui_color_dialog
 *
 * \param lib     An \ref sgui_lib instance through which to create a window.
 * \param caption The caption to write onto the window
 * \param accept  The text to write onto the accept button
 * \param reject  The text to write onto the reject button
 *
 * \return A pointer to a new color dialog on success, NULL on failure
 */
SGUI_DLL sgui_dialog* sgui_color_dialog_create( sgui_lib *lib,
                                                const char* caption,
                                                const char* accept,
                                                const char* reject );

/**
 * \brief Change the currently selected color of a color dialog
 *
 * \memberof sgui_color_dialog
 *
 * \param dialog A pointer to a color dialog
 * \param rgba   A pointer to a 4 component RGBA color vector
 */
SGUI_DLL void sgui_color_dialog_set_rgba( sgui_dialog* dialog,
                                          const unsigned char* rgba );

/**
 * \brief Change the currently selected color of a color dialog
 *
 * \memberof sgui_color_dialog
 *
 * \param dialog A pointer to a color dialog
 * \param hsva   A pointer to a 4 component HSVA color vector
 */
SGUI_DLL void sgui_color_dialog_set_hsva( sgui_dialog* dialog,
                                          const unsigned char* hsva );

#ifdef __cplusplus
}
#endif

#endif /* SGUI_COLOR_DIALOG_H */

