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
#include "sgui_font.h"

#include <stdlib.h>
#include <string.h>

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include <X11/extensions/Xrender.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#ifndef SGUI_NO_OPENGL
#include <GL/glx.h>

#define LOAD_GLFUN( name ) glXGetProcAddress( (const GLubyte*)(name) )

#ifndef GLX_VERSION_1_3
    typedef struct GLXFBConfigRec* GLXFBConfig;

    #define GLX_X_RENDERABLE  0x8012
    #define GLX_X_VISUAL_TYPE 0x22
    #define GLX_TRUE_COLOR    0x8002
#endif

#ifndef GLX_ARB_create_context
    #define GLX_CONTEXT_MAJOR_VERSION_ARB             0x2091
    #define GLX_CONTEXT_MINOR_VERSION_ARB             0x2092
    #define GLX_CONTEXT_FLAGS_ARB                     0x2094

    #define GLX_CONTEXT_DEBUG_BIT_ARB                 0x0001
    #define GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB    0x0002
#endif

#ifndef GLX_ARB_create_context_profile
    #define GLX_CONTEXT_PROFILE_MASK_ARB              0x9126
    #define GLX_CONTEXT_CORE_PROFILE_BIT_ARB          0x00000001
    #define GLX_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB 0x00000002
#endif

typedef GLXFBConfig* (* CHOOSEFBCFGPROC )(Display*,int,const int*,int*);
typedef XVisualInfo* (* GETVISUALFROMFBCPROC )(Display*, GLXFBConfig);
typedef GLXContext   (* CREATECONTEXTATTRIBSPROC )( Display*, GLXFBConfig,
                                                    GLXContext, Bool,
                                                    const int* );
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

    int resizeable;           /* remembers whether the window is resizeable */
    unsigned int mouse_warped;/* mouse warp counter */

#ifndef SGUI_NO_OPENGL
    GLXContext gl;
#endif

    struct _sgui_window_xlib* next;  /* linked list stuff */
}
sgui_window_xlib;

extern XIM im;
extern Display* dpy;
extern Atom atom_wm_delete;
extern FT_Library freetype;

/* in platform.c: add a window to the list for the main loop */
void add_window( sgui_window_xlib* window );

/* in platform.c: remove a window from the list */
void remove_window( sgui_window_xlib* window );

/* in platform.c: find the first window with an OpenGL context in the list */
sgui_window_xlib* find_gl_window( void );

/* in window.c: process an XEvent */
void handle_window_events( sgui_window_xlib* wnd, XEvent* e );

/* in canvas.c: create an xlib canvas */
sgui_canvas* canvas_xlib_create( Window wnd, unsigned int width,
                                 unsigned int height );

/* in canvas.c: display a canvas on a same sized X window */
void canvas_xlib_display( sgui_canvas* cv, int x, int y,
                          unsigned int width, unsigned int height );

#ifndef SGUI_NO_OPENGL
/* in OpenGL.c: get a framebuffer configuration */
GLXFBConfig get_fb_config( void );

/* in OpenGL.c: get a visual from a framebuffer configuration */
XVisualInfo* get_visual_from_fbc( GLXFBConfig fbc );

/* in OpenGL.c: get a visual without a framebuffer configuration */
XVisualInfo* get_visual_old( void );

/* in OpenGL.c: try to create an OpenGL context with the maximum version */
GLXContext create_context( GLXFBConfig cfg, GLXContext share );
#endif

/* in keycode_translate.c: initialise keycode symbol lookup table */
void init_keycodes( );

/* in keycode_translate.c: convert KeySym to an sgui key code */
int key_entries_translate( KeySym key );

#endif /* INTERNAL_H */

