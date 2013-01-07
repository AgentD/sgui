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
SGUI_DLL void sgui_widget_get_position( sgui_widget* w, int* x, int* y );

/**
 * \brief Get the size of a widget
 *
 * \param w      The widget to get the size of
 * \param width  Returns the width of the widget
 * \param height Returns the height of the widget
 */
SGUI_DLL void sgui_widget_get_size( sgui_widget* w,
                                    unsigned int* width,
                                    unsigned int* height );

/**
 * \brief Returns non-zero if the given widget is configured to be rendered
 */
SGUI_DLL int sgui_widget_is_visible( sgui_widget* w );

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
SGUI_DLL void sgui_widget_get_rect( sgui_widget* w, sgui_rect* r );

/**
 * \brief Returns non-zero if a point is inside the bounding box of a widget
 *
 * \param w The widget to test
 * \param x The x component of the position to test
 * \param y The y component of the position to test
 */
SGUI_DLL int sgui_widget_is_point_inside( sgui_widget* w, int x, int y );

/**
 * \brief Send a window event to a widget
 *
 * This is called inside the sgui_window_update function to make widgets
 * interact with user input.
 *
 * \param widget The widget to send the event to
 * \param type   The event type
 * \param event  The event to send
 */
SGUI_DLL void sgui_widget_send_window_event( sgui_widget* widget, int type,
                                             sgui_event* event );

/**
 * \brief Draw a widget
 *
 * This is called to draw a widget onto a canvas.
 *
 * \param widget The widget to update.
 * \param cv     The canvas to draw to.
 */
SGUI_DLL void sgui_widget_draw( sgui_widget* widget, sgui_canvas* cv );



#ifdef __cplusplus
}
#endif

#endif /* SGUI_WIDGET_H */

