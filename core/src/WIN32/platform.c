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

sgui_lib_w32 w32;

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

static void update_windows(sgui_lib *lib)
{
	sgui_window *i;

	sgui_internal_lock_mutex();

	for (i = lib->wndlist; i != NULL; i = i->next)
		update_window((sgui_window_w32 *)i);

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
	sgui_lib_w32 *lib = (sgui_lib_w32 *)this->lib;
	WCHAR *buffer = NULL;
	HANDLE hDATA;
	(void)this;

	sgui_internal_lock_mutex();

	free(lib->clipboard);
	lib->clipboard = NULL;

	if (!OpenClipboard(NULL))
		goto out;

	hDATA = GetClipboardData(CF_UNICODETEXT);
	if (!hDATA)
		goto out_close;

	buffer = (WCHAR *)GlobalLock(hDATA);
	if (!buffer)
		goto out_close;

	lib->clipboard = utf16_to_utf8(buffer);
	GlobalUnlock(hDATA);
out_close:
	CloseClipboard();
out:
	sgui_internal_unlock_mutex();
	return lib->clipboard;
}

void sgui_internal_lock_mutex(void)
{
	EnterCriticalSection(&w32.mutex);
}

void sgui_internal_unlock_mutex(void)
{
	LeaveCriticalSection(&w32.mutex);
}

static void w32_destroy(sgui_lib *lib)
{
	sgui_lib_w32 *w32_lib = (sgui_lib_w32 *)lib;

	if (lib->ev)
		lib->ev->destroy(lib->ev);

	sgui_interal_skin_deinit_default();
	sgui_internal_memcanvas_cleanup();
	font_deinit();

	UnregisterClassA(w32_lib->wndclass, w32_lib->hInstance);
	DeleteCriticalSection(&w32_lib->mutex);
	free(w32_lib->clipboard);

	memset(w32_lib, 0, sizeof(*w32_lib));
}

static int w32_main_loop_step(sgui_lib *lib)
{
	MSG msg;

	update_windows(lib);

	if (PeekMessageA(&msg, 0, 0, 0, PM_REMOVE)) {
		TranslateMessage(&msg);
		DispatchMessageA(&msg);
	}

	sgui_event_process(lib->ev);

	return sgui_lib_have_active_windows(lib) || sgui_event_queued(lib->ev);
}

static void w32_main_loop(sgui_lib *lib)
{
	MSG msg;

	while (sgui_lib_have_active_windows(lib)) {
		update_windows(lib);
		GetMessageA(&msg, 0, 0, 0);
		TranslateMessage(&msg);
		DispatchMessageA(&msg);
		sgui_event_process(lib->ev);
	}

	while (sgui_event_queued(lib->ev)) {
		sgui_event_process(lib->ev);
	}
}

sgui_lib *sgui_init(void *arg)
{
	WNDCLASSEXA wc;

	if (arg)
		return NULL;

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
	((sgui_lib *)&w32)->ev = sgui_event_queue_create();
	if (!((sgui_lib *)&w32)->ev)
		goto fail;

	((sgui_lib *)&w32)->destroy = w32_destroy;
	((sgui_lib *)&w32)->main_loop = w32_main_loop;
	((sgui_lib *)&w32)->main_loop_step = w32_main_loop_step;
	((sgui_lib *)&w32)->create_window = window_create_w32;
	return (sgui_lib *)&w32;
fail:
	w32_destroy((sgui_lib *)&w32);
	return NULL;
}
