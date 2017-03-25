/*
 * scroll_bar.c
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
#include "sgui_scroll_bar.h"
#include "sgui_skin.h"
#include "sgui_event.h"
#include "sgui_internal.h"
#include "sgui_widget.h"

#include <stdlib.h>
#include <string.h>


typedef struct {
	sgui_widget super;
	unsigned int bw, bh, length, p_length, v_length;
	unsigned int p_offset, v_offset, v_max;
	int horizontal, inc_button_state, dec_button_state;

	sgui_scrollbar_callback scroll_fun;
	void *userptr;
} sgui_scroll_bar;


static void scroll_down(sgui_scroll_bar *this)
{
	unsigned int l = this->length - 2 * this->bh, v_new, p_new;

	v_new = this->v_offset + this->v_length + this->v_length / 4;
	p_new = this->p_offset + this->p_length + this->p_length / 4;

	if ((v_new < this->v_max) && (p_new < l)) {
		this->p_offset += this->p_length / 4;
		this->v_offset += this->v_length / 4;
	} else {
		this->p_offset = l - this->p_length;
		this->v_offset = this->v_max - this->v_length;
	}
}

static void scroll_up(sgui_scroll_bar *this)
{
	if ((this->p_offset > (this->p_length / 4)) &&
		(this->v_offset > (this->v_length / 4))) {
		this->p_offset -= this->p_length / 4;
		this->v_offset -= this->v_length / 4;
	} else {
		this->p_offset = 0;
		this->v_offset = 0;
	}
}

static void scroll_bar_on_event(sgui_widget* super, const sgui_event *e)
{
	sgui_scroll_bar *this = (sgui_scroll_bar *)super;
	unsigned int old;
	sgui_rect r;

	sgui_internal_lock_mutex();
	old = this->v_offset;

	switch (e->type) {
	case SGUI_MOUSE_WHEEL_EVENT:
		if (e->arg.i < 0) {
			scroll_down(this);
		} else {
			scroll_up(this);
		}
		break;
	case SGUI_MOUSE_RELEASE_EVENT:
	case SGUI_MOUSE_LEAVE_EVENT:
		if (this->dec_button_state || this->inc_button_state) {
			sgui_widget_get_absolute_rect(super, &r);
			sgui_canvas_add_dirty_rect(super->canvas, &r);
		}

		this->dec_button_state = this->inc_button_state = 0;
		break;
	case SGUI_MOUSE_PRESS_EVENT:
		if (this->horizontal) {
			this->dec_button_state = e->arg.i3.x < (int)this->bw;
			this->inc_button_state = e->arg.i3.x >
					((int)this->length - (int)this->bw);
		} else {
			this->dec_button_state = e->arg.i3.y < (int)this->bh;
			this->inc_button_state = e->arg.i3.y >
					((int)this->length - (int)this->bh);
		}

		sgui_widget_get_absolute_rect(super, &r);
		sgui_canvas_add_dirty_rect(super->canvas, &r);

		if (this->inc_button_state) {
			scroll_down(this);
		} else if (this->dec_button_state) {
			scroll_up(this);
		}
		break;
	}

	if (this->v_offset != old) {
		if (this->scroll_fun)
			this->scroll_fun(this->userptr, this->v_offset,
					this->v_offset-old);

		sgui_widget_get_absolute_rect(super, &r);
		sgui_canvas_add_dirty_rect(super->canvas, &r);
	}
	sgui_internal_unlock_mutex();
}

static void scroll_bar_draw(sgui_widget *super)
{
	sgui_scroll_bar *this = (sgui_scroll_bar *)super;
	sgui_skin *skin = sgui_skin_get();

	skin->draw_scroll_bar(skin, super->canvas,
				super->area.left, super->area.top,
				this->length, !this->horizontal,
				this->p_offset, this->p_length,
				this->inc_button_state,
				this->dec_button_state);
}

sgui_widget *sgui_scroll_bar_create(int x, int y, int horizontal,
					unsigned int length,
					unsigned int scroll_area_length,
					unsigned int disp_area_length)
{
	sgui_scroll_bar *this = calloc(1, sizeof(*this));
	sgui_widget *super = (sgui_widget *)this;
	sgui_skin *skin = sgui_skin_get();
	unsigned int w=0, h=0;
	sgui_rect r;

	if (!this)
		return NULL;

	skin->get_scroll_bar_button_extents(skin, &r);

	this->bw = SGUI_RECT_WIDTH(r);
	this->bh = SGUI_RECT_HEIGHT(r);

	if (horizontal) {
		w = length;
		h = skin->get_scroll_bar_width(skin);
	} else {
		w = skin->get_scroll_bar_width(skin);
		h = length;
	}

	sgui_widget_init( super, x, y, w, h );

	super->window_event = scroll_bar_on_event;
	super->draw = scroll_bar_draw;
	super->destroy = (void(*)(sgui_widget*))free;
	super->flags = SGUI_WIDGET_VISIBLE;
	this->horizontal = horizontal;
	this->length = length;
	this->v_length = disp_area_length;
	this->v_max = scroll_area_length;
	this->p_length = ((this->v_length << 8) / this->v_max) *
			(length - 2 * (horizontal ? this->bw : this->bh));

	this->p_length >>= 8;
	return super;
}

void sgui_scroll_bar_on_scroll(sgui_widget *super,
				sgui_scrollbar_callback fun, void *userptr)
{
	sgui_scroll_bar *this = (sgui_scroll_bar *)super;

	this->scroll_fun = fun;
	this->userptr = userptr;
}

void sgui_scroll_bar_set_offset( sgui_widget *super, unsigned int offset)
{
	sgui_scroll_bar *this = (sgui_scroll_bar *)super;
	unsigned int l;
	sgui_rect r;

	sgui_internal_lock_mutex();

	l = this->length - 2 * (this->horizontal ? this->bh : this->bw);

	if ((offset + this->v_length) < this->v_max) {
		this->v_offset = offset;
		this->p_offset = (((offset << 8) / this->v_max) * l) >> 8;
	} else {
		this->v_offset = this->v_max - this->v_length;
		this->p_offset = l - this->p_length;
	}

	if (super->canvas) {
		sgui_widget_get_absolute_rect(super, &r);
		sgui_canvas_add_dirty_rect(super->canvas, &r);
	}
	sgui_internal_unlock_mutex();
}

unsigned int sgui_scroll_bar_get_offset(sgui_widget *this)
{
	return ((sgui_scroll_bar *)this)->v_offset;
}

void sgui_scroll_bar_set_area(sgui_widget *super,
				unsigned int scroll_area_length,
				unsigned int disp_area_length)
{
	sgui_scroll_bar *this = (sgui_scroll_bar *)super;
	unsigned int bar_length;
	sgui_rect r;

	sgui_internal_lock_mutex();

	bar_length = this->length;
	bar_length -= 2 * (this->horizontal ? this->bw : this->bh);

	this->v_length = disp_area_length;
	this->v_max = scroll_area_length;
	this->p_length = ((this->v_length << 8) / this->v_max) * bar_length;

	this->p_length >>= 8;

	if (super->canvas) {
		sgui_widget_get_absolute_rect(super, &r);
		sgui_canvas_add_dirty_rect(super->canvas, &r);
	}
	sgui_internal_unlock_mutex();
}

void sgui_scroll_bar_set_length(sgui_widget *super, unsigned int length)
{
	sgui_scroll_bar *this = (sgui_scroll_bar *)super;
	unsigned int bar_length;
	sgui_rect r;

	sgui_internal_lock_mutex();
	if (length == this->length)
		goto out;

	if (this->length < length && super->canvas) {
		sgui_widget_get_absolute_rect(super, &r);
		sgui_canvas_add_dirty_rect(super->canvas, &r);
	}

	bar_length = length - 2 * (this->horizontal ? this->bw : this->bh);

	this->length = length;
	this->p_length = ((this->v_length << 8) / this->v_max) * bar_length;

	this->p_length >>= 8;

	if (this->horizontal) {
		super->area.right = super->area.left + length;
	} else {
		super->area.bottom = super->area.top + length;
	}

	if (this->length > length && super->canvas) {
		sgui_widget_get_absolute_rect(super, &r);
		sgui_canvas_add_dirty_rect(super->canvas, &r);
	}
out:
	sgui_internal_unlock_mutex();
}
