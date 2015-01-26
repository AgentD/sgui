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

/**
 * \file sgui_tab.h
 *
 * \brief Contains the declarations of the sgui_tab and sgui_tab_group widgets
 */
#ifndef SGUI_TAB_H
#define SGUI_TAB_H



#include "sgui_predef.h"



/**
 * \struct sgui_tab
 *
 * \extends sgui_widget
 *
 * \brief A single tab in a tab group
 *
 * \image html tab.png "Multiple tabs in a tab group"
 */

/**
 * \struct sgui_tab_group
 *
 * \extends sgui_widget
 *
 * \brief Manages a group of tab widgets
 *
 * \image html tab.png "A tab group managing multiple tabs"
 */



#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Create a tab group widget
 *
 * \memberof sgui_tab_group
 *
 * This creates a new tab group widget. Tabs have to be added to the widget by
 * creating them using sgui_tab_create and attaching them using
 * sgui_widget_add_child. Widgets can be added to the tab by using
 * sgui_widget_add_child on the tab widget. A tab can be selected by using
 * sgui_widget_set_visible.
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
 * \brief Create a tab widget
 *
 * \memberof sgui_tab
 *
 * This creates a new tab widget. The tab widget has to be added to a tab
 * group manually using sgui_widget_add_child. Widget can be added to the
 * tab by using sgui_widget_add_child. A tab can be selected by using
 * sgui_widget_set_visible.
 *
 * \param parent  A pointer to the parent group to copy the size from. The new
 *                widget is not added to the tab group.
 * \param caption A caption to print onto the new tab widget.
 */
SGUI_DLL sgui_widget* sgui_tab_create( sgui_widget* parent,
                                       const char* caption );

#ifdef __cplusplus
}
#endif

#endif /* SGUI_TAB_H */

