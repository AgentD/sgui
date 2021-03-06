/*
 * sgui_subview.h
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
 * \file sgui_subview.h
 *
 * \brief Contains the declarations of the sgui_subview widget
 */
#ifndef SGUI_SUBVIEW_H
#define SGUI_SUBVIEW_H



#include "sgui_predef.h"
#include "sgui_window.h"



/**
 * \brief subview widget draw callback
 *
 * \param subview The view that triggered the callback
 */
typedef void (* sgui_subview_draw_fun )( sgui_widget* subview );

/**
 * \brief subview window event callback
 *
 * \param subview The subview widget that triggered the event
 * \param event   A pointer to an event structure with more information
 */
typedef void (* sgui_subview_window_fun )( sgui_widget* subview,
                                           const sgui_event* event );



/**
 * \struct sgui_subview
 *
 * \extends sgui_widget
 *
 * \brief A widget that manages a sub window
 *
 * \image html glwidget.png "A sub window with an OpenGL&reg; context"
 */


#ifdef __cplusplus
extern "C"
{
#endif

/**
 * \brief Create a subview widget
 *
 * \memberof sgui_subview
 *
 * A subview widget is a widget that creates and manages a small subwindow as
 * a rendering area within a window, so you can, for instance, create an
 * OpenGL rendering area within a normal window, without having to create an
 * OpenGL context for the window itself and still being able to render normal
 * widgets around it.
 *
 * \param parent  The window for which to create the subview widget. This is
 *                required, because the widget actually creates a sub window
 *                inside this window.
 * \param x       The distance from the left of the parent window or widget.
 * \param y       The distance from the top of the parent window or widget.
 * \param width   The width of the rendering area.
 * \param height  The height of the rendering area.
 * \param backend The rendering backend to create for the subwindow.
 *                (SGUI_NATIVE, SGUI_OPENGL_CORE, etc... see
 *                sgui_window_create).
 * \param cfg     A frame buffer configuration for the window, or NULL to use
 *                a default RGBA32, depth16 singlebuffer configuration.
 */
SGUI_DLL sgui_widget* sgui_subview_create(sgui_window* parent, int x, int y,
                                          unsigned int width,
                                          unsigned int height,
                                          int backend,
                                          const sgui_window_description* cfg);

/**
 * \brief Get a direct pointer to the window managed by a subview widget
 *
 * \memberof sgui_subview
 */
SGUI_DLL sgui_window* sgui_subview_get_window( sgui_widget* subview );

/**
 * \brief Register a callback to be called when a subview requires redrawing
 *
 * \memberof sgui_subview
 *
 * \param subview The subview to register the callback to
 * \param drawcb The callback
 */
SGUI_DLL void sgui_subview_set_draw_callback( sgui_widget* subview,
                                              sgui_subview_draw_fun drawcb );

/**
 * \brief Register a callback to be called when the subwindow of a subview
 *        widget triggers an event (e.g. key pressed, mouse moved, etc...)
 *
 * \memberof sgui_subview
 *
 * \param subview  The subview widget to register the callback to
 * \param windowcb The callback
 */
SGUI_DLL void sgui_subview_on_window_event( sgui_widget* subview,
                                            sgui_subview_window_fun windowcb );

/**
 * \brief Enforce a redraw of a subview widget
 *
 * \memberof sgui_subview
 *
 * \param subview A pointer to a subview widget
 */
SGUI_DLL void sgui_subview_refresh( sgui_widget* subview );

#ifdef __cplusplus
}
#endif

#endif /* SGUI_SUBVIEW_H */

