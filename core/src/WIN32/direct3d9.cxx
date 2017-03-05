/*
 * direct3d9.cxx
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
#include "sgui_event.h"
#include "direct3d9.h"
#include "window.h"

#ifndef SGUI_NO_D3D9
static const int device_flags[] = {
	D3DCREATE_MULTITHREADED | D3DCREATE_HARDWARE_VERTEXPROCESSING,
	D3DCREATE_MULTITHREADED | D3DCREATE_MIXED_VERTEXPROCESSING,
	D3DCREATE_MULTITHREADED | D3DCREATE_SOFTWARE_VERTEXPROCESSING,
	D3DCREATE_HARDWARE_VERTEXPROCESSING,
	D3DCREATE_MIXED_VERTEXPROCESSING,
	D3DCREATE_SOFTWARE_VERTEXPROCESSING,
};

static const D3DFORMAT ds_formats[] = {
	D3DFMT_D24S8,
	D3DFMT_D24X4S4,
	D3DFMT_D15S1,
};

static const D3DFORMAT d_formats[] = {
	D3DFMT_D32,
	D3DFMT_D24X8,
	D3DFMT_D16,
};

static HINSTANCE libd3d9 = 0;
static IDirect3D9 *d3d9 = NULL;

typedef IDirect3D9 *( __stdcall *D3DCREATE9)(UINT);

static int load_d3d9(void)
{
	D3DCREATE9 D3DCreate9 = NULL;

	sgui_internal_lock_mutex();

	if (d3d9) {
		d3d9->AddRef();
		goto out;
	}

	libd3d9 = LoadLibrary("d3d9.dll");
	if (!libd3d9)
		goto fail;

	D3DCreate9 = (D3DCREATE9)GetProcAddress(libd3d9, "Direct3DCreate9");
	if (!D3DCreate9)
		goto fail;

	d3d9 = D3DCreate9(D3D_SDK_VERSION);
	if (!d3d9)
		goto fail;
out:
	sgui_internal_unlock_mutex();
	return 1;
fail:
	if (libd3d9)
		FreeLibrary(libd3d9);
	sgui_internal_unlock_mutex();
	return 0;
}

static void release_d3d9(void)
{
	sgui_internal_lock_mutex();

	if (d3d9 && (d3d9->Release() == 0)) {
		FreeLibrary(libd3d9);
		libd3d9 = 0;
		d3d9 = NULL;
	}

	sgui_internal_unlock_mutex();
}

static int check_depthstencil_format(D3DPRESENT_PARAMETERS *pr,
					int adapter, D3DDEVTYPE devtype)
{
	if (d3d9->CheckDeviceFormat(adapter, devtype, pr->BackBufferFormat,
				D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE,
				pr->AutoDepthStencilFormat) < 0) {
		return 0;
	}

	if (d3d9->CheckDepthStencilMatch(adapter, (D3DDEVTYPE)devtype,
				pr->BackBufferFormat, pr->BackBufferFormat,
				pr->AutoDepthStencilFormat) < 0) {
		return 0;
	}
	return 1;
}

static int configure_depth_stencil(sgui_d3d9_context *ctx, int adapter,
					D3DDEVTYPE devtype,
					const sgui_window_description *desc)
{
	size_t i;

	/* no depth-stencil buffer */
	if ((desc->depth_bits <= 0) && (desc->stencil_bits <= 0)) {
		ctx->present.EnableAutoDepthStencil = FALSE;
		return 1;
	}

	ctx->present.EnableAutoDepthStencil = TRUE;

	if (desc->stencil_bits <= 0)
		goto depth_only;

	for (i = 0; i < sizeof(ds_formats) / sizeof(ds_formats[0]); ++i) {
		ctx->present.AutoDepthStencilFormat = ds_formats[i];

		if (check_depthstencil_format(&ctx->present, adapter,
						devtype)) {
			return 1;
		}
	}

depth_only:
	for (i = 0; i < sizeof(d_formats) / sizeof(d_formats[0]); ++i) {
		ctx->present.AutoDepthStencilFormat = d_formats[i];

		if (check_depthstencil_format(&ctx->present, adapter,
						devtype)) {
			return 1;
		}
	}

	return 0;
}

static void configure_msaa(sgui_d3d9_context *ctx, int adapter,
				D3DDEVTYPE devtype,
				const sgui_window_description *desc)
{
	D3DMULTISAMPLE_TYPE samp = D3DMULTISAMPLE_16_SAMPLES;
	DWORD qualitylevels = 0;
	HRESULT hr;

	if (desc->samples <= 0)
		return;

	if (desc->samples < 16)
		samp = (D3DMULTISAMPLE_TYPE)desc->samples;

	do {
		hr = d3d9->CheckDeviceMultiSampleType(adapter, devtype,
					ctx->present.BackBufferFormat, TRUE,
					samp, &qualitylevels);

		if (hr >= 0) {
			ctx->present.MultiSampleType = samp;
			ctx->present.MultiSampleQuality = qualitylevels - 1;
			break;
		}
	} while ((samp = (D3DMULTISAMPLE_TYPE)((int)samp - 1)));
}

static IDirect3DDevice9 *create_device(int adapter, D3DDEVTYPE devtype,
				HWND hWnd, D3DPRESENT_PARAMETERS *params)
{
	IDirect3DDevice9 *dev = NULL;
	HRESULT hr;
	size_t i;

	for (i = 0; i < sizeof(device_flags) / sizeof(device_flags[0]); ++i) {
		hr = d3d9->CreateDevice(adapter, devtype, hWnd,
					device_flags[i], params, &dev);

		if (hr >= 0)
			break;

		dev = NULL;
	}

	return dev;
}

/****************************************************************************/

static void context_d3d9_destroy(sgui_context *ctx)
{
	((sgui_d3d9_context *)ctx)->device->Release();
	free(ctx);
	release_d3d9();
}

static void *context_d3d9_get_internal(sgui_context *ctx)
{
	return ((sgui_d3d9_context *)ctx)->device;
}

static void d3d9_swap_buffers(sgui_window *wnd)
{
	IDirect3DDevice9 *dev;

	sgui_internal_lock_mutex();
	dev = ((sgui_d3d9_context *)wnd->ctx.ctx)->device;

	dev->Present(NULL, NULL, NULL, NULL);
	sgui_internal_unlock_mutex();
}

static void d3d9_set_vsync(sgui_window *wnd, int interval)
{
	sgui_d3d9_context *ctx = (sgui_d3d9_context *)wnd->ctx.ctx;

	sgui_internal_lock_mutex();
	if (interval) {
		ctx->present.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
	} else {
		ctx->present.PresentationInterval =
						D3DPRESENT_INTERVAL_IMMEDIATE;
	}
	sgui_internal_unlock_mutex();
}

sgui_context *d3d9_context_create(sgui_window *wnd,
					const sgui_window_description *desc)
{
	sgui_d3d9_context *ctx = (sgui_d3d9_context *)calloc(1, sizeof(*ctx));
	sgui_context *super = (sgui_context *)ctx;
	D3DDEVTYPE devtype = D3DDEVTYPE_HAL;
	D3DDISPLAYMODE d3ddm;
	int adapter = 0;

	if (!ctx)
		return NULL;

	if (!load_d3d9()) {
		free(ctx);
		return NULL;
	}

	sgui_internal_lock_mutex();

	if (d3d9->GetAdapterDisplayMode(adapter, &d3ddm) < 0)
		goto fail;

	if (desc->flags & SGUI_DOUBLEBUFFERED)
		ctx->present.BackBufferCount = 1;

	ctx->present.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
	ctx->present.BackBufferFormat = d3ddm.Format;
	ctx->present.SwapEffect = D3DSWAPEFFECT_DISCARD;
	ctx->present.Windowed = TRUE;

	configure_msaa(ctx, adapter, devtype, desc);

	if (!configure_depth_stencil(ctx, adapter, devtype, desc))
		goto fail;

	ctx->device = create_device(adapter, devtype, TO_W32(wnd)->hWnd,
					&ctx->present);
	if (!ctx->device)
		goto fail;

	ctx->wnd = wnd;
	super->destroy = context_d3d9_destroy;
	super->get_internal = context_d3d9_get_internal;

	wnd->swap_buffers = d3d9_swap_buffers;
	wnd->set_vsync = d3d9_set_vsync;

	sgui_internal_unlock_mutex();
	return super;
fail:
	free(ctx);
	release_d3d9();
	sgui_internal_unlock_mutex();
	return NULL;
}

void send_event_if_d3d9_lost(sgui_window *wnd)
{
	IDirect3DDevice9 *dev = ((sgui_d3d9_context *)wnd->ctx.ctx)->device;
	sgui_event e;

	if (dev->TestCooperativeLevel() == D3DERR_DEVICELOST) {
		e.type = SGUI_D3D9_DEVICE_LOST;
		e.src.window = wnd;
		sgui_internal_window_fire_event(wnd, &e);
	}
}
#else
sgui_context *d3d9_context_create(sgui_window *wnd,
				const sgui_window_description *desc)
{
	(void)wnd; (void)desc;
	return NULL;
}

void send_event_if_d3d9_lost(sgui_window *wnd)
{
	(void)wnd;
}
#endif /* SGUI_NO_D3D9 */
