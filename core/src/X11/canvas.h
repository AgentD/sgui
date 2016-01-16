/*
 * canvas.h
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
#ifndef X11_CANVAS_H
#define X11_CANVAS_H

#include "sgui_canvas.h"
#include "sgui_font_cache.h"

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>

#include <X11/extensions/Xrender.h>


/* default size of the font cache pixmap */
#define FONT_MAP_WIDTH 256
#define FONT_MAP_HEIGHT 256


typedef struct sgui_canvas_x11
{
    sgui_canvas super;
    Drawable wnd;

    sgui_icon_cache* cache; /* a font cache by the canvas */

    void(* set_clip_rect )( struct sgui_canvas_x11* cv,
                            int left, int top, int width, int height );
}
sgui_canvas_x11;

typedef struct
{
    sgui_canvas_x11 super;
    Picture pic;

    Picture pen;
    Pixmap penmap;
}
sgui_canvas_xrender;

typedef struct
{
    sgui_canvas_x11 super;
    GC gc;
    unsigned char bg[4];
}
sgui_canvas_xlib;

#ifdef __cplusplus
extern "C" {
#endif

sgui_canvas* canvas_x11_create( Drawable wnd, unsigned int width,
                                unsigned int height, int sendexpose );

#ifdef __cplusplus
}
#endif

#endif /* X11_CANVAS_H */

