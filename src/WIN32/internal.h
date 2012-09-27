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
#include "sgui_canvas.h"
#include "sgui_rect.h"
#include "sgui_internal.h"

#define WIN32_LEAN_AND_MEAN
#define VC_EXTRA_LEAN
#define NOMINMAX

#include <windows.h>

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifndef MAPVK_VSC_TO_VK_EX
    #define MAPVK_VSC_TO_VK_EX 3
#endif

#if defined(__amd64__) || defined(__amd64) || defined(__x86_64__) ||\
    defined(__x86_64) || defined(_M_X64) || defined(_M_AMD64)

    #define SET_USER_PTR( hwnd, ptr )\
            SetWindowLongPtr( hwnd, GWLP_USERDATA, (LONG_PTR)ptr )

    #define GET_USER_PTR( hwnd ) GetWindowLongPtr( hwnd, GWLP_USERDATA )
#else
    #define SET_USER_PTR( hwnd, ptr )\
            SetWindowLong( hwnd, GWL_USERDATA, (LONG)ptr )

    #define GET_USER_PTR( hwnd ) GetWindowLong( hwnd, GWL_USERDATA )
#endif




typedef struct
{
    sgui_canvas canvas;

    void* data;
    HDC dc;
    BITMAPINFO info;
    HBITMAP bitmap;
}
sgui_canvas_gdi;

sgui_canvas_gdi* sgui_canvas_create( unsigned int width,
                                     unsigned int height );

void sgui_canvas_destroy( sgui_canvas_gdi* canvas );

void sgui_canvas_resize( sgui_canvas_gdi* canvas, unsigned int width,
                         unsigned int height );


struct sgui_window
{
    HWND hWnd;
    HINSTANCE hInstance;
    sgui_canvas_gdi* back_buffer;

    unsigned int w, h;

    int visible;

    sgui_widget_manager* mgr;
    sgui_window_callback event_fun;
};


#define SEND_EVENT( wnd, event, e )\
            if( wnd->event_fun )\
                wnd->event_fun( wnd, event, e );\
            sgui_widget_manager_send_window_event( wnd->mgr, event, e );



#endif /* INTERNAL_H */

