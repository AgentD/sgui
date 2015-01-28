/*
 * d3d9_wm.h
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
#ifndef D3D9_WM_H
#define D3D9_WM_H



#include "sgui_predef.h"



#if defined(SGUI_WINDOWS) && !defined(SGUI_NO_D3D9)
#include "sgui_ctx_window.h"
#include "sgui_window.h"
#include "sgui_ctx_wm.h"
#include "sgui_d3d9.h"



typedef struct
{
    sgui_ctx_wm super;
    IDirect3DTexture9* skintex;
}
sgui_d3d9_wm;

typedef struct
{
    FLOAT x, y, z, rhw;
    /*FLOAT u, v;*/
}
WINDOWVERTEX;

#define WINDOWFVF (D3DFVF_XYZRHW /*| D3DFVF_TEX1*/)



#ifdef __cplusplus
extern "C" {
#endif

sgui_ctx_wm* d3d9_wm_create( sgui_window* wnd );

#ifdef __cplusplus
}
#endif

#endif /* SGUI_WINDOWS && !SGUI_NO_D3D9 */

#endif /* D3D9_WM_H */

