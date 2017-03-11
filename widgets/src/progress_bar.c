/*
 * progress_bar.c
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
#include "sgui_progress_bar.h"
#include "sgui_skin.h"
#include "sgui_internal.h"
#include "sgui_widget.h"

#include <stdlib.h>
#include <string.h>


typedef struct {
	sgui_widget super;

	int flags;
	unsigned int progress;
} sgui_progress_bar;


static void progress_draw(sgui_widget *super)
{
	sgui_progress_bar *this = (sgui_progress_bar *)super;
	sgui_skin *skin = sgui_skin_get();
	unsigned int length;

	if (this->flags & SGUI_PROGRESS_BAR_VERTICAL) {
		length = SGUI_RECT_HEIGHT(super->area);
	} else {
		length = SGUI_RECT_WIDTH(super->area);
	}


	skin->draw_progress_bar(skin, super->canvas, super->area.left,
				super->area.top, length, this->flags, 
				this->progress);
}


sgui_widget *sgui_progress_bar_create(int x, int y, int flags,
					unsigned int progress,
					unsigned int length)
{
	sgui_skin *skin = sgui_skin_get();
	unsigned int width, height;
	sgui_progress_bar* this;
	sgui_widget* super;

	if (progress > 100)
		progress = 100;

	if (flags & ~(SGUI_PROGRESS_BAR_CONTINUOUS|SGUI_PROGRESS_BAR_VERTICAL))
		return NULL;

	this = calloc(1, sizeof(*this));
	super = (sgui_widget *)this;

	if (!this)
		return NULL;

	sgui_widget_init(super, 0, 0, 0, 0);

	super->draw = progress_draw;
	super->flags = SGUI_WIDGET_VISIBLE;
	super->destroy = (void(*)(sgui_widget*))free;
	this->flags = flags;
	this->progress = progress;

	if (flags & SGUI_PROGRESS_BAR_VERTICAL) {
		width = skin->get_progess_bar_width(skin);
		height = length;
	} else {
		width = length;
		height = skin->get_progess_bar_width(skin);
	}

	sgui_rect_set_size(&super->area, x, y, width, height);
	return super;
}

void sgui_progress_bar_set_progress(sgui_widget *super, unsigned int progress)
{
	sgui_progress_bar *this = ((sgui_progress_bar *)super);
	sgui_rect r;

	sgui_internal_lock_mutex();
	if (progress > 100)
		progress = 100;

	if (this->progress == progress)
		goto out;

	this->progress = progress;

	if (super->canvas) {
		sgui_widget_get_absolute_rect(super, &r);
		sgui_canvas_add_dirty_rect(super->canvas, &r);
	}
out:
	sgui_internal_unlock_mutex();
}

unsigned int sgui_progress_bar_get_progress(sgui_widget *this)
{
	return ((sgui_progress_bar *)this)->progress;
}
