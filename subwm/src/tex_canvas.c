/*
 * tex_canvas.c
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
#include "sgui_tex_canvas.h"
#include "sgui_context.h"
#include "sgui_window.h"

#include "gl_canvas.h"
#include "d3d9_canvas.h"

#include <stddef.h>



sgui_canvas* sgui_tex_canvas_create( sgui_window* wnd, sgui_context* ctx,
                                     unsigned width, unsigned int height )
{
    (void)ctx;

    if( !wnd || !width || !height )
        return NULL;

    switch( wnd->backend )
    {
#ifndef SGUI_NO_OPENGL
    case SGUI_OPENGL_CORE:
    case SGUI_OPENGL_COMPAT:
        (void)wnd; (void)ctx;
        return sgui_gl_canvas_create( width, height );
#endif

#if defined(SGUI_WINDOWS) && !defined(SGUI_NO_D3D9)
    case SGUI_DIRECT3D_9:
        (void)wnd;
        return sgui_d3d9_canvas_create( ctx, width, height );
#endif

#if defined(SGUI_WINDOWS) && !defined(SGUI_NO_D3D11)
    case SGUI_DIRECT3D_11:
        break;
#endif
    }

    return NULL;
}

void* sgui_tex_canvas_get_texture( sgui_canvas* this )
{
    return this ? ((sgui_tex_canvas*)this)->get_texture( this ) : NULL;
}

