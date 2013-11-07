/*
 * internal.h
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
#ifndef INTERNAL_H
#define INTERNAL_H



#include "sgui_window.h"
#include "sgui_skin.h"
#include "sgui_canvas.h"
#include "sgui_rect.h"
#include "sgui_internal.h"
#include "sgui_font.h"
#include "sgui_pixmap.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#ifndef SGUI_NO_OPENGL
#include <GL/gl.h>
#endif

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#if defined(__amd64__) || defined(__amd64) || defined(__x86_64__) ||\
    defined(__x86_64) || defined(_M_X64) || defined(_M_AMD64)

    #define SET_USER_PTR( hwnd, ptr )\
            SetWindowLongPtr( hwnd, GWLP_USERDATA, (LONG_PTR)ptr )

    #define GET_USER_PTR( hwnd ) GetWindowLongPtr( hwnd, GWLP_USERDATA )
#else
    #define SET_USER_PTR( hwnd, ptr )\
            SetWindowLong( hwnd, GWL_USERDATA, (LONG)ptr )

    #define GET_USER_PTR( hwnd ) GetWindowLong( hwnd, GWL_USERDATA )
#endif

#ifndef MAPVK_VSC_TO_VK_EX
    #define MAPVK_VSC_TO_VK_EX 3
#endif

#define TO_W32( window ) ((sgui_window_w32*)window)



typedef struct _sgui_window_w32
{
    sgui_window super;

    HWND hWnd;
    HDC hDC;

    void* data;
    BITMAPINFO info;
    HBITMAP bitmap;
    HBRUSH bgbrush;

#ifndef SGUI_NO_OPENGL
    HGLRC hRC;
#endif

    struct _sgui_window_w32* next;
}
sgui_window_w32;

extern HINSTANCE hInstance;
extern const char* wndclass;
extern FT_Library freetype;

sgui_pixmap* get_skin_pixmap( void );

/* in platform.c: add a window to the list used by the main loop */
void add_window( sgui_window_w32* wnd );

/* in platform.c: remove a window */
void remove_window( sgui_window_w32* wnd );

/* in window.c: invalidate all dirty rects of the canvas */
void update_window( sgui_window_w32* wnd );

/* in window.c: handle window messages */
int handle_window_events( sgui_window_w32* wnd, UINT msg,
                          WPARAM wp, LPARAM lp );

/* in opengl.c: create OpenGL context with maximum version,
   return non-zero on success, zero on failure */
int create_gl_context( sgui_window_w32* wnd, sgui_window_description* desc );

/* in opengl.c: destroy OpenGL context of a window */
void destroy_gl_context( sgui_window_w32* wnd );

/* in opengl.c: swap buffers of an OpenGL context of a window */
void gl_swap_buffers( sgui_window* wnd );

/* in opengl.c: make the OpenGL context of a window current */
void gl_make_current( sgui_window_w32* wnd );

/* in opengl.c: turn vsync on or off */
void gl_set_vsync( sgui_window_w32* wnd, int vsync_on );


#endif /* INTERNAL_H */

