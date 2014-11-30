/*
 * gl_wm.h
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
#ifndef GL_WM_H
#define GL_WM_H



#include "sgui_subwm_predef.h"
#include "sgui_ctx_window.h"
#include "sgui_internal.h"
#include "sgui_ctx_wm.h"
#include "sgui_event.h"
#include "sgui_skin.h"



#ifndef SGUI_NO_OPENGL
#ifdef SGUI_WINDOWS
    #include <windows.h>
#endif

#ifdef SGUI_OS_X
    #include <OpenGL/gl.h>
#else
    #include <GL/gl.h>
#endif



typedef struct
{
    sgui_ctx_wm super;

    GLuint wndtex;
}
sgui_gl_wm;



#ifdef __cplusplus
extern "C" {
#endif

sgui_ctx_wm* gl_wm_create( sgui_window* wnd );

#ifdef __cplusplus
}
#endif

#endif /* SGUI_NO_OPENGL */

#endif /* GL_WM_H */

