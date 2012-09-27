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

#define DPY_WIDTH DisplayWidth( wnd->dpy, DefaultScreen(wnd->dpy) )
#define DPY_HEIGHT DisplayHeight( wnd->dpy, DefaultScreen(wnd->dpy) )

#ifndef MIN
    #define MIN( a, b ) (((a)<(b)) ? (a) : (b))
#endif

#ifndef MAX
    #define MAX( a, b ) (((a)>(b)) ? (a) : (b))
#endif

#define SEND_EVENT( wnd, event, e )\
            if( wnd->event_fun )\
                wnd->event_fun( wnd, event, e );\
            sgui_widget_manager_send_window_event( wnd->mgr, event, e );



typedef struct
{
    sgui_canvas canvas;

    void* data;
    XImage* img;
}
sgui_canvas_xlib;

sgui_canvas_xlib* sgui_canvas_create( unsigned int width, unsigned int height,
                                      Display* dpy );

void sgui_canvas_destroy( sgui_canvas_xlib* canvas );

void sgui_canvas_resize( sgui_canvas_xlib* canvas, unsigned int width,
                         unsigned int height, Display* dpy );



struct sgui_window
{
    Display* dpy;
    Window wnd;
    Atom wmDelete;
    GC gc;
    XIM im;
    XIC ic;

    sgui_canvas_xlib* back_buffer;

    sgui_widget_manager* mgr;

    int x, y;
    unsigned int w, h;
    int resizeable;
    int mapped;
    unsigned int mouse_warped;

    sgui_window_callback event_fun;
};



/* initialise keycode symbol lookup table */
void init_keycodes( );

/* convert KeySym to SGUI_KEY_CODE */
SGUI_KEY_CODE key_entries_translate( KeySym key );



#endif /* INTERNAL_H */

