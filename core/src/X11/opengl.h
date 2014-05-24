/*
 * opengl.h
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
#ifndef X11_OPENGL_H
#define X11_OPENGL_H



#ifndef SGUI_NO_OPENGL
#include "sgui_window.h"
#include "sgui_context.h"

#include <GL/glx.h>



#define LOAD_GLFUN( name ) glXGetProcAddress( (const GLubyte*)(name) )

#ifndef GLX_ARB_create_context
    #define GLX_CONTEXT_MAJOR_VERSION_ARB             0x2091
    #define GLX_CONTEXT_MINOR_VERSION_ARB             0x2092
    #define GLX_CONTEXT_FLAGS_ARB                     0x2094

    #define GLX_CONTEXT_DEBUG_BIT_ARB                 0x0001
    #define GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB    0x0002
#endif

#ifndef GLX_ARB_create_context_profile
    #define GLX_CONTEXT_PROFILE_MASK_ARB              0x9126
    #define GLX_CONTEXT_CORE_PROFILE_BIT_ARB          0x00000001
    #define GLX_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB 0x00000002
#endif


typedef GLXContext (* CREATECONTEXTATTRIBSPROC )( Display*, GLXFBConfig,
                                                  GLXContext, Bool,
                                                  const int* );

struct sgui_context
{
    GLXContext gl;
};



#ifdef __cplusplus
extern "C" {
#endif

/* get a framebuffer configuration, visual and colormap creating a
   framebuffer configuration may fail, returns non-zero on success,
   zero if creating a visual or colormap failed */
int get_fbc_visual_cmap( GLXFBConfig* fbc, XVisualInfo** vi, Colormap* cmap,
                         const sgui_window_description* desc );

/* sgui_window_swap_buffers implementation for window with glx context */
void gl_swap_buffers( sgui_window* wnd );

#ifdef __cplusplus
}
#endif

#endif /* SGUI_NO_OPENGL */

#endif /* X11_OPENGL_H */

