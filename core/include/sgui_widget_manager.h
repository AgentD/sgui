/*
 * sgui_widget_manager.h
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
#ifndef SGUI_WIDGET_MANAGER_H
#define SGUI_WIDGET_MANAGER_H



#include "sgui_predef.h"



#ifdef __cplusplus
extern "C"
{
#endif



/**
 * \brief A callback for listening to widget events
 *
 * \param widget The widget that triggered the event
 * \param type   An identifyer describing the event
 * \param user   User data that was registered with the callback function
 */
typedef void (* sgui_widget_callback ) ( sgui_widget* widget, int type,
                                         void* user );



/**
 * \brief Create a widget manager
 *
 * A widget manager holds and manages widgets.
 */
SGUI_DLL sgui_widget_manager* sgui_widget_manager_create( void );

/** \brief Destroy a widget manager */
SGUI_DLL void sgui_widget_manager_destroy( sgui_widget_manager* mgr );

/**
 * \brief Get the root node of a widget manager
 *
 * \param mgr A pointer to the widget manager
 *
 * \return A pointer to the root widget
 */
SGUI_DLL sgui_widget* sgui_widget_manager_get_root(sgui_widget_manager* mgr);

/**
 * \brief Add a dirty rect (area that needs redraw) to a widget manager
 *
 * \param mgr The widget manager
 * \param r   The dirty rectangle
 */
SGUI_DLL void sgui_widget_manager_add_dirty_rect( sgui_widget_manager* mgr,
                                                  sgui_rect* r );

/**
 * \brief Get the number of dirty rectangles from a widget manager
 *
 * \param mgr The widget manager
 *
 * \return The number of dirty rectangles
 */
SGUI_DLL unsigned int sgui_widget_manager_num_dirty_rects(sgui_widget_manager*
                                                          mgr );

/**
 * \brief Get a dirty rectangle from a widget manager by index
 *
 * \param mgr  The widget manager
 * \param rect A pointer to a rectangle to write to
 * \param i    The index of the dirty rectangle
 */
SGUI_DLL void sgui_widget_manager_get_dirty_rect( sgui_widget_manager* mgr,
                                                  sgui_rect* rect,
                                                  unsigned int i );

/**
 * \brief Clear the dirty rects of a widget manager
 *
 * \param mgr The widget manager
 */
SGUI_DLL void sgui_widget_manager_clear_dirty_rects( sgui_widget_manager*
                                                     mgr );

/**
 * \brief Redraw all widgets of a widget manager that are flaged visible and
 *        are within a given rect
 *
 * \param mgr The widget manager
 * \param cv  The canvas to draw to
 * \param r   The area to redraw, or NULL to redraw everything
 */
SGUI_DLL void sgui_widget_manager_draw( sgui_widget_manager* mgr,
                                        sgui_canvas* cv,
                                        sgui_rect* r );

/**
 * \brief Send a window event to all widgets held by a widget manager
 *
 * The widget manager automatically manages keyboard focus and generates mouse
 * enter, mouse leave, focus and focus lost events for the widgets it holds.
 *
 * \param mgr   The widget manager
 * \param event The event type to send
 * \param e     The event data to send
 */
SGUI_DLL void sgui_widget_manager_send_window_event( sgui_widget_manager* mgr,
                                                     int event,
                                                     sgui_event* e );


/**
 * \brief Register a callback to be called on a widget event
 *
 * \param mgr  The widget manager
 * \param fun  The function to call when a widget event occours
 * \param user A user data pointer that is passed to the given function
 */
SGUI_DLL void sgui_widget_manager_on_event( sgui_widget_manager* mgr,
                                            sgui_widget_callback fun,
                                            void* user );

/**
 * \brief Trigger a widget event
 *
 * \param mgr    The widget manager
 * \param widget The widget that triggered the event (must not be NULL)
 * \param event  The event that got triggered
 */
SGUI_DLL void sgui_widget_manager_fire_widget_event( sgui_widget_manager* mgr,
                                                     sgui_widget* widget,
                                                     int event );


#ifdef __cplusplus
}
#endif

#endif /* SGUI_WIDGET_MANAGER_H */

