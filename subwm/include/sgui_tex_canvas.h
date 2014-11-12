/*
 * sgui_tex_canvas.h
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
 * \file sgui_tex_canvas.h
 *
 * \brief Contains an sgui memory canvas that synchronizes to a texture
 */
#ifndef SGUI_TEX_CANVAS_H
#define SGUI_TEX_CANVAS_H



#include "sgui_predef.h"
#include "sgui_canvas.h"



/**
 * \struct sgui_tex_canvas
 *
 * \extends sgui_mem_canvas
 *
 * \brief A memory canvas that synchronizes its contents with a texture object
 */
typedef struct
{
    sgui_mem_canvas super;

    /** \copydoc sgui_tex_canvas_get_texture */
    void* (* get_texture )( sgui_canvas* canvas );
}
sgui_tex_canvas;


/**
 * \struct sgui_d3d9_canvas
 *
 * \extends sgui_tex_canvas
 *
 * \brief A Direct3D 9 implementation of a texture canvas
 */

/**
 * \struct sgui_d3d11_canvas
 *
 * \extends sgui_tex_canvas
 *
 * \brief A Direct3D 11 implementation of a texture canvas
 */

/**
 * \struct sgui_gl_canvas
 *
 * \extends sgui_tex_canvas
 *
 * \brief An OpenGL implementation of a texture canvas
 *
 * \note An OpenGL canvas needs the context that owns the texture object to be
 *       current when sgui_canvas_end is called on the context
 */



#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Create a texture canvas
 *
 * \memberof sgui_tex_canvas
 *
 * \note If the context is an OpenGL context, this function assumes that the
 *       context is current and tries to create a texture object.
 *
 * \param wnd    A pointer to a window with a rendering context
 * \param ctx    A pointer to a rendering context to use (either the context
 *               from the window, or a context that shares resources with it)
 * \param width  The width of the canvas in pixels
 * \param height The height of the canvas in pixels
 *
 * \return A pointer to a texture canvas, or NULL on failure
 */
sgui_canvas* sgui_tex_canvas_create( sgui_window* wnd, sgui_context* ctx,
                                     unsigned width, unsigned int height );

/**
 * \brief Get a pointer to the internal texture object of a texture canvas
 *
 * \memberof sgui_tex_canvas
 *
 * \param canvas A pointer to a canvas
 *
 * For an OpenGL canvas, this returns a pointer to a GLuint texture object.
 *
 * \return A pointer to the texture object.
 */
void* sgui_tex_canvas_get_texture( sgui_canvas* canvas );

#ifdef __cplusplus
}
#endif

#endif /* SGUI_TEX_CANVAS_H */

