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
#include "sgui_widget_manager.h"
#include "sgui_canvas.h"
#include "sgui_rect.h"
#include "sgui_internal.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

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



typedef struct
{
    sgui_window base;

    HWND hWnd;

#ifndef SGUI_NO_OPENGL
    HDC hDC;
    HGLRC hRC;
#endif
}
sgui_window_w32;

extern HINSTANCE hInstance;
extern const char* wndclass;

/* in platform.c: allocates storate for a window and initialises it */
sgui_window_w32* add_window( void );

/* in platform.c: uninitialises a window and frees its memory */
void remove_window( sgui_window_w32* wnd );

/* in window.c: handle window messages */
int handle_window_events( sgui_window_w32* wnd, UINT msg,
                          WPARAM wp, LPARAM lp );

#ifndef SGUI_NO_OPENGL
/* in opengl.c: set the pixel format for a device context */
int set_pixel_format( HDC hDC );

/* in opengl.c: create OpenGL context with maximum version */
HGLRC create_context( HDC hDC, int compatibillity );
#endif

/* in canvas.c: display the canvas on a same sized window */
void display_canvas( HDC dc, sgui_canvas* cv, int x, int y,
                     unsigned int width, unsigned int height );

#endif /* INTERNAL_H */

