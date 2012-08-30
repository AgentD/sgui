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



#include "sgui_canvas.h"
#include "sgui_event.h"



#ifdef __cplusplus
extern "C" {
#endif



typedef struct sgui_widget sgui_widget;



/**
 * \brief Change the position of a widget
 *
 * \param w The widget to reposition
 * \param x The x component of the position
 * \param y The y component of the position
 */
void sgui_widget_set_position( sgui_widget* w, int x, int y );

/**
 * \brief Get the position of a widget
 *
 * \param w The widget to get the position from
 * \param x Returns the x component of the position
 * \param y Returns the y component of the position
 */
void sgui_widget_get_position( sgui_widget* w, int* x, int* y );

/**
 * \brief Get the size of a widget
 *
 * \param w      The widget to get the size of
 * \param width  Returns the width of the widget
 * \param height Returns the height of the widget
 */
void sgui_widget_get_size( sgui_widget* w,
                           unsigned int* width, unsigned int* height );

/**
 * \brief Returns non-zero if a widget intersects a given area, zero if not
 *
 * \param w      The widget to test
 * \param x      The x component of the position of the region to test
 * \param y      The y component of the position of the region to test
 * \param width  The width of the region to test
 * \param height The height of the region to test
 */
int sgui_widget_intersects_area( sgui_widget* w, int x, int y,
                                 unsigned int width, unsigned int height );

/**
 * \brief Returns non-zero if a point is inside the bounding box of a widget
 *
 * \param w The widget to test
 * \param x The x component of the position to test
 * \param y The y component of the position to test
 */
int sgui_widget_is_point_inside( sgui_widget* w, int x, int y );

/**
 * \brief Update a widget (e.g. advance animations, et cetera)
 *
 * This is called inside the sgui_window_update function to update widgets
 * assigned to a window.
 *
 * \param widget The widget to update
 */
void sgui_widget_update( sgui_widget* widget );

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
void sgui_widget_send_window_event( sgui_widget* widget, int type,
                                    sgui_event* event );

/**
 * \brief Draw a widget
 *
 * This is called to draw a widget onto a canvas.
 *
 * \param widget The widget to update.
 * \param cv     The canvas to draw to.
 */
void sgui_widget_draw( sgui_widget* widget, sgui_canvas* cv );

/**
 * \brief Returns non-zero if a widget needs to be redrawn
 *
 * If desired, after calling, the internal state of the widget is reset, so it
 * no longer reports that it needs to be redrawn.
 *
 * This is called inside the sgui_window_update function to check which
 * widgets assigned to a window need to be redrawn.
 *
 * \param keep Non-zero to keep the state of the flag, zero to reset it
 *
 * \param widget The widget to test
 */
int sgui_widget_need_redraw( sgui_widget* widget, int keep );



/**
 * \brief Register a function to be called on a given object if the given
 *        widget triggers an event
 *
 * \param widget   The widget to register the callback to.
 * \param event    The event to listen to.
 * \param callback A function that is called when the event occours.
 * \param object   An object to execute the function on (first and only
 *                 parameter).
 */
void sgui_widget_on_event( sgui_widget* widget, int event, void* callback,
                           void* object );

/**
 * \brief Register a function to be called on a given object if the given
 *        widget triggers an event
 *
 * \param widget   The widget to register the callback to.
 * \param event    The event to listen to.
 * \param callback A function that is called when the event occours.
 * \param object   An object to execute the function on (first parameter).
 * \param arg      Second pararmeter to the function.
 */
void sgui_widget_on_event_f( sgui_widget* widget, int event, void* callback,
                             void* object, float arg );

/** \copydoc sgui_widget_on_event_f */
void sgui_widget_on_event_i( sgui_widget* widget, int event, void* callback,
                             void* object, int arg );

/** \copydoc sgui_widget_on_event_f */
void sgui_widget_on_event_ui( sgui_widget* widget, int event, void* callback,
                              void* object, unsigned int arg );

/**
 * \brief Register a function to be called on a given object if the given
 *        widget triggers an event
 *
 * \param widget   The widget to register the callback to.
 * \param event    The event to listen to.
 * \param callback A function that is called when the event occours.
 * \param object   An object to execute the function on (first parameter).
 * \param px       Second pararmeter to the function.
 * \param py       Third pararmeter to the function.
 */
void sgui_widget_on_event_i2( sgui_widget* widget, int event, void* callback,
                              void* object, int px, int py );

/** \copydoc sgui_widget_on_event_ui2 */
void sgui_widget_on_event_ui2( sgui_widget* widget, int event, void* callback,
                               void* object, unsigned int px,
                               unsigned int py );

/**
 * \brief Register a function to be called on a given object if the given
 *        widget triggers an event, with a parameter returned from an other
 *        function, executet on an other object.
 *
 * \param widget       The widget to register the callback to.
 * \param event        The event to listen to.
 * \param callback     A function that is called when the event occours.
 * \param object       An object to execute the function on (first parameter).
 * \param get_callback A function to execute on get_object (only parameter to
 *                     it) that returns a float value, passed as second
 *                     paramter into the callback function.
 * \param get_object   The object to execute get_callback on.
 */
void sgui_widget_on_event_f_fun( sgui_widget* widget, int event,
                                 void* callback, void* object,
                                 void* get_callback, void* get_object );

/**
 * \brief Register a function to be called on a given object if the given
 *        widget triggers an event, with a parameter returned from an other
 *        function, executet on an other object.
 *
 * \param widget       The widget to register the callback to.
 * \param event        The event to listen to.
 * \param callback     A function that is called when the event occours.
 * \param object       An object to execute the function on (first parameter).
 * \param get_callback A function to execute on get_object (only parameter to
 *                     it) that returns an int value, passed as second
 *                     paramter into the callback function.
 * \param get_object   The object to execute get_callback on.
 */
void sgui_widget_on_event_i_fun( sgui_widget* widget, int event,
                                 void* callback, void* object,
                                 void* get_callback, void* get_object );

/**
 * \brief Register a function to be called on a given object if the given
 *        widget triggers an event, with a parameter returned from an other
 *        function, executet on an other object.
 *
 * \param widget       The widget to register the callback to.
 * \param event        The event to listen to.
 * \param callback     A function that is called when the event occours.
 * \param object       An object to execute the function on (first parameter).
 * \param get_callback A function to execute on get_object (only parameter to
 *                     it) that returns an unsigned int value, passed as
 *                     second paramter into the callback function.
 * \param get_object   The object to execute get_callback on.
 */
void sgui_widget_on_event_ui_fun( sgui_widget* widget, int event,
                                  void* callback, void* object,
                                  void* get_callback, void* get_object );



#ifdef __cplusplus
}
#endif

#endif /* SGUI_WIDGET_H */

