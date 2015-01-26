/*
 * sgui_pass_box.h
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
 * \file sgui_pass_box.h
 *
 * \brief Contains the declarations of the sgui_pass_box widget
 */
#ifndef SGUI_PASS_BOX_H
#define SGUI_PASS_BOX_H



#include "sgui_predef.h"
#include "sgui_edit_box.h"



/**
 * \struct sgui_pass_box
 *
 * \extends sgui_edit_box
 *
 * \brief A password entry edit box that does not display the entered text
 */



#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Create a password entry box
 *
 * \memberof sgui_pass_box
 *
 * A password entry box is an edit box that does not display the entered text.
 * A sequence of replacement characters (something like '*') is displayed
 * instead. The entered text can not be cut or copied by the user.
 *
 * \param x         The distance from the left of the parent widget
 * \param y         The distance from the top of the parent widget
 * \param width     The width of the box in pixels
 * \param max_chars The maximum number of characters that can be entered
 *
 * \return A pointer to a password edit box on success, NULL on failure
 */
SGUI_DLL sgui_widget* sgui_pass_box_create( int x, int y, unsigned int width,
                                            unsigned int max_chars );

/**
 * \brief Get the text currently entered into a password edit box
 *
 * \memberof sgui_pass_box
 *
 * \param pbox A pointer to a password edit box
 *
 * \return A pointer to the actual entered text
 */
SGUI_DLL const char* sgui_pass_box_get_text( sgui_widget* pbox );

#ifdef __cplusplus
}
#endif

#endif /* SGUI_PASS_BOX_H */

