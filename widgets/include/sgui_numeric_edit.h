/*
 * sgui_numeric_edit.h
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
#ifndef SGUI_NUMERIC_EDIT_H
#define SGUI_NUMERIC_EDIT_H



#include "sgui_predef.h"
#include "sgui_edit_box.h"



/**
 * \struct sgui_numeric_edit
 *
 * \extends sgui_edit_box
 *
 * \brief An edit box for numeric values
 */

/**
 * \struct sgui_spin_box
 *
 * \extends sgui_numeric_edit
 *
 * \brief A numeric edit box with spin box buttons
 */



#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Create a numeric only edit box
 *
 * \memberof sgui_numeric_edit
 *
 * \param x       The distance from the left of the parent widget
 * \param y       The distance from the top of the parent widget
 * \param width   The width of the box in pixels
 * \param min     The minimum value that can be entered
 * \param max     The maximum value that can be entered
 * \param current The currently set value
 *
 * \return A pointer to a numeric edit box on success, NULL on failure
 */
SGUI_DLL sgui_widget* sgui_numeric_edit_create( int x, int y,
                                                unsigned int width,
                                                int min, int max,
                                                int current );

/**
 * \brief Create a spin box widget
 *
 * \memberof sgui_spin_box
 *
 * \param x        The distance from the left of the parent widget
 * \param y        The distance from the top of the parent widget
 * \param width    The width of the box in pixels
 * \param min      The minimum value that can be entered
 * \param max      The maximum value that can be entered
 * \param current  The currently set value
 * \param stepsize The value to add/subtract when the spin buttons are used
 * \param editable Non-zero if the number should be editable, zero if not
 *
 * \return A pointer to a spin box on success, NULL on failure
 */
SGUI_DLL sgui_widget* sgui_spin_box_create( int x, int y, unsigned int width,
                                            int min, int max, int current,
                                            unsigned int stepsize,
                                            int editable );

/**
 * \brief Get the value currently set in a numeric edit box
 *
 * \memberof sgui_numeric_edit
 *
 * \param eb A pointer to a numeric edit box
 *
 * \return The value currently set
 */
SGUI_DLL int sgui_numeric_edit_get_value( sgui_widget* eb );

/**
 * \brief Set the value of a numeric edit box
 *
 * \memberof sgui_numeric_edit
 *
 * \param eb    A pointer to a numeric edit box
 * \param value The value to set
 */
SGUI_DLL void sgui_numeric_edit_set_value( sgui_widget* eb, int value );

#ifdef __cplusplus
}
#endif

#endif /* SGUI_NUMERIC_EDIT_H */

