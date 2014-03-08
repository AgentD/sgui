/*
 * sgui_predef.h
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
#ifndef SGUI_PREDEF_H
#define SGUI_PREDEF_H



#ifdef SGUI_WINDOWS
    #ifdef SGUI_BUILDING_DLL
        #define SGUI_DLL __declspec(dllexport)
    #else
        #define SGUI_DLL __declspec(dllimport)
    #endif
#else
    #define SGUI_DLL
#endif



typedef struct sgui_font_cache sgui_font_cache;

typedef struct sgui_font sgui_font;

typedef struct sgui_rect sgui_rect;

typedef struct sgui_canvas sgui_canvas;

typedef struct sgui_widget sgui_widget;

typedef struct sgui_window sgui_window;

typedef struct sgui_pixmap sgui_pixmap;

typedef struct sgui_skin sgui_skin;

typedef struct sgui_filesystem sgui_filesystem;

typedef struct sgui_gl_context sgui_gl_context;

typedef struct sgui_event sgui_event;

typedef void(* sgui_funptr )( );



#endif /* SGUI_PREDEF_H */

