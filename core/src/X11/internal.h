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
#include "sgui_keycodes.h"
#include "sgui_canvas.h"
#include "sgui_rect.h"
#include "sgui_internal.h"
#include "sgui_font.h"

#include <stdlib.h>
#include <string.h>

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include <X11/extensions/Xrender.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#ifdef SGUI_NO_OPENGL
    typedef void* GLXFBConfig;
#else
    #include <GL/glx.h>

    #define LOAD_GLFUN( name ) glXGetProcAddress( (const GLubyte*)(name) )
#endif /* !SGUI_NO_OPENGL */


#define DPY_WIDTH DisplayWidth( dpy, DefaultScreen(dpy) )
#define DPY_HEIGHT DisplayHeight( dpy, DefaultScreen(dpy) )


#define TO_X11( window ) ((sgui_window_xlib*)window)
#define SEND_EVENT( WND, ID, E )\
        sgui_internal_window_fire_event( (sgui_window*)WND, ID, E )



typedef struct _sgui_window_xlib
{
    sgui_window base;

    Window wnd;
    XIC ic;

    int is_child;             /* Non-zero for child windows */
    int resizeable;           /* remembers whether the window is resizeable */
    unsigned int mouse_warped;/* mouse warp counter */

#ifndef SGUI_NO_OPENGL
    GLXContext gl;
    GLXWindow glx_wnd;
#endif

    struct _sgui_window_xlib* next;  /* linked list stuff */
}
sgui_window_xlib;

typedef struct
{
    sgui_pixmap pm;

    Pixmap pix;
    Picture pic;
}
xlib_pixmap;

extern XIM im;
extern Display* dpy;
extern Atom atom_wm_delete;
extern FT_Library freetype;

/* in platform.c: add a window to the list for the main loop */
void add_window( sgui_window_xlib* window );

/* in platform.c: remove a window from the list */
void remove_window( sgui_window_xlib* window );

/* in window.c: process an XEvent */
void handle_window_events( sgui_window_xlib* wnd, XEvent* e );

/* in canvas.c: create an xlib canvas */
sgui_canvas* canvas_xlib_create( Window wnd, unsigned int width,
                                 unsigned int height );

/* in canvas.c: display a canvas on a same sized X window */
void canvas_xlib_display( sgui_canvas* cv, int x, int y,
                          unsigned int width, unsigned int height );

#ifndef SGUI_NO_OPENGL
/* in OpenGL.c: get a framebuffer configuration, visual and colormap
   creating an framebuffer configuration may fail, returns non-zero on
   success, zero if creating a visual or colormap failed */
int get_fbc_visual_cmap( GLXFBConfig* fbc, XVisualInfo** vi, Colormap* cmap,
                         sgui_window_description* desc );

/* in OpenGL.c: try to create an OpenGL context */
int create_context( GLXFBConfig cfg, int core, sgui_window_xlib* wnd );

/* in OpenGL.c: sgui_window_swap_buffers implementation */
void gl_swap_buffers( sgui_window* wnd );
#endif

/* pixmap.c: create an xlib pixmap */
sgui_pixmap* xlib_pixmap_create( unsigned int width, unsigned int height,
                                 int format, Window wnd );

/* in keycode_translate.c: initialise keycode symbol lookup table */
void init_keycodes( );

/* in keycode_translate.c: convert KeySym to an sgui key code */
int key_entries_translate( KeySym key );



int create_font_cache( void );

void destroy_font_cache( void );

/* in font_cache.c: draw a glyph onto a destination picture using a given
   pen picture. Load the glyph into the cache if required. */
int draw_glyph( sgui_font* font, unsigned int codepoint, int x, int y,
                sgui_canvas* cv, unsigned char* color );

#endif /* INTERNAL_H */

