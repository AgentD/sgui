/*
 * messagebox_w32.c
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
#include "sgui_messagebox.h"

#ifdef SGUI_WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#ifndef SGUI_NO_NATIVE_MESSAGEBOX
void sgui_message_box_emergency(const char *caption, const char *text)
{
	MessageBox(0, text, caption, MB_OK | MB_ICONEXCLAMATION);
}
#elif defined(SGUI_NOP_IMPLEMENTATIONS)
void sgui_message_box_emergency(const char *caption, const char *text)
{
	(void)caption; (void)text;
}
#endif /* !SGUI_NO_NATIVE_MESSAGEBOX */

#endif /* SGUI_WINDOWS */
