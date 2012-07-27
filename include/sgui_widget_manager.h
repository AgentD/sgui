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
#include "sgui_event.h"



#ifdef __cplusplus
extern "C"
{
#endif



/**
 * \brief Create a widget manager
 *
 * A widget manager holds and manages widgets.
 */
sgui_widget_manager* sgui_widget_manager_create( void );

/** \brief Destroy a widget manager */
void sgui_widget_manager_destroy( sgui_widget_manager* mgr );

/**
 * \brief Add a widget to a widget manager
 *
 * \param mgr    The widget manager
 * \param widget The widget to add
 */
void sgui_widget_manager_add_widget( sgui_widget_manager* mgr,
                                     sgui_widget* widget );

/**
 * \brief Remove a widget from a widget manager
 *
 * \param mgr    The widget manager
 * \param widget The widget to remove
 */
void sgui_widget_manager_remove_widget( sgui_widget_manager* mgr,
                                        sgui_widget* widget );

/**
 * \brief Update all widgets managed by a widget manager
 *
 * \param mgr The widget manager
 * \param wnd The window to draw to, in case a widget needs redrawing
 */
int sgui_widget_manager_update( sgui_widget_manager* mgr,
                                sgui_window* wnd );

/**
 * \brief Send an event to all widgets held by a widget manager
 *
 * The widget manager automatically generates mouse enter and mouse leave
 * events for the widgets that the mouse cursor crosses, using mouse move
 * events, passed to this function.
 *
 * \param mgr   The widget manager
 * \param wnd   The window sending the event
 * \param event The event type to send
 * \param e     The event data to send
 */
void sgui_widget_manager_send_event( sgui_widget_manager* mgr,
                                     sgui_window* wnd, int event,
                                     sgui_event* e );



#ifdef __cplusplus
}
#endif

#endif /* SGUI_WIDGET_MANAGER_H */

