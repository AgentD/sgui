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



#define TO_X11( window ) ((sgui_window_xlib*)window)
#define ALL_FLAGS (SGUI_FIXED_SIZE|SGUI_DOUBLEBUFFERED)



typedef struct _sgui_window_xlib
{
    sgui_window super;

    Window wnd;
    XIC ic;

    int is_child;             /* Non-zero for child windows */
    int flags;                /* remembers the initial window flags */
    unsigned int mouse_warped;/* mouse warp counter */

#ifndef SGUI_NO_OPENGL
    GLXFBConfig cfg;
#endif

    struct _sgui_window_xlib* next;  /* linked list stuff */
}
sgui_window_xlib;


#ifdef __cplusplus
extern "C" {
#endif

/* process an XEvent */
void handle_window_events( sgui_window_xlib* wnd, XEvent* e );

/* generate expose events for dirty rectangles */
void update_window( sgui_window_xlib* wnd );

#ifdef __cplusplus
}
#endif

#endif /* X11_WINDOW_H */

