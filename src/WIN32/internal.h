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
#define VC_EXTRA_LEAN
#define NOMINMAX

#include <windows.h>

#include <stdlib.h>
#include <string.h>
#include <ctype.h>



typedef struct
{
    sgui_window base;

    HWND hWnd;
}
sgui_window_w32;

extern HINSTANCE hInstance;
extern const char* wndclass;

/* in platform.c: allocates storate for a window and initialises it */
sgui_window_w32* add_window( void );

/* in platform.c: uninitialises a window and frees its memory */
void remove_window( sgui_window_w32* wnd );

/* in window.c: window callback */
extern LRESULT CALLBACK WindowProcFun( HWND hWnd, UINT msg,
                                       WPARAM wp, LPARAM lp );

/* in window.c: handle window messages */
void update_window( sgui_window_w32* wnd );

/* in canvas.c: display the canvas on a same sized window */
void display_canvas( HDC dc, sgui_canvas* cv, int x, int y,
                     unsigned int width, unsigned int height );

#endif /* INTERNAL_H */

