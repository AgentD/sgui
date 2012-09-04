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



#include "sgui_widget.h"
#include "sgui_canvas.h"
#include "sgui_event.h"



#ifdef __cplusplus
extern "C"
{
#endif



typedef struct sgui_widget_manager sgui_widget_manager;



/**
 * \brief Create a widget manager
 *
 * A widget manager holds and manages widgets.
 */
sgui_widget_manager* sgui_widget_manager_create( void );

/** \brief Destroy a widget manager */
void sgui_widget_manager_destroy( sgui_widget_manager* mgr );

/**
 * \brief Enable clearing the background behind a widget: default on
 *
 * If clearing is enabled, the background behind a widget is cleared before
 * rendering it. Default is enabled
 *
 * \param mgr    The widget manager
 * \param enable Whether to enable clearing
 */
void sgui_widget_manager_enable_clear( sgui_widget_manager* mgr, int enable );

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
 *
 * \return non-zero if at least one widget needs redrawing
 */
int sgui_widget_manager_update( sgui_widget_manager* mgr );

/**
 * \brief Redraw all widgets of a widget manager that need a redaw and ara
 *        flaged visible
 *
 * \param mgr The widget manager
 * \param cv  The canvas to draw to
 */
void sgui_widget_manager_draw( sgui_widget_manager* mgr, sgui_canvas* cv );

/**
 * \brief Force redrawing of all widgets of a widget manager that are inside
 *        a given area and flaged visible
 *
 * \param mgr The widget manager
 * \param cv  The canvas to draw to
 * \param x   The distance from the left of the canvas to the redraw area
 * \param y   The distance from the top of the canvas to the redraw area
 * \param w   The width of the redraw area
 * \param h   The height of the redraw area
 */
void sgui_widget_manager_force_draw( sgui_widget_manager* mgr,
                                     sgui_canvas* cv, int x, int y,
                                     unsigned int w, unsigned int h );

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
void sgui_widget_manager_send_window_event( sgui_widget_manager* mgr,
                                            int event, sgui_event* e );



#ifdef __cplusplus
}
#endif

#endif /* SGUI_WIDGET_MANAGER_H */

