/*
 * messagebox_x11.c
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
#include "sgui_internal.h"

#ifdef SGUI_UNIX
#ifndef SGUI_NO_NATIVE_MESSAGEBOX
#include <stdio.h>
#include <string.h>

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>

void sgui_message_box_emergency(const char *caption, const char *text)
{
	int okX1, okY1, okX2, okY2, okBaseX, okBaseY, okWidth, okHeight, W, H;
	int black, white, length, height, direction, ascent, descent, X, Y;
	int buttonFocus = 0, offset, i, j, lines, screen, run = 1;
	Atom atom_wm_delete;
	Display *dpy = NULL;
	XCharStruct overall;
	XFontStruct *font;
	XSizeHints hints;
	Window wnd = 0;
	GC gc = 0;
	XEvent e;

	dpy = XOpenDisplay(0);
	if (!dpy)
		goto fail;

	atom_wm_delete = XInternAtom(dpy, "WM_DELETE_WINDOW", True);
	screen = DefaultScreen(dpy);
	black = BlackPixel(dpy, screen);
	white = WhitePixel(dpy, screen);

	wnd = XCreateSimpleWindow(dpy, DefaultRootWindow(dpy), 0, 0, 100, 100,
					0, black, black);

	if (!wnd)
		goto fail;

	XSelectInput(dpy, wnd, ExposureMask | StructureNotifyMask |
				KeyReleaseMask | PointerMotionMask |
				ButtonPressMask | ButtonReleaseMask);

	XStoreName(dpy, wnd, caption);
	XSetWMProtocols(dpy, wnd, &atom_wm_delete, 1);
	XFlush(dpy);

	gc = XCreateGC(dpy, wnd, 0, 0);

	if (!gc)
		goto fail;

	XSetForeground(dpy, gc, white);
	XSetBackground(dpy, gc, black);

	font = XQueryFont(dpy, XGContextFromGC(gc));
	if (!font)
		goto fail;

	for (lines=1, i=0, j=0, length=0, height=0; text[j]; i=j+1, ++lines) {
		for (j = i; text[j] && text[j]!='\n'; ++j)
			;

		XTextExtents(font, text + i, j - i, &direction,
				&ascent, &descent, &overall);

		length = MAX(overall.width, length);
		height = MAX((ascent + descent), height);
	}

	W = length + 20;
	H = lines*height + height + 40;
	X = (DisplayWidth(dpy, screen) - W) / 2;
	Y = (DisplayHeight(dpy, screen) - H) / 2;

	memset(&hints, 0, sizeof(hints));
	hints.flags = PSize | PMinSize | PMaxSize;
	hints.min_width = hints.max_width = hints.base_width = W;
	hints.min_height = hints.max_height = hints.base_height = H;

	XMoveResizeWindow(dpy, wnd, X, Y, W, H);
	XSetWMNormalHints(dpy, wnd, &hints);

	/* Determine the size of the OK button */
	XTextExtents(font, "OK", 2, &direction, &ascent, &descent, &overall);

	okWidth = overall.width;
	okHeight = ascent + descent;

	okX1 = (W - okWidth) / 2 - 15;
	okX2 = okX1 + okWidth + 30;
	okY1 = lines * height + 20 + 5;
	okY2 = okY1 + 4 + okHeight;
	okBaseX = okX1 + 15;
	okBaseY = okY2 - 2;

	XFreeFontInfo(NULL, font, 1);

	/* Event loop */
	XMapRaised(dpy, wnd);
	XFlush(dpy);

	while (run) {
		XNextEvent(dpy, &e);
		offset = 0;

		if (e.type == MotionNotify) {
			i = e.xmotion.x >= okX1 && e.xmotion.x <= okX2 &&
				e.xmotion.y >= okY1 && e.xmotion.y <= okY2;

			if (i != buttonFocus)
				e.type = Expose;

			buttonFocus = i;
		}

		switch (e.type) {
		case ButtonPress:
		case ButtonRelease:
			if(e.xbutton.button != Button1)
				break;

			if (buttonFocus) {
				offset = e.type == ButtonPress ? 1 : 0;

				if (!offset)
					run = 0;
			}
		case Expose:
		case MapNotify:
			XClearWindow(dpy, wnd);
			Y = 10 + height;

			for (i = 0, j = 0; text[j]; i = j + 1) {
				for (j = i; text[j] && text[j] != '\n'; ++j)
					;

				XDrawString(dpy, wnd, gc, 10, Y,
						text + i, j - i);
				Y += height;
			}

			/* Draw OK button */
			if (buttonFocus) {
				XFillRectangle(dpy, wnd, gc, offset + okX1,
						offset + okY1, okX2 - okX1,
						okY2 - okY1);
				XSetForeground(dpy, gc, black);
			} else {
				XDrawRectangle(dpy, wnd, gc, okX1, okY1,
						okX2 - okX1, okY2 - okY1);
			}

			XDrawString(dpy, wnd, gc, offset + okBaseX,
					offset + okBaseY, "OK", 2);

			XSetForeground(dpy, gc, white);
			XFlush(dpy);
			break;
		case KeyRelease:
			if (XLookupKeysym(&e.xkey, 0) == XK_Escape)
				run = 0;
			break;
		case ClientMessage:
			if (e.xclient.data.l[0] == (long)atom_wm_delete)
				run = 0;
			break;
		}
	}

	/* Clean up */
	XFreeGC(dpy, gc);
	XDestroyWindow(dpy, wnd);
	XCloseDisplay(dpy);
	return;
fail:
	if (gc)
		XFreeGC(dpy, gc);
	if (wnd)
		XDestroyWindow(dpy, wnd);
	if (dpy)
		XCloseDisplay(dpy);

	/* last chance: print to error stream */
	fprintf(stderr, "***** %s *****\n%s\n\n", caption, text);
}
#elif defined(SGUI_NOP_IMPLEMENTATIONS)
void sgui_message_box_emergency(const char *caption, const char *text)
{
	(void)caption; (void)text;
}
#endif /* !SGUI_NO_NATIVE_MESSAGEBOX */
#endif /* SGUI_UNIX */
