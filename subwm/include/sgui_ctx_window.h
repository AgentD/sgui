/*
 * sgui_ctx_window.h
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
 * \file sgui_ctx_window.h
 *
 * \brief Contains a texture canvas based window implementation
 */
#ifndef SGUI_CTX_WINDOW_H
#define SGUI_CTX_WINDOW_H



#include "sgui_subwm_predef.h"
#include "sgui_window.h"



/**
 * \struct sgui_ctx_window
 *
 * \brief A texture canvas based window implementation
 *
 * An sgui_ctx_window is a window implementation that uses a texture canvas
 * created through another window to render sub windows within an sgui window
 * that has a 3D rendering context. The sub windows are managed by a simple
 * window manager implementation.
 *
 * Context sub windows work mostly like normal windows, except that cannot
 * have sub windows themselves.
 */
struct sgui_ctx_window
{
    sgui_window super;

    /** \brief A pointer to the window manager responsible for this window */
    sgui_ctx_wm* wm;

    /** \brief A pointer to the window that contains this window */
    sgui_window* parent;

    /** \brief Linked list pointer used by the window manager */
    sgui_ctx_window* next;
};



#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Create a context sub window
 *
 * \memberof sgui_ctx_window
 *
 * The window returned by this function has to be destroyed
 * using sgui_window_destroy.
 *
 * \param parent A pointer to window to create the context sub window in. The
 *               window must not have an SGUI_NATIVE or SGUI_CUSTOM backend.
 * \param width  The width of the window in pixels, including window
 *               decoration
 * \param height The height of the window in pixels, including window
 *               decoration
 * \param flags  A set of window flags. The only valid flags is
 *               SGUI_FIXED_SIZE.
 *
 * \return A pointer to a new sgui_ctx_window object, or NULL on failure
 */
sgui_window* sgui_ctx_window_create( sgui_window* parent,
                                     unsigned int width, unsigned int height,
                                     int flags );

/**
 * \brief Rerender the widgets of a window into the underlying texture canvas
 *
 * \memberof sgui_ctx_window
 *
 * \param wnd A pointer to an sgui_ctx_window
 */
void sgui_ctx_window_update_canvas( sgui_window* wnd );

/**
 * \brief Get the texture of the texture canvas of an sgui_ctx_window
 *
 * \memberof sgui_ctx_window
 *
 * \see sgui_tex_canvas_get_texture
 *
 * \param wnd A pointer to an sgui_ctx_window
 *
 * \return A pointer to the underlying texture object
 */
void* sgui_ctx_window_get_texture( sgui_window* wnd );

/**
 * \brief Inject an event to a window
 *
 * \param wnd
 * \param ev
 */
void sgui_ctx_window_inject_event( sgui_window* wnd, const sgui_event* ev );

#ifdef __cplusplus
}
#endif

#endif /* SGUI_CTX_WINDOW_H */

