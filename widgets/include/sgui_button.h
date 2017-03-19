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

/**
 * \file sgui_button.h
 *
 * \brief Contains the declarations of the sgui_button widget
 */
#ifndef SGUI_BUTTON_H
#define SGUI_BUTTON_H



#include "sgui_predef.h"



/**
 * \struct sgui_button
 *
 * \extends sgui_widget
 *
 * \brief A button widget
 *
 * \image html button.png "A push button"
 * \image html check.png "Check boxes"
 * \image html group.png "A group of radio buttons"
 */



#ifdef __cplusplus
extern "C"
{
#endif

/**
 * \brief Create a button widget
 *
 * \memberof sgui_button
 *
 * \param x          X component of the buttons position
 * \param x          Y component of the buttons position
 * \param width      The width of the button in pixels, set to zero to
 *                   automatically determie the ideal size.
 * \param height     The height of the button in pixels, set to zero to
 *                   automatically determie the ideal size.
 * \param text       UTF8 text written onto the button
 * \param type       What kind of button to create. Valid value are
 *                   \ref SGUI_BUTTON, \ref SGUI_TOGGLE_BUTTON,
 *                   \ref SGUI_CHECKBOX and \ref SGUI_RADIO_BUTTON.
 *
 * \return A button widget
 */
SGUI_DLL sgui_widget* sgui_button_create( int x, int y,
                                          unsigned int width,
                                          unsigned int height,
                                          const char* text,
                                          int type );

/**
 * \brief Create a button with an icon instead of a text displayed on it
 *
 * \memberof sgui_button
 *
 * \param x          X component of the buttons position
 * \param x          Y component of the buttons position
 * \param width      The width of the button in pixels, set to zero to
 *                   automatically determie the ideal size.
 * \param height     The height of the button in pixels, set to zero to
 *                   automatically determie the ideal size.
 * \param cache      A pointer to an icon cache object owning the icon
 * \param icon       The icon to display
 * \param type       What kind of button to create. Valid value are
 *                   \ref SGUI_BUTTON, \ref SGUI_TOGGLE_BUTTON,
 *                   \ref SGUI_CHECKBOX and \ref SGUI_RADIO_BUTTON.
 */
SGUI_DLL sgui_widget* sgui_icon_button_create( int x, int y,
                                               unsigned int width,
                                               unsigned int height,
                                               sgui_icon_cache* cache,
                                               sgui_icon* icon,
                                               int type );

/**
 * \brief Connect button widgets to a button group (e.g. a bunch of radio
 *        buttons to a button group)
 *
 * \memberof sgui_button
 *
 * \note Button connections MUST NOT be circular (e.g. connecting the
 *       last to the first)!
 *
 * \param button   The button to connect
 * \param previous The precceding button in the button group or NULL for none
 * \param next     The next button in the button grup or NULL for none
 */
SGUI_DLL void sgui_button_group_connect( sgui_widget* button,
                                         sgui_widget* previous,
                                         sgui_widget* next );

/**
 * \brief Set the text to be printed onto a button
 *
 * \memberof sgui_button
 *
 * \param button The button to alter.
 * \param text   The text printed on the button.
 */
SGUI_DLL void sgui_button_set_text( sgui_widget* button, const char* text );

/**
 * \brief Set an icon to be printed onto a button
 *
 * \memberof sgui_button
 *
 * \param button The button to alter.
 * \param cache  A pointer to the icon cache that holds the icon.
 * \param icon   The icon to display.
 */
SGUI_DLL void sgui_button_set_icon( sgui_widget* button,
                                    sgui_icon_cache* cache,
                                    sgui_icon* icon );

/**
 * \brief Set the state of a checkbox or radio button
 *
 * \memberof sgui_button
 *
 * \param button The button to alter.
 * \param state  Non-zero for checked, zero for unchecked
 */
SGUI_DLL void sgui_button_set_state( sgui_widget* button, int state );

/**
 * \brief Returns the state of a button
 *
 * \memberof sgui_button
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

