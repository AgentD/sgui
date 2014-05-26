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


typedef struct
{
    sgui_canvas super;

    Window wnd;
    Picture wndpic;

    Picture pen;
    Pixmap penmap;

    Picture pic;
    Pixmap pixmap;

    sgui_font_cache* cache;
}
sgui_canvas_xrender;

#ifdef __cplusplus
extern "C" {
#endif

/* create an xrender based canvas */
sgui_canvas* canvas_xrender_create( Window wnd, unsigned int width,
                                    unsigned int height );

/* display a canvas on a same sized X window */
void canvas_xrender_display( sgui_canvas* cv, int x, int y,
                             unsigned int width, unsigned int height );

#ifdef __cplusplus
}
#endif

#endif /* X11_CANVAS_H */

