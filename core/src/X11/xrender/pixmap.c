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

void xrender_pixmap_destroy(sgui_pixmap *super)
{
	xrender_pixmap *this = (xrender_pixmap *)super;

	sgui_internal_lock_mutex();
	XRenderFreePicture(x11.dpy, this->pic);
	XFreePixmap(x11.dpy, this->pix);
	sgui_internal_unlock_mutex();
	free(this);
}

void xrender_pixmap_load(sgui_pixmap *super, int dstx, int dsty,
			const unsigned char *data, unsigned int scan,
			unsigned int width, unsigned int height, int format)
{
	xrender_pixmap *this = (xrender_pixmap *)super;
	const unsigned char *src, *row;
	unsigned int i, j;
	XRenderColor c;
	Picture pic;
	int val;

	sgui_internal_lock_mutex();

	pic = this->pic;

	if (format == SGUI_RGBA8) {
		for (src = data, j = 0; j < height; ++j, src += scan * 4) {
			for (row = src, i = 0; i < width; ++i, row += 4) {
				/* Xrender expects premultiplied alpha! */
				c.red = row[0] * row[3];
				c.green = row[1] * row[3];
				c.blue = row[2] * row[3];
				c.alpha = row[3] << 8;

				XRenderFillRectangle(x11.dpy, PictOpSrc, pic,
							&c, dstx + i, dsty + j,
							1, 1);
			}
		}
	} else if (format==SGUI_RGB8) {
		for (src = data, j = 0; j < height; ++j, src += scan * 3) {
			for (row = src, i = 0; i < width; ++i, row += 3) {
				c.red = row[0] << 8;
				c.green = row[1] << 8;
				c.blue = row[2] << 8;
				c.alpha = 0xFFFF;

				XRenderFillRectangle(x11.dpy, PictOpSrc, pic,
							&c, dstx + i, dsty + j,
							1, 1);
			}
		}
	} else {
		for (src = data, j = 0; j < height; ++j, src += scan) {
			for (row = src, i = 0; i < width; ++i, ++row) {
				val = (*row) << 8;
				c.red = c.green = c.blue = c.alpha = val;

				XRenderFillRectangle(x11.dpy, PictOpSrc, pic,
							&c, dstx + i, dsty + j,
							1, 1);
			}
		}
	}

	sgui_internal_unlock_mutex();
}

sgui_pixmap *xrender_pixmap_create(sgui_canvas *cv, unsigned int width,
				unsigned int height, int format)
{
	Drawable wnd = ((sgui_canvas_x11 *)cv)->wnd;
	xrender_pixmap *this = calloc(1, sizeof(*this));
	sgui_pixmap *super = (sgui_pixmap *)this;
	XRenderPictFormat *fmt;
	int type;

	if (!this)
		return NULL;

	super->width = width;
	super->height = height;
	super->destroy = xrender_pixmap_destroy;
	super->load = xrender_pixmap_load;

	sgui_internal_lock_mutex();

	this->pix = XCreatePixmap(x11.dpy, wnd, width, height,
				format == SGUI_RGBA8 ? 32 :
				format == SGUI_RGB8 ? 24 : 8);

	if (!this->pix)
		goto fail;

	type = (format == SGUI_RGBA8) ? PictStandardARGB32 :
		(format == SGUI_RGB8 ? PictStandardRGB24 : PictStandardA8);

	fmt = XRenderFindStandardFormat(x11.dpy, type);
	this->pic = XRenderCreatePicture(x11.dpy, this->pix, fmt, 0, NULL);

	if (!this->pic) {
		XFreePixmap(x11.dpy, this->pix);
		goto fail;
	}

	sgui_internal_unlock_mutex();
	return (sgui_pixmap*)this;
fail:
	sgui_internal_unlock_mutex();
	free(this);
	return NULL;
}
