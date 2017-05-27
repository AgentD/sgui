/*
 * opengl.c
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

#include "platform.h"
#include "opengl.h"

#ifndef SGUI_NO_OPENGL
#include <GL/gl.h>

typedef struct {
	sgui_context super;

	sgui_window_w32 *wnd;
	HGLRC hRC;
} sgui_gl_context;

static int versions[][2] = { {4,5}, {4,4}, {4,3}, {4,2}, {4,1}, {4,0},
				{3,3}, {3,2}, {3,1}, {3,0} };

/* transform a pixel attribute array into a pixelformat descriptor */
static int get_descriptor_from_array(PIXELFORMATDESCRIPTOR *pfd,
					int *pixel_attribs)
{
	int i, need_new = 0;

#define ATTRIB(field) (field) = pixel_attribs[++i]
#define FLAG(flag)\
	pfd->dwFlags=pixel_attribs[++i] ? (pfd->dwFlags | (flag)) :\
						(pfd->dwFlags & ~(flag))

	ZeroMemory(pfd, sizeof(*pfd));
	pfd->nSize = sizeof(*pfd);
	pfd->nVersion = 1;

	for (i = 0; pixel_attribs[i]; ++i) {
		switch (pixel_attribs[i]) {
		case WGL_DRAW_TO_WINDOW_ARB:
			FLAG(PFD_DRAW_TO_WINDOW);
			break;
		case WGL_DRAW_TO_BITMAP_ARB:
			FLAG(PFD_DRAW_TO_BITMAP);
			break;
		case WGL_SUPPORT_OPENGL_ARB:
			FLAG(PFD_SUPPORT_OPENGL);
			break;
		case WGL_DOUBLE_BUFFER_ARB:
			FLAG(PFD_DOUBLEBUFFER);
			break;
		case WGL_SUPPORT_GDI_ARB:
			FLAG(PFD_SUPPORT_GDI);
			break;
		case WGL_STEREO_ARB:
			FLAG(PFD_STEREO);
			break;
		case WGL_COLOR_BITS_ARB:
			ATTRIB(pfd->cColorBits);
			break;
		case WGL_DEPTH_BITS_ARB:
			ATTRIB(pfd->cDepthBits);
			break;
		case WGL_STENCIL_BITS_ARB:
			ATTRIB(pfd->cStencilBits);
			break;
		case WGL_RED_BITS_ARB:
			ATTRIB(pfd->cRedBits);
			break;
		case WGL_RED_SHIFT_ARB:
			ATTRIB(pfd->cRedShift);
			break;
		case WGL_GREEN_BITS_ARB:
			ATTRIB(pfd->cGreenBits);
			break;
		case WGL_GREEN_SHIFT_ARB:
			ATTRIB(pfd->cGreenShift);
			break;
		case WGL_BLUE_BITS_ARB:
			ATTRIB(pfd->cBlueBits);
			break;
		case WGL_BLUE_SHIFT_ARB:
			ATTRIB(pfd->cBlueShift);
			break;
		case WGL_ALPHA_BITS_ARB:
			ATTRIB(pfd->cAlphaBits);
			break;
		case WGL_ALPHA_SHIFT_ARB:
			ATTRIB(pfd->cAlphaShift);
			break;
		case WGL_ACCUM_BITS_ARB:
			ATTRIB(pfd->cAccumBits);
			break;
		case WGL_ACCUM_RED_BITS_ARB:
			ATTRIB(pfd->cAccumRedBits);
			break;
		case WGL_ACCUM_GREEN_BITS_ARB:
			ATTRIB(pfd->cAccumGreenBits);
			break;
		case WGL_ACCUM_BLUE_BITS_ARB:
			ATTRIB(pfd->cAccumBlueBits);
			break;
		case WGL_ACCUM_ALPHA_BITS_ARB:
			ATTRIB(pfd->cAccumAlphaBits);
			break;
		case WGL_AUX_BUFFERS_ARB:
			ATTRIB(pfd->cAuxBuffers);
			break;
		case WGL_PIXEL_TYPE_ARB:
			if (pixel_attribs[++i] == WGL_TYPE_RGBA_ARB) {
				pfd->iPixelType = PFD_TYPE_RGBA;
			} else {
				pfd->iPixelType = PFD_TYPE_COLORINDEX;
			}
			break;
		default:
			++i;
			need_new = 1;
		}
	}
#undef FLAG
#undef ATTRIB
	return need_new;
}

/*
    On Windows, to create an OpenGL context, the SetPixelFormat function has
    to be called on a Window/HDC pair to set a framebuffer format. Framebuffer
    formats are identifed by unique IDs. We can get such an ID by using
    ChosePixelFormat which gets a pointer to a description struct.

    Later, when multisampling and other things were added, the function
    wglChoosePixelFormatARB was introduced to get a framebuffer format ID
    from an array of key-value pairs instead of a struct.

    If we want multisampling, we need wglChoosePixelFormatARB. However,
    this function is an extension. To get the extension, we need an OpenGL
    context. To get a context, we need to set a pixel format to a window
    first. However, we can only set the pixel format once per window, so we
    have to set up a dummy window, set a dummy pixel format the old way,
    create a dummy OpenGL context to get the extension and determine the
    unique ID for the actual format we want to use.

    This function abstracts all of this and determines the unique ID for
    a key-value array.
 */
static int determine_pixel_format(sgui_lib_w32 *lib, int *pixel_attribs,
				int only_new)
{
	WGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB;
	int pixelformat = 0, format, need_new = 0;
	PIXELFORMATDESCRIPTOR pfd;
	HGLRC temprc, oldctx;
	HDC tempdc, olddc;
	UINT numFormats;
	HWND tempwnd;

	need_new = get_descriptor_from_array(&pfd, pixel_attribs);

	tempwnd = CreateWindow(lib->wndclass, "", 0, 0, 0, 100, 100, 0, 0,
				lib->hInstance, 0);
	if (!tempwnd)
		goto out;

	tempdc = GetWindowDC(tempwnd);
	if (!tempdc)
		goto outwnd;

	pixelformat = ChoosePixelFormat(tempdc, &pfd);
	if (!pixelformat || !need_new)
		goto outdc;

	format = pixelformat;
	if (only_new)
		pixelformat = 0;

	if (!SetPixelFormat(tempdc, format, NULL))
		goto outdc;

	oldctx = wglGetCurrentContext();
	olddc = wglGetCurrentDC();

	temprc = wglCreateContext(tempdc);
	if (!temprc)
		goto outdc;

	if (!wglMakeCurrent(tempdc, temprc))
        	goto outrc;

	wglChoosePixelFormatARB = (WGLCHOOSEPIXELFORMATARBPROC)
			wglGetProcAddress("wglChoosePixelFormatARB");

	if (!wglChoosePixelFormatARB)
		goto outreset;

	wglChoosePixelFormatARB(tempdc, pixel_attribs, NULL, 1,
				&format, &numFormats);

	pixelformat = numFormats >= 1 ? format : pixelformat;

outreset:
	wglMakeCurrent(olddc, oldctx);
outrc:
	wglDeleteContext(temprc);
outdc:
	ReleaseDC(tempwnd, tempdc);
outwnd:
	DestroyWindow(tempwnd);
out:
	return pixelformat;
}

static void set_attributes(int *attr, int bpp, int depth, int stencil,
				int doublebuffer, int samples)
{
#define ATTRIB(name, value) *(attr++) = (name); *(attr++) = (value)
	ATTRIB(WGL_DRAW_TO_WINDOW_ARB, GL_TRUE);
	ATTRIB(WGL_SUPPORT_OPENGL_ARB, GL_TRUE);
	ATTRIB(WGL_COLOR_BITS_ARB, bpp);
	ATTRIB(WGL_RED_BITS_ARB, bpp == 16 ? 5 : 8);
	ATTRIB(WGL_GREEN_BITS_ARB, bpp == 16 ? 6 : 8);
	ATTRIB(WGL_BLUE_BITS_ARB, bpp == 16 ? 5 : 8);

	if (bpp == 32) {
		ATTRIB(WGL_ALPHA_BITS_ARB, 8);
	}
	if (depth) {
		ATTRIB(WGL_DEPTH_BITS_ARB, depth);
	}
	if (stencil) {
		ATTRIB(WGL_STENCIL_BITS_ARB, stencil);
	}
	if (doublebuffer) {
		ATTRIB(WGL_DOUBLE_BUFFER_ARB, GL_TRUE);
	}

	if (samples) {
		ATTRIB(WGL_SAMPLE_BUFFERS_ARB, GL_TRUE);
		ATTRIB(WGL_SAMPLES_ARB, samples);
	}

	ATTRIB(0, 0);
#undef ATTRIB
}

int set_pixel_format(sgui_window_w32 *this, sgui_lib *lib,
			const sgui_window_description *desc)
{
	int attribs[20], format = 0, samples = desc->samples;

	this->hDC = GetDC(this->hWnd);
	if (!this->hDC)
		return 0;

	while (samples && !format) {
		set_attributes(attribs, desc->bits_per_pixel, desc->depth_bits,
				desc->stencil_bits,
				desc->flags & SGUI_DOUBLEBUFFERED, samples--);

		format = determine_pixel_format((sgui_lib_w32 *)lib,
						attribs, 1);
	}

	if (!format && !(format = determine_pixel_format((sgui_lib_w32 *)lib,
							attribs, 0))) {
		goto fail;
	}

	SetPixelFormat(this->hDC, format, NULL);
	return 1;
fail:
	ReleaseDC(this->hWnd, this->hDC);
	return 0;
}
/****************************************************************************/
static void gl_swap_buffers(sgui_window *this)
{
	sgui_internal_lock_mutex();
	glFlush();
	SwapBuffers(((sgui_window_w32 *)this)->hDC);
	sgui_internal_unlock_mutex();
}

static void gl_set_vsync(sgui_window *this, int interval)
{
	WGLSWAPINTERVALEXT wglSwapIntervalEXT;
	(void)this;

	sgui_internal_lock_mutex();

	wglSwapIntervalEXT = (WGLSWAPINTERVALEXT)
				wglGetProcAddress("wglSwapIntervalEXT");

	if (wglSwapIntervalEXT)
		wglSwapIntervalEXT(interval);

	sgui_internal_unlock_mutex();
}
/****************************************************************************/
static sgui_context *context_gl_create_share(sgui_context *super)
{
	sgui_gl_context *this = (sgui_gl_context *)super;

	return gl_context_create(this->wnd, this->wnd->super.backend,
				this->wnd->super.ctx);
}

static void context_gl_destroy(sgui_context *this)
{
	sgui_internal_lock_mutex();
	if (wglGetCurrentContext() == ((sgui_gl_context *)this)->hRC)
		wglMakeCurrent(NULL, NULL);

	wglDeleteContext(((sgui_gl_context *)this)->hRC);
	sgui_internal_unlock_mutex();
}

static void context_gl_make_current(sgui_context *this, sgui_window *wnd)
{
	sgui_internal_lock_mutex();
	wglMakeCurrent(TO_W32(wnd)->hDC, ((sgui_gl_context *)this)->hRC);
	sgui_internal_unlock_mutex();
}

static void context_gl_release_current(sgui_context *this)
{
	(void)this;
	wglMakeCurrent(NULL, NULL);
}

static sgui_funptr context_gl_load(sgui_context *this, const char *name)
{
	(void)this;
	return (sgui_funptr)wglGetProcAddress(name);
}

static void *context_gl_get_internal(sgui_context *this)
{
	return &(((sgui_gl_context*)this)->hRC);
}

sgui_context *gl_context_create(sgui_window_w32 *wnd, int backend,
				sgui_context *share)
{
	HGLRC temp, oldctx, src = share ? ((sgui_gl_context *)share)->hRC : 0;
	WGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = NULL;
	sgui_gl_context *this = calloc(1, sizeof(*this));
	sgui_context *super = (sgui_context *)this;
	int attribs[20];
	unsigned int i;
	HDC olddc;

	if (!this)
		return NULL;

	sgui_internal_lock_mutex();
	this->hRC = 0;

	temp = wglCreateContext(wnd->hDC);
	if (!temp)
		goto fail;

	oldctx = wglGetCurrentContext();
	olddc = wglGetCurrentDC();

	if (!wglMakeCurrent(wnd->hDC, temp)) {
		wglDeleteContext(temp);
		goto fail;
	}

	wglCreateContextAttribsARB = (WGLCREATECONTEXTATTRIBSARBPROC)
			wglGetProcAddress("wglCreateContextAttribsARB");

	if (backend == SGUI_OPENGL_CORE && wglCreateContextAttribsARB) {
		attribs[0] = WGL_CONTEXT_MAJOR_VERSION_ARB;
		attribs[1] = 0;
		attribs[2] = WGL_CONTEXT_MINOR_VERSION_ARB;
		attribs[3] = 0;
		attribs[4] = WGL_CONTEXT_PROFILE_MASK_ARB;
		attribs[5] = WGL_CONTEXT_CORE_PROFILE_BIT_ARB;
		attribs[6] = WGL_CONTEXT_FLAGS_ARB;
		attribs[7] = WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB;
		attribs[8] = 0;

		for (i = 0; i < sizeof(versions) / sizeof(versions[0]); ++i) {
			attribs[1] = versions[i][0];
			attribs[3] = versions[i][1];
			this->hRC = wglCreateContextAttribsARB(wnd->hDC, src,
								attribs);
			if (this->hRC)
				break;
		}
	}

	if (this->hRC) {
		wglMakeCurrent(olddc, oldctx);
		wglDeleteContext(temp);
	} else {
		this->hRC = temp;

		if (src)
			wglShareLists(src, this->hRC);

		wglMakeCurrent(olddc, oldctx);
	}

	this->wnd = wnd;

	((sgui_window *)wnd)->swap_buffers = gl_swap_buffers;
	((sgui_window *)wnd)->set_vsync = gl_set_vsync;

	super->create_share = context_gl_create_share;
	super->destroy = context_gl_destroy;
	super->make_current = context_gl_make_current;
	super->release_current = context_gl_release_current;
	super->get_internal = context_gl_get_internal;
	super->load = context_gl_load;

	sgui_internal_unlock_mutex();
	return super;
fail:
	free(this);
	sgui_internal_unlock_mutex();
	return NULL;
}
#else
int set_pixel_format(sgui_window_w32 *wnd, const sgui_window_description *desc)
{
	(void)wnd; (void)desc;
	return 0;
}

sgui_context *gl_context_create(sgui_window_w32 *wnd, int backend,
				sgui_context *share)
{
	(void)wnd; (void)backend; (void)share;
	return NULL;
}
#endif
