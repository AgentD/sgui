/*
 * sgui_tab.h
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
#ifndef SGUI_TAB_H
#define SGUI_TAB_H



#include "sgui_predef.h"



#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Create a tab group widget
 *
 * \param x      Horizontal component of the tab groups position
 * \param y      Vertical component of the tab groups position
 * \param width  Width of the tab
 * \param height Height of the tab
 *
 * \return A pointer to a new tab group widget
 */
SGUI_DLL sgui_widget* sgui_tab_group_create( int x, int y,
                                             unsigned int width,
                                             unsigned int height );

/**
 * \brief Add a tab to a tab group widget
 *
 * \param tab     The tab group widget
 * \param caption The caption of the new tab
 *
 * \return The index of the new tab
 */
SGUI_DLL int sgui_tab_group_add_tab( sgui_widget* tab, const char* caption );

/**
 * \brief Add a widget to a tab in a tab group
 *
 * \param tab The tab group widget
 * \param idx The index of the tab in the tab group
 * \param w   The widget to add
 */
SGUI_DLL void sgui_tab_group_add_widget( sgui_widget* tab, int idx,
                                         sgui_widget* w );

#ifdef __cplusplus
}
#endif

#endif /* SGUI_TAB_H */

