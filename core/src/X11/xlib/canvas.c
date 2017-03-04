/*
 * canvas.c
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

static void canvas_xlib_destroy(sgui_canvas *super)
{
    	sgui_canvas_xlib *this = (sgui_canvas_xlib *)super;

	sgui_internal_lock_mutex();
	if (((sgui_canvas_x11 *)this)->cache)
		sgui_icon_cache_destroy(((sgui_canvas_x11 *)this)->cache);

	XFreeGC(x11.dpy, this->gc);
	sgui_internal_unlock_mutex();

	free(this);
}

static void canvas_xlib_set_clip_rect(sgui_canvas_x11 *super,
					int left, int top,
					int width, int height)
{
	sgui_canvas_xlib *this = (sgui_canvas_xlib *)super;
	XRectangle r;

	r.x = left;
	r.y = top;
	r.width  = width;
	r.height = height;

	sgui_internal_lock_mutex();
	XSetClipRectangles(x11.dpy, this->gc, 0, 0, &r, 1, Unsorted);
	sgui_internal_unlock_mutex();
}

static void canvas_xlib_draw_box(sgui_canvas *super, sgui_rect *r,
				const unsigned char *color, int format)
{
	sgui_canvas_xlib *this = (sgui_canvas_xlib *)super;
	unsigned long R, G, B, A, iA;

	if (format == SGUI_RGB8) {
		R = color[0];
		G = color[1];
		B = color[2];
	} else if(format == SGUI_RGBA8) {
		A = color[3];
		iA = 0xFF - A;

		R = ((color[0] * A + this->bg[0] * iA) >> 8) & 0x00FF;
		G = ((color[1] * A + this->bg[1] * iA) >> 8) & 0x00FF;
		B = ((color[2] * A + this->bg[2] * iA) >> 8) & 0x00FF;
	} else {
		R = G = B = color[0];
	}

	sgui_internal_lock_mutex();
	XSetForeground(x11.dpy, this->gc, (R<<16) | (G<<8) | B);
	XFillRectangle(x11.dpy, ((sgui_canvas_x11 *)this)->wnd,
			this->gc, r->left, r->top,
			SGUI_RECT_WIDTH_V(r), SGUI_RECT_HEIGHT_V(r));
	sgui_internal_unlock_mutex();
}

static void canvas_xlib_blit(sgui_canvas *super, int x, int y,
				sgui_pixmap *pixmap, sgui_rect *srcrect)
{
	sgui_canvas_xlib *this = (sgui_canvas_xlib *)super;
	xlib_pixmap *pix = (xlib_pixmap *)pixmap;

	if (pix->is_stencil)
		return;

	sgui_internal_lock_mutex();
	XCopyArea(x11.dpy, pix->data.xpm, ((sgui_canvas_x11 *)this)->wnd,
		this->gc, srcrect->left, srcrect->top,
		SGUI_RECT_WIDTH_V(srcrect), SGUI_RECT_HEIGHT_V(srcrect), x, y);
	sgui_internal_unlock_mutex();
}

static void canvas_xlib_blend_glyph(sgui_canvas *super, int x, int y,
					sgui_pixmap *pixmap, sgui_rect *r,
					const unsigned char *color)

{
	sgui_canvas_xlib *this = (sgui_canvas_xlib *)super;
	xlib_pixmap *pix = (xlib_pixmap *)pixmap;
	unsigned char *src, *src_row;
	int X, Y, C[4], sc;

	src = pix->data.pixels + (r->top * pixmap->width + r->left);
	C[3] = (color[0] << 16) | (color[1] << 8) | color[2];
	C[2] = ((3 * color[0] / 4 + this->bg[0] / 4) << 16) |
		((3 * color[1] / 4 + this->bg[1] / 4) << 8) |
		(3 * color[2] / 4 + this->bg[2] / 4);
	C[1] = ((color[0] / 2 + this->bg[0] / 2) << 16) |
		((color[1] / 2 + this->bg[1] / 2) << 8) |
		(color[2] / 2 + this->bg[2] / 2);
	C[0] = ((color[0] / 4 + 3 * this->bg[0] / 4) << 16) |
		((color[1] / 4 + 3 * this->bg[1] / 4) << 8) |
		(color[2] / 4 + 3 * this->bg[2] / 4);

	sgui_internal_lock_mutex();
	for (Y = r->top; Y <= r->bottom; ++Y, src += pixmap->width) {
		src_row = src;

		for (X = r->left; X <= r->right; ++X, ++src_row) {
			if ((*src_row) <= 0x20)
				continue;

			sc = C[(*src_row) >> 6];

			XSetForeground(x11.dpy, this->gc, sc);
			XDrawPoint(x11.dpy, ((sgui_canvas_x11 *)this)->wnd,
				this->gc, x + X - r->left, y + Y - r->top);
		}
	}
	sgui_internal_unlock_mutex();
}

sgui_canvas *canvas_xlib_create(Drawable wnd, unsigned int width,
				unsigned int height, int sendexpose)
{
	sgui_canvas_xlib *this = calloc(1, sizeof(*this));
	sgui_canvas *super = (sgui_canvas *)this;

	if (!this)
		return NULL;

	sgui_internal_lock_mutex();

	if (!(this->gc = XCreateGC(x11.dpy, wnd, 0, NULL)))
		goto fail;

	if (!sgui_canvas_init(super, width, height))
		goto fail;

	memcpy(this->bg, sgui_skin_get()->window_color, 4);

	sgui_internal_unlock_mutex();

	super->destroy = canvas_xlib_destroy;
	super->blit = canvas_xlib_blit;
	super->blend = canvas_xlib_blit;
	super->blend_glyph = canvas_xlib_blend_glyph;
	super->create_pixmap = xlib_pixmap_create;
	super->draw_box = canvas_xlib_draw_box;

	canvas_x11_init(super, wnd, canvas_xlib_set_clip_rect, sendexpose);
	return (sgui_canvas *)this;
fail:
	if (this->gc)
		XFreeGC(x11.dpy, this->gc);
	sgui_internal_unlock_mutex();
	free(this);
	return NULL;
}