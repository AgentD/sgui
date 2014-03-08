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



#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Create an OpenGL context that shares resources with the context
 *        of a given window
 *
 * \param wnd   A pointer to a window to get the frame buffer description from
 * \param share If not NULL, a pointer to a context to share resources with
 * \param core  Non-zero for core profile, zero for compatibillity
 *
 * \return A pointer to a context object
 */
SGUI_DLL sgui_gl_context* sgui_gl_context_create( sgui_window* wnd,
                                                  sgui_gl_context* share,
                                                  int core );

/**
 * \brief Destroy an OpenGL context
 *
 * \param ctx A pointer to a context object
 */
SGUI_DLL void sgui_gl_context_destroy( sgui_gl_context* ctx );

/**
 * \brief Make an OpenGL context current
 *
 * \param ctx A pointer to a context object or NULL to release the current
 * \param wnd A pointer to a rendering window to bind the context to
 */
SGUI_DLL void sgui_gl_context_make_current( sgui_gl_context* ctx,
                                            sgui_window* wnd );

/**
 * \brief Load an extension function pointer from an OpenGL context
 *
 * \param ctx  A pointer to a context object
 * \param name The name of the function
 *
 * \return Either a pointer to the function or NULL on failure
 */
SGUI_DLL sgui_funptr sgui_gl_context_load( sgui_gl_context* ctx,
                                           const char* name );

#ifdef __cplusplus
}
#endif

#endif /* SGUI_OPENGL_H */

