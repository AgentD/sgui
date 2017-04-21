/*
 * platform.h
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
#ifndef X11_PLATFORM_H
#define X11_PLATFORM_H



#include "sgui_skin.h"
#include "sgui_rect.h"
#include "sgui_lib.h"

#include "opengl.h"
#include "window.h"
#include "canvas.h"
#include "font.h"
#include "keycode_translate.h"

#include <stdlib.h>
#include <string.h>

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>

#include <X11/extensions/Xrender.h>


#define DPY_WIDTH DisplayWidth( x11.dpy, x11.screen )
#define DPY_HEIGHT DisplayHeight( x11.dpy, x11.screen )

#define DOUBLE_CLICK_MS 750

typedef struct {
	sgui_lib super;

	XIM im;                         /* X11 input method */
	Display *dpy;                   /* X11 Display connection */
	Window root;                    /* root window */
	int screen;                     /* default screen of root window */

	char *clipboard_buffer;
	unsigned int clipboard_size;
	unsigned int clipboard_strlen;

	Atom atom_wm_delete;
	Atom atom_targets;
	Atom atom_text;
	Atom atom_pty;
	Atom atom_inc;
	Atom atom_UTF8;
	Atom atom_clipboard;

	sgui_window_xlib *clicked;  /* last window clicked for double click */
	unsigned long click_time;   /* last click time for double click */

	pthread_mutex_t mutex;          /* global mutex */

	sgui_window_xlib* list;         /* internal list of Xlib windows */
} sgui_lib_x11;

extern sgui_lib_x11 x11;

#ifdef __cplusplus
extern "C" {
#endif

/* implementation of window clipboard_write */
void xlib_window_clipboard_write(sgui_window *super, const char *text,
				unsigned int length);

/* implementation of window clipboard_read */
const char *xlib_window_clipboard_read(sgui_window *super);

/* add a window to the list for the main loop */
void add_window(sgui_window_xlib *window);

/* remove a window from the list */
void remove_window(sgui_window_xlib *window);

/* called from window.c when window is clicked,
   returns non-zero if double click */
int check_double_click(sgui_window_xlib *window);

/* called from window.c when mouse moves or
   otherwise interrupts double click */
void interrupt_double_click(void);

#ifdef __cplusplus
}
#endif

#endif /* X11_PLATFORM_H */

