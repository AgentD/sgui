/*
 * window.h
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
#ifndef W32_WINDOW_H
#define W32_WINDOW_H

#include "sgui_internal.h"
#include "sgui_window.h"

#define TO_W32( window ) ((sgui_window_w32*)window)
#define ALL_FLAGS (SGUI_FIXED_SIZE|SGUI_DOUBLEBUFFERED)

#define WIN32_LEAN_AND_MEAN
#include <windows.h>



typedef struct _sgui_window_w32
{
    sgui_window super;

    HWND hWnd;
    HDC hDC;

    void* data;
    BITMAPINFO info;
    HBITMAP bitmap;
    HBRUSH bgbrush;

    struct _sgui_window_w32* next;
}
sgui_window_w32;



#ifdef __cplusplus
extern "C" {
#endif

/* in window.c: invalidate all dirty rects of the canvas */
void update_window( sgui_window_w32* wnd );

/* in window.c: handle window messages */
int handle_window_events( sgui_window_w32* wnd, UINT msg,
                          WPARAM wp, LPARAM lp );

#ifdef __cplusplus
}
#endif

#endif /* W32_WINDOW_H */

