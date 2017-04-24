/*
 * subview.c
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
#include "sgui_subview.h"
#include "sgui_event.h"
#include "sgui_widget.h"
#include "sgui_internal.h"
#include "sgui_lib.h"

#include <stdlib.h>
#include <string.h>

#ifndef SGUI_NO_SUBVIEW
typedef struct {
	sgui_widget super;

	sgui_window *subwnd;

	sgui_subview_window_fun window_fun;
	sgui_subview_draw_fun draw_fun;
} sgui_subview;



/* events from the parent canvas containing a subview widget */
static void subview_on_parent_event(sgui_widget *super, const sgui_event *e)
{
	sgui_subview *this = (sgui_subview *)super;

	switch (e->type) {
	case SGUI_MOUSE_ENTER_EVENT:
	case SGUI_MOUSE_LEAVE_EVENT:
	case SGUI_FOCUS_EVENT:
	case SGUI_FOCUS_LOSE_EVENT:
		if (this->window_fun)
			this->window_fun(super, e);
		break;
	}
}

/* window system events from the child window */
static void subview_on_subwindow_event(sgui_subview *this,
					const sgui_event *e)
{
	switch (e->type) {
	case SGUI_USER_CLOSED_EVENT:	/* sub window -> not possible */
	case SGUI_API_DESTROY_EVENT:	/* caused by sgui_widget_destroy */
		break;
	case SGUI_SIZE_CHANGE_EVENT:
	case SGUI_EXPOSE_EVENT:
		if (this->draw_fun) {
			sgui_window_make_current(this->subwnd);
			this->draw_fun((sgui_widget *)this);
			sgui_window_swap_buffers(this->subwnd);
			sgui_window_release_current(this->subwnd);
		}
	default:
		if (this->window_fun)
			this->window_fun((sgui_widget *)this, e);
	}
}

static void subview_on_state_change(sgui_widget *super, int change)
{
	sgui_subview *this = (sgui_subview *)super;
	unsigned int ww, wh;
	int x, y, visible;

	if (change & SGUI_WIDGET_PARENT_CHANGED) {
		change |= SGUI_WIDGET_POSITION_CHANGED;
		change |= SGUI_WIDGET_VISIBILLITY_CHANGED;
	}

	if (change & SGUI_WIDGET_POSITION_CHANGED) {
		sgui_widget_get_absolute_position(super, &x, &y);
		sgui_widget_get_size(super, &ww, &wh);

		sgui_window_move(this->subwnd, x, y);
		sgui_window_set_size(this->subwnd, ww, wh);
    	}

	if (change & SGUI_WIDGET_VISIBILLITY_CHANGED) {
		visible = sgui_widget_is_absolute_visible(super);

		sgui_window_set_visible(this->subwnd, visible);
	}
}

static void subview_destroy(sgui_widget *this)
{
	sgui_window_destroy(((sgui_subview *)this)->subwnd);
	free(this);
}

/****************************************************************************/

sgui_widget *sgui_subview_create(sgui_window *parent, int x, int y,
				unsigned int width, unsigned int height,
				int backend,
				const sgui_window_description *cfg)
{
	sgui_subview *this = calloc(1, sizeof(*this));
	sgui_widget *super = (sgui_widget *)this;
	sgui_window_description desc;

	if (!this)
		return NULL;

	sgui_widget_init(super, x, y, width, height);
	super->window_event = subview_on_parent_event;
	super->state_change_event = subview_on_state_change;
	super->destroy = subview_destroy;
	super->flags = SGUI_FOCUS_ACCEPT | SGUI_FOCUS_DROP_ESC |
			SGUI_FOCUS_DROP_TAB | SGUI_WIDGET_VISIBLE;

	memset(&desc, 0, sizeof(desc));

	if (cfg) {
		desc = *cfg;
	} else {
		desc.width = width;
		desc.height = height;
		desc.bits_per_pixel = 32;
		desc.depth_bits = 16;
		desc.backend = backend;
	}

	desc.parent = parent;
	desc.flags |= SGUI_FIXED_SIZE;

	this->subwnd = parent->lib->create_window(parent->lib, &desc);

	if (!this->subwnd) {
		free(this);
		return NULL;
	}

	sgui_window_on_event(this->subwnd,
			(sgui_window_callback)subview_on_subwindow_event);
	sgui_window_set_userptr(this->subwnd, this);
	return super;
}

sgui_window *sgui_subview_get_window(sgui_widget *this)
{
	return ((sgui_subview *)this)->subwnd;
}

void sgui_subview_set_draw_callback(sgui_widget *this,
					sgui_subview_draw_fun drawcb)
{
	((sgui_subview *)this)->draw_fun = drawcb;
}

void sgui_subview_on_window_event(sgui_widget *this,
					sgui_subview_window_fun windowcb)
{
	((sgui_subview *)this)->window_fun = windowcb;
}

void sgui_subview_refresh(sgui_widget *this)
{
	sgui_rect r;

	sgui_rect_set_size(&r, 0, 0, SGUI_RECT_WIDTH(this->area),
					SGUI_RECT_HEIGHT(this->area));

	sgui_window_force_redraw(((sgui_subview *)this)->subwnd, &r);
}
#elif defined(SGUI_NOP_IMPLEMENTATIONS)
sgui_widget *sgui_subview_create(sgui_window *parent, int x, int y,
				unsigned int width, unsigned int height,
				int backend,
				const sgui_window_description *cfg)
{
	(void)parent; (void)x; (void)y; (void)width; (void)height;
	(void)backend; (void)cfg;
	return NULL;
}
sgui_window *sgui_subview_get_window(sgui_widget *this)
{
	(void)this;
	return NULL;
}
void sgui_subview_set_draw_callback(sgui_widget *this,
					sgui_subview_draw_fun drawcb)
{
	(void)this; (void)drawcb;
}
void sgui_subview_on_window_event(sgui_widget *this,
				sgui_subview_window_fun windowcb)
{
	(void)this; (void)windowcb;
}
void sgui_subview_refresh(sgui_widget *this)
{
	(void)this;
}
#endif /* SGUI_NO_SUBVIEW */
