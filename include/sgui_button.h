/*
 * sgui_button.h
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
#ifndef SGUI_BUTTON_H
#define SGUI_BUTTON_H



#include "sgui_widget.h"



#ifdef __cplusplus
extern "C"
{
#endif



#define SGUI_BUTTON_CLICK_EVENT     0

#define SGUI_CHECKBOX_CHECK_EVENT   1
#define SGUI_CHECKBOX_UNCHECK_EVENT 2

#define SGUI_BUTTON_NORMAL   0
#define SGUI_BUTTON_CHECKBOX 1



/**
 * \brief Create a button widget
 *
 * \param x      X component of the buttons position
 * \param x      Y component of the buttons position
 * \param width  The width of the button in pixels, ignored for checkboxes
 * \param height The height of the button in pixels, ignored for checkboxes
 * \param text   UTF8 text written onto the button
 * \param type   The type of button to create (SGUI_BUTTON_NORMAL or
 *               SGUI_BUTTON_CHECKBOX)
 *
 * \return A button widget
 */
sgui_widget* sgui_button_create( int x, int y,
                                 unsigned int width, unsigned int height,
                                 const unsigned char* text, int type );

/**
 * \brief Destroy a button widget
 *
 * \param button The button widget
 */
void sgui_button_destroy( sgui_widget* button );

/**
 * \brief Set the text to be printed onto a button
 *
 * \param button The button to alter.
 * \param text   The text printed on the button.
 */
void sgui_button_set_text( sgui_widget* button, const unsigned char* text );

/**
 * \brief Set the state of a checkbox button
 *
 * \param button The button to alter.
 * \param state  Non-zero for checked, zero for unchecked
 */
void sgui_button_set_state( sgui_widget* button, int state );

/**
 * \brief Returns the state of a button
 *
 * \param button The button to get the state from
 *
 * \return For checkboxes, non-zero means checked, for normal buttons,
 *         non-zero means pressed down.
 */
int sgui_button_get_state( sgui_widget* button );



#ifdef __cplusplus
}
#endif

#endif /* SGUI_BUTTON_H */

