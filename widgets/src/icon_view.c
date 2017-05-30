/*
 * icon_view.c
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
#include "sgui_icon_cache.h"
#include "sgui_icon_view.h"
#include "sgui_internal.h"
#include "sgui_widget.h"
#include "sgui_event.h"
#include "sgui_model.h"
#include "sgui_skin.h"

#include <stdlib.h>
#include <string.h>

#if !defined(SGUI_NO_ICON_CACHE) && !defined(SGUI_NO_ICON_VIEW) &&\
    !defined(SGUI_NO_MODEL)
typedef struct {
	const sgui_item *item;
	sgui_rect icon_area;
	sgui_rect text_area;
	unsigned int selected : 1;
} icon;

typedef struct {
	sgui_widget super;
	sgui_model *model;
	sgui_widget *v_bar;	/* vertical scroll bar */
	icon *icons;		/* an array of icons */
	unsigned int num_icons;	/* number of icons */
	unsigned int v_bar_dist;/* distance of the vertical scroll bar */
	sgui_rect selection;	/* selection rect */
	unsigned int icon_col;
	unsigned int txt_col;
	int offset;		/* the offset from the border of the view */
	unsigned int multiselect : 1;
	unsigned int drag : 1;
	unsigned int selectbox : 1;
	unsigned int draw_bg : 1;

	const sgui_item *itemlist;
} icon_view;



#define SELECT(icon, state)\
	do {\
		(icon).selected = (state);\
		get_icon_bounding_box(this, &(icon), &r);\
		r.bottom -= offset;\
		r.top -= offset;\
		sgui_canvas_add_dirty_rect(this->super.canvas, &r);\
	} while (0)


static void draw_icon(icon_view *this, icon *i, sgui_skin *skin)
{
	sgui_rect r;

	sgui_icon_cache_draw_icon(sgui_model_get_icon_cache(this->model),
			sgui_item_icon(this->model, i->item, this->icon_col),
			i->icon_area.left, i->icon_area.top);

	if (i->selected) {
		r = sgui_item_text(this->model, i->item, this->txt_col) ?
					i->text_area : i->icon_area;

		skin->draw_focus_box(skin, this->super.canvas, &r);
	}

	sgui_skin_draw_text(this->super.canvas,
			i->text_area.left, i->text_area.top,
			sgui_item_text(this->model, i->item, this->txt_col));
}

static void ideal_grid_size(icon_view *this, unsigned int *grid_w,
				unsigned int *grid_h)
{
	unsigned int w, h, i;

	*grid_w = 0;
	*grid_h = 0;

	for (i = 0; i < this->num_icons; ++i) {
		w = SGUI_RECT_WIDTH(this->icons[i].icon_area);
		*grid_w = MAX(*grid_w, w);

		w = SGUI_RECT_WIDTH(this->icons[i].text_area);
		*grid_w = MAX(*grid_w, w);

		h = SGUI_RECT_HEIGHT(this->icons[i].icon_area) +
			SGUI_RECT_HEIGHT(this->icons[i].text_area);
		*grid_h = MAX(*grid_h, h);
	}

	*grid_w += *grid_w / 5;
	*grid_h += *grid_h / 10;
}

static void view_on_scroll_v(void *userptr, int new_offset, int delta)
{
	sgui_widget *this = userptr;
	sgui_rect r;
	(void)new_offset;
	(void)delta;

	if (this->canvas) {
		sgui_widget_get_absolute_rect(this, &r);
		sgui_canvas_add_dirty_rect(this->canvas, &r);
	}
}

static void gridify(icon_view *this)
{
	unsigned int x, y, grid_w, grid_h, total_w;
	unsigned int txt_w, txt_h, img_w, img_h, i;
	int dx, dy;

	ideal_grid_size(this, &grid_w, &grid_h);

	x = y = this->offset;
	total_w = SGUI_RECT_WIDTH(this->super.area) - 2 * this->offset;

	for (i = 0; i < this->num_icons; ++i) {
		if ((x + grid_w) >= total_w) {
			x = this->offset;
			y += grid_h;
		}

		txt_w = SGUI_RECT_WIDTH(this->icons[i].text_area);
		txt_h = SGUI_RECT_HEIGHT(this->icons[i].text_area);
		img_w = SGUI_RECT_WIDTH(this->icons[i].icon_area);
		img_h = SGUI_RECT_HEIGHT(this->icons[i].icon_area);

		dx = img_w < grid_w ? (grid_w - img_w) / 2 : 0;
		dy = (img_h + txt_h) < grid_h ?
			(grid_h - img_h - txt_h) / 2 : 0;

		sgui_rect_set_size(&this->icons[i].icon_area,
					x + dx, y + dy, img_w, img_h);

		dx = txt_w < grid_w ? (grid_w - txt_w) / 2 : 0;
		sgui_rect_set_size(&this->icons[i].text_area, x + dx,
				this->icons[i].icon_area.bottom, txt_w, txt_h);

		x += grid_w;
	}

	view_on_scroll_v(this, 0, 0);
}

static void get_icon_bounding_box(icon_view *this, icon *i, sgui_rect *r)
{
	int x, y;

	*r = i->text_area;
	sgui_rect_join(r, &i->icon_area, 0);
	sgui_widget_get_absolute_position(&this->super, &x, &y);
	sgui_rect_add_offset(r, x, y);
}

static void set_all_icons(icon_view *this, int state)
{
	unsigned int i;

	for (i = 0; i < this->num_icons; ++i)
		this->icons[i].selected = state;

	view_on_scroll_v(this, 0, 0);
}

static icon *icon_from_point(icon_view *this, int x, int y)
{
	icon *top = NULL, *ic = this->icons;
	unsigned int i;

	for (i = 0; i < this->num_icons; ++i, ++ic) {
		if (top && top->selected && !ic->selected)
			continue;

		if (sgui_rect_is_point_inside(&ic->icon_area, x, y) ||
			sgui_rect_is_point_inside(&ic->text_area, x, y)) {
			top = ic;
		}
	}
	return top;
}

static void event_for_each_selected(icon_view *this, int type)
{
	unsigned int i;
	sgui_event ev;

	for (i = 0; i < this->num_icons; ++i) {
		if (this->icons[i].selected) {
			ev.src.other = (void *)this->icons[i].item;
			ev.type = type;
			sgui_event_post(&ev);
		}
	}
}

static void update_scroll_area(icon_view *this)
{
	unsigned int i, length = 0, displength;
	sgui_rect r;

	if (this->num_icons) {
		r = this->icons[0].icon_area;
		sgui_rect_join(&r, &this->icons[0].text_area, 0);

		for (i = 1; i < this->num_icons; ++i) {
			sgui_rect_join(&r, &this->icons[i].icon_area, 0);
			sgui_rect_join(&r, &this->icons[i].text_area, 0);
		}

		r.top = 0;
		length = SGUI_RECT_HEIGHT(r) + 10;
	}

	displength = SGUI_RECT_HEIGHT(this->super.area);
	length = MAX(length, displength);

	sgui_scroll_bar_set_area(this->v_bar, length, displength);
	sgui_widget_set_visible(this->v_bar, length != displength);

	if (length == displength)
		sgui_scroll_bar_set_offset(this->v_bar, 0);
}

static void mark_selection_dirty(icon_view *this)
{
	sgui_widget *super = (sgui_widget *)this;
	int x, y, dy;
	sgui_rect r;

	r = this->selection;
	sgui_rect_repair(&r);

	dy = sgui_scroll_bar_get_offset(this->v_bar);
	r.top -= dy;
	r.bottom -= dy;

	sgui_widget_get_absolute_position(&this->super, &x, &y);
	sgui_rect_add_offset(&r, x, y);
	sgui_rect_extend(&r, 1, 1);
	sgui_canvas_add_dirty_rect(super->canvas, &r);
}

static void icon_view_on_key_release(icon_view *this, const sgui_event *e)
{
	int offset = sgui_scroll_bar_get_offset(this->v_bar);
	unsigned int i;
	sgui_event ev;
	sgui_rect r;
	icon *new;

	switch (e->arg.i) {
	case SGUI_KC_CONTROL:
	case SGUI_KC_LCONTROL:
	case SGUI_KC_RCONTROL:
		this->multiselect = 0;
		break;
	case SGUI_KC_HOME:
		this->multiselect = 0;
		set_all_icons(this, 0);
		this->icons[0].selected = 1;
		break;
	case SGUI_KC_END:
		this->multiselect = 0;
		set_all_icons(this, 0);
		this->icons[this->num_icons - 1].selected = 1;
		break;
	case SGUI_KC_LEFT:
	case SGUI_KC_UP:
		this->multiselect = 0;
		if (this->icons[0].selected) {
			for (i = 1; i < this->num_icons; ++i) {
				if (this->icons[i].selected)
					SELECT(this->icons[i], 0);
			}
		} else {
			new = NULL;
			for (i = 0; i < this->num_icons; ++i) {
				if (!new && (i + 1) < this->num_icons &&
					this->icons[i + 1].selected) {
					new = this->icons + i;
				}
				if (this->icons[i].selected)
					SELECT(this->icons[i], 0);
			}
			if (new)
				SELECT(*new, 1);
		}
		break;
	case SGUI_KC_RIGHT:
	case SGUI_KC_DOWN:
		this->multiselect = 0;
		for (new = NULL, i = 0; i < this->num_icons; ++i) {
			if (this->icons[i].selected &&
				(i + 1) < this->num_icons) {
				SELECT(this->icons[i], 0);
				new = this->icons + i + 1;
			}
		}
		if (new)
			SELECT(*new, 1);
		break;
	case SGUI_KC_RETURN:
	case SGUI_KC_SPACE:
		event_for_each_selected(this, SGUI_ICON_SELECTED_EVENT);
		break;
	case SGUI_KC_COPY:
		event_for_each_selected(this, SGUI_ICON_COPY_EVENT);
		break;
	case SGUI_KC_PASTE:
		ev.src.other = this;
		ev.type = SGUI_ICON_PASTE_EVENT;
		sgui_event_post(&ev);
		break;
	case SGUI_KC_CUT:
		event_for_each_selected(this, SGUI_ICON_CUT_EVENT);
		break;
	case SGUI_KC_DELETE:
		event_for_each_selected(this, SGUI_ICON_DELETE_EVENT);
		break;
	}
}

static void icon_view_drag(icon_view *this, int dx, int dy)
{
	sgui_widget *super = (sgui_widget *)this;
	int x, y, offset;
	sgui_rect r, r1;
	unsigned int i;

	offset = sgui_scroll_bar_get_offset(this->v_bar);

	/* clamp movement vector to keep icons inside area */
	y = SGUI_RECT_HEIGHT(super->area) - this->offset;

	for (i = 0; i < this->num_icons; ++i) {
		if (!this->icons[i].selected)
			continue;

		r = this->icons[i].icon_area;
		sgui_rect_join(&r,&this->icons[i].text_area,0);

		if ((r.left + dx) < this->offset)
			dx = this->offset - r.left;
		if ((r.top + dy) < this->offset)
			dy = this->offset - r.top;
		if ((r.right + dx) > (int)this->v_bar_dist)
			dx = this->v_bar_dist - r.right;
	}

	r1.left = r1.top = r1.right = r1.bottom = 0;

	for (i = 0; i < this->num_icons; ++i) {
		if (!this->icons[i].selected)
			continue;

		/* accumulate dirty rect */
		r = this->icons[i].text_area;
		sgui_rect_join(&r, &this->icons[i].icon_area, 0);

		if (r1.left == r1.right || r1.top == r1.bottom) {
			r1 = r;
		} else {
			sgui_rect_join(&r1, &r, 0);
		}

		sgui_rect_add_offset(&r, dx, dy);
		sgui_rect_join(&r1, &r, 0);

		/* move */
		sgui_rect_add_offset(&this->icons[i].icon_area, dx, dy);
		sgui_rect_add_offset(&this->icons[i].text_area, dx, dy);
	}

	if (r1.bottom != r1.top) {
		sgui_widget_get_absolute_position(super, &x, &y);
		sgui_rect_add_offset(&r1, x, y - offset);
		sgui_canvas_add_dirty_rect(super->canvas, &r1);
	}
}

static void update_selection(icon_view *this, const sgui_rect *r, int offset)
{
	sgui_widget *super = (sgui_widget *)this;
	icon *ic = this->icons;
	unsigned int i, hit;
	sgui_rect r1;

	for (i = 0; i < this->num_icons; ++i, ++ic) {
		hit = sgui_rect_get_intersection(0, r, &ic->icon_area) ||
			sgui_rect_get_intersection(0, r, &ic->text_area);

		if (hit ^ ic->selected) {
			ic->selected = hit;
			get_icon_bounding_box(this, ic, &r1);
			r1.top -= offset;
			r1.bottom -= offset;
			sgui_canvas_add_dirty_rect(super->canvas, &r1);
		}
	}
}

static void icon_view_on_event(sgui_widget *super, const sgui_event *e)
{
	icon_view *this = (icon_view *)super;
	int x, y, dx, dy, offset;
	sgui_rect r, r1;
	sgui_event ev;
	icon *new;

	sgui_internal_lock_mutex();
	if (!this->icons)
		goto out;

	switch (e->type) {
	case SGUI_MOUSE_PRESS_EVENT:
		if (e->arg.i3.z != SGUI_MOUSE_BUTTON_LEFT)
			break;

		offset = sgui_scroll_bar_get_offset(this->v_bar);

		x = e->arg.i3.x;
		y = e->arg.i3.y + offset;
		sgui_rect_set_size(&this->selection, x, y, 0, 0);
		new = icon_from_point(this, x, y);

		if (!(this->multiselect) && !(new && new->selected))
			set_all_icons(this, 0);

		if (new) {
			this->drag = 1;
			new->selected = !(new->selected && this->multiselect);
			get_icon_bounding_box(this, new, &r);
			r.top -= offset;
			r.bottom -= offset;
			sgui_canvas_add_dirty_rect(super->canvas, &r);
		} else if (!this->multiselect) {
			this->selectbox = 1;
			this->drag = 0;
		}
		break;
	case SGUI_DOUBLE_CLICK_EVENT:
		x = e->arg.i2.x;
		y = e->arg.i2.y + sgui_scroll_bar_get_offset(this->v_bar);
		set_all_icons(this, 0);
		this->selectbox = 0;
		new = icon_from_point(this, x, y);

		if (new) {
			new->selected = 1;
			ev.type = SGUI_ICON_SELECTED_EVENT;
			ev.src.other = (void*)new->item;
			sgui_event_post(&ev);
		}
		break;
	case SGUI_MOUSE_WHEEL_EVENT:
		dy = SGUI_RECT_HEIGHT(super->area) / 4;
		offset = sgui_scroll_bar_get_offset(this->v_bar);

		if (e->arg.i > 0) {
			dy = (dy > offset) ? offset : dy;
			sgui_scroll_bar_set_offset(this->v_bar, offset - dy);
			view_on_scroll_v(this, offset - dy, -dy);
		} else if (e->arg.i < 0) {
			sgui_scroll_bar_set_offset(this->v_bar, offset + dy);
			view_on_scroll_v(this, offset + dy, dy);
		}
		break;
	case SGUI_MOUSE_MOVE_EVENT:
		offset = sgui_scroll_bar_get_offset(this->v_bar);
		x = e->arg.i2.x;
		y = e->arg.i2.y + offset;

		if (this->selectbox) {
			/* get affected area */
			r1 = this->selection;
			sgui_rect_repair(&r1);

			this->selection.right = x;
			this->selection.bottom = y;

			r = this->selection;
			sgui_rect_repair(&r);
			sgui_rect_join(&r1, &r, 0);

			/* flag area as dity */
			sgui_widget_get_absolute_position(super, &x, &y);
			sgui_rect_add_offset(&r1, x, y);
			sgui_rect_extend(&r1, 1, 1);
			r1.top -= offset;
			r1.bottom -= offset;
			sgui_canvas_add_dirty_rect(super->canvas, &r1);

			/* get selected icons */
			update_selection(this, &r, offset);
		} else if (this->drag) {
			dx = x - this->selection.left;
			dy = y - this->selection.top;
			sgui_rect_set_size(&this->selection, x, y, 0, 0);

			icon_view_drag(this, dx, dy);
			update_scroll_area(this);
		}
		break;
	case SGUI_KEY_PRESSED_EVENT:
		switch (e->arg.i) {
		case SGUI_KC_CONTROL:
		case SGUI_KC_LCONTROL:
		case SGUI_KC_RCONTROL:
			this->multiselect = 1;
			break;
		case SGUI_KC_SELECT_ALL:
			set_all_icons(this, 1);
			break;
		}
		break;
	case SGUI_KEY_RELEASED_EVENT:
		icon_view_on_key_release(this, e);
		break;
	case SGUI_MOUSE_RELEASE_EVENT:
		if (e->arg.i3.z != SGUI_MOUSE_BUTTON_LEFT)
			break;
		/* falls through */
	case SGUI_MOUSE_LEAVE_EVENT:
		if (this->drag) {
			sgui_widget_get_absolute_rect(super, &r);
			sgui_canvas_add_dirty_rect(super->canvas, &r);
		} else if (this->selectbox) {
			mark_selection_dirty(this);
		}

		this->selectbox = 0;
		this->drag = 0;
		break;
	case SGUI_FOCUS_LOSE_EVENT:
		set_all_icons(this, 0);
		this->multiselect = 0;
		this->selectbox = 0;
		this->drag = 0;
		break;
	}
out:
	sgui_internal_unlock_mutex();
}

static void icon_view_draw(sgui_widget *super)
{
	icon_view *this = (icon_view *)super;
	sgui_skin *skin = sgui_skin_get();
	unsigned int i, offset;
	sgui_rect r, sc;
	int ox, oy;

	offset = sgui_scroll_bar_get_offset(this->v_bar);

	if (this->draw_bg)
		skin->draw_frame(skin, super->canvas, &super->area);

	if (this->selectbox) {
		r = this->selection;
		sgui_rect_repair(&r);
		r.top -= offset;
		r.bottom -= offset;
		sgui_rect_add_offset(&r, super->area.left, super->area.top);
		skin->draw_focus_box(skin, super->canvas, &r);
	}

	/* draw selected icons on top of non-selected */
	sgui_widget_get_absolute_rect(super, &r);
	sgui_canvas_get_offset(super->canvas, &ox, &oy);
	sgui_canvas_set_offset(super->canvas, r.left, r.top - offset);

	sgui_widget_get_absolute_rect(super, &r);
	sgui_rect_extend(&r, -this->offset, -this->offset);
	sgui_canvas_get_scissor_rect(super->canvas, &sc);
	sgui_rect_get_intersection(&r, &sc, &r);
	sgui_canvas_set_scissor_rect(super->canvas, &r);

	for (i = 0; i < this->num_icons; ++i) {
		if (!this->icons[i].selected)
			draw_icon(this, this->icons + i, skin);
	}

	for (i = 0; i < this->num_icons; ++i) {
		if (this->icons[i].selected)
			draw_icon(this, this->icons + i, skin);
	}

	sgui_canvas_set_offset(super->canvas, ox, oy);
	sgui_canvas_set_scissor_rect(super->canvas, &sc);
}

static void icon_view_destroy(sgui_widget *super)
{
	icon_view *this = (icon_view *)super;
	sgui_widget *i;

	for (i = super->children; i != NULL; i = i->next) {
		if (i->next == this->v_bar) {
			i->next = i->next->next;
			sgui_widget_destroy(this->v_bar);
			break;
		}
	}

	sgui_model_free_item_list(this->model, this->itemlist);
	free(this->icons);
	free(this);
}

/***************************************************************************/

sgui_widget *sgui_icon_view_create(int x, int y, unsigned width,
				unsigned int height, sgui_model *model,
				int background, unsigned int icon_col,
				unsigned int txt_col)
{
	sgui_skin *skin = sgui_skin_get();
	sgui_widget *super;
	icon_view *this;
	unsigned int w;
	sgui_rect r;

	if (!model)
		return NULL;

	this = calloc(1, sizeof(*this));
	super = (sgui_widget *)this;

	if (!this)
		return NULL;

	sgui_widget_init(super, x, y, width, height);

	this->model = model;
	this->draw_bg = background;
	this->offset = background ? skin->get_frame_border_width(skin) : 0;
	this->icon_col = icon_col;
	this->txt_col= txt_col;

	skin->get_scroll_bar_button_extents(skin, &r);
	w = SGUI_RECT_WIDTH(r);

	this->v_bar_dist = width - w - this->offset;
	this->v_bar = sgui_scroll_bar_create(this->v_bar_dist, this->offset, 0,
						height - 2 * this->offset,
						height - 2 * this->offset,
						height - 2 * this->offset);

	if (!this->v_bar) {
		free(this);
		return NULL;
	}

	sgui_scroll_bar_on_scroll(this->v_bar, view_on_scroll_v, this);
	sgui_widget_set_visible(this->v_bar, 0 );
	sgui_widget_add_child(super, this->v_bar);

	super->window_event = icon_view_on_event;
	super->draw = icon_view_draw;
	super->destroy = icon_view_destroy;
	super->flags = SGUI_FOCUS_ACCEPT|SGUI_FOCUS_DROP_ESC|
			SGUI_FOCUS_DROP_TAB|SGUI_WIDGET_VISIBLE;
	return super;
}

void sgui_icon_view_populate(sgui_widget *super, sgui_item *root)
{
	icon_view *this = (icon_view *)super;
	const sgui_item *i;
	const char *subtext;
	const sgui_icon *ic;
	unsigned int j;

	sgui_internal_lock_mutex();
	sgui_model_free_item_list(this->model, this->itemlist);
	this->itemlist = sgui_model_query_items(this->model, root, 0, 0);

	if (!this->itemlist)
		goto fail;

	this->num_icons = sgui_model_item_children_count(this->model, root);
	this->icons = realloc(this->icons, this->num_icons * sizeof(icon));

	if (!this->icons)
		goto fail;

	memset(this->icons, 0, this->num_icons * sizeof(icon));

	i = this->itemlist;
	for (j = 0; i && j < this->num_icons; i = i->next, ++j) {
		ic = sgui_item_icon(this->model, i, this->icon_col);
		subtext = sgui_item_text(this->model, i, this->txt_col);

		sgui_icon_get_area(ic, &this->icons[j].icon_area);
		sgui_skin_get_text_extents(subtext, &this->icons[j].text_area);

		this->icons[j].item = i;
	}

	gridify(this);
	update_scroll_area(this);
	sgui_internal_unlock_mutex();
	return;
fail:
	free(this->icons);
	sgui_model_free_item_list(this->model, this->itemlist);
	this->itemlist = NULL;
	this->num_icons = 0;
	sgui_internal_unlock_mutex();
}

void sgui_icon_view_snap_to_grid(sgui_widget *super)
{
	icon_view *this = (icon_view *)super;
	unsigned int i, grid_w, grid_h;
	sgui_rect r;
	int dx, dy;

	sgui_internal_lock_mutex();
	ideal_grid_size(this, &grid_w, &grid_h);

	for (i = 0; i < this->num_icons; ++i) {
		r = this->icons[i].icon_area;
		sgui_rect_join(&r, &this->icons[i].text_area, 0);
		dx = -((r.left - this->offset) % grid_w);
		dy = -((r.top  - this->offset) % grid_h);
		sgui_rect_add_offset(&this->icons[i].text_area, dx, dy);
		sgui_rect_add_offset(&this->icons[i].icon_area, dx, dy);
	}

	update_scroll_area(this);
	view_on_scroll_v(super, 0, 0);
	sgui_internal_unlock_mutex();
}
/****************************************************************************/
static int compare(icon_view *this, sgui_item_compare_fun fun,
			icon *a, icon *b)
{
	if (fun)
		return fun(this->model, a->item, b->item);

	return strcmp(sgui_item_text(this->model, a->item, this->txt_col),
			sgui_item_text(this->model, b->item, this->txt_col));
}

static void sink(icon_view *this, sgui_item_compare_fun fun,
		icon *pq, unsigned int k, unsigned int n)
{
	unsigned int j;
	icon temp;

	for (j = 2 * k; j <= n; k = j, j *= 2) {
		if (j < n && compare(this, fun, pq + j - 1, pq + j) < 0)
			++j;

		if (compare(this, fun, pq + k - 1, pq + j - 1) >= 0)
			break;

		temp = pq[k - 1];
		pq[k - 1] = pq[j - 1];
		pq[j - 1] = temp;
	}
}

void sgui_icon_view_sort(sgui_widget *super, sgui_item_compare_fun fun)
{
	icon_view *this = (icon_view *)super;
	unsigned int k, n;
	icon temp, *last;

	if (!this->icons)
		return;

	sgui_internal_lock_mutex();
	for (k = this->num_icons / 2; k>=1; --k)
		sink(this, fun, this->icons, k, this->num_icons);

	for (n = this->num_icons, last = this->icons+n-1; n > 1; --last) {
		temp = this->icons[0];
		this->icons[0] = *last;
		*last = temp;
		sink(this, fun, this->icons, 1, --n);
	}

	gridify(this);
	update_scroll_area(this);
	sgui_internal_unlock_mutex();
}
#elif defined(SGUI_NOP_IMPLEMENTATIONS)
sgui_widget* sgui_icon_view_create(int x, int y, unsigned width,
				unsigned int height, sgui_model *model,
				int bg, unsigned int icon_col,
				unsigned int txt_col)
{
	(void)x; (void)y; (void)width; (void)height; (void)model; (void)bg;
	(void)icon_col; (void)txt_col;
	return NULL;
}
void sgui_icon_view_sort(sgui_widget *super, sgui_item_compare_fun fun)
{
	(void)super; (void)fun;
}
void sgui_icon_view_populate(sgui_widget *view, sgui_item *root)
{
	(void)view; (void)root;
}
void sgui_icon_view_snap_to_grid(sgui_widget *super)
{
	(void)super;
}
#endif /* !defined(SGUI_NO_ICON_CACHE) && !defined(SGUI_NO_ICON_VIEW) */
