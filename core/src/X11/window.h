
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
#ifndef X11_WINDOW_H
#define X11_WINDOW_H


#include "sgui_internal.h"
#include "sgui_window.h"

#include "opengl.h"

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>


#define TO_X11(window) ((sgui_window_xlib *)window)
#define TO_GLX(window) ((sgui_window_glx *)window)

#define IS_CHILD 0x8000

#define X11_EVENT_MASK (ExposureMask | StructureNotifyMask |\
			PointerMotionMask | KeyPressMask | FocusChangeMask |\
			ButtonReleaseMask | KeyReleaseMask |\
			PropertyChangeMask | ButtonPressMask)


typedef struct _sgui_window_xlib {
	sgui_window super;

	Window wnd;
	XIC ic;

	unsigned int mouse_warped;       /* mouse warp counter */
} sgui_window_xlib;

#ifndef SGUI_NO_OPENGL
typedef struct {
	sgui_window_xlib super;

	GLXFBConfig cfg;
} sgui_window_glx;
#endif

#ifdef __cplusplus
extern "C" {
#endif

sgui_window *x11_window_create(sgui_lib *lib,
				const sgui_window_description *desc);

void handle_window_events(sgui_window_xlib *wnd, XEvent *e);

#ifdef __cplusplus
}
#endif

#endif /* X11_WINDOW_H */

