/*
 * sgui_screen.h
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
#ifndef SGUI_SCREEN_H
#define SGUI_SCREEN_H



#include "sgui_event.h"
#include "sgui_canvas.h"
#include "sgui_widget.h"



typedef struct sgui_screen sgui_screen;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Create a screen
 *
 * A screen is used for managing widgets and having them drawn onto a canvas
 * in a way similar to a window, but without using a window. An example for
 * using a screen would be having widgets drawn into an OpenGL texture with an
 * OpenGL canvas, while still responding to user input correctly.
 *
 * \param cv The canvas object to use for drawing
 */
sgui_screen* sgui_screen_create( sgui_canvas* cv );

/**
 * \brief Destroy a screen object
 *
 * \note This does NOT destroy the screens canvas object!
 */
void sgui_screen_destroy( sgui_screen* screen );

/**
 * \brief Add a widget to a screen
 *
 * \param screen The screen to add the widget to
 * \param widget The widget to add
 */
void sgui_screen_add_widget( sgui_screen* screen, sgui_widget* widget );

/**
 * \brief Remove a widget from a screen
 *
 * \param screen The screen to remove the widget from
 * \param widget The widget to remove
 */
void sgui_screen_remove_widget( sgui_screen* screen, sgui_widget* widget );

/**
 * \brief Inject a window event to a screen
 *
 * \param screen The screen to inject the event to
 * \param type   The event type to inject
 * \param event  Addition data for the event
 */
void sgui_screen_inject_event( sgui_screen* screen, int type,
                               sgui_event* event );

/** \brief Redraw all dirty areas of a screen */
void sgui_screen_update( sgui_screen* screen );

#ifdef __cplusplus
}
#endif

#endif /* SGUI_SCREEN_H */

