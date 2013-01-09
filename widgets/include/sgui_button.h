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



#include "sgui_predef.h"



#ifdef __cplusplus
extern "C"
{
#endif

/**
 * \brief Create a button widget
 *
 * \param x      X component of the buttons position
 * \param x      Y component of the buttons position
 * \param width  The width of the button in pixels
 * \param height The height of the button in pixels
 * \param text   UTF8 text written onto the button
 *
 * \return A button widget
 */
SGUI_DLL sgui_widget* sgui_button_create( int x, int y,
                                          unsigned int width,
                                          unsigned int height,
                                          const char* text );

/**
 * \brief Create a checkbox button widget
 *
 * \param x    X component of the buttons position
 * \param x    Y component of the buttons position
 * \param text UTF8 text written next to the check box
 *
 * \return A checkbox button widget
 */
SGUI_DLL sgui_widget* sgui_checkbox_create( int x, int y, const char* text );

/**
 * \brief Create a radio button widget
 *
 * \param x      X component of the radio buttons position
 * \param x      Y component of the radio buttons position
 * \param text   UTF8 text written next to the button
 *
 * \return A radio button widget
 */
SGUI_DLL sgui_widget* sgui_radio_button_create( int x, int y,
                                                const char* text );

/**
 * \brief Connect radio button widgets to a radio button menu
 *
 * \note Radio button connections MUST NOT be circular (e.g. connecting the
 *       last to the first)!
 *
 * \param radio    The radio button to connect
 * \param previous The precceding radio button in the radio button menu
 * \param next     The next radio button in the radio button menu
 */
SGUI_DLL void sgui_radio_button_connect( sgui_widget* radio,
                                         sgui_widget* previous,
                                         sgui_widget* next );

/**
 * \brief Destroy a button widget
 *
 * \param button The button widget
 */
SGUI_DLL void sgui_button_destroy( sgui_widget* button );

/**
 * \brief Set the text to be printed onto a button
 *
 * \param button The button to alter.
 * \param text   The text printed on the button.
 */
SGUI_DLL void sgui_button_set_text( sgui_widget* button, const char* text );

/**
 * \brief Set the state of a checkbox or radio button
 *
 * \param button The button to alter.
 * \param state  Non-zero for checked, zero for unchecked
 */
SGUI_DLL void sgui_button_set_state( sgui_widget* button, int state );

/**
 * \brief Returns the state of a button
 *
 * \param button The button to get the state from
 *
 * \return For checkboxes, non-zero means checked, for normal buttons,
 *         non-zero means pressed down.
 */
SGUI_DLL int sgui_button_get_state( sgui_widget* button );

#ifdef __cplusplus
}
#endif

#endif /* SGUI_BUTTON_H */
