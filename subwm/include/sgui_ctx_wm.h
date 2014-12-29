/*
 * sgui_ctx_wm.h
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
 * \file sgui_ctx_wm.h
 *
 * \brief Contains a window manager for sgui_ctx_window instances
 */
#ifndef SGUI_CTX_WM_H
#define SGUI_CTX_WM_H



#include "sgui_subwm_predef.h"



/**
 * \struct sgui_ctx_wm
 *
 * \brief A window manager for sgui_ctx_window instances
 */
struct sgui_ctx_wm
{
    /** \brief A list of windows */
    sgui_ctx_window* list;

    /** \brief The window that contains the managed sub windows */
    sgui_window* wnd;

    /** \brief The window that the mouse is currently hovering over */
    sgui_ctx_window* mouseover;

    /** \brief The window that currently has keyboard focus */
    sgui_ctx_window* focus;

    /** \brief Non-zero if currently dragging a window */
    int draging;

    /** \brief Last coordinates of the mouse cursor while dragging */
    int grabx, graby;

    /**
     * \brief Draw all visible windows an their contents
     *
     * \param wm A pointer to an sgui_ctx_wm object
     */
    void (* draw_gui )( sgui_ctx_wm* wm );

    /**
     * \brief Free all resources used by a window manager implementation
     *
     * This function is called after all windows are removed and destroyed, so
     * the window manager has no more windows when this is called.
     *
     * \param wm A pointer to an sgui_ctx_wm object
     */
    void (* destroy )( sgui_ctx_wm* wm );
};




/**
 * \struct sgui_gl_wm
 *
 * \implements sgui_ctx_wm
 *
 * \brief An OpenGL implementation of a window manager
 */



#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Create a new sub window manager
 *
 * \memberof sgui_ctx_wm
 *
 * \param wnd A pointer to an sgui_window to manage sgui_ctx_window
 *            objects for
 *
 * For OpenGL windows, the context has to be current when calling this.
 * OpenGL state is preserved, except for vertex array objects in OpenGL
 * 3.0+ core contexts.
 *
 * \return A pointer to a new sgui_ctx_wm or NULL on failure
 */
sgui_ctx_wm* sgui_ctx_wm_create( sgui_window* wnd );

/**
 * \brief Destroy a sub window manager and all its windows and free the
 *        memory allocated for them
 *
 * \memberof sgui_ctx_wm
 *
 * For OpenGL windows, the context has to be current when calling this.
 * OpenGL state is preserved.
 *
 * \param wm A pointer to an sgui_ctx_wm object
 */
void sgui_ctx_wm_destroy( sgui_ctx_wm* wm );

/**
 * \brief Create a new window
 *
 * \memberof sgui_ctx_wm
 *
 * \param wm     A pointer to an sgui_ctx_wm object
 * \param width  The width of the sub window in pixels
 * \param height The height of the sub window in pixles
 * \param flags  Window flags. The only valid flag is SGUI_FIXED_SIZE
 *
 * \return A pointer to a new sgui_ctx_window or NULL on failure
 */
sgui_window* sgui_ctx_wm_create_window( sgui_ctx_wm* wm, unsigned int width,
                                        unsigned int height, int flags );

/**
 * \brief Remove, but don't delete a window from a ctx window manager
 *
 * \memberof sgui_ctx_wm
 *
 * \param wm  A pointer to an sgui_ctx_wm object
 * \param wnd A pointer to a n sgui_ctx_window currently managed by the
 *            window manager
 */
void sgui_ctx_wm_remove_window( sgui_ctx_wm* wm, sgui_window* wnd );

/**
 * \brief Draw all windows for an sgui_ctx_wm using the underlying rendering
 *        system
 *
 * \memberof sgui_ctx_wm
 *
 * For OpenGL windows, all states immediately needed for rendering the UI are
 * saved and restored after rendering (shader objects used, active texture
 * unit, 2D textures bound), except for vertex array objects in OpenGL 3.0+
 * core contexts. Further more, commonly used functionallity that effects UI
 * rendering is changed and restored too, including viewport settings,
 * blending, blending factors, depth test, depth write, culling, polygon mode,
 * fixed function lighting, fixed function transformation matrices, currently
 * set matrix mode.
 *
 * If you use something not mentioned above that alters the behaviour of
 * rendering the UI (e.g. scissor test or alpha test) it must be disabled
 * manually.
 *
 * Note that for OpenGL below 3.0, the texture environment and active unit are
 * left untouched. Please set texturing to use unit 0 with normal replace mode
 * for the texture environment.
 *
 * \param wm A pointer to an sgui_ctx_wm object
 */
void sgui_ctx_wm_draw_gui( sgui_ctx_wm* wm );

/**
 * \brief Get a pointer to a window from its coordinates
 *
 * \memberof sgui_ctx_wm
 *
 * \param wm A pointer to an sgui_ctx_wm object
 * \param x  The distance from the left of the parent window
 * \param y  The distance from the top of the parent window
 *
 * \return A pointer to the topmost window at the given position or NULL if
 *         there is no window.
 */
sgui_ctx_window* sgui_ctx_wm_window_from_point( sgui_ctx_wm* wm,
                                                int x, int y );

/**
 * \brief Process an event from a parent window
 *
 * \memberof sgui_ctx_wm
 *
 * \param wm    A pointer to an sgui_ctx_wm object
 * \param event A pointer to the event to process
 */
void sgui_ctx_wm_inject_event( sgui_ctx_wm* wm, const sgui_event* event );

#ifdef __cplusplus
}
#endif

#endif /* SGUI_CTX_WM_H */

