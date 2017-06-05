/*
 * sgui_lib.h
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
#ifndef SGUI_LIB_H
#define SGUI_LIB_H

#include "sgui_predef.h"

/**
 * \struct sgui_lib
 *
 * \brief Encapsulates the global library state, provides backend entry points
 *
 * This structure is created by the backend and provides both an interface for
 * the backend and contains the global state of the backend.
 */
struct sgui_lib {
	/** \brief Linked list of windows managed by this instance */
	sgui_window *wndlist;

	sgui_event_queue *ev;

	/**
	 * \brief Uninitialise the backend and clean up all internal state
	 *
	 * Call this once you are done using sgui.
	 */
	void (*destroy)(sgui_lib *lib);

	/**
	 * \brief Enter the sgui main loop
	 *
	 * This function processes window system messages, relays them to
	 * windows and asks them to update. The function does not return
	 * as long as there are windows visible.
	 */
	void (*main_loop)(sgui_lib *lib);

	/**
	 * \brief Execute a single step of the main loop
	 *
	 * This function checks if there are system messages present and
	 * returns immediately if not. If there are system messages, a
	 * single one is fetched and processed and sgui events are processed,
	 * essentially executing one iteration of the regular main loop.
	 *
	 * \return Non-zero if there is at least one window visible,
	 *         zero if there are no more visible windows.
	 */
	int (*main_loop_step)(sgui_lib *lib);

	/**
	 * \brief Create a window
	 *
	 * Creates a window using the platforms native window system. The
	 * window has to be destroyed again using sgui_window_destroy( ).
	 *
	 * \note There is an easier to use wrapper function called
	 *       \ref sgui_window_create_simple
	 *
	 * \param desc A pointer to a structure holding a description of the
	 *             window that is to be created.
	 *
	 * \return Either a valid pointer to a window or NULL on error
	 */
	sgui_window *(*create_window)(sgui_lib *lib,
					const sgui_window_description *desc);
};


/**
 * \brief Connect an event to a callback for an \ref sgui_lib event queue
 *
 * A macro that wraps \ref sgui_event_connect for the event queue inside an
 * \ref sgui_lib
 */
#define sgui_connect(lib, sender, type, ...) \
	sgui_event_connect(lib->ev, sender, type, __VA_ARGS__)

/**
 * \brief Disconnect an event from a callback for an \ref sgui_lib event queue
 *
 * A macro that wraps \ref sgui_event_connect for the event queue inside an
 * \ref sgui_lib
 */
#define sgui_disconnect(lib, sender, type, callback, receiver) \
	sgui_event_disconnect(lib->ev, sender, type, callback, receiver)


#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Initialise sgui
 *
 * Call this before any other sgui function.
 *
 * \param arg Reserved for future use. Must be NULL.
 *
 * \return A pointer to an \ref sgui_lib instance on success, NULL on failure.
 */
SGUI_DLL sgui_lib *sgui_init(void *arg);

#ifdef __cplusplus
}
#endif

#endif /* SGUI_LIB_H */

