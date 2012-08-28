/*
 * sgui_radio_menu.h
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
#ifndef SGUI_RADIO_MENU_H
#define SGUI_RADIO_MENU_H



#include "sgui_widget.h"



#ifdef __cplusplus
extern "C"
{
#endif



#define SGUI_RADIO_MENU_SELECT_EVENT 0



/**
 * \brief Create a radio button menu
 *
 * \param x              Distance from the left of the menu to the left of the
 *                       window.
 * \param y              Distance from the top of the menu ot the top of the
 *                       window.
 * \param num_options    The number of menu entries available.
 * \param options        An array of strings with the individual entries.
 * \param initial_option Index of the initially selected option.
 */
sgui_widget* sgui_radio_menu_create( int x, int y, unsigned int num_options,
                                     const char** options,
                                     unsigned int initial_option );

/** \brief Destroy a radio menu widget */
void sgui_radio_menu_destroy( sgui_widget* menu );

/** \brief Returns the currently selected item of a radio menu */
unsigned int sgui_radio_menu_get_selection( sgui_widget* menu );

/** \brief Alter the currently selected item of a radio menu */
void sgui_radio_menu_set_selection( sgui_widget* menu, unsigned int index );



#ifdef __cplusplus
}
#endif

#endif /* SGUI_RADIO_MENU_H */

