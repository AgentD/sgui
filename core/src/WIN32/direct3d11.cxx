/*
 * direct3d11.cxx
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
#define SGUI_BUILDING_DLL
#include "direct3d11.h"
#include "window.h"

#include <stdio.h>

#ifndef SGUI_NO_D3D11
typedef HRESULT (__stdcall *CREATEDEVSWAPCHAIN)(IDXGIAdapter *,
						D3D_DRIVER_TYPE,
						HMODULE, UINT,
						const D3D_FEATURE_LEVEL *,
						UINT, UINT,
						const DXGI_SWAP_CHAIN_DESC *,
						IDXGISwapChain **,
						ID3D11Device **,
						D3D_FEATURE_LEVEL *,
						ID3D11DeviceContext **);

static HINSTANCE libd3d11 = 0;
static size_t refcount = 0;
static CREATEDEVSWAPCHAIN CreateDeviceAndSwapChain = NULL;

static const D3D_FEATURE_LEVEL levels[] = {
	D3D_FEATURE_LEVEL_11_0,
	D3D_FEATURE_LEVEL_10_1,
	D3D_FEATURE_LEVEL_10_0,
	D3D_FEATURE_LEVEL_9_3,
	D3D_FEATURE_LEVEL_9_2,
	D3D_FEATURE_LEVEL_9_1
};

static const D3D_DRIVER_TYPE drivers[] = {
	D3D_DRIVER_TYPE_HARDWARE,
	D3D_DRIVER_TYPE_WARP,
	D3D_DRIVER_TYPE_REFERENCE
};

#define NUMDRIVERS (sizeof(drivers) / sizeof(drivers[0]))
#define NUMLEVELS (sizeof(levels) / sizeof(levels[0]))

static int load_d3d11(void)
{
	sgui_internal_lock_mutex();

	if (!libd3d11) {
		libd3d11 = LoadLibrary("d3d11.dll");

		if (!libd3d11)
			goto fail;

		CreateDeviceAndSwapChain = (CREATEDEVSWAPCHAIN)
		GetProcAddress(libd3d11, "D3D11CreateDeviceAndSwapChain");

		if (!CreateDeviceAndSwapChain) {
			FreeLibrary(libd3d11);
			libd3d11 = 0;
			goto fail;
		}
	}

	++refcount;
	sgui_internal_unlock_mutex();
	return 1;
fail:
	sgui_internal_unlock_mutex();
	return 0;
}

static void release_d3d11(void)
{
	sgui_internal_lock_mutex();

	if (libd3d11 && (--refcount) == 0) {
		FreeLibrary(libd3d11);
		libd3d11 = 0;
		CreateDeviceAndSwapChain = NULL;
	}

	sgui_internal_unlock_mutex();
}

/****************************************************************************/

static void context_d3d11_destroy(sgui_context *super)
{
	sgui_d3d11_context *ctx = (sgui_d3d11_context *)super;

	if (ctx->dsv)
		ctx->dsv->Release();

	if (ctx->ds_texture)
		ctx->ds_texture->Release();

	if (ctx->backbuffer)
		ctx->backbuffer->Release();

	if (ctx->ctx)
		ctx->ctx->Release();

	if (ctx->dev)
		ctx->dev->Release();

	if (ctx->swapchain)
		ctx->swapchain->Release();

	free(ctx);
	release_d3d11();
}

static void *context_d3d11_get_internal(sgui_context *ctx)
{
	return ((sgui_d3d11_context *)ctx)->dev;
}

static void d3d11_swap_buffers(sgui_window *wnd)
{
	sgui_d3d11_context *ctx = (sgui_d3d11_context *)wnd->ctx.ctx;

	ctx->swapchain->Present(ctx->syncrate, 0);
}

static void d3d11_set_vsync(sgui_window *wnd, int interval)
{
	sgui_d3d11_context *ctx = (sgui_d3d11_context *)wnd->ctx.ctx;

	ctx->syncrate = interval ? 1 : 0;
}

void d3d11_resize(sgui_context *super)
{
	sgui_d3d11_context *ctx = (sgui_d3d11_context *)super;
	D3D11_DEPTH_STENCIL_VIEW_DESC ds_view_desc;
	D3D11_TEXTURE2D_DESC ds_tex_desc;
	ID3D11Texture2D *pBackBuffer;
	ID3D11RenderTargetView *tv;
	DXGI_SWAP_CHAIN_DESC scd;
	D3D11_VIEWPORT viewport;
	bool is_bound, has_ds;

	ctx->ctx->OMGetRenderTargets(1, &tv, NULL);

	is_bound = (tv == ctx->backbuffer);
	has_ds = (ctx->dsv != NULL);

	if (tv)
		tv->Release();

	if (is_bound)
		ctx->ctx->OMSetRenderTargets(0, NULL, NULL);

	ctx->swapchain->GetDesc(&scd);
	ctx->backbuffer->Release();

	if (has_ds) {
		ctx->ds_texture->GetDesc(&ds_tex_desc);

		ctx->ds_texture->Release();
		ctx->dsv->Release();
	}

	ctx->swapchain->ResizeBuffers(scd.BufferCount, ctx->wnd->w,
					ctx->wnd->h, scd.BufferDesc.Format,
					scd.Flags);

	ctx->swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D),
					(void **)&pBackBuffer);

	ctx->dev->CreateRenderTargetView((ID3D11Resource *)pBackBuffer,
					NULL, &ctx->backbuffer);

	pBackBuffer->Release();

	if (has_ds) {
		ds_tex_desc.Width = ctx->wnd->w;
		ds_tex_desc.Height = ctx->wnd->h;

		memset(&ds_view_desc, 0, sizeof(ds_view_desc));
		ds_view_desc.Format = ds_tex_desc.Format;
		ds_view_desc.ViewDimension = scd.SampleDesc.Count > 0 ?
					D3D11_DSV_DIMENSION_TEXTURE2DMS :
					D3D11_DSV_DIMENSION_TEXTURE2D;

		ctx->dev->CreateTexture2D(&ds_tex_desc, NULL,
						&ctx->ds_texture);

		ctx->dev->CreateDepthStencilView(ctx->ds_texture,
						&ds_view_desc, &ctx->dsv);
	}

	if (is_bound) {
		ctx->ctx->OMSetRenderTargets(1, &ctx->backbuffer, ctx->dsv);

		memset(&viewport, 0, sizeof(viewport));
		viewport.Width = ctx->wnd->w;
		viewport.Height = ctx->wnd->h;
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;

		ctx->ctx->RSSetViewports(1, &viewport);
	}
}

sgui_context *d3d11_context_create(sgui_window *wnd,
					const sgui_window_description *desc)
{
	D3D11_DEPTH_STENCIL_VIEW_DESC ds_view_desc;
	D3D11_TEXTURE2D_DESC ds_tex_desc;
	ID3D11Texture2D *pBackBuffer;
	DXGI_SWAP_CHAIN_DESC scd;
	sgui_d3d11_context *ctx;
	D3D11_VIEWPORT viewport;
	sgui_context *super;
	HRESULT hr;
	size_t i;

	if (!load_d3d11())
		return NULL;

	ctx = (sgui_d3d11_context *)calloc(1, sizeof(*ctx));
	super = (sgui_context*)ctx;

	if (!ctx) {
		release_d3d11();
		return NULL;
	}

	memset(&scd, 0, sizeof(scd));

	scd.BufferDesc.RefreshRate.Numerator = 60;
	scd.BufferDesc.RefreshRate.Denominator = 1;
	scd.BufferDesc.Format = (desc->bits_per_pixel == 16) ?
				DXGI_FORMAT_B5G6R5_UNORM :
				DXGI_FORMAT_R8G8B8A8_UNORM;
	scd.SampleDesc.Count = desc->samples>0 ? desc->samples : 1;
	scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	scd.BufferCount = (desc->flags & SGUI_DOUBLEBUFFERED) ? 2 : 1;
	scd.OutputWindow = TO_W32(wnd)->hWnd;
	scd.Windowed = TRUE;
	scd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	scd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	for (i = 0; i < NUMDRIVERS; ++i) {
		hr = CreateDeviceAndSwapChain(NULL, drivers[i], NULL, 0,
						levels, NUMLEVELS,
						D3D11_SDK_VERSION, &scd,
						&ctx->swapchain, &ctx->dev,
						NULL, &ctx->ctx);

		if (hr >= 0)
			break;
	}

	if (hr < 0)
		goto fail;

	hr = ctx->swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D),
					(void **)&pBackBuffer);

	if (hr < 0)
		goto fail;

	hr = ctx->dev->CreateRenderTargetView(pBackBuffer, NULL,
						&ctx->backbuffer);

	pBackBuffer->Release();

	if (hr < 0)
		goto fail;

	if (desc->depth_bits > 0 || desc->stencil_bits > 0) {
		memset(&ds_tex_desc, 0, sizeof(ds_tex_desc));
		memset(&ds_view_desc, 0, sizeof(ds_view_desc));

		ds_tex_desc.Width = desc->width;
		ds_tex_desc.Height = desc->height;
		ds_tex_desc.MipLevels = 1;
		ds_tex_desc.ArraySize = 1;
		ds_tex_desc.SampleDesc.Count = scd.SampleDesc.Count;
		ds_tex_desc.Usage = D3D11_USAGE_DEFAULT;
		ds_tex_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

		if (desc->stencil_bits) {
			ds_tex_desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		} else if (desc->depth_bits <= 16) {
			ds_tex_desc.Format = DXGI_FORMAT_D16_UNORM;
		} else {
			ds_tex_desc.Format = DXGI_FORMAT_D32_FLOAT;
		}

		hr = ctx->dev->CreateTexture2D(&ds_tex_desc, NULL,
						&ctx->ds_texture);

		if (hr < 0)
			goto fail;

		ds_view_desc.Format = ds_tex_desc.Format;
		ds_view_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		if (desc->samples) {
			ds_view_desc.ViewDimension = 
					D3D11_DSV_DIMENSION_TEXTURE2DMS;
		}

		hr = ctx->dev->CreateDepthStencilView(ctx->ds_texture,
						&ds_view_desc, &ctx->dsv);

		if (hr < 0)
			goto fail;
	}

	ctx->ctx->OMSetRenderTargets(1, &ctx->backbuffer, ctx->dsv);

	memset(&viewport, 0, sizeof(viewport));

	viewport.Width = desc->width;
	viewport.Height = desc->height;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	ctx->ctx->RSSetViewports(1, &viewport);

	wnd->swap_buffers = d3d11_swap_buffers;
	wnd->set_vsync = d3d11_set_vsync;

	ctx->wnd = wnd;
	super->destroy = context_d3d11_destroy;
	super->get_internal = context_d3d11_get_internal;
	return super;
fail:
	context_d3d11_destroy(super);
	return NULL;
}
#else
void d3d11_resize(sgui_context *ctx)
{
	(void)ctx;
}

sgui_context *d3d11_context_create(sgui_window *wnd,
					const sgui_window_description *desc)
{
	(void)wnd; (void)desc;
	return NULL;
}
#endif /* SGUI_NO_D3D11 */
