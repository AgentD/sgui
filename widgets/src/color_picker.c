/*
 * color_picker.c
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
#include "sgui_color_picker.h"
#include "sgui_internal.h"
#include "sgui_widget.h"
#include "sgui_canvas.h"
#include "sgui_pixmap.h"
#include "sgui_event.h"

#include <stdlib.h>
#include <string.h>



#if !defined(SGUI_NO_COLOR_PICKER) || !defined(SGUI_NO_COLOR_DIALOG)
#define IMAGE_W 256
#define IMAGE_H 256
#define DELTA_H 1
#define DELTA_S 1
#define DELTA_V 1
#define DELTA_A 1

#define BAR_W 20

#define DISP_W 50
#define DISP_H 30
#define DISP_GAP 10
#define DISP_GAP_H 10

#define CHANGED_NONE 0
#define CHANGED_HS 1
#define CHANGED_V 2
#define CHANGED_A 3



typedef struct {
	sgui_widget super;
	unsigned char *hsdata;		/* static hue-saturation picker image*/
	unsigned char *vbardata;	/* dynamic bar slider image */
	unsigned char *abardata;	/* dynamic alpha slider image */
	sgui_pixmap *hs;		/* static hue-saturation picker */
	sgui_pixmap *vbar;		/* dynamic value slider */
	sgui_pixmap *abar;		/* dynamic alpha slider */

	int last_changed;
	unsigned char hsva[4];		/* currently set color */
} sgui_color_picker;



/* source: http://web.mit.edu/storborg/Public/hsvtorgb.c */
static sgui_color hsva_to_rgba(int h, int s, int v, int a)
{
	int region, fpart, p, q, t;
	sgui_color col;

	/* grayscale */
	if (s == 0) {
		col.c.r = col.c.g = col.c.b = v;
		goto out;
	}

	region = h / 43;		/* circle sector index (0-5) */
	fpart = (h - (region * 43)) * 6;/* fraction scaled to range 0-255 */

	p = (v * (255 - s)) >> 8;
	q = (v * (255 - ((s * fpart) >> 8))) >> 8;
	t = (v * (255 - ((s * (255 - fpart)) >> 8))) >> 8;

	switch (region) {
	case 0:  col.c.r = v; col.c.g = t; col.c.b = p; break;
	case 1:  col.c.r = q; col.c.g = v; col.c.b = p; break;
	case 2:  col.c.r = p; col.c.g = v; col.c.b = t; break;
	case 3:  col.c.r = p; col.c.g = q; col.c.b = v; break;
	case 4:  col.c.r = t; col.c.g = p; col.c.b = v; break;
	default: col.c.r = v; col.c.g = p; col.c.b = q; break;
	}
out:
	col.c.a = a;
	return col;
}

static void generate_v_bar(sgui_color_picker *this)
{
	unsigned char *ptr = this->vbardata;
	sgui_color rgba;
	int x, y, v;

	for (v = 0xFF, y = 0; y < IMAGE_H; ++y, v -= DELTA_V) {
		rgba = hsva_to_rgba(this->hsva[0], this->hsva[1], v, 0xFF);

		for (x = 0; x < BAR_W; ++x, ptr += 3) {
			ptr[0] = rgba.c.r;
			ptr[1] = rgba.c.g;
			ptr[2] = rgba.c.b;
		}
	}
}

static void generate_a_bar(sgui_color_picker *this)
{
	unsigned char *ptr = this->abardata;
	sgui_color rgba;
	int x, y, a, c;

	rgba = hsva_to_rgba(this->hsva[0], this->hsva[1],
				this->hsva[2], this->hsva[3]);

	for (a = 0xFF, y = 0; y < IMAGE_H; ++y, a -= DELTA_A) {
		for (x = 0; x < BAR_W; ++x, ptr += 3) {
			c = (((x & 4) == 0) ^ ((y & 4) == 0)) * 0x80 + 0x80;

			ptr[0] = (c * (0xFF - a) + rgba.c.r * a) >> 8;
			ptr[1] = (c * (0xFF - a) + rgba.c.g * a) >> 8;
			ptr[2] = (c * (0xFF - a) + rgba.c.b * a) >> 8;
		}
	}
}

static void color_picker_draw(sgui_widget *super)
{
	sgui_color_picker *this = (sgui_color_picker *)super;
	sgui_color black = { .c = { 0x00, 0x00, 0x00, 0xFF } };
	sgui_color white = { .c = { 0xFF, 0xFF, 0xFF, 0xFF } };
	sgui_color gray = { .c = { 0x80, 0x80, 0x80, 0xFF } };
	sgui_color rgba;
	int x, y, x1 ,y1;
	sgui_rect r, r0;

	if (!this->hs || !this->vbar || !this->abar)
		return;

	/* background pixmaps */
	sgui_canvas_draw_pixmap(super->canvas, super->area.left,
				super->area.top, this->hs,
				NULL, SGUI_CANVAS_BLEND);

	sgui_canvas_draw_pixmap(super->canvas,
				super->area.left + IMAGE_W + BAR_W / 2,
				super->area.top, this->vbar,
				NULL, SGUI_CANVAS_BLEND);

	sgui_canvas_draw_pixmap(super->canvas,
				super->area.left + IMAGE_W + 2 * BAR_W,
				super->area.top, this->abar,
				NULL, SGUI_CANVAS_BLEND);

	/* cross hair */
	sgui_canvas_draw_line(super->canvas,
			super->area.left + this->hsva[0] / DELTA_H - 5,
			super->area.top + (0xFF - this->hsva[1]) / DELTA_S,
			11, 1, black, 0);

	sgui_canvas_draw_line(super->canvas,
			super->area.left + this->hsva[0] / DELTA_H,
			super->area.top + (0xFF - this->hsva[1]) / DELTA_S - 5,
			11, 0, black, 0);

	/* value selector */
	sgui_canvas_draw_line(super->canvas,
			super->area.left + IMAGE_W + BAR_W / 2 - BAR_W / 4,
			super->area.top + (0xFF - this->hsva[2]) / DELTA_V,
			BAR_W + BAR_W / 2, 1, black, 0);

	/* alpha selector */
	sgui_canvas_draw_line(super->canvas,
			super->area.left + IMAGE_W + 2 * BAR_W - BAR_W / 4,
			super->area.top + (0xFF - this->hsva[3]) / DELTA_A,
			BAR_W + BAR_W / 2, 1, black, 0);

	/* color display */
	rgba = hsva_to_rgba(this->hsva[0], this->hsva[1],
				this->hsva[2], this->hsva[3]);

	r.left = super->area.left;
	r.right = super->area.left + DISP_W - 1;
	r.top = super->area.top + IMAGE_H + DISP_GAP;
	r.bottom = super->area.bottom;
	sgui_canvas_draw_box(super->canvas, &r, rgba, 0);

	white.c.r = (white.c.r * (0xFF - rgba.c.a) + rgba.c.r * rgba.c.a) >> 8;
	white.c.g = (white.c.g * (0xFF - rgba.c.a) + rgba.c.g * rgba.c.a) >> 8;
	white.c.b = (white.c.b * (0xFF - rgba.c.a) + rgba.c.b * rgba.c.a) >> 8;

	gray.c.r = (gray.c.r * (0xFF - rgba.c.a) + rgba.c.r * rgba.c.a) >> 8;
	gray.c.g = (gray.c.g * (0xFF - rgba.c.a) + rgba.c.g * rgba.c.a) >> 8;
	gray.c.b = (gray.c.b * (0xFF - rgba.c.a) + rgba.c.b * rgba.c.a) >> 8;

	r.left += DISP_W + DISP_GAP_H;
	r.right += DISP_W + DISP_GAP_H;
	sgui_canvas_draw_box(super->canvas, &r, white, 0);

	for (y = 0; y < DISP_H; y += 4) {
		y1 = (y + 3) >= DISP_H ? (DISP_H - 1) : (y + 3);

		r0.top = r.top + y;
		r0.bottom = r.top + y1;

		for (x = 0; x < DISP_W; x += 4) {
			x1 = (x + 3) >= DISP_W ? (DISP_W - 1) : (x + 3);

			r0.left = r.left + x;
			r0.right = r.left + x1;

			if (((x & 4) == 0) ^ ((y & 4) == 0)) {
				sgui_canvas_draw_box(super->canvas, &r0,
							gray, 0);
			}
		}
	}
}

static void color_picker_destroy(sgui_widget *super)
{
	sgui_color_picker *this = (sgui_color_picker *)super;

	free(this->vbardata);
	free(this->abardata);
	free(this->hsdata);
	if (this->hs)
		sgui_pixmap_destroy(this->hs);
	if (this->vbar)
		sgui_pixmap_destroy(this->vbar);
	if (this->abar)
		sgui_pixmap_destroy(this->abar);
	free(this);
}

static void color_picker_on_event(sgui_widget *super, const sgui_event *e)
{
	sgui_color_picker *this = (sgui_color_picker *)super;
	unsigned char hsva[4];
	int fire_event = 0;
	sgui_color col;
	sgui_event ev;

	sgui_internal_lock_mutex();
	memcpy(hsva, this->hsva, 4);

	switch (e->type) {
	case SGUI_KEY_PRESSED_EVENT:
		if (this->last_changed == CHANGED_HS) {
			if (e->arg.i == SGUI_KC_UP && hsva[1] < 0xFF) {
				++hsva[1];
				fire_event = 1;
			} else if (e->arg.i == SGUI_KC_DOWN && hsva[1] > 0) {
				--hsva[1];
				fire_event = 1;
			} else if (e->arg.i == SGUI_KC_LEFT && hsva[0] > 0) {
				--hsva[0];
				fire_event = 1;
			} else if (e->arg.i == SGUI_KC_RIGHT &&
					hsva[0] < 0xFF) {
				++hsva[0];
				fire_event = 1;
			}
		} else if (this->last_changed == CHANGED_V) {
			if (e->arg.i == SGUI_KC_UP && hsva[2] < 0xFF) {
				++hsva[2];
				fire_event = 1;
			} else if (e->arg.i == SGUI_KC_DOWN && hsva[2] > 0) {
				--hsva[2];
				fire_event = 1;
			}
		} else if (this->last_changed == CHANGED_A) {
			if (e->arg.i == SGUI_KC_UP && hsva[3] < 0xFF) {
				++hsva[3];
				fire_event = 1;
			} else if (e->arg.i == SGUI_KC_DOWN && hsva[3] > 0) {
				--hsva[3];
				fire_event = 1;
			}
		}
		break;
	case SGUI_MOUSE_PRESS_EVENT:
	case SGUI_MOUSE_RELEASE_EVENT:
		if (e->arg.i3.y >= IMAGE_H)
			break;

		/* hue-saturation selector */
		if (e->arg.i3.x < IMAGE_W) {
			hsva[0] = e->arg.i3.x * DELTA_H;
			hsva[1] = 0xFF - e->arg.i3.y * DELTA_S;
			fire_event = 1;
		}

		/* value slider */
		if (e->arg.i3.x >= (IMAGE_W + BAR_W / 2) &&
			e->arg.i3.x <= (IMAGE_W + 3 * BAR_W / 2)) {
			hsva[2] = 0xFF - e->arg.i3.y * DELTA_V;
			fire_event = 1;
		}

		/* alpha slider */
		if (e->arg.i3.x >= (IMAGE_W + 2 * BAR_W) &&
			e->arg.i3.x <= (IMAGE_W + 3 * BAR_W)) {
			hsva[3] = 0xFF - e->arg.i3.y * DELTA_A;
			fire_event = 1;
		}
		break;
	default:
		goto out;
	}

	sgui_color_picker_set_hsv(super, hsva);

	if (fire_event) {
		memcpy(ev.arg.color, this->hsva, 4);
		ev.type = SGUI_HSVA_CHANGED_EVENT;
		ev.src.widget = super;
		sgui_event_post(&ev);

		col = hsva_to_rgba(this->hsva[0], this->hsva[1],
					this->hsva[2], this->hsva[3]);

		ev.type = SGUI_RGBA_CHANGED_EVENT;
		ev.src.widget = super;
		ev.arg.color[0] = col.c.r;
		ev.arg.color[1] = col.c.g;
		ev.arg.color[2] = col.c.b;
		ev.arg.color[3] = col.c.a;
		sgui_event_post(&ev);
	}
out:
	sgui_internal_unlock_mutex();
}

static void color_picker_on_state_change(sgui_widget *super, int change)
{
	sgui_color_picker *this = (sgui_color_picker *)super;
	unsigned int w, h;

	if (!(change & SGUI_WIDGET_CANVAS_CHANGED))
		return;

	sgui_internal_lock_mutex();
	sgui_widget_get_size(super, &w, &h);

	if (this->hs)
		sgui_pixmap_destroy(this->hs);
	if (this->vbar)
		sgui_pixmap_destroy(this->vbar);
	if (this->abar)
		sgui_pixmap_destroy(this->abar);

	this->hs = NULL;
	this->vbar = NULL;
	this->abar = NULL;

	if (!super->canvas)
		goto out;

	this->hs = sgui_canvas_create_pixmap(super->canvas, IMAGE_W, IMAGE_H,
						SGUI_RGB8);
	if (!this->hs)
		goto out;

	this->vbar = sgui_canvas_create_pixmap(super->canvas, BAR_W, IMAGE_H,
						SGUI_RGB8);
	if (!this->vbar) {
		sgui_pixmap_destroy(this->hs);
		this->hs = NULL;
		goto out;
	}

	this->abar = sgui_canvas_create_pixmap(super->canvas, BAR_W, IMAGE_H,
						SGUI_RGB8);
	if (!this->abar) {
		sgui_pixmap_destroy(this->hs);
		sgui_pixmap_destroy(this->vbar);
		this->hs = NULL;
		this->vbar = NULL;
		goto out;
	}

	/* upload data to new pixmaps */
	sgui_pixmap_load(this->hs, 0, 0, this->hsdata, 0, 0,
			IMAGE_W, IMAGE_H, IMAGE_W, SGUI_RGB8);

	sgui_pixmap_load(this->vbar, 0, 0, this->vbardata, 0, 0,
			BAR_W, IMAGE_H, BAR_W, SGUI_RGB8);

	sgui_pixmap_load(this->abar, 0, 0, this->abardata, 0, 0,
			BAR_W, IMAGE_H, BAR_W, SGUI_RGB8);
out:
	sgui_internal_unlock_mutex();
}

/****************************************************************************/

sgui_widget *sgui_color_picker_create(int x, int y)
{
	sgui_color_picker *this = calloc(1, sizeof(*this));
	sgui_widget *super = (sgui_widget *)this;
	unsigned char *ptr;
	sgui_color col;
	int i, j, h, s;

	if (!this)
		return NULL;

	sgui_widget_init(super, x, y, IMAGE_W + 3 * BAR_W + BAR_W / 4,
					IMAGE_H + DISP_H + DISP_GAP);

	/* allocate image data */
	if (!(this->hsdata = malloc(IMAGE_W * IMAGE_H * 3)))
		goto fail;

	if (!(this->vbardata = malloc(IMAGE_H * BAR_W * 3)))
		goto fail;

	if (!(this->abardata = malloc(IMAGE_H * BAR_W * 3)))
		goto fail;

	/* fill images */
	this->hsva[0] = 0x80;
	this->hsva[1] = 0x80;
	this->hsva[2] = 0x80;
	this->hsva[3] = 0x80;

	ptr = this->hsdata;

	for (s = 0xFF, i = 0; i < IMAGE_H; ++i, s -= DELTA_S) {
		for (h = 0, j = 0; j < IMAGE_W; ++j, ptr += 3, h += DELTA_H) {
			col = hsva_to_rgba(h, s, 0xFF, 0xFF);
			ptr[0] = col.c.r;
			ptr[1] = col.c.g;
			ptr[2] = col.c.b;
		}
	}

	generate_v_bar(this);
	generate_a_bar(this);

	super->draw = color_picker_draw;
	super->state_change_event = color_picker_on_state_change;
	super->destroy = color_picker_destroy;
	super->window_event = color_picker_on_event;
	super->flags = SGUI_FOCUS_ACCEPT | SGUI_FOCUS_DROP_ESC |
			SGUI_FOCUS_DROP_TAB | SGUI_WIDGET_VISIBLE;
	return super;
fail:
	free(this->vbardata);
	free(this->hsdata);
	free(this);
	return NULL;
}

void sgui_color_picker_set_hsv(sgui_widget *super,
				const unsigned char *hsva)
{
	sgui_color_picker *this = (sgui_color_picker *)super;
	unsigned char oldhsva[4];
	sgui_rect r;

	sgui_internal_lock_mutex();
	memcpy(oldhsva, this->hsva, 4);
	memcpy(this->hsva, hsva, 4);

	if (!memcmp(oldhsva, hsva, 4)) {
		sgui_internal_unlock_mutex();
		return;
	}

	/* regenerate slider images if neccessary */
	if (oldhsva[0] != hsva[0] || oldhsva[1] != hsva[1]) {
		generate_v_bar(this);

		if (this->vbar) {
			sgui_pixmap_load(this->vbar, 0, 0, this->vbardata, 0,
					0, BAR_W, IMAGE_H, BAR_W, SGUI_RGB8);
		}
	}

	if (oldhsva[0] != hsva[0] || oldhsva[1] != hsva[1] ||
		oldhsva[2] != hsva[2]) {
		generate_a_bar(this);

		if (this->abar) {
			sgui_pixmap_load(this->abar, 0, 0, this->abardata, 0,
					0, BAR_W, IMAGE_H, BAR_W, SGUI_RGB8);
		}
	}

	/* flag dirty */
	if (super->canvas) {
		sgui_widget_get_absolute_rect(super, &r);

		if (oldhsva[0] == hsva[0] && oldhsva[1] == hsva[1]) {
			r.left += IMAGE_W + BAR_W / 4;

			if (oldhsva[2] == hsva[2]) {
				r.left += BAR_W + BAR_W / 2;
				this->last_changed = CHANGED_A;
			} else {
				this->last_changed = CHANGED_V;
			}

			sgui_canvas_add_dirty_rect(super->canvas, &r);

			sgui_widget_get_absolute_rect(super, &r);
			r.right = r.left + 2 * DISP_W + DISP_GAP_H - 1;
			r.top = r.bottom - DISP_H - 1;
			sgui_canvas_add_dirty_rect(super->canvas, &r);
		} else {
			this->last_changed = CHANGED_HS;
			sgui_canvas_add_dirty_rect(super->canvas, &r);
		}
	}

	sgui_internal_unlock_mutex();
}

void sgui_color_picker_set_rgb(sgui_widget *super, const unsigned char *rgba)
{
	unsigned char hsva[4], min, max;
	int h = 0;

	min = MIN(MIN(rgba[0], rgba[1]), rgba[2]);
	max = MAX(MAX(rgba[0], rgba[1]), rgba[2]);

	if (max == min) {
		h = 0;
	} else if (max == rgba[0]) {
		h = (43 * (rgba[1] - rgba[2])) / (max - min);
	} else if (max == rgba[1]) {
		h = 86 + (43 * (rgba[2] - rgba[0])) / (max - min);
	} else if (max == rgba[2]) {
		h = 172 + (43 * (rgba[0] - rgba[1])) / (max - min);
	}

	hsva[0] = h < 0 ? (0xFF + h) : h;
	hsva[1] = max ? (((max - min) * 0xFF) / max) : 0;
	hsva[2] = max;
	hsva[3] = rgba[3];

	sgui_color_picker_set_hsv(super, hsva);
}

void sgui_color_picker_get_hsv(const sgui_widget *super, unsigned char *hsva)
{
	sgui_color_picker *this = (sgui_color_picker *)super;
	memcpy(hsva, this->hsva, 4);
}

void sgui_color_picker_get_rgb(const sgui_widget *super, unsigned char *rgba)
{
	sgui_color_picker *this = (sgui_color_picker *)super;
	sgui_color col;

	col = hsva_to_rgba(this->hsva[0], this->hsva[1],
				this->hsva[2], this->hsva[3]);

	rgba[0] = col.c.r;
	rgba[1] = col.c.g;
	rgba[2] = col.c.b;
	rgba[3] = col.c.a;
}
#elif defined(SGUI_NOP_IMPLEMENTATIONS)
sgui_widget *sgui_color_picker_create(int x, int y)
{
	(void)x; (void)y;
	return NULL;
}
void sgui_color_picker_set_hsv(sgui_widget *super, const unsigned char *hsva)
{
	(void)super; (void)hsva;
}
void sgui_color_picker_set_rgb(sgui_widget *super, const unsigned char *rgba)
{
	(void)super; (void)rgba;
}
void sgui_color_picker_get_hsv(const sgui_widget *super, unsigned char *hsva)
{
	(void)super; (void)hsva;
}
void sgui_color_picker_get_rgb(const sgui_widget *super, unsigned char *rgba)
{
	(void)super; (void)rgba;
}
#endif
