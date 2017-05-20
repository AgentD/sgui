/*
 * lib.c
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
#include "sgui_internal.h"
#include "sgui_lib.h"



void sgui_internal_add_window(sgui_lib *lib, sgui_window *wnd)
{
	wnd->next = lib->wndlist;
	lib->wndlist = wnd;
}

void sgui_internal_remove_window(sgui_lib *lib, sgui_window *wnd)
{
	sgui_window *i;

	if (lib->wndlist == wnd) {
		lib->wndlist = wnd->next;
	} else {
		for (i = lib->wndlist; i->next != NULL; i = i->next) {
			if (i->next == wnd) {
				i->next = wnd->next;
				break;
			}
		}
	}
}

int sgui_lib_have_active_windows(sgui_lib *lib)
{
	sgui_window *i;

	sgui_internal_lock_mutex();
	for (i = lib->wndlist; i != NULL; i = i->next) {
		if (i->flags & SGUI_VISIBLE)
			break;
	}
	sgui_internal_unlock_mutex();

	return i != NULL;
}
