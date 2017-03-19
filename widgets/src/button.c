/*
 * button.c
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
#include "sgui_button.h"
#include "sgui_skin.h"
#include "sgui_event.h"
#include "sgui_canvas.h"
#include "sgui_font.h"
#include "sgui_internal.h"
#include "sgui_widget.h"
#include "sgui_utf8.h"
#include "sgui_icon_cache.h"

#include <stdlib.h>
#include <string.h>

typedef struct sgui_button {
	sgui_widget super;

	union {
		char *text;

#ifndef SGUI_NO_ICON_CACHE
		struct {
			sgui_icon_cache *cache;
			sgui_icon *i;
		} icon;
#endif
	} dpy;

	unsigned int type : 3;
	unsigned int selected : 1;
	unsigned int have_icon : 1;

	/* button: text or icon offset  */
	unsigned int cx, cy;

	struct sgui_button *prev; /* radio button menu: next in menu */
	struct sgui_button *next; /* radio button menu: previous in menu */
} sgui_button;

static void button_toggle(sgui_button *this, int postevent)
{
	sgui_canvas *cv = ((sgui_widget *)this)->canvas;
	sgui_event ev;
	sgui_rect r;

	this->selected = !this->selected;

	if (cv) {
		sgui_widget_get_absolute_rect((sgui_widget *)this, &r);
		sgui_canvas_add_dirty_rect(cv, &r);
	}

	if (postevent) {
		ev.src.widget = (sgui_widget *)this;
		ev.type = this->selected ? SGUI_BUTTON_IN_EVENT :
						SGUI_BUTTON_OUT_EVENT;
		sgui_event_post(&ev);
	}
}

static void button_select(sgui_button *this, int selected, int postevent)
{
	sgui_button *i;

	sgui_internal_lock_mutex();
	if (selected && this->selected)
		goto done;

	if (!selected && !this->selected)
		goto done;

	button_toggle(this, postevent);

	for (i = this->prev; i != NULL; i = i->prev) {
		if (i->selected) {
			button_toggle(i, 0);
			goto done;
		}
	}

	for (i = this->next; i != NULL; i = i->next) {
		if (i->selected) {
			button_toggle(i, 0);
			goto done;
		}
	}
done:
	sgui_internal_unlock_mutex();
}

static void button_draw(sgui_widget *super)
{
	sgui_button *this = (sgui_button *)super;
	int in = (this->selected ? 1 : 0), x, y;
	sgui_skin* skin = sgui_skin_get();

	skin->draw_button(skin, super->canvas, &super->area, this->type, in);

	x = super->area.left + this->cx;
	y = super->area.top  + this->cy;

	if (this->type == SGUI_BUTTON || this->type == SGUI_TOGGLE_BUTTON) {
		x += in;
		y += in;
	}

	if (this->have_icon) {
#ifndef SGUI_NO_ICON_CACHE
		sgui_icon_cache_draw_icon(this->dpy.icon.cache,
					this->dpy.icon.i, x, y);
#endif
	} else {
		sgui_skin_draw_text(super->canvas, x, y, this->dpy.text);
	}
}

static void toggle_button_on_event(sgui_widget *super, const sgui_event *e)
{
	sgui_button *this = (sgui_button *)super, *select = NULL;

	if (e->type == SGUI_MOUSE_PRESS_EVENT) {
		if (!this->prev && !this->next)
			goto out_toggle;
		select = this;
	} else if (e->type == SGUI_KEY_PRESSED_EVENT) {
		switch (e->arg.i) {
		case SGUI_KC_UP:
		case SGUI_KC_LEFT:
			if (this->selected)
				select = this->prev;
			break;
		case SGUI_KC_DOWN:
		case SGUI_KC_RIGHT:
			if (this->selected)
				select = this->next;
			break;
		case SGUI_KC_RETURN:
		case SGUI_KC_SPACE:
			if (!this->prev && !this->next)
				goto out_toggle;
			if (!this->selected)
				select = this;
			break;
		}
	}

	if (select) {
		sgui_canvas_set_focus(((sgui_widget *)select)->canvas,
					(sgui_widget *)select);
		button_select(select, 1, 1);
	}
	return;
out_toggle:
	button_toggle(this, 1);
}

static void button_on_event(sgui_widget *super, const sgui_event *e)
{
	sgui_button *this = (sgui_button *)super;

	switch (e->type) {
	case SGUI_MOUSE_LEAVE_EVENT:
		button_select(this, 0, 0);
		break;
	case SGUI_KEY_RELEASED_EVENT:
		if (e->arg.i != SGUI_KC_RETURN && e->arg.i != SGUI_KC_SPACE)
			break;
	case SGUI_MOUSE_RELEASE_EVENT:
		button_select(this, 0, 1);
		break;
	case SGUI_KEY_PRESSED_EVENT:
		if (e->arg.i != SGUI_KC_RETURN && e->arg.i != SGUI_KC_SPACE)
			break;
	case SGUI_MOUSE_PRESS_EVENT:
		button_select(this, 1, 0);
		break;
	}
}

static void button_destroy(sgui_widget *super)
{
	sgui_button *this = (sgui_button *)super;

	sgui_internal_lock_mutex();
	if (this->prev)
		this->prev->next = this->next;
	if (this->next)
		this->next->prev = this->prev;
	sgui_internal_unlock_mutex();

	if (!this->have_icon)
		free(this->dpy.text);

	free(this);
}

static void determine_shape(sgui_button *this, int type, unsigned int *width,
			unsigned int *height, unsigned int text_w,
			unsigned int text_h)
{
	sgui_skin *skin = sgui_skin_get();
	sgui_rect r;

	skin->get_button_extents(skin, type, &r);

	if (type == SGUI_BUTTON || type == SGUI_TOGGLE_BUTTON) {
		if (!*width)
			*width = SGUI_RECT_WIDTH(r) + text_w;
		if (!*height)
			*height = SGUI_RECT_HEIGHT(r) + text_h;

		this->cx = (*width - text_w) / 2;
		this->cy = (*height - text_h) / 2;
	} else {
		this->cx = SGUI_RECT_WIDTH(r);
		this->cy = SGUI_RECT_HEIGHT(r);

		*width = this->cx + text_w;

		if (this->cy > text_h) {
			*height = this->cy;
			this->cy = (this->cy - text_h) / 2;
		} else {
			*height = text_h;
			this->cy = 0;
		}
	}
}

static void update_shape(sgui_button *this, const sgui_rect *contents)
{
	unsigned int width, height, text_w, text_h;
	sgui_widget *super = (sgui_widget *)this;

	width = SGUI_RECT_WIDTH(super->area);
	height = SGUI_RECT_HEIGHT(super->area);

	text_w = SGUI_RECT_WIDTH_V(contents);
	text_h = SGUI_RECT_WIDTH_V(contents);

	determine_shape(this, this->type, &width, &height, text_w, text_h);

	sgui_rect_set_size(&super->area, super->area.left, super->area.top,
					width, height);
}

static sgui_widget *button_create_common(int x, int y, unsigned int width,
					unsigned int height, sgui_icon *icon,
					sgui_icon_cache *cache,
					const char *text, int type,
					int have_icon)
{
	sgui_button *this = calloc(1, sizeof(*this));
	sgui_widget *super = (sgui_widget *)this;
	unsigned int text_width, text_height;
	sgui_rect r;

	if (!this)
		return NULL;

	if (have_icon) {
#ifndef SGUI_NO_ICON_CACHE
		this->dpy.icon.cache = cache;
		this->dpy.icon.i = icon;
		sgui_icon_get_area(icon, &r);
#else
		(void)icon; (void)cache;
		memset(&r, 0, sizeof(r));
#endif
	} else {
		if (!(this->dpy.text = sgui_strdup(text))) {
			free(this);
			return NULL;
		}
		sgui_skin_get_text_extents(text, &r);
	}

	text_width = SGUI_RECT_WIDTH(r);
	text_height = SGUI_RECT_HEIGHT(r);

	determine_shape(this, type, &width, &height, text_width, text_height);

	sgui_widget_init(super, x, y, width, height);
	this->type = type;
	this->have_icon = have_icon;

	if (type == SGUI_BUTTON) {
		this->super.window_event = button_on_event;
	} else {
		this->super.window_event = toggle_button_on_event;
	}

	super->draw = button_draw;
	super->destroy = button_destroy;
	return (sgui_widget *)this;
}

/***************************************************************************/
sgui_widget *sgui_icon_button_create(int x, int y, unsigned int width,
				unsigned int height, sgui_icon_cache *cache,
				sgui_icon *icon, int type)
{
	return button_create_common(x, y, width, height, icon, cache, NULL,
					type, 1);
}

sgui_widget* sgui_button_create(int x, int y, unsigned int width,
				unsigned int height, const char *text,
				int type)
{
	return button_create_common(x, y, width, height, 0, NULL, text,
					type, 0);
}

void sgui_button_group_connect(sgui_widget *super, sgui_widget *previous,
				sgui_widget *next)
{
	sgui_button *this = (sgui_button *)super;

	sgui_internal_lock_mutex();
	if (this->prev)
		this->prev->next = this->next;
	if (this->next)
		this->next->prev = this->prev;

	this->prev = (sgui_button *)previous;
	this->next = (sgui_button *)next;

	if (this->prev)
		this->prev->next = this;
	if (this->next)
		this->next->prev = this;

	sgui_internal_unlock_mutex();
}

void sgui_button_set_state(sgui_widget *this, int state)
{
	if (((sgui_button *)this)->type != SGUI_BUTTON)
		button_select((sgui_button *)this, state, 0);
}

int sgui_button_get_state(sgui_widget *this)
{
	return ((sgui_button *)this)->selected;
}

void sgui_button_set_text(sgui_widget *super, const char *text)
{
	sgui_button *this = (sgui_button *)super;
	sgui_rect r;

	sgui_skin_get_text_extents(text, &r);

	sgui_internal_lock_mutex();

	if (!this->have_icon)
		free(this->dpy.text);

	this->have_icon = 0;
	this->dpy.text = sgui_strdup(text);

	update_shape(this, &r);
	sgui_internal_unlock_mutex();
}
#ifndef SGUI_NO_ICON_CACHE
void sgui_button_set_icon(sgui_widget *super, sgui_icon_cache *cache,
			sgui_icon *icon)
{
	sgui_button *this = (sgui_button *)super;
	sgui_rect r;

	sgui_icon_get_area(icon, &r);
	sgui_internal_lock_mutex();

	if (!this->have_icon)
		free(this->dpy.text);

	this->have_icon = 1;
	this->dpy.icon.cache = cache;
	this->dpy.icon.i = icon;

	update_shape(this, &r);
	sgui_internal_unlock_mutex();
}
#elif defined(SGUI_NOP_IMPLEMENTATIONS)
void sgui_button_set_icon(sgui_widget *super, sgui_icon_cache *cache,
			sgui_icon *icon)
{
	(void)super; (void)cache; (void)icon;
}
#endif
