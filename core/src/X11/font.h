/*
 * font.h
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
#ifndef X11_FONT_H
#define X11_FONT_H

#include "sgui_font.h"

#include <ft2build.h>
#include FT_FREETYPE_H

#ifdef SGUI_UNIX
    #define SYS_FONT_PATH "/usr/share/fonts/TTF/"
#elif defined SGUI_WINDOWS
    #define SYS_FONT_PATH "C:\\Windows\\Fonts\\"
#else
    #define NO_SYS_FONT_PATH 1
#endif

typedef struct {
	sgui_font super;
	FT_Face face;
	void *buffer;
	unsigned int current_glyph;
} sgui_x11_font;

#ifdef __cplusplus
extern "C" {
#endif

/* called by sgui_init( ) to initialize the font rendering system */
int font_init(void);

/* called by sgui_deinit( ) to clean up the font rendering system */
void font_deinit(void);

#ifdef __cplusplus
}
#endif

#endif /* X11_FONT_H */

