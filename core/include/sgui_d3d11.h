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

    /** \brief Non-zero if vsync is enabled, zero if not */
    int syncrate;

    /** \brief A pointer to the swap chain crated for the window */
    IDXGISwapChain* swapchain;

    /** \brief A pointer to the device crated for the window */
    ID3D11Device* dev;

    /** \brief A pointer to an immediate context crated from the device */
    ID3D11DeviceContext* ctx;

    /**
     * \brief A pointer to a render target view created from the swapchain
     *
     * When the window size changes, the swapchain is automatically resized
     * and the backbuffer is recreated. If and only if this render target view
     * was bound before resizing, it is bound again afterwards and the
     * viewport is set to the entire window size.
     *
     * \note In order for the automatic resizing to work, you MUST NOT do
     *       anything that increases the reference count of this object.
     */
    ID3D11RenderTargetView* backbuffer;

    /**
     * \brief A pointer to a depth stencil texture or NULL if not requested
     *
     * If a depth buffer or stencil buffer was requested during window
     * creation, a window sized depth/stencil texture is created along with
     * a depth stencil view. This holds a pointer to the depth/stencil texture
     * if requested, or NULL if not requested.
     *
     * The texture is automatically resized when the window size changes. If
     * and only if the swap chain back buffer was bound before resizing, the
     * depth stencil view created for the texture is bound along with the
     * back buffer after resizing is done.
     *
     * \note In order for the automatic resizing to work, you MUST NOT do
     *       anything that increases the reference count of this object.
     */
    ID3D11Texture2D* ds_texture;

    /**
     * \brief A pointer to a depth stencil texture or NULL if not requested
     *
     * If a depth buffer or stencil buffer was requested during window
     * creation, a window sized depth/stencil texture is created along with
     * a depth stencil view. This holds a pointer to the depth/stencil view
     * if requested, or NULL if not requested.
     *
     * The underlying texture is automatically resized when the window size
     * changes and the view is recreated. If and only if the swap chain back
     * buffer was bound before resizing, the depth stencil view is bound
     * along with the back buffer after resizing is done.
     *
     * \note In order for the automatic resizing to work, you MUST NOT do
     *       anything that increases the reference count of this object.
     */
    ID3D11DepthStencilView* dsv;
}
sgui_d3d11_context;



#endif /* SGUI_D3D11_H */

