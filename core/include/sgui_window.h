/*
 * sgui_window.h
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
#ifndef SGUI_WINDOW_H
#define SGUI_WINDOW_H



#include "sgui_predef.h"
#include "sgui_widget_manager.h"



#ifdef __cplusplus
extern "C" {
#endif



#define SGUI_RESIZEABLE 1
#define SGUI_FIXED_SIZE 0

#define SGUI_VISIBLE   1
#define SGUI_INVISIBLE 0

#define SGUI_NATIVE        0
#define SGUI_OPENGL_CORE   1
#define SGUI_OPENGL_COMPAT 2



/**
 * \brief A callback for listening to window events
 *
 * \param wnd   The window that triggered the event
 * \param type  An identifyer describing the event
 * \param event Additional data to the event
 */
typedef void (* sgui_window_callback ) ( sgui_window* wnd, int type,
                                         sgui_event* event );



/**
 * \brief Create a window
 *
 * Creates a window using the platforms native window system. The window has
 * to be destroyed again using sgui_window_destroy( ), thus freeing up it's
 * resources.
 *
 * \note The window is created invisible and has to be made visible by calling
 *       sgui_window_set_visible( ).
 *
 * \param width      The width of the window(without borders and decoration).
 * \param height     The height of the window(without borders and decoration)
 * \param resizeable Non-zero if the window should be resizeable by the user,
 *                   zero if the should remain at a fixed size. The symbolic
 *                   constants SGUI_RESIZEABLE and SGUI_FIXED_SIZE can be used
 *                   to generate more readable code.
 * \param backend    What back end to use. 0 or SGUI_NATIVE for native window
 *                   system back end, SGUI_OPENGL_CORE to create a core
 *                   profile OpenGL context for the window, SGUI_OPENGL_COMPAT
 *                   for compatibillity profile. OpenGL contexts are created
 *                   for the highest version available on the current system.
 *
 * \return Either a valid pointer to a window or NULL if there was an error
 */
sgui_window* SGUI_DLL sgui_window_create( unsigned int width,
                                          unsigned int height,
                                          int resizeable,
                                          int backend );

/**
 * \brief Make the rendering context for the window current
 *
 * If the window was created with, for instance, an OpenGL rendering context
 * (see sgui_window_create), this function makes the context current for the
 * calling thread.
 */
void SGUI_DLL sgui_window_make_current( sgui_window* window );

/**
 * \brief Swap the back and the front buffer of a window
 *
 * If the window was created with, for instance, an OpenGL rendering context
 * (see sgui_window_create), this function swappes the back buffer of the
 * context with the front buffer.
 */
void SGUI_DLL sgui_window_swap_buffers( sgui_window* window );

/**
 * \brief Destroy a previously created window
 *
 * The window is closed and all it's resources are freed up, so the window
 * pointer itself is nolonger valid after a call to this function.
 *
 * This function triggers the window close event with SGUI_API_DESTROYED as
 * how paramter, no matter whether the window is visible or not.
 */
void SGUI_DLL sgui_window_destroy( sgui_window* wnd );



/**
 * \brief Get the position of the mouse pointer within a window
 *
 * \param x Returns the distance of the pointer from the left of the window
 * \param y Returns the distance of the pointer from the top of the window
 */
void SGUI_DLL sgui_window_get_mouse_position( sgui_window* wnd,
                                              int* x, int* y );

/**
 * \brief Set the mouse pointer to a position within a window
 *
 * \param x          The distance of the pointer from the left of the window.
 * \param y          The distance of the pointer from the top of the window.
 * \param send_event Non-zero if the function should generate a mouse movement
 *                   event, zero if it shouldn't.
 */
void SGUI_DLL sgui_window_set_mouse_position( sgui_window* wnd, int x, int y,
                                              int send_event );



/**
 * \brief Make a window visible or invisible
 *
 * This function shows or hides a window. The window is hidden by default
 * after creation.
 *
 * If the window is set invisible, the window close event is triggered with
 * the parameter SGUI_API_MADE_INVISIBLE.
 *
 * \param wnd     A pointer to the window
 * \param visible Non-zero to turn the window visible, zero to turn it
 *                invisible. The symbolic constants SGUI_VISIBLE and
 *                SGUI_INVISIBLE can be used to generat more readable code.
 */
void SGUI_DLL sgui_window_set_visible( sgui_window* wnd, int visible );

/** \brief Returns non-zero if a given window is visible, zero otherwise */
int SGUI_DLL sgui_window_is_visible( sgui_window* wnd );

/**
 * \brief Change the title of a window
 *
 * Most windowsystems decorate windows with borders which tend to have a
 * configurable titlebar with a text in it. This function can be used to alter
 * that title.
 *
 * \param wnd   A pointer to a window
 * \param title The new title as a NULL terminated ASCII string
 */
void SGUI_DLL sgui_window_set_title( sgui_window* wnd, const char* title );

/**
 * \brief Change the size of a window
 *
 * A call to this function will trigger a size change event.
 *
 * \param wnd     A pointer to a window
 * \param width   The width of the window(without borders and decoration).
 *                If zero, the entire screen width is used.
 * \param height  The height of the window(without borders and decoration)
 *                If zero, the entire screen height is used.
 */
void SGUI_DLL sgui_window_set_size( sgui_window* wnd,
                                    unsigned int width, unsigned int height );

/**
 * \brief Get the size of a window
 *
 * \param width  Returns width of the window. Pass NULL if you're not
 *               iteressted in it.
 * \param height Returns height of the window. Pass NULL if you're not
 *               iteressted in it.
 */
void SGUI_DLL sgui_window_get_size( sgui_window* wnd,
                                    unsigned int* width,
                                    unsigned int* height );

/** \brief Relocate a window to the center of the screen */
void SGUI_DLL sgui_window_move_center( sgui_window* wnd );

/**
 * \brief Move a window to a specified position
 *
 * \param wnd A pointer to a window
 * \param x   The distance of the left of the window to the left of the screen
 * \param y   The distance to the top of the window to the top of the screen
 */
void SGUI_DLL sgui_window_move( sgui_window* wnd, int x, int y );

/**
 * \brief Get the position of a window
 *
 * \param wnd A pointer to a window
 * \param x   The distance of the left of the window to the left of the
 *            screen. Pass NULL if you're not interessted in it.
 * \param y   The distance to the top of the window to the top of the screen.
 *            Pass NULL if you're not interessted in it.
 */
void SGUI_DLL sgui_window_get_position( sgui_window* wnd, int* x, int* y );

/**
 * \brief Set a window's event callback
 *
 * Each window may have ONE callback registered that gets called when an event
 * occours. The callback receives the pointer of the window that triggered it,
 * an event identifyer and a pointer to a structure or other datatype with
 * additional information on the event
 *
 * \param wnd A pointer to a window
 * \param fun The callback function, or NULL to unset
 */
void SGUI_DLL sgui_window_on_event( sgui_window* wnd,
                                    sgui_window_callback fun );


/**
 * \brief Add a widget to a window
 *
 * Ownership remains with the caller.
 *
 * \param wnd    The window to add the widget to
 * \param widget The widget to add
 */
void SGUI_DLL sgui_window_add_widget( sgui_window* wnd, sgui_widget* widget );

/**
 * \brief Remove a widget from a window
 *
 * \param wnd    The window to remove the widget from
 * \param widget The widget to remove
 */
void SGUI_DLL sgui_window_remove_widget( sgui_window* wnd,
                                         sgui_widget* widget );

/**
 * \brief Register a callback to be called on a widget event
 *
 * \param wnd  A pointer to a window
 * \param fun  The function to call when a widget event occours
 * \param user A user data pointer that is passed to the given function
 */
void SGUI_DLL sgui_window_on_widget_event( sgui_window* wnd,
                                           sgui_widget_callback fun,
                                           void* user );



/** \brief Get a pointer to the back buffer canvas object of the window */
sgui_canvas* SGUI_DLL sgui_window_get_canvas( sgui_window* wnd );



#ifdef __cplusplus
}
#endif

#endif /* SGUI_H */

