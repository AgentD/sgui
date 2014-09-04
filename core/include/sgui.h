/*
 * sgui.h
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
 * \file sgui.h
 *
 * This is the global sgui header. It includes all other sgui headers and
 * declares the initialization, cleanup and main loop functions.
 */

/**
 * \mainpage SGUI pre alpha reference manual
 *
 * \section intro Introduction
 *
 * SGUI is a miniature cross platform GUI library, written in plain ANSI C
 * (C89) with minimalistic, simple, lightweight desing in mind.
 *
 * The library offers a range of features:
 * <ul>
 * <li>Lightweight
 * <ul>
 * <li> Comparatively small size
 * <li> Comparatively small memory footprint
 * </ul>
 * <li>Easy to use aplication programming interface
 * <ul>
 * <li> Object-oriented desing
 * <li> Written entirely in ANSI C89
 * </ul>
 * <li>Cross platform
 * <ul>
 * <li> Tested on various GNU/Linux distributions, 32 and 64 bit x86
 * <li> Tested on Mircrosoft Windows(R) 98, XP and 7 (32 bit and 64 bit)
 * </ul>
 * <li>Threadsafe
 * <li>Interacts with OS clipboard (copy/cut & paste)
 * <li>Supports keyboard only user interaction
 * <li>Handwritten manual with example application
 *     in addition to doxygen reference
 * <li>OpenGL(R) support
 * <ul>
 * <li> core profile as well as compatibillity profile
 * <li> OpenGL(R) widget for rendering inside a normal GUI application
 * <li> can create a window with OpenGL(R) context
 * </ul>
 * <li>Direct3D(R) support, verions 9 and 11 (feature level 9.0 to 11.0)
 * <li>Permissive Free Software (aka "Open Source") license
 * <ul>
 * <li> X11, aka MIT license (License text added below)
 * <li> Allows you to do whatever you want, as long as credit is given
 * </ul>
 * </ul>
 *
 * \section example A simple example
 *
 * The code below creates a 400*300 pixel window with a text label in it and
 * a button that closes the window when pressed:
 * \code
 * #include "sgui.h"
 * #include <stdio.h>
 *
 * int main( )
 * {
 *     sgui_window* wnd;
 *     sgui_widget* text;
 *     sgui_widget* button;
 *
 *     sgui_init( );
 *
 *     wnd = sgui_window_create( NULL, 400, 300, SGUI_RESIZEABLE );
 *
 *     sgui_window_set_title( wnd, "Simple Sample" );
 *     sgui_window_move_center( wnd );
 *     sgui_window_set_visible( wnd, SGUI_VISIBLE );
 *
 *     text = sgui_label_create( 10, 10, "To close the window,\n"
 *                                       "press <color=\"#FF0000\">close");
 *
 *     button = sgui_button_create( 30, 60, 75, 30, "Close", 0 );
 *
 *     sgui_window_add_widget( wnd, text );
 *     sgui_window_add_widget( wnd, button );
 *
 *     sgui_event_connect( button, SGUI_BUTTON_OUT_EVENT,
 *                         sgui_window_set_visible, wnd,
 *                         SGUI_INT, SGUI_INVISIBLE );
 *
 *     sgui_main_loop( );
 *
 *     sgui_window_destroy( wnd );
 *     sgui_widget_destroy( text );
 *     sgui_widget_destroy( button );
 *     sgui_deinit( );
 *
 *     return 0;
 * }
 * \endcode
 *
 * \example clipboard.c
 * \example d3d9_0.c
 * \example d3d9_1.c
 * \example d3d9_2.c
 * \example d3d9_3.c
 * \example d3d11_0.c
 * \example d3d11_1.c
 * \example d3d11_2.c
 * \example d3d11_3.c
 * \example dialogs.c
 * \example events.c
 * \example gl0.c
 * \example gl1.c
 * \example gl2.c
 * \example gl3.c
 * \example simple.c
 * \example widget.c
 */
#ifndef SGUI_H
#define SGUI_H



#include "sgui_canvas.h"
#include "sgui_context.h"
#include "sgui_event.h"
#include "sgui_filesystem.h"
#include "sgui_font.h"
#include "sgui_icon_cache.h"
#include "sgui_keycodes.h"
#include "sgui_pixmap.h"
#include "sgui_predef.h"
#include "sgui_rect.h"
#include "sgui_skin.h"
#include "sgui_utf8.h"
#include "sgui_window.h"

#include "sgui_widget.h"
#include "sgui_button.h"
#include "sgui_color_picker.h"
#include "sgui_edit_box.h"
#include "sgui_frame.h"
#include "sgui_group_box.h"
#include "sgui_icon_view.h"
#include "sgui_image.h"
#include "sgui_label.h"
#include "sgui_numeric_edit.h"
#include "sgui_pass_box.h"
#include "sgui_progress_bar.h"
#include "sgui_scroll_bar.h"
#include "sgui_slider.h"
#include "sgui_subview.h"
#include "sgui_tab.h"

#include "sgui_messagebox.h"
#include "sgui_color_dialog.h"


#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Initialise sgui
 *
 * Call this before any other sgui function.
 *
 * \return Non-zero on success, zero on error.
 */
SGUI_DLL int sgui_init( void );

/**
 * \brief Uninitialise sgui
 *
 * Call this once you are done using sgui.
 */
SGUI_DLL void sgui_deinit( void );

/**
 * \brief Enter the sgui main loop
 *
 * This function processes window system messages, relays them to windows and
 * asks them to update. The function does not return as long as there are
 * windows visible.
 */
SGUI_DLL void sgui_main_loop( void );

/**
 * \brief Execute a single step of the main loop
 *
 * This function executes a single step of the main loop inside the
 * sgui_main_loop function, but returns after processing a single system
 * message.
 * Unlike the main loop function, it does not wait for system messages and
 * returns immediately if there are no messages left.
 *
 * \return Non-zero if there is at least one window visible, zero if there
 *         are no more visible windows.
 */
SGUI_DLL int sgui_main_loop_step( void );

#ifdef __cplusplus
}
#endif

#endif /* SGUI_H */

