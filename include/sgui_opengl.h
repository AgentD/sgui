/*
 * sgui_opengl.h
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
#ifndef SGUI_OPENGL_H
#define SGUI_OPENGL_H



#include "sgui_predef.h"



typedef enum
{
    SGUI_OPENGL_COMPATIBILITY_PROFILE = 0x01,
    SGUI_OPENGL_FORWARD_COMPATIBLE    = 0x02,
    SGUI_OPENGL_DEBUG                 = 0x04
}
SGUI_OPENGL_FLAGS;



#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Creates a canvas object that renders to an OpenGL texture
 *
 * Note, that an OpenGL context has to be made current, before the creating,
 * destroying, calling begin or end on an OpenGL canvas.
 *
 * \param width  The width of the canvas
 * \param height The height of the canvas
 *
 * \return A pointer to an canvas that uses an OpenGL texture back end.
 */
sgui_canvas* SGUI_DLL sgui_opengl_canvas_create( unsigned int width,
                                                 unsigned int height );

/**
 * \brief Resize an OpenGL canvas
 *
 * \param canvas The OpenGL canvas to resize
 * \param width  The new width of the canvas
 * \param height The new height of the canvas
 */
void SGUI_DLL sgui_opengl_canvas_resize( sgui_canvas* canvas,
                                         unsigned int width,
                                         unsigned int height );

/** \brief Destroy an OpenGL canvas */
void SGUI_DLL sgui_opengl_canvas_destroy( sgui_canvas* canvas );

/** \brief Get the OpenGL texture handle from an OpenGL canvas */
unsigned int SGUI_DLL sgui_opengl_canvas_get_texture( sgui_canvas* canvas );


/**
 * \brief Create a window with an OpenGL context
 *
 * The window is created with a double buffered context with an 8 bit per
 * channel RGBA true color frame buffer, a 24 bit depth buffer and an 8 bit
 * stencil buffer. The context is NOT made current and one must explicitly
 * call sgui_opengl_window_make_current.
 *
 * Unlike a normal window, the window does not have a widget manager or a
 * canvas. An OpenGL canvas (see sgui_opengl_canvas_create) and a "screen"
 * (see sgui_screen_create) have to be used for handling widgets, the canvas
 * has to be rendered manually and the screen has to be supplied with window
 * events.
 *
 * \param width          The width of the window
 * \param height         The height of the window
 * \param resizeable     Wether the window should be resizeable
 *                       (see sgui_window_create).
 * \param version_major  Major version number of desired OpenGL version (e.g.
 *                       4 for 4.3). Set to 0 to choose the highest available.
 * \param version_minor  Minor version number of desired OpenGL version (e.g.
 *                       3 for 4.3). Set to 0 to choose the highest available.
 * \param flags          A bitwise or for SGUI_OPENGL_FLAGS values.
 *
 * \return A pointer to a window on success, NULL otherwise.
 */
sgui_window* SGUI_DLL sgui_opengl_window_create( unsigned int width,
                                                 unsigned int height,
                                                 int resizeable,
                                                 int version_major,
                                                 int version_minor,
                                                 int flags );

/** \brief Destroy a window created through sgui_opengl_window_create */
void SGUI_DLL sgui_opengl_window_destroy( sgui_window* window );

/**
 * \brief Make the OpenGL context of an OpenGL window current
 *
 * Only one OpenGL context can be "current" (activated for rendering) per
 * thread and it can only be current in one thread.
 * Using this function, you can make the context of an OpenGL window current
 * for the calling thread.
 *
 * \param window A pointet to the OpenGL window or NULL, to release the
 *               current context of the thread.
 */
void SGUI_DLL sgui_opengl_window_make_current( sgui_window* window );

/** \brief Swap the back and the front buffer of an OpenGL window */
void SGUI_DLL sgui_opengl_window_swap_buffers( sgui_window* window );

#ifdef __cplusplus
}
#endif

#endif /* SGUI_OPENGL_H */

