/*
 * direct3d9.h
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
#ifndef SGUI_DIRECT3D9_H
#define SGUI_DIRECT3D9_H

#include "sgui_context.h"
#include "sgui_internal.h"

#ifndef SGUI_NO_D3D9
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <d3d9.h>



typedef struct
{
    sgui_context super;
    sgui_window* wnd;

	IDirect3DDevice9* device;
    D3DPRESENT_PARAMETERS present;
}
sgui_d3d9_context;



#ifdef __cplusplus
extern "C" {
#endif

/* swap buffers of a Direct3D 9 window */
void d3d9_swap_buffers( sgui_window* wnd );

/* turn vsync on or off for a Direct3D 9 window */
void d3d9_set_vsync( sgui_window* wnd, int interval );

/* create a Direct3D 9 context context */
sgui_context* d3d9_context_create( sgui_window* wnd,
                                   const sgui_window_description* desc,
                                   sgui_d3d9_context* share );

#ifdef __cplusplus
}
#endif

#endif /* SGUI_NO_D3D9 */

#endif /* SGUI_DIRECT3D9_H */

