/*
 * sgui_d3d11.h
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
#ifndef SGUI_D3D11_H
#define SGUI_D3D11_H



#include "sgui_context.h"
#include "sgui_window.h"

#define WIN32_LEAN_AND_MEAN

#ifndef COBJMACROS
    #define COBJMACROS
#endif

#ifdef WIDL_C_INLINE_WRAPPERS
    #undef WIDL_C_INLINE_WRAPPERS
#endif

#include <windows.h>
#include <d3d11.h>



typedef struct
{
    sgui_context super;
    sgui_window* wnd;

    int syncrate;

    IDXGISwapChain* swapchain;
    ID3D11Device* dev;
    ID3D11DeviceContext* ctx;

    ID3D11RenderTargetView* backbuffer;

    ID3D11Texture2D* ds_texture;
    ID3D11DepthStencilView* dsv;
}
sgui_d3d11_context;



#endif /* SGUI_D3D11_H */

