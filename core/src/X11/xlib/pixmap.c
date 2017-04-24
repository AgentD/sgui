/*
 * pixmap.c
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
#include "internal.h"

void xlib_pixmap_destroy(sgui_pixmap *super)
{
	xlib_pixmap *this = (xlib_pixmap *)super;
	sgui_lib_x11 *lib = (sgui_lib_x11 *)this->lib;

	if (this->is_stencil) {
		free(this->data.pixels);
	} else {
		sgui_internal_lock_mutex();
		XFreePixmap(lib->dpy, this->data.xpm);
		sgui_internal_unlock_mutex();
	}

	free(this);
}

void xlib_pixmap_load(sgui_pixmap *super, int dstx, int dsty,
			const unsigned char *data, unsigned int scan,
			unsigned int width, unsigned int height, int format)
{
	xlib_pixmap *this = (xlib_pixmap *)super;
	sgui_lib_x11 *lib = (sgui_lib_x11 *)this->lib;
	const unsigned char *src, *row;
	unsigned int i, j, c;
	unsigned char *dst;
	sgui_rect locked;
	sgui_color pt;
	XRectangle r;

	if (this->is_stencil && format != SGUI_A8)
		return;

	sgui_internal_lock_mutex();

	r.x = r.y = 0;
	r.width = super->width;
	r.height = super->height;
	XSetClipRectangles(lib->dpy, this->owner->gc, 0, 0, &r, 1, Unsorted);

	if (this->is_stencil) {
		dst = this->data.pixels + (dsty * super->width + dstx);

		for (j = 0; j < height; ++j) {
			memcpy(dst, data, width);
			data += scan;
			dst += super->width;
		}
	} else if (format == SGUI_RGBA8) {
		for (src = data, j = 0; j < height; ++j, src += scan * 4) {
			for (row = src, i = 0; i < width; ++i, row += 4) {
				pt = *((sgui_color *)row);

				pt = sgui_color_mix(this->owner->bg,
							pt, pt.c.a);

				c = (pt.c.r << 16) | (pt.c.g << 8) | pt.c.b;
				XSetForeground(lib->dpy, this->owner->gc, c);

				XDrawPoint(lib->dpy, this->data.xpm,
						this->owner->gc,
						dstx + i, dsty + j);
			}
		}
	} else if (format == SGUI_RGB8) {
		for (src = data, j = 0; j < height; ++j, src += scan * 3) {
			for (row = src, i = 0; i < width; ++i, row += 3) {
				c = (row[0]<<16) | (row[1]<<8) | row[2];

				XSetForeground(lib->dpy, this->owner->gc, c);
				XDrawPoint(lib->dpy, this->data.xpm,
					this->owner->gc, dstx + i, dsty + j);
			}
		}
	}

	if (((sgui_canvas *)this->owner)->flags & SGUI_CANVAS_BEGAN) {
		locked = ((sgui_canvas *)this->owner)->locked;
		r.x = locked.left;
		r.y = locked.top;
		r.width = SGUI_RECT_WIDTH(locked);
		r.height = SGUI_RECT_WIDTH(locked);
		XSetClipRectangles(lib->dpy, this->owner->gc, 0, 0,
					&r, 1, Unsorted);
	}
	sgui_internal_unlock_mutex();
}

sgui_pixmap *xlib_pixmap_create(sgui_canvas *cv, unsigned int width,
				unsigned int height, int format)
{
	sgui_canvas_xlib *owner = (sgui_canvas_xlib *)cv;
	xlib_pixmap *this = calloc(1, sizeof(*this));
	Drawable wnd = ((sgui_canvas_x11 *)cv)->wnd;
	sgui_pixmap *super = (sgui_pixmap *)this;
	sgui_lib_x11 *lib = (sgui_lib_x11 *)cv->lib;

	if (!this)
		return NULL;

	super->width = width;
	super->height = height;
	super->destroy = xlib_pixmap_destroy;
	super->load = xlib_pixmap_load;

	this->is_stencil = format == SGUI_A8;
	this->owner = owner;
	this->lib = cv->lib;

	if (format == SGUI_A8) {
		this->data.pixels = malloc(width * height);
	} else {
		sgui_internal_lock_mutex();
		this->data.xpm = XCreatePixmap(lib->dpy, wnd,
						width, height, 24);
		sgui_internal_unlock_mutex();
	}

	if (!this->data.pixels || !this->data.xpm)
		goto fail;

	return (sgui_pixmap*)this;
fail:
	free(this);
	return NULL;
}
