/*
 * image.c
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
#include "sgui_image.h"
#include "sgui_canvas.h"
#include "sgui_widget.h"
#include "sgui_internal.h"
#include "sgui_pixmap.h"

#include <stdlib.h>
#include <string.h>


typedef struct {
	sgui_widget super;

	void *data;
	unsigned int op : 9;
	unsigned int useptr : 1;
	unsigned int from_pixmap : 1;
	unsigned int format : 5;

	sgui_rect src;

	sgui_pixmap *pixmap;
} sgui_image;

static void image_draw(sgui_widget *super)
{
	sgui_image *this = (sgui_image *)super;

	if (!this->pixmap)
		return;

	sgui_canvas_draw_pixmap(super->canvas, super->area.left,
				super->area.top, this->pixmap,
				&this->src, this->op);
}

static void image_destroy(sgui_widget *super)
{
	sgui_image *this = (sgui_image *)super;

	if (!this->useptr)
		free(this->data);

	if (this->pixmap && !this->from_pixmap)
		sgui_pixmap_destroy(this->pixmap);
	free(this);
}

static void image_on_state_change(sgui_widget *super, int change)
{
	sgui_image *this = (sgui_image *)super;
	unsigned int w, h;
	sgui_pixmap *p;

	if (change & SGUI_WIDGET_CANVAS_CHANGED) {
		sgui_internal_lock_mutex();

		if (this->from_pixmap) {
			sgui_internal_unlock_mutex();
			return;
		}

		if (this->pixmap) {
			sgui_pixmap_destroy(this->pixmap);
			this->pixmap = NULL;
		}

		if (!super->canvas) {
			sgui_internal_unlock_mutex();
			return;
		}

		sgui_widget_get_size(super, &w, &h);
		p = sgui_canvas_create_pixmap(super->canvas, w, h,
						this->format);
		if (p) {
			sgui_pixmap_load(p, 0, 0, this->data, 0, 0,
					w, h, w, this->format);
		}
		this->pixmap = p;
		sgui_internal_unlock_mutex();
	}
}

/****************************************************************************/

sgui_widget *sgui_image_create(int x, int y, unsigned int width,
				unsigned int height, const void *data,
				int format, int flags)
{
	sgui_widget *super;
	sgui_image *this;
	size_t num_bytes;

	if (flags & ~(SGUI_IMAGE_BLEND | SGUI_IMAGE_KEEP_PTR))
		return NULL;

	this = calloc(1, sizeof(*this));
	super = (sgui_widget *)this;

	if (!this)
		return NULL;

	sgui_rect_set_size(&this->src, 0, 0, width, height);
	sgui_widget_init(super, x, y, width, height);

	if (flags & SGUI_IMAGE_KEEP_PTR) {
		this->data = (unsigned char *)data;
	} else {
		num_bytes = width * height * (format == SGUI_RGBA8 ? 4 :
						(format == SGUI_RGB8 ? 3 : 1));
		this->data = malloc(num_bytes);

		if (!this->data) {
			free(this);
			return NULL;
		}

		memcpy(this->data, data, num_bytes);
	}

	super->draw = image_draw;
	super->state_change_event = image_on_state_change;
	super->destroy = image_destroy;
	super->flags = SGUI_WIDGET_VISIBLE;
	this->pixmap = NULL;
	this->format = format;
	this->op = (flags & SGUI_IMAGE_BLEND) ? SGUI_CANVAS_BLEND : 0;
	this->useptr = (flags & SGUI_IMAGE_KEEP_PTR) ? 1 : 0;
	return super;
}

sgui_widget* sgui_image_from_pixmap(int x, int y, unsigned int width,
				unsigned int height, const sgui_pixmap *pixmap,
				int src_x, int src_y, int flags)
{
	sgui_widget *super;
	sgui_image *this;

	if (flags & ~(SGUI_IMAGE_BLEND))
		return NULL;

	this = calloc(1, sizeof(*this));
	super = (sgui_widget *)this;

	if (!this)
		return NULL;

	sgui_rect_set_size(&this->src, src_x, src_y, width, height);
	sgui_widget_init(super, x, y, width, height);

	super->draw = image_draw;
	super->state_change_event = image_on_state_change;
	super->destroy = image_destroy;
	super->flags = SGUI_WIDGET_VISIBLE;
	this->pixmap = (sgui_pixmap *)pixmap;
	this->op = (flags & SGUI_IMAGE_BLEND) ? SGUI_CANVAS_BLEND : 0;
	this->from_pixmap = 1;
	return super;
}

void sgui_image_reload(sgui_widget *super, unsigned int x, unsigned int y,
			unsigned int width, unsigned int height)
{
	sgui_image *this = (sgui_image *)super;
	unsigned int scan, imgheight;
	sgui_rect r;

	if (!this->pixmap || this->from_pixmap)
		return;

	scan = SGUI_RECT_WIDTH(super->area);
	imgheight = SGUI_RECT_HEIGHT(super->area);

	/* clamp image area */
	if (x >= scan || y >= imgheight)
		return;

	if ((x + width) > scan)
		width = scan - x;

	if ((y + height) > imgheight)
		height = imgheight - y;

	if (!width || !height)
		return;

	/* reupload image portion */
	sgui_pixmap_load(this->pixmap, x, y, this->data, x, y, width, height,
			scan, this->format);

	/* flag changed area dirty */
	if (super->canvas) {
		sgui_widget_get_absolute_rect(super, &r);
		r.left += x;
		r.top += y;
		r.right = r.left + width - 1;
		r.bottom = r.top + height - 1;
		sgui_canvas_add_dirty_rect(super->canvas, &r);
	}
}
