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
#include "sgui_keycodes.h"
#include "sgui_canvas.h"
#include "sgui_rect.h"
#include "sgui_internal.h"

#include <stdlib.h>
#include <string.h>

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include <GL/glx.h>



#define DPY_WIDTH DisplayWidth( dpy, DefaultScreen(dpy) )
#define DPY_HEIGHT DisplayHeight( dpy, DefaultScreen(dpy) )


#define TO_X11( window ) ((sgui_window_xlib*)window)
#define SEND_EVENT( WND, ID, E )\
        sgui_internal_window_fire_event( (sgui_window*)WND, ID, E )



typedef struct
{
    sgui_window base;

    Window wnd;
    XIC ic;

    int resizeable;
    unsigned int mouse_warped;

    union
    {
        GC xlib;
        GLXContext gl;
    }
    context;
}
sgui_window_xlib;

extern XIM im;
extern Display* dpy;
extern Atom atom_wm_delete;

/* in platform.c: allocates storate for a window and initialises it */
sgui_window_xlib* add_window( void );

/* in platform.c: uninitialises a window and frees its memory */
void remove_window( sgui_window_xlib* window );

/* in window.c: process an XEvent */
void handle_window_events( sgui_window_xlib* wnd, XEvent* e );

/* in window.c: implementation for Xlib window */
void window_x11_get_mouse_position( sgui_window* wnd, int* x, int* y );

/* in window.c: implementation for Xlib window */
void window_x11_set_mouse_position( sgui_window* wnd, int x, int y );

/* in window.c: implementation for Xlib window */
void window_x11_set_visible( sgui_window* wnd, int visible );

/* in window.c: implementation for Xlib window */
void window_x11_set_title( sgui_window* wnd, const char* title );

/* in window.c: implementation for Xlib window */
void window_x11_set_size( sgui_window* wnd,
                          unsigned int width, unsigned int height );

/* in window.c: implementation for Xlib window */
void window_x11_move_center( sgui_window* wnd );

/* in window.c: implementation for Xlib window */
void window_x11_move( sgui_window* wnd, int x, int y );

/* in canvas.c: display a canvas on a same sized X window */
void display_canvas( Window wnd, GC gc, sgui_canvas* cv, int x, int y,
                     unsigned int width, unsigned int height );


/* in keycode_translate.c: initialise keycode symbol lookup table */
void init_keycodes( );

/* in keycode_translate.c: convert KeySym to SGUI_KEY_CODE */
SGUI_KEY_CODE key_entries_translate( KeySym key );

#endif /* INTERNAL_H */

