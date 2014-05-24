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

#include "opengl.h"
#include "window.h"
#include "canvas.h"
#include "pixmap.h"
#include "font.h"
#include "keycode_translate.h"

#include <stdlib.h>
#include <string.h>

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>

#include <X11/extensions/Xrender.h>


#define DPY_WIDTH DisplayWidth( dpy, DefaultScreen(dpy) )
#define DPY_HEIGHT DisplayHeight( dpy, DefaultScreen(dpy) )

extern XIM im;
extern Display* dpy;
extern Window root;
extern Atom atom_wm_delete;

/* implementation of window clipboard_write */
void xlib_window_clipboard_write( sgui_window* super, const char* text,
                                  unsigned int length );

/* implementation of window clipboard_read */
const char* xlib_window_clipboard_read( sgui_window* super );

/* add a window to the list for the main loop */
void add_window( sgui_window_xlib* window );

/* remove a window from the list */
void remove_window( sgui_window_xlib* window );

#endif /* X11_PLATFORM_H */

