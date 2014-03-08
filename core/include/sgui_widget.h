/*
 * sgui_widget.h
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
#ifndef SGUI_WIDGET_H
#define SGUI_WIDGET_H



#include "sgui_predef.h"



#ifdef __cplusplus
extern "C" {
#endif



/**
 * \brief Destroy a widget, freeing up all its resources
 *
 * \param widget A pointer to the widget to destroy
 */
SGUI_DLL void sgui_widget_destroy( sgui_widget* widget );

/**
 * \brief Change the position of a widget
 *
 * \param w The widget to reposition
 * \param x The x component of the position
 * \param y The y component of the position
 */
SGUI_DLL void sgui_widget_set_position( sgui_widget* w, int x, int y );

/**
 * \brief Get the position of a widget
 *
 * \param w The widget to get the position from
 * \param x Returns the x component of the position
 * \param y Returns the y component of the position
 */
SGUI_DLL void sgui_widget_get_position( const sgui_widget* w,
                                        int* x, int* y );

/**
 * \brief Get the absolute position of a widget (i.e. not parent relative
 *        but relative to the window)
 *
 * \param w The widget to get the position from
 * \param x Returns the x component of the position
 * \param y Returns the y component of the position
 */
SGUI_DLL void sgui_widget_get_absolute_position( const sgui_widget* w,
                                                 int* x, int* y );

/**
 * \brief Get the size of a widget
 *
 * \param w      The widget to get the size of
 * \param width  Returns the width of the widget
 * \param height Returns the height of the widget
 */
SGUI_DLL void sgui_widget_get_size( const sgui_widget* w,
                                    unsigned int* width,
                                    unsigned int* height );

/**
 * \brief Returns non-zero if the given widget is configured to be rendered
 */
SGUI_DLL int sgui_widget_is_visible( const sgui_widget* w );

/**
 * \brief Returns non-zero if the given widget is visible and all its parents
 *        are visible too
 */
SGUI_DLL int sgui_widget_is_absolute_visible( const sgui_widget* w );

/**
 * \brief Set whether a given widget should be rendered or not
 *
 * \param w       The widget
 * \param visible Non-zero to allow rendering of the widget, zero to prohibit.
 */
SGUI_DLL void sgui_widget_set_visible( sgui_widget* w, int visible );

/**
 * \brief Get the bounding rectangle of a widget
 *
 * \param w The widget
 * \param r Returns the rectangle
 */
SGUI_DLL void sgui_widget_get_rect( const sgui_widget* w, sgui_rect* r );

/**
 * \brief Get the bounding box of a widget in absolute coordinates (i.e. not
 *        parent relative but relative to the window)
 *
 * \param w The widget to get the position from
 * \param r Returns the rect
 */
SGUI_DLL void sgui_widget_get_absolute_rect( const sgui_widget* w,
                                             sgui_rect* r );

/**
 * \brief Send a parent widget or window event to a widget
 *
 * This is called inside the sgui_window_update function to make widgets
 * interact with user input or by parent widgets to send things like tab group
 * deselect notifications.
 *
 * \param widget    The widget to send the event to
 * \param event     The event to send
 * \param propagate Nonzero if the event should be propagate to the widgets
 *                  children, zero if it should not
 */
SGUI_DLL void sgui_widget_send_event( sgui_widget* widget,
                                      const sgui_event* event,
                                      int propagate );

/**
 * \brief Draw a widget
 *
 * This is called to draw a widget onto a canvas.
 *
 * \param widget The widget to update.
 */
SGUI_DLL void sgui_widget_draw( sgui_widget* widget );

/**
 * \brief Add a widget as child widget to an other widget
 *
 * \param parent The parent to add the child widget to
 * \param child  The child widget to add to the parent
 */
SGUI_DLL void sgui_widget_add_child( sgui_widget* parent, sgui_widget* child );

/**
 * \brief Remove a widget from the widget hirarchy
 *
 * This function removes a widget from its parent and sets its canvas pointer
 * to NULL, as well as all the canvas pointers of all the children of the
 * given widget.
 *
 * \param widget The widget to remove from its parent
 */
SGUI_DLL void sgui_widget_remove_from_parent( sgui_widget* widget );

/**
 * \brief Get a pointer to a child widget from a point within a widget
 *
 * The function first tests if the given point is inside the given widget.
 * If it is, it continues to test against all children of the given widget
 * and, when it finds a match, transforms the point to child local coordinates
 * and recursively continues with that child widget, until the least child at
 * the bottom of the tree is found that has the given point inside it.
 *
 * If at any point the function runs out of children (i.e. the point does not
 * lie inside any child), the last widget above in the hirarchy is returned.
 *
 * If the point does not even lie in the given widget to begin with, NULL is
 * returned.
 *
 * Also, widgets are skiped if they are not flagged visible.
 *
 * \param widget The widget to test against
 * \param x The distance from the left side of the widget
 * \param y The distance from the top side of the widget
 *
 * \return A pointer to a widget or NULL if the point does not lie inside any
 *         widget.
 */
SGUI_DLL sgui_widget* sgui_widget_get_child_from_point(
                                            const sgui_widget* widget,
                                            int x, int y );

/**
 * \brief Assuming that the given widget has focus, find the next widget
 *        that will get focus when the tab key gets pressed
 *
 * \param w A pointer to a widget that is assumed to have focus
 *
 * \return A pointer to the next widget in the tab order
 */
SGUI_DLL sgui_widget* sgui_widget_find_next_focus( const sgui_widget* w );


#ifdef __cplusplus
}
#endif

#endif /* SGUI_WIDGET_H */

