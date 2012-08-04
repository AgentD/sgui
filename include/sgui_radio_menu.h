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



#include "sgui_predef.h"



#ifdef __cplusplus
extern "C"
{
#endif



#define SGUI_RADIO_MENU_SELECT_0_EVENT    0
#define SGUI_RADIO_MENU_SELECT_1_EVENT    1
#define SGUI_RADIO_MENU_SELECT_2_EVENT    2
#define SGUI_RADIO_MENU_SELECT_3_EVENT    3
#define SGUI_RADIO_MENU_SELECT_4_EVENT    4
#define SGUI_RADIO_MENU_SELECT_5_EVENT    5
#define SGUI_RADIO_MENU_SELECT_6_EVENT    6
#define SGUI_RADIO_MENU_SELECT_7_EVENT    7
#define SGUI_RADIO_MENU_SELECT_8_EVENT    8
#define SGUI_RADIO_MENU_SELECT_9_EVENT    9
#define SGUI_RADIO_MENU_SELECT_10_EVENT  10
#define SGUI_RADIO_MENU_SELECT_11_EVENT  11
#define SGUI_RADIO_MENU_SELECT_12_EVENT  12
#define SGUI_RADIO_MENU_SELECT_13_EVENT  13
#define SGUI_RADIO_MENU_SELECT_14_EVENT  14
#define SGUI_RADIO_MENU_SELECT_15_EVENT  15
#define SGUI_RADIO_MENU_SELECT_16_EVENT  16
#define SGUI_RADIO_MENU_SELECT_17_EVENT  17
#define SGUI_RADIO_MENU_SELECT_18_EVENT  18
#define SGUI_RADIO_MENU_SELECT_19_EVENT  19

#define SGUI_RADIO_MENU_SELECT_EVENT     20



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
                                     const unsigned char** options,
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

