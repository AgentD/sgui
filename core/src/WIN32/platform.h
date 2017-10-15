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
#ifndef PLATFORM_H
#define PLATFORM_H

#include "sgui_skin.h"
#include "sgui_canvas.h"
#include "sgui_rect.h"
#include "sgui_internal.h"
#include "sgui_pixmap.h"
#include "sgui_lib.h"

#include "window.h"
#include "opengl.h"
#include "direct3d9.h"
#include "direct3d11.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#if defined(__amd64__) || defined(__amd64) || defined(__x86_64__) || \
	defined(__x86_64) || defined(_M_X64) || defined(_M_AMD64)

	#define SET_USER_PTR(hwnd, ptr) \
		SetWindowLongPtrA(hwnd, GWLP_USERDATA, (LONG_PTR)ptr)

	#define GET_USER_PTR(hwnd) GetWindowLongPtrA(hwnd, GWLP_USERDATA)
#else
	#define SET_USER_PTR(hwnd, ptr) \
		SetWindowLongA(hwnd, GWL_USERDATA, (LONG)ptr)

	#define GET_USER_PTR(hwnd) GetWindowLongA(hwnd, GWL_USERDATA)
#endif

#ifndef MAPVK_VSC_TO_VK_EX
	#define MAPVK_VSC_TO_VK_EX 3
#endif

typedef struct {
	sgui_lib super;

	HINSTANCE hInstance;		/* instance handle */
	const char *wndclass;		/* window class name */
	CRITICAL_SECTION mutex;		/* global sgui mutex */
	char *clipboard;		/* clipboard translaton buffer */
} sgui_lib_w32;

extern sgui_lib_w32 w32;

#ifdef __cplusplus
extern "C" {
#endif

/* convert an UTF-8 string to UTF-16 */
WCHAR *utf8_to_utf16(const char *utf8, int rdbytes);

/* convert an UTF-16 string to UTF-8 */
char *utf16_to_utf8(WCHAR *utf16);

/* implementation of the clipboard write function */
void w32_window_write_clipboard(sgui_window *wnd, const char *text,
				unsigned int length);

/* implementation of the clipboard read function */
const char *w32_window_read_clipboard(sgui_window *wnd);

#ifdef __cplusplus
}
#endif

#endif /* PLATFORM_H */

