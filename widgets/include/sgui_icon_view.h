/*
 * sgui_icon_view.h
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
#ifndef SGUI_ICON_VIEW_H
#define SGUI_ICON_VIEW_H



#include "sgui_predef.h"



/**
 * \brief A function used to compare the user data pointers of two icons
 *
 * \param a A pointer to the user data of the first icon
 * \param b A pointer to the user data of the second icon
 *
 * \return A value <0 if the first is smaller than the second, >0 if the
 *         second is smaller than the first and =0 if they are equal
 */
typedef int (* sgui_icon_compare_fun )( void* a, void* b );



#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Create an icon view widget
 *
 * An icon view is an area that can display icons. Icons can be draged around,
 * selected, etc... Icons have an asociated user data pointer that gets passed
 * as event source pointer when an icon gets double clicked or enter is
 * pressed on a selected icon. The event type used is SGUI_ICON_SELECTED.
 * Similarly, an SGUI_ICON_DELETE event is generated when delete gets pressed
 * on an icon, SGUI_ICON_COPY and SGUI_ICON_CUT for copy an paste shortcuts.
 * The only event that actually has the icon view widget as source is
 * SGUI_ICON_PASTE, generated when the icon view receives a keyboard paste
 * shortcut.
 *
 * \param x          The distance of the view area from the left of the canvas
 * \param y          The distance of the view area from the top of the canvas
 * \param width      The horizontal length of the icon view area
 * \param height     The vertical length of the icon view area
 * \param cache      An icon cache object holding icons
 * \param background Non-zero to draw a frame widget style background around
 *                   the icon view area, zero to disable
 *
 * \return A pointer to an icon view widget
 */
SGUI_DLL sgui_widget* sgui_icon_view_create( int x, int y, unsigned width,
                                             unsigned int height,
                                             sgui_icon_cache* cache,
                                             int background );

/**
 * \brief Add an icon to an icon view widget
 *
 * \param view    A pointer to an icon view widget
 * \param x       The distance from the left of the view area to the icon
 * \param y       The distance from the top of the view area to the icon
 * \param subtext If not NULL, a text to drawn under the icon
 * \param id      The id of icon used by the icon cache object
 * \param user    A user data pointer to asociated with the icon. When an icon
 *                gets clicked, this pointer is given as event source.
 *                (SGUI_ICON_SELECTED event)
 */
SGUI_DLL void sgui_icon_view_add_icon( sgui_widget* view, int x, int y,
                                       const char* subtext,
                                       unsigned int id, void* user );

/**
 * \brief Snap icons in an icon view to a grid
 *
 * This function computes the largest common grid cell size of all icons in an
 * icon view and snaps their positions to the nearest gird coordinate.
 *
 * \param view A pointe to an icon view widget
 */
SGUI_DLL void sgui_icon_view_snap_to_grid( sgui_widget* view );

/**
 * \brief Sort the icons of an icon view and arange them in a grid
 *
 * \param view A pointer to an icon view widget
 * \param fun  A pointer to a comparison function
 */
SGUI_DLL void sgui_icon_view_sort( sgui_widget* view,
                                   sgui_icon_compare_fun fun );

#ifdef __cplusplus
}
#endif

#endif /* SGUI_ICON_VIEW_H */

