/*
 * sgui_internal.h
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
 * \file sgui_internal.h
 *
 * \brief Contains internal functions and helper macros.
 */
#ifndef SGUI_INTERNAL_H
#define SGUI_INTERNAL_H



#include "sgui_predef.h"
#include "sgui_rect.h"
#include "sgui_window.h"
#include "sgui_canvas.h"
#include "sgui_skin.h"


#ifndef MAX
    #define MAX( a, b ) (((a)>(b)) ? (a) : (b))
#endif

#ifndef MIN
    #define MIN( a, b ) (((a)<(b)) ? (a) : (b))
#endif


SGUI_DLL extern sgui_skin sgui_default_skin;


#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Lock the global sgui mutex. Blocks until mutex is locked.
 *
 * The mutex is recursive, so for a certain number lock calls, the same number
 * of unlock calls is required to unlock the mutex.
 */
SGUI_DLL void sgui_internal_lock_mutex( void );

/**
 * \brief Unlock the global sgui mutex
 */
SGUI_DLL void sgui_internal_unlock_mutex( void );

/**
 * \brief Perform common operations at the end of sgui_window_create
 *
 * \memberof sgui_window
 * \protected
 *
 * This function stores the size and backend of a window in a window
 * structure.
 *
 * \param window  A pointer to a window structure
 * \param width   The width to store in the structure
 * \param height  The height to store in the structure
 * \param backend The backend to store in the structure
 */
SGUI_DLL void sgui_internal_window_post_init( sgui_window* window,
                                              unsigned int width,
                                              unsigned int height,
                                              int backend );

/**
 * \brief Propagate a window event
 *
 * \memberof sgui_window
 * \protected
 *
 * \param wnd   The window that trigered the event
 * \param e     A pointer ot a struct with additional information for an event
 */
SGUI_DLL void sgui_internal_window_fire_event( sgui_window* wnd,
                                               const sgui_event* e );

/**
 * \brief Initialise the sgui_default_skin structue with the default skin
 */
SGUI_DLL void sgui_interal_skin_init_default( void );

/**
 * \brief Uninitialize the sgui_default_skin structue
 */
SGUI_DLL void sgui_interal_skin_deinit_default( void );

SGUI_DLL void sgui_internal_memcanvas_cleanup( void );

#ifdef __cplusplus
}
#endif

#endif /* SGUI_INTERNAL_H */

