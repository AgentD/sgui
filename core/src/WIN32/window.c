/*
 * window.c
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
#include "sgui_event.h"

static void resize_pixmap(sgui_window_w32 *this)
{
	sgui_window *super = (sgui_window *)this;

	this->info.bmiHeader.biWidth  = super->w;
	this->info.bmiHeader.biHeight = -((int)super->h);

	SelectObject(this->hDC, 0);
	DeleteObject(this->bitmap);

	this->bitmap = CreateDIBSection(this->hDC, &this->info, DIB_RGB_COLORS,
					&this->data, 0, 0);

	SelectObject(this->hDC, this->bitmap);

	sgui_memory_canvas_set_buffer(super->canvas, TO_W32(this)->data);
}

static void create_canvas(sgui_window_w32 *this, sgui_lib *lib,
			const sgui_window_description *desc)
{
	sgui_window *super = (sgui_window *)this;
	sgui_color color;

	this->info.bmiHeader.biSize = sizeof(this->info.bmiHeader);
	this->info.bmiHeader.biBitCount = 32;
	this->info.bmiHeader.biCompression = BI_RGB;
	this->info.bmiHeader.biPlanes = 1;
	this->info.bmiHeader.biWidth = desc->width;
	this->info.bmiHeader.biHeight = -((int)desc->height);

	color = sgui_skin_get()->window_color;

	if (!(this->hDC = CreateCompatibleDC(NULL)))
		return;

	this->bitmap = CreateDIBSection(this->hDC, &this->info, DIB_RGB_COLORS,
					&this->data, 0, 0);

	if (!this->bitmap)
		goto faildc;

	this->bgbrush = CreateSolidBrush(RGB(color.c.r, color.c.g, color.c.b));

	if (!this->bgbrush)
		goto faildib;

	super->canvas = sgui_memory_canvas_create(lib, this->data,
						desc->width, desc->height,
						SGUI_RGBA8, 1);
	if (!super->canvas)
		goto failbrush;

	SelectObject(this->hDC, this->bitmap);
	return;
failbrush:
	DeleteObject(this->bgbrush);
faildib:
	DeleteObject(this->bitmap);
faildc:
	DeleteDC(this->hDC);
}

/****************************************************************************/

static void w32_window_get_mouse_position(sgui_window *this, int *x, int *y)
{
	POINT pos = { 0, 0 };

	sgui_internal_lock_mutex();
	GetCursorPos(&pos);
	ScreenToClient(TO_W32(this)->hWnd, &pos);
	sgui_internal_unlock_mutex();

	*x = pos.x;
	*y = pos.y;
}

static void w32_window_set_mouse_position(sgui_window *this, int x, int y)
{
	POINT pos;

	pos.x = x;
	pos.y = y;

	sgui_internal_lock_mutex();
	ClientToScreen(TO_W32(this)->hWnd, &pos);
	SetCursorPos(pos.x, pos.y);
	sgui_internal_unlock_mutex();
}

static int w32_window_toggle_flags(sgui_window *super, int diff)
{
	sgui_window_w32 *this = (sgui_window_w32 *)super;
	int ret = 1;
	LONG lv;

	sgui_internal_lock_mutex();
	if (diff & SGUI_DOUBLEBUFFERED) {
		ret = 0;
		goto out;
	}

	if (diff & SGUI_VISIBLE) {
		ShowWindow(this->hWnd, (super->flags & SGUI_VISIBLE) ?
					SW_HIDE : SW_SHOWNORMAL);

		super->flags ^= SGUI_VISIBLE;
	}

	if (diff & SGUI_FIXED_SIZE) {
		lv = GetWindowLongA(this->hWnd, GWL_STYLE);
		if (!lv) {
			ret = 0;
			goto out;
		}

		if (lv & WS_CHILD)
			goto out;

		lv &= ~(WS_CAPTION | WS_SYSMENU | WS_OVERLAPPEDWINDOW);

		if (super->flags & SGUI_FIXED_SIZE) {
			lv |= WS_OVERLAPPEDWINDOW;
		} else {
			lv |= (WS_CAPTION | WS_SYSMENU);
		}

		if (!SetWindowLongA(this->hWnd, GWL_STYLE, lv)) {
			ret = 0;
			goto out;
		}

		super->flags ^= SGUI_FIXED_SIZE;
	}

out:
	sgui_internal_unlock_mutex();
	return ret;
}

static void w32_window_set_title(sgui_window *this, const char *title)
{
	WCHAR *utf16 = NULL;
	int isascii = 1;
	unsigned int i;

	for (i = 0; isascii && title[i]; ++i)
		isascii &= (title[i] & 0x80) >> 7;

	if (!isascii && !(utf16 = utf8_to_utf16(title, -1)))
		return;

	sgui_internal_lock_mutex();

	if (isascii) {
		SetWindowTextA(TO_W32(this)->hWnd, title);
	} else {
		SetWindowTextW(TO_W32(this)->hWnd, utf16);
        }

	sgui_internal_unlock_mutex();

	free(utf16);
}

static void w32_window_set_size(sgui_window *this,
				unsigned int width, unsigned int height)
{
	RECT rcClient, rcWindow;
	POINT ptDiff;

	sgui_internal_lock_mutex();

	GetClientRect(TO_W32(this)->hWnd, &rcClient);
	GetWindowRect(TO_W32(this)->hWnd, &rcWindow);

	ptDiff.x = (rcWindow.right - rcWindow.left) - rcClient.right;
	ptDiff.y = (rcWindow.bottom - rcWindow.top ) - rcClient.bottom;

	MoveWindow(TO_W32(this)->hWnd, rcWindow.left, rcWindow.top,
			(int)width + ptDiff.x, (int)height + ptDiff.y, TRUE);

	this->w = width;
	this->h = height;

	if (this->canvas) {
		resize_pixmap(TO_W32(this));
		sgui_canvas_resize(this->canvas, width, height);
	}

	if (this->backend == SGUI_DIRECT3D_11)
		d3d11_resize(this->ctx);

	sgui_internal_unlock_mutex();
}

static void w32_window_move_center(sgui_window *this)
{
	RECT desktop, window;
	int w, h, dw, dh;

	sgui_internal_lock_mutex();

	GetClientRect(GetDesktopWindow(), &desktop);
	GetWindowRect(TO_W32(this)->hWnd, &window);

	w = window.right - window.left;
	h = window.bottom - window.top;

	dw = desktop.right - desktop.left;
	dh = desktop.bottom - desktop.top;

	MoveWindow(TO_W32(this)->hWnd, (dw - w)/2, (dh - h)/2, w, h, TRUE);

	sgui_internal_unlock_mutex();
}

static void w32_window_move(sgui_window *this, int x, int y)
{
	RECT r;

	sgui_internal_lock_mutex();

	GetWindowRect(TO_W32(this)->hWnd, &r);

	MoveWindow(TO_W32(this)->hWnd, x, y, r.right - r.left,
		r.bottom - r.top, TRUE);

	sgui_internal_unlock_mutex();
}

static void w32_window_force_redraw(sgui_window *this, sgui_rect *r)
{
	RECT r0;

	sgui_internal_lock_mutex();
	SetRect(&r0, r->left, r->top, r->right + 1, r->bottom + 1);
	InvalidateRect(TO_W32(this)->hWnd, &r0, TRUE);
	sgui_internal_unlock_mutex();
}

static void w32_window_get_platform_data(const sgui_window *this, void *window)
{
	*((HWND*)window) = TO_W32(this)->hWnd;
}

static void w32_window_make_topmost(sgui_window *this)
{
	sgui_internal_lock_mutex();
	if (this->flags & SGUI_VISIBLE) {
		SetWindowPos(TO_W32(this)->hWnd, HWND_TOP, 0, 0, 0, 0,
				SWP_NOSIZE | SWP_NOMOVE);
	}
	sgui_internal_unlock_mutex();
}

static void w32_window_destroy(sgui_window *this)
{
	sgui_internal_lock_mutex();
	sgui_internal_remove_window(this->lib, this);
	SET_USER_PTR(TO_W32(this)->hWnd, NULL);

	if (this->canvas)
		sgui_canvas_destroy(this->canvas);

	if (this->ctx)
		this->ctx->destroy(this->ctx);

	switch (this->backend) {
	case SGUI_NATIVE:
		SelectObject(TO_W32(this)->hDC, 0);
		DeleteObject(TO_W32(this)->bitmap);
		DeleteObject(TO_W32(this)->bgbrush);
		DeleteDC(TO_W32(this)->hDC);
		break;
	case SGUI_OPENGL_CORE:
	case SGUI_OPENGL_COMPAT:
		ReleaseDC(TO_W32(this)->hWnd, TO_W32(this)->hDC);
		break;
	}

	DestroyWindow(TO_W32(this)->hWnd);
	sgui_internal_unlock_mutex();

	free(this);
}

/****************************************************************************/

void update_window(sgui_window_w32 *this)
{
	sgui_window *super = (sgui_window *)this;
	unsigned int i, num;
	sgui_rect sr;
	RECT r;

	if (super->canvas) {
		num = sgui_canvas_num_dirty_rects(super->canvas);

		for (i = 0; i < num; ++i) {
			sgui_canvas_get_dirty_rect(super->canvas, &sr, i);

			SetRect(&r, sr.left, sr.top,
				sr.right + 1, sr.bottom + 1);
			InvalidateRect(this->hWnd, &r, TRUE);
		}

		sgui_canvas_redraw_widgets(super->canvas, 1);
	}

	if (super->backend == SGUI_DIRECT3D_9)
		send_event_if_d3d9_lost(super);
}

static int handle_key_event(sgui_window_w32 *this, UINT msg,
				WPARAM wp, LPARAM lp)
{
	UINT key = (UINT)wp;
	sgui_event e;

	if (key == VK_SHIFT || key == VK_CONTROL || key == VK_MENU)
		key = MapVirtualKey((lp >> 16) & 0xFF, MAPVK_VSC_TO_VK_EX);

	if ((lp & 0x1000000) && (key == VK_CONTROL))
		key = VK_RCONTROL;

	if ((lp & 0x1000000) && (key == VK_MENU))
		key = VK_RMENU;

	if (msg == WM_KEYDOWN || msg == WM_SYSKEYDOWN ) {
		e.type = SGUI_KEY_PRESSED_EVENT;
	} else {
		e.type = SGUI_KEY_RELEASED_EVENT;
	}

	e.src.window = (sgui_window *)this;
	e.arg.i = key;
	sgui_internal_window_fire_event((sgui_window *)this, &e);

	/* let DefWindowProc handle system keys, except ALT */
	if (msg == WM_SYSKEYUP || msg == WM_SYSKEYDOWN) {
		if (key == VK_MENU || key == VK_LMENU || key == VK_RMENU)
			return 0;
		return DefWindowProcA(this->hWnd, msg, wp, lp);
	}
	return 0;
}

static int handle_repaint(sgui_window_w32 *this, UINT msg, WPARAM wp, LPARAM lp)
{
	sgui_window *super = (sgui_window *)this;
	BLENDFUNCTION ftn;
	PAINTSTRUCT ps;
	sgui_event e;
	HDC hDC;
	RECT r;

	if (super->backend == SGUI_NATIVE) {
		ftn.BlendOp = AC_SRC_OVER;
		ftn.BlendFlags = 0;
		ftn.SourceConstantAlpha = 0xFF;
		ftn.AlphaFormat = AC_SRC_ALPHA;

		SetRect(&r, 0, 0, super->w, super->h);

		hDC = BeginPaint(this->hWnd, &ps);
		FillRect(hDC, &r, this->bgbrush);
		AlphaBlend(hDC, 0, 0, super->w, super->h, this->hDC,
				0, 0, super->w, super->h, ftn);
		EndPaint(this->hWnd, &ps);
	} else {
		e.type = SGUI_EXPOSE_EVENT;
		e.src.window = super;
		sgui_rect_set_size(&e.arg.rect, 0, 0, super->w, super->h);
		sgui_internal_window_fire_event(super, &e);
	}

	return DefWindowProcA(this->hWnd, msg, wp, lp);
}

static int handle_resize(sgui_window_w32 *this, LPARAM lp)
{
	sgui_window *super = (sgui_window *)this;
	sgui_event e;

	super->w = LOWORD(lp);
	super->h = HIWORD(lp);

	e.type = SGUI_SIZE_CHANGE_EVENT;
	e.src.window = super;
	e.arg.ui2.x = super->w;
	e.arg.ui2.y = super->h;

	if (super->backend == SGUI_NATIVE)
		resize_pixmap(this);

	if (super->canvas)
		sgui_canvas_resize(super->canvas, super->w, super->h);

	if (super->backend == SGUI_DIRECT3D_11)
		d3d11_resize(super->ctx);

	sgui_internal_window_fire_event(super, &e);

	if (super->backend == SGUI_NATIVE)
		sgui_canvas_draw_widgets(super->canvas, 1);
	return 0;
}

int handle_window_events(sgui_window_w32 *this, UINT msg, WPARAM wp, LPARAM lp)
{
	sgui_window *super = (sgui_window *)this;
	sgui_event e;
	WCHAR c[2];

	e.src.window = super;

	switch (msg) {
	case WM_SETFOCUS:
		e.type = SGUI_FOCUS_EVENT;
		goto send_ev;
	case WM_KILLFOCUS:
		e.type = SGUI_FOCUS_LOSE_EVENT;
		goto send_ev;
	case WM_LBUTTONDBLCLK:
		e.type = SGUI_DOUBLE_CLICK_EVENT;
		goto event_xy;
	case WM_MOUSEMOVE:
		e.type = SGUI_MOUSE_MOVE_EVENT;
		goto event_xy;
	case WM_DESTROY:
		e.type = SGUI_USER_CLOSED_EVENT;
		super->flags &= ~SGUI_VISIBLE;
		goto send_ev;
	case WM_MOUSEWHEEL:
		e.arg.i = GET_WHEEL_DELTA_WPARAM(wp) / WHEEL_DELTA;
		e.type = SGUI_MOUSE_WHEEL_EVENT;
		goto send_ev;
	case WM_LBUTTONDOWN:
		e.type = SGUI_MOUSE_PRESS_EVENT;
		e.arg.i3.z = SGUI_MOUSE_BUTTON_LEFT;
		goto event_xy;
	case WM_MBUTTONDOWN:
		e.type = SGUI_MOUSE_PRESS_EVENT;
		e.arg.i3.z = SGUI_MOUSE_BUTTON_MIDDLE;
		goto event_xy;
	case WM_RBUTTONDOWN:
		e.type = SGUI_MOUSE_PRESS_EVENT;
		e.arg.i3.z = SGUI_MOUSE_BUTTON_RIGHT;
		goto event_xy;
	case WM_LBUTTONUP:
		e.type = SGUI_MOUSE_RELEASE_EVENT;
		e.arg.i3.z = SGUI_MOUSE_BUTTON_LEFT;
		goto event_xy;
	case WM_MBUTTONUP:
		e.type = SGUI_MOUSE_RELEASE_EVENT;
		e.arg.i3.z = SGUI_MOUSE_BUTTON_MIDDLE;
		goto event_xy;
	case WM_RBUTTONUP:
		e.type = SGUI_MOUSE_RELEASE_EVENT;
		e.arg.i3.z = SGUI_MOUSE_BUTTON_RIGHT;
		goto event_xy;
	case WM_CHAR:
		c[0] = (WCHAR)wp;
		c[1] = '\0';

		if ((c[0] < 0x80) && iscntrl(c[0]))
			break;

		WideCharToMultiByte(CP_UTF8, 0, c, 2, e.arg.utf8, 8,
					NULL, NULL);
		e.type = SGUI_CHAR_EVENT;
		goto send_ev;
	case WM_MOVE:
		super->x = LOWORD(lp);
		super->y = HIWORD(lp);
		return 0;
	case WM_SYSKEYUP:
	case WM_SYSKEYDOWN:
	case WM_KEYDOWN:
	case WM_KEYUP:
		return handle_key_event(this, msg, wp, lp);
	case WM_SIZE:
		return handle_resize(this, lp);
	case WM_PAINT:
		return handle_repaint(this, msg, wp, lp);
	}

	return DefWindowProcA(this->hWnd, msg, wp, lp);
event_xy:
	e.arg.i3.x = LOWORD(lp);
	e.arg.i3.y = HIWORD(lp);
send_ev:
	sgui_internal_window_fire_event(super, &e);
	return 0;
}

/****************************************************************************/

sgui_window *window_create_w32(sgui_lib *slib,
				const sgui_window_description *desc)
{
	HWND parent_hnd = desc->parent ? TO_W32(desc->parent)->hWnd : 0;
	DWORD style = WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
	sgui_lib_w32 *lib = (sgui_lib_w32 *)slib;
	sgui_window_w32 *this;
	sgui_window *super;
	RECT r;

	if (!desc->width || !desc->height)
		return NULL;

	if (desc->flags & ~SGUI_ALL_WINDOW_FLAGS)
		return NULL;

	this = calloc(1, sizeof(*this));
	super = (sgui_window*)this;

	if (!this)
		return NULL;

	sgui_internal_lock_mutex();
	SetRect(&r, 0, 0, desc->width, desc->height);

	if (desc->parent) {
		style |= WS_CHILD;
	} else {
		if (desc->flags & SGUI_FIXED_SIZE) {
			style |= (WS_CAPTION | WS_SYSMENU);
		} else {
			style |= WS_OVERLAPPEDWINDOW;
		}
		AdjustWindowRect(&r, style, FALSE);
	}

	this->hWnd = CreateWindowExA(0, lib->wndclass, "", style, 0, 0,
					r.right - r.left, r.bottom - r.top,
					parent_hnd, 0, lib->hInstance, 0);

	if (!this->hWnd)
		goto failwnd;

	switch (desc->backend) {
	case SGUI_NATIVE:
		create_canvas(this, slib, desc);
		if (!super->canvas)
			goto failcv;
		break;
	case SGUI_OPENGL_CORE:
	case SGUI_OPENGL_COMPAT:
		if (!set_pixel_format(this, slib, desc))
			break;
		super->ctx = gl_context_create(this, desc->backend,
							desc->share);
		if (!super->ctx)
			goto faildc;
		break;
	case SGUI_DIRECT3D_9:
		super->ctx = d3d9_context_create(super, desc);
		if (!super->ctx)
			goto failcv;
		break;
	case SGUI_DIRECT3D_11:
		super->ctx = d3d11_context_create(super, desc);
		if (!super->ctx)
			goto failcv;
		break;
	}

	if (desc->flags & SGUI_VISIBLE)
		ShowWindow(this->hWnd, SW_SHOWNORMAL);

	SET_USER_PTR(this->hWnd, this);

	super->flags = desc->flags;
	super->get_mouse_position = w32_window_get_mouse_position;
	super->set_mouse_position = w32_window_set_mouse_position;
	super->toggle_flags = w32_window_toggle_flags;
	super->set_title = w32_window_set_title;
	super->set_size = w32_window_set_size;
	super->move_center = w32_window_move_center;
	super->move = w32_window_move;
	super->force_redraw = w32_window_force_redraw;
	super->get_platform_data = w32_window_get_platform_data;
	super->make_topmost = w32_window_make_topmost;
	super->destroy = w32_window_destroy;
	super->write_clipboard = w32_window_write_clipboard;
	super->read_clipboard = w32_window_read_clipboard;
	super->w = desc->width;
	super->h = desc->height;
	super->backend = desc->backend;
	super->modmask = 0;
	super->lib = slib;

	if (super->canvas) {
		sgui_canvas_begin(super->canvas, NULL);
		sgui_canvas_clear(super->canvas, NULL);
		sgui_canvas_end(super->canvas);
	}

	sgui_internal_add_window(slib, super);
	sgui_internal_unlock_mutex();
	return (sgui_window *)this;
faildc:
	ReleaseDC(this->hWnd, this->hDC);
failcv:
	DestroyWindow(this->hWnd);
failwnd:
	free(this);
	sgui_internal_unlock_mutex();
	return NULL;
}
