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


static sgui_pixmap *skin_pixmap = NULL;


static void canvas_xrender_destroy(sgui_canvas *super)
{
	sgui_canvas_xrender *this = (sgui_canvas_xrender *)super;

	sgui_internal_lock_mutex();

	if (((sgui_canvas_x11 *)this)->cache)
		sgui_icon_cache_destroy(((sgui_canvas_x11 *)this)->cache);

	if (this->pic)
		XRenderFreePicture(x11.dpy, this->pic);
	if (this->pen)
		XRenderFreePicture(x11.dpy, this->pen);
	if (this->penmap)
		XFreePixmap(x11.dpy, this->penmap);

	sgui_internal_unlock_mutex();
	free(this);
}

static void canvas_xrender_set_clip_rect(sgui_canvas_x11 *super,
					int left, int top,
					int width, int height)
{
	sgui_canvas_xrender *this = (sgui_canvas_xrender *)super;
	XRectangle r;

	r.x = left;
	r.y = top;
	r.width = width;
	r.height = height;

	sgui_internal_lock_mutex();
	XRenderSetPictureClipRectangles(x11.dpy, this->pic, 0, 0, &r, 1);
	sgui_internal_unlock_mutex();
}

static void canvas_xrender_draw_box(sgui_canvas *super, const sgui_rect *r,
					sgui_color color, int op)
{
	sgui_canvas_xrender *this = (sgui_canvas_xrender *)super;
	XRenderColor c;
	int xop;

	c.red = color.c.r << 8;
	c.green = color.c.g << 8;
	c.blue = color.c.b << 8;

	if (op == SGUI_CANVAS_BLEND) {
		c.alpha = color.c.a << 8;
		xop = PictOpOver;
	} else {
		c.alpha = 0xFFFF;
		xop = PictOpSrc;
	}

	sgui_internal_lock_mutex();
	XRenderFillRectangle(x11.dpy, xop, this->pic, &c, r->left, r->top,
				SGUI_RECT_WIDTH_V(r), SGUI_RECT_HEIGHT_V(r));
	sgui_internal_unlock_mutex();
}

static void canvas_xrender_blit(sgui_canvas *super, int x, int y,
				const sgui_pixmap *pixmap,
				const sgui_rect *srcrect, int op)
{
	sgui_canvas_xrender *this = (sgui_canvas_xrender *)super;
	unsigned int w = SGUI_RECT_WIDTH_V(srcrect);
	unsigned int h = SGUI_RECT_HEIGHT_V(srcrect);
	XRenderColor c;

	c.red = c.green = c.blue = 0;
	c.alpha = 0xFFFF;

	sgui_internal_lock_mutex();
	if (op != SGUI_CANVAS_BLEND) {
		XRenderFillRectangle(x11.dpy, PictOpSrc, this->pic, &c,
					x, y, w, h);
	}

	XRenderComposite(x11.dpy, PictOpOver, ((xrender_pixmap*)pixmap)->pic,
			0, this->pic, srcrect->left, srcrect->top, 0, 0,
			x, y, w, h);
	sgui_internal_unlock_mutex();
}

static void canvas_xrender_blend_glyph(sgui_canvas *super, int x, int y,
					const sgui_pixmap *pixmap,
					const sgui_rect *r,
					const sgui_color color)
{
	sgui_canvas_xrender *this = (sgui_canvas_xrender *)super;
	XRenderColor c;

	c.red = color.c.r << 8;
	c.green = color.c.g << 8;
	c.blue = color.c.b << 8;
	c.alpha = 0xFFFF;

	sgui_internal_lock_mutex();
	XRenderFillRectangle(x11.dpy, PictOpSrc, this->pen, &c, 0, 0, 1, 1);

	XRenderComposite(x11.dpy, PictOpOver, this->pen,
			((xrender_pixmap *)pixmap)->pic,
			this->pic, 0, 0, r->left, r->top, x, y,
			SGUI_RECT_WIDTH_V(r), SGUI_RECT_HEIGHT_V(r));
	sgui_internal_unlock_mutex();
}

static const sgui_pixmap *canvas_xrender_get_skin_pixmap(sgui_canvas *super)
{
	unsigned int width, height;
	int format;
	sgui_skin *skin;

	sgui_internal_lock_mutex();

	if (!skin_pixmap) {
		skin = sgui_skin_get();

		skin->get_skin_pixmap_size(skin, &width, &height, &format);

		skin_pixmap = super->create_pixmap(super, width,
							height, format);

		if (skin_pixmap)
			skin->init_skin_pixmap(skin, skin_pixmap);
	}

	sgui_internal_unlock_mutex();
	return skin_pixmap;
}

void canvas_xrender_cleanup_skin_pixmap(void)
{
	if (skin_pixmap)
		skin_pixmap->destroy(skin_pixmap);
}

sgui_canvas *canvas_xrender_create(Drawable wnd, unsigned int width,
				unsigned int height, int sendexpose)
{
	sgui_canvas_xrender *this;
	sgui_canvas *super = NULL;
	XRenderPictFormat *fmt;
	XRenderPictureAttributes attr;
	int base, error;

	sgui_internal_lock_mutex();
	if (!XRenderQueryExtension(x11.dpy, &base, &error))
		goto fail;

	fmt = XRenderFindStandardFormat(x11.dpy, PictStandardRGB24);
	if (!fmt)
		goto fail;

	this = calloc(1, sizeof(*this));
	super = (sgui_canvas *)this;

	if (!this)
		goto fail;

	this->penmap = XCreatePixmap(x11.dpy, wnd, 1, 1, 24);
	if (!this->penmap)
		goto failfree;

	this->pic = XRenderCreatePicture(x11.dpy, wnd, fmt, 0, NULL);
	if (!this->pic)
		goto failfree;

	attr.repeat = RepeatNormal;
	this->pen = XRenderCreatePicture(x11.dpy, this->penmap, fmt,
					CPRepeat, &attr);
	if (!this->pen)
		goto failfree;

	if (!sgui_canvas_init(super, width, height))
		goto failfree;
	sgui_internal_unlock_mutex();

	super->destroy = canvas_xrender_destroy;
	super->blit = canvas_xrender_blit;
	super->blend_glyph = canvas_xrender_blend_glyph;
	super->create_pixmap = xrender_pixmap_create;
	super->draw_box = canvas_xrender_draw_box;
	super->get_skin_pixmap = canvas_xrender_get_skin_pixmap;

	canvas_x11_init(super, wnd, canvas_xrender_set_clip_rect, sendexpose);
	return (sgui_canvas *)this;
failfree:
	sgui_internal_unlock_mutex();
fail:
	if (super)
		canvas_xrender_destroy(super);
	return NULL;
}
