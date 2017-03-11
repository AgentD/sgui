/*
 * platform.c
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
#include "sgui.h"
#include "platform.h"

struct w32_state w32;

static LRESULT CALLBACK WindowProcFun(HWND hWnd, UINT msg,
					WPARAM wp, LPARAM lp)
{
	sgui_window_w32 *wnd;
	int result = 0;

	sgui_internal_lock_mutex();
	wnd = (sgui_window_w32 *)GET_USER_PTR(hWnd);

	if (!wnd && (msg == WM_DESTROY || msg == WM_NCDESTROY))
		goto out;

	result = wnd ? handle_window_events(wnd, msg, wp, lp) :
			DefWindowProcA(hWnd, msg, wp, lp);
out:
	sgui_internal_unlock_mutex();
	return result;
}

static int is_window_active(void)
{
	sgui_window_w32 *i = w32.list;

	sgui_internal_lock_mutex();
	while (i != NULL && !(i->super.flags & SGUI_VISIBLE)) {
		i = i->next;
	}
	sgui_internal_unlock_mutex();

	return i != NULL;
}

static void update_windows(void)
{
	sgui_window_w32 *i;

	sgui_internal_lock_mutex();

	for (i = w32.list; i != NULL; i = i->next)
		update_window(i);

	sgui_internal_unlock_mutex();
}

WCHAR *utf8_to_utf16(const char *utf8, int rdbytes)
{
	size_t length;
	WCHAR *out;

	length = MultiByteToWideChar(CP_UTF8, 0, utf8, rdbytes, NULL, 0);
	out = malloc(sizeof(WCHAR) * (length + 1));

	if (out) {
		MultiByteToWideChar(CP_UTF8, 0, utf8, -1, out, length);
		out[length] = '\0';
	}
	return out;
}

char *utf16_to_utf8(WCHAR *utf16)
{
	size_t size;
	char *out;

	size = WideCharToMultiByte(CP_UTF8, 0, utf16, -1, 0, 0, 0, 0);
	out = malloc(size + 1);
	if (!out)
		return NULL;

	WideCharToMultiByte(CP_UTF8, 0, utf16, -1, out, size, NULL, NULL);
	out[size] = '\0';

	return out;
}

void w32_window_write_clipboard( sgui_window* this, const char* text,
                                 unsigned int length )
{
	WCHAR *ptr = NULL;
	unsigned int i;
	HGLOBAL hDATA;
	LPVOID dst;
	(void)this;

	sgui_internal_lock_mutex();

	if (!OpenClipboard(NULL))
		goto out;

	EmptyClipboard();

	ptr = utf8_to_utf16(text, length);
	if (!ptr)
		goto out_close;

	i = lstrlenW(ptr);
	hDATA = GlobalAlloc(GMEM_MOVEABLE, sizeof(WCHAR) * (i + 1));
	if (!hDATA)
		goto out_free;

	dst = GlobalLock(hDATA);
	if (!dst) {
		GlobalFree(hDATA);
		goto out_free;
	}

	memcpy(dst, ptr, sizeof(WCHAR) * (i + 1));
	GlobalUnlock(hDATA);
	SetClipboardData(CF_UNICODETEXT, hDATA);
out_free:
	free(ptr);
out_close:
	CloseClipboard();
out:
	sgui_internal_unlock_mutex();
}

const char *w32_window_read_clipboard(sgui_window *this)
{
	WCHAR *buffer = NULL;
	HANDLE hDATA;
	(void)this;

	sgui_internal_lock_mutex();

	free(w32.clipboard);
	w32.clipboard = NULL;

	if (!OpenClipboard(NULL))
		goto out;

	hDATA = GetClipboardData(CF_UNICODETEXT);
	if (!hDATA)
		goto out_close;

	buffer = (WCHAR *)GlobalLock(hDATA);
	if (!buffer)
		goto out_close;

	w32.clipboard = utf16_to_utf8(buffer);
	GlobalUnlock(hDATA);
out_close:
	CloseClipboard();
out:
	sgui_internal_unlock_mutex();
	return w32.clipboard;
}

void add_window(sgui_window_w32 *this)
{
	this->next = w32.list;
	w32.list = this;
}

void remove_window(sgui_window_w32 *this)
{
	sgui_window_w32 *i;

	if (w32.list == this) {
		w32.list = w32.list->next;
	} else {
		for (i = w32.list; i->next != NULL; i = i->next) {
			if (i->next == this) {
				i->next = this->next;
				break;
			}
		}
	}
}

void sgui_internal_lock_mutex(void)
{
	EnterCriticalSection(&w32.mutex);
}

void sgui_internal_unlock_mutex(void)
{
	LeaveCriticalSection(&w32.mutex);
}

int sgui_init(void)
{
	WNDCLASSEXA wc;

	memset(&w32, 0, sizeof(w32));
	w32.wndclass = "sgui_wnd_class";

	InitializeCriticalSection(&w32.mutex);

	if (!font_init())
		goto fail;

	if (!(w32.hInstance = GetModuleHandleA(NULL)))
		goto fail;

	memset(&wc, 0, sizeof(WNDCLASSEXA));

	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	wc.lpfnWndProc = WindowProcFun;
	wc.hInstance = w32.hInstance;
	wc.lpszClassName = w32.wndclass;
	wc.hCursor = LoadCursorA(NULL, IDC_ARROW);

	if (RegisterClassExA(&wc) == 0)
		goto fail;

	sgui_interal_skin_init_default();
	sgui_event_reset();
	return 1;
fail:
	sgui_deinit();
	return 0;
}

void sgui_deinit(void)
{
	sgui_event_reset();
	sgui_interal_skin_deinit_default();
	sgui_internal_memcanvas_cleanup();
	font_deinit();

	UnregisterClassA(w32.wndclass, w32.hInstance);
	DeleteCriticalSection(&w32.mutex);
	free(w32.clipboard);

	memset(&w32, 0, sizeof(w32));
}

int sgui_main_loop_step(void)
{
	MSG msg;

	update_windows();

	if (PeekMessageA(&msg, 0, 0, 0, PM_REMOVE)) {
		TranslateMessage(&msg);
		DispatchMessageA(&msg);
	}

	sgui_event_process();

	return is_window_active() || sgui_event_queued();
}

void sgui_main_loop(void)
{
	MSG msg;

	while (is_window_active()) {
		update_windows();
		GetMessageA(&msg, 0, 0, 0);
		TranslateMessage(&msg);
		DispatchMessageA(&msg);
		sgui_event_process();
	}

	while (sgui_event_queued()) {
		sgui_event_process();
	}
}
