/*
 * skin_default.c
 * This file is part of sgui
 *
 * Copyright (C) 2012 - David Oberhollenzer
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions
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
#include "sgui_skin.h"
#include "sgui_font.h"
#include "sgui_rect.h"
#include "sgui_pixmap.h"
#include "sgui_internal.h"

#include <stddef.h>



#define FONT "SourceSansPro-Regular.ttf"
#define FONT_ITAL "SourceSansPro-It.ttf"
#define FONT_BOLD "SourceSansPro-Semibold.ttf"
#define FONT_BOLD_ITAL "SourceSansPro-SemiboldIt.ttf"

#define FONT_ALT_PATH "font/"
#define FONT_ALT_PATH2 "../font/"

#define FONT_HEIGHT 16


static sgui_rect chkbox = { 0, 0, 11, 11 };
static sgui_rect chkbox_ticked = { 12, 0, 23, 11 };

static sgui_rect radio = { 0, 12, 11, 23 };
static sgui_rect radio_ticked = { 12, 12, 23, 23 };

static sgui_rect arrow_l = { 24, 0, 29, 10 };
static sgui_rect arrow_r = { 29, 0, 34, 10 };
static sgui_rect arrow_up = { 24, 0, 34, 5 };
static sgui_rect arrow_dn = { 24, 5, 34, 10 };

static sgui_rect spin_up = { 26, 0, 32, 3 };
static sgui_rect spin_dn = { 26, 7, 32, 10 };

#define PIXMAP_W 35
#define PIXMAP_H 24


/* COLORMAP 4 */
static const unsigned char colormap[4*4] = {
	0x00, 0x00, 0x00, 0x00,	/* completely transparent */
	0x00, 0x00, 0x00, 0xFF,	/* black */
	0xFF, 0xFF, 0xFF, 0xFF,	/* white */
	0x00, 0x00, 0x00, 0x80,	/* semi transparent black */
};

/* SIZE 35x24 */
static const unsigned char pixmap_data[266] = {
0x85,011,012,0x85,011,012,0x40,2,0x40,1,0x85,033,021,0x85,033,020,0,2,022,0x40,
013,0x84,033,032,013,0x83,033,023,032,0,2,0x52,0,1,0x85,033,021,0x83,033,022,
033,020,2,0x83,022,0,013,0x84,033,032,013,023,033,022,023,032,2,0x84,022,1,
0x85,033,021,032,023,022,023,033,0x86,022,013,0x84,033,032,013,0x52,023,033,
032,2,0x84,022,1,0x85,033,021,033,022,023,0x5B,020,2,0x83,022,0,013,0x84,033,
032,013,033,023,0x5B,032,0,2,0x52,0,1,0x85,033,021,0x85,033,020,0,2,022,0x40,
013,0x84,033,032,013,0x84,033,032,0x40,2,0x40,2,0x8B,022,020,0x87,0,0x49,0x84,
0,0x49,0x88,0,1,013,033,031,010,0,1,013,033,031,010,0x86,0,1,0x84,033,020,1,
0x84,033,020,0x86,0,013,0x83,033,032,0,013,032,022,023,032,0x86,0,013,0x84,
033,032,013,032,0x52,023,032,0x85,0,1,0x85,033,021,033,0x83,022,033,020,0x85,
0,013,0x84,033,032,013,032,0x52,023,032,0x85,0,1,0x85,033,021,033,0x83,022,
033,020,0x85,0,1,0x84,033,020,1,033,0x52,033,020,0x86,0,013,0x83,033,032,0,
013,0x83,033,032,0x87,0,022,0x5B,022,0x40,022,0x5B,022,0x88,0,2,022,020,0x83,
0,2,022,020,0x89,0,1,023,0x88,0
};


static int is_init = 0;
static const unsigned char black[4]       = { 0x00, 0x00, 0x00, 0xFF };
static const unsigned char white[4]       = { 0xFF, 0xFF, 0xFF, 0xFF };
static const unsigned char darkoverlay[4] = { 0x00, 0x00, 0x00, 0x80 };
static const unsigned char focusbox[4]    = { 0xFF, 0x80, 0x25, 0xFF };
static const unsigned char yellow[4]      = { 0xFF, 0xFF, 0x00, 0xFF };



static void default_get_checkbox_extents(sgui_skin *this, sgui_rect *r)
{
	(void)this;
	sgui_rect_set_size(r, 0, 0, 20, 12);
}

static unsigned int default_get_edit_box_height(sgui_skin *this)
{
	(void)this;
	return 30;
}

static unsigned int default_get_edit_box_border_width(sgui_skin *this)
{
	(void)this;
	return 2;
}

static unsigned int default_get_frame_border_width(sgui_skin *this)
{
	(void)this;
	return 1;
}

static unsigned int default_get_progess_bar_width(sgui_skin *this)
{
	(void)this;
	return 30;
}

static unsigned int default_get_scroll_bar_width(sgui_skin *this)
{
	(void)this;
	return 20;
}

static unsigned int default_get_focus_box_width(sgui_skin *skin)
{
	(void)skin;
	return 3;
}

static void default_get_scroll_bar_button_extents(sgui_skin *this,
						sgui_rect *r)
{
	(void)this;
	sgui_rect_set_size(r, 0, 0, 20, 20);
}

static void default_get_tap_caption_extents(sgui_skin *this, sgui_rect *r)
{
	(void)this;
	sgui_rect_set_size(r, 0, 0, 16, 25);
}

static void default_get_slider_extents(sgui_skin *this, sgui_rect *r,
					int vertical)
{
	(void)this;
	sgui_rect_set_size(r, 0, 0, vertical ? 20 : 10, vertical ? 10 : 20);
}

static void default_get_spin_buttons(sgui_skin *this, sgui_rect *up,
					sgui_rect *down)
{
	(void)this;
	sgui_rect_set_size(up, 5, 5, 15, 10);
	sgui_rect_set_size(down, 5, 15, 15, 10);
}

static void default_draw_checkbox(sgui_skin *this, sgui_canvas *canvas,
					int x, int y, int checked)
{
	const sgui_pixmap *skin_pixmap;
	sgui_rect *r;
	(void)this;

	r = checked ? &chkbox_ticked : &chkbox;
	skin_pixmap = canvas->get_skin_pixmap(canvas);
	sgui_canvas_draw_pixmap(canvas, x, y, skin_pixmap, r, 1);
}

static void default_draw_radio_button(sgui_skin *this, sgui_canvas *canvas,
					int x, int y, int checked)
{
	const sgui_pixmap *skin_pixmap;
	sgui_rect *r;
	(void)this;

	r = checked ? &radio_ticked : &radio;
	skin_pixmap = canvas->get_skin_pixmap(canvas);
	sgui_canvas_draw_pixmap(canvas, x, y, skin_pixmap, r, 1);
}

static void default_draw_button(sgui_skin *skin, sgui_canvas *canvas,
				sgui_rect *r, int pressed)
{
	int w = SGUI_RECT_WIDTH_V(r), h = SGUI_RECT_HEIGHT_V(r);
	int x = r->left, y = r->top;
	const unsigned char *lt, *rb;
	(void)skin;

	lt = pressed ? black : white;
	rb = pressed ? white : black;

	sgui_canvas_draw_line(canvas, x, y, w, 1, lt, SGUI_RGB8);
	sgui_canvas_draw_line(canvas, x, y, h, 0, lt, SGUI_RGB8);
	sgui_canvas_draw_line(canvas, x, y + h - 1, w, 1, rb, SGUI_RGB8);
	sgui_canvas_draw_line(canvas, x + w - 1, y, h, 0, rb, SGUI_RGB8);
}

static void default_draw_editbox(sgui_skin *this, sgui_canvas *canvas,
				sgui_rect *r, const char *text, int offset,
				int cursor, int selection, int numeric,
				int spinbuttons)
{
	unsigned char cur[4] = { 0x7F, 0x7F, 0x7F, 0xFF };
	unsigned char selcolor[4] = { 0xFF, 0x80, 0x25, 0xFF };
	int x = r->left, y = r->top;
	int w=SGUI_RECT_WIDTH_V(r), h = SGUI_RECT_HEIGHT_V(r);
	const sgui_pixmap *skin_pixmap;
	int cx, dx = 0;
	sgui_rect r0;
	(void)this;

	sgui_canvas_draw_box(canvas, r, darkoverlay, SGUI_RGBA8);
	text += offset;
	cursor -= offset;
	selection -= offset;

	if (numeric) {
		dx = sgui_skin_default_font_extents(text, -1, 0, 0);
		dx = w - 2 - dx;
	}

	/* draw selection */
	if (cursor >= 0 && cursor != selection) {
		r0.left = sgui_skin_default_font_extents(text, cursor, 0, 0);

		if (selection > 0) {
			r0.right = sgui_skin_default_font_extents(text,
							selection, 0, 0);
		} else {
			r0.right = 0;
		}

		if (r0.left > r0.right) {
			cx = r0.left;
			r0.left = r0.right;
			r0.right = cx;
		}

		r0.left += x + 2 + dx;
		r0.right += x + 2 + dx;
		r0.top = y + 2;
		r0.bottom = y + h - 3;
		sgui_canvas_draw_box(canvas, &r0, selcolor, SGUI_RGB8);
	}

	/* draw text */
	sgui_canvas_draw_text_plain(canvas, x + 2 + dx, y + 4, 0, 0,
						white, text, -1);

	/* draw cursor */
	if (cursor >= 0) {
		cx = sgui_skin_default_font_extents(text, cursor, 0, 0) + 2;
		sgui_canvas_draw_line(canvas, dx + x + cx, y + 5, h - 10, 0,
					cur, SGUI_RGB8);
	}

	/* draw borders */
	sgui_canvas_draw_line(canvas, x, y, w, 1, black, SGUI_RGB8);
	sgui_canvas_draw_line(canvas, x, y, h, 0, black, SGUI_RGB8);
	sgui_canvas_draw_line(canvas, x, y + h - 1, w, 1, white, SGUI_RGB8);
	sgui_canvas_draw_line(canvas, x + w - 1, y, h, 0, white, SGUI_RGB8);

	if (spinbuttons) {
		skin_pixmap = canvas->get_skin_pixmap(canvas);

		sgui_canvas_draw_pixmap(canvas, x + 9, y + 7,
						skin_pixmap, &spin_up, 1);
		sgui_canvas_draw_pixmap(canvas, x + 9, y + 18,
						skin_pixmap, &spin_dn, 1);
	}
}

static void default_draw_frame(sgui_skin *this, sgui_canvas *canvas,
				sgui_rect *r)
{
	int w = SGUI_RECT_WIDTH_V(r), h = SGUI_RECT_HEIGHT_V(r);
	int x = r->left, y = r->top;
	(void)this;

	sgui_canvas_draw_box(canvas, r, darkoverlay, SGUI_RGBA8);

	sgui_canvas_draw_line(canvas, x, y, w, 1, black, SGUI_RGB8);
	sgui_canvas_draw_line(canvas, x, y, h, 0, black, SGUI_RGB8);
	sgui_canvas_draw_line(canvas, x, y + h - 1, w, 1, white, SGUI_RGB8);
	sgui_canvas_draw_line(canvas, x + w - 1, y, h, 0, white, SGUI_RGB8);
}

static void default_draw_group_box( sgui_skin* this, sgui_canvas* canvas,
                                    sgui_rect* r, const char* caption )
{
	int w = SGUI_RECT_WIDTH_V(r), h = SGUI_RECT_HEIGHT_V(r);
	int x = r->left, y = r->top;
	int txw = sgui_skin_default_font_extents(caption, -1, 0, 0);
	(void)this;

	y += 8;
	h -= 8;

	sgui_canvas_draw_line(canvas, x + 1, y + 1, h - 1, 0,
				white, SGUI_RGB8);
	sgui_canvas_draw_line(canvas, x + 1, y + h - 1, w - 1, 1,
				white, SGUI_RGB8);
	sgui_canvas_draw_line(canvas, x + w - 1, y + 1, h - 1, 0,
				white, SGUI_RGB8);
	sgui_canvas_draw_line(canvas, x + 1, y + 1, 12, 1, white, SGUI_RGB8);

	sgui_canvas_draw_line(canvas, x, y, h - 1, 0, black, SGUI_RGB8);
	sgui_canvas_draw_line(canvas, x, y + h - 2, w - 2, 1,
				black, SGUI_RGB8);
	sgui_canvas_draw_line(canvas, x + w - 2, y, h - 1, 0,
				black, SGUI_RGB8);
	sgui_canvas_draw_line(canvas, x, y, 13, 1, black, SGUI_RGB8);

	sgui_canvas_draw_line(canvas, x + 18 + txw, y + 1, w - txw - 20, 1,
				white, SGUI_RGB8);
	sgui_canvas_draw_line(canvas, x + 18 + txw, y, w - txw - 20, 1,
				black, SGUI_RGB8);

	sgui_canvas_draw_text_plain(canvas, x + 15, r->top, 0, 0, white,
				caption, -1);
}

static void default_draw_progress_bar(sgui_skin *this, sgui_canvas *canvas,
					int x, int y, unsigned int length,
					int flags, unsigned int percentage)
{
	unsigned int w, h, i, bar = (percentage * (length - 2)) / 100;
	sgui_rect r;
	(void)this;

	if (flags & SGUI_PROGRESS_BAR_VERTICAL) {
		sgui_rect_set_size(&r, x, y, 30, length);
	} else {
		sgui_rect_set_size(&r, x, y, length, 30);
	}

	default_draw_frame(this, canvas, &r);

	if (flags & SGUI_PROGRESS_BAR_CONTINUOUS) {
		if (flags & SGUI_PROGRESS_BAR_VERTICAL) {
			y += length - bar - 1;
			w = 28;
			h = bar;
		} else {
			y += 1;
			w = bar;
			h = 28;
		}
		sgui_rect_set_size(&r, x + 1, y, w, h);
		sgui_canvas_draw_box(canvas, &r, yellow, SGUI_RGB8);
	} else {
		if (flags & SGUI_PROGRESS_BAR_VERTICAL) {
			for (i = 5; i < bar; i += 12) {
				sgui_rect_set_size(&r, x + 5,
							y + length - 7 - i,
							20, 7);
				sgui_canvas_draw_box(canvas, &r, white,
							SGUI_RGB8);
			}
		} else {
			for (i = 5; i < bar; i += 12) {
				sgui_rect_set_size(&r, x + i, y + 5, 7, 20);
				sgui_canvas_draw_box(canvas, &r, white,
							SGUI_RGB8);
			}
		}
	}
}

static void default_draw_scroll_bar( sgui_skin* this, sgui_canvas* canvas,
                                     int x, int y, unsigned int length,
                                     int vertical, int pane_offset,
                                     unsigned int pane_length,
                                     int decbutton, int incbutton )
{
	const sgui_pixmap *skin_pixmap;
	sgui_rect r;

	skin_pixmap = canvas->get_skin_pixmap(canvas);

	if (vertical) {
		/* background */
		sgui_rect_set_size( &r, x, y, 20, length );
		sgui_canvas_draw_box( canvas, &r,
					sgui_default_skin.window_color,
					SGUI_RGB8 );

		/* upper button */
		sgui_rect_set_size( &r, x, y, 20, 20 );
		default_draw_button( this, canvas, &r, incbutton );

		sgui_canvas_draw_pixmap(canvas, x + 5 - incbutton,
						y + 7 - incbutton,
						skin_pixmap, &arrow_up, 1);

		/* lower button */
		sgui_rect_set_size( &r, x, y+length-20, 20, 20 );
		default_draw_button( this, canvas, &r, decbutton );

		sgui_canvas_draw_pixmap(canvas, x + 5 - decbutton,
						y + length - 12 - decbutton,
						skin_pixmap, &arrow_dn, 1);

		/* pane */
		sgui_rect_set_size( &r, x, y+20+pane_offset, 20, pane_length );
		default_draw_button( this, canvas, &r, 0 );
	} else {
		/* background */
		sgui_rect_set_size( &r, x, y, length, 20 );
		sgui_canvas_draw_box( canvas, &r,
					sgui_default_skin.window_color,
					SGUI_RGB8 );

		/* left button */
		sgui_rect_set_size( &r, x, y, 20, 20 );
		default_draw_button( this, canvas, &r, incbutton );

		sgui_canvas_draw_pixmap(canvas, x + 7 - incbutton,
						y + 5 - incbutton,
						skin_pixmap, &arrow_l, 1);

		/* right button */
		sgui_rect_set_size( &r, x+length-20, y, 20, 20 );
		default_draw_button( this, canvas, &r, decbutton );

		sgui_canvas_draw_pixmap(canvas, x + length - 13 - decbutton,
						y + 5 - decbutton,
						skin_pixmap, &arrow_r, 1);

		/* pane */
		sgui_rect_set_size( &r, x+20+pane_offset, y, pane_length, 20 );
		default_draw_button( this, canvas, &r, 0 );
	}
}

static void default_draw_tab_caption(sgui_skin *this, sgui_canvas *canvas,
					int x, int y, const char *caption,
					unsigned int text_width)
{
	(void)this;
	sgui_canvas_draw_line(canvas, x, y, text_width, 1, white, SGUI_RGB8);
	sgui_canvas_draw_line(canvas, x, y, 25, 0, white, SGUI_RGB8);
	sgui_canvas_draw_line(canvas, x + text_width - 1, y, 25, 0,
				black, SGUI_RGB8);
	sgui_canvas_draw_text_plain(canvas, x + 8, y + 1, 0, 0,
				white, caption, -1);
}

static void default_draw_tab(sgui_skin *this, sgui_canvas *canvas,
				sgui_rect *r, unsigned int gap,
				unsigned int gap_width)
{
	int w = SGUI_RECT_WIDTH_V(r), h = SGUI_RECT_HEIGHT_V(r);
	int x = r->left, y = r->top;
	(void)this;

	sgui_canvas_draw_line(canvas, x, y, h, 0, white, SGUI_RGB8);
	sgui_canvas_draw_line(canvas, x, y, gap - x + 1, 1, white, SGUI_RGB8);
	sgui_canvas_draw_line(canvas, gap + gap_width - 1, y,
				w - gap_width - gap, 1, white, SGUI_RGB8);

	sgui_canvas_draw_line(canvas, x, y + h - 1, w, 1, black, SGUI_RGB8);
	sgui_canvas_draw_line(canvas, x + w - 1, y, h, 0, black, SGUI_RGB8);
}

static void default_draw_focus_box(sgui_skin *skin, sgui_canvas *canvas,
					sgui_rect *r)
{
	int w = SGUI_RECT_WIDTH_V(r), h = SGUI_RECT_HEIGHT_V(r);
	int x = r->left, y = r->top, i, l;
	(void)skin;

	for (i = 0; i < w - 2; i += 6) {
		l = w - 2 - i;
		l = l >= 6 ? 3 : l;
		sgui_canvas_draw_line(canvas, x + 1 + i, y + 1, l, 1,
					focusbox, SGUI_RGB8);
		sgui_canvas_draw_line(canvas, x + 1 + i, y + h - 2, l, 1,
					focusbox, SGUI_RGB8);
	}

	for (i = 0; i < h - 2; i += 6) {
		l = h - 2 - i;
		l = l >= 6 ? 3 : l;
		sgui_canvas_draw_line(canvas, x + 1, y + 1 + i, 3, 0,
					focusbox, SGUI_RGB8);
		sgui_canvas_draw_line(canvas, x + w - 2, y + 1 + i, 3, 0,
					focusbox, SGUI_RGB8);
	}
}

static void default_draw_slider(sgui_skin *skin, sgui_canvas *canvas,
				sgui_rect *r, int vertical, int min, int max,
				int value, int steps)
{
	int x, y, i, delta, draglen;
	sgui_rect r0;
	(void)skin;

	draglen = vertical ? SGUI_RECT_HEIGHT_V(r) : SGUI_RECT_WIDTH_V(r);
	draglen -= 10;

	/* dashes for discrete slider */
	if (steps) {
		delta = draglen / (steps-1);

		if (vertical) {
			x = r->left;
			y = r->top + 5;

			for (i = 0; i < steps; ++i, y += delta) {
				sgui_canvas_draw_line(canvas, x, y, 6, 1,
							white, SGUI_RGB8);
			}
		} else {
			x = r->left + 5;
			y = r->bottom - 6;

			for (i = 0; i < steps; ++i, x += delta) {
				sgui_canvas_draw_line(canvas, x, y, 6, 0,
							white, SGUI_RGB8);
			}
		}
	}

	/* sliding area */
	if (vertical) {
		x = (r->left + r->right) / 2;
		y = r->top;

		sgui_canvas_draw_line(canvas, x, y + 3, draglen + 6, 0,
					black, SGUI_RGB8);
		sgui_canvas_draw_line(canvas, x + 1, y + 3, draglen + 6, 0,
					white, SGUI_RGB8);
	} else {
		x = r->left;
		y = (r->top + r->bottom) / 2;

		sgui_canvas_draw_line(canvas, x + 3, y, draglen + 6, 1,
					black, SGUI_RGB8);
		sgui_canvas_draw_line(canvas, x + 3, y + 1, draglen + 6, 1,
					white, SGUI_RGB8);
	}

	/* slider box */
	if (steps) {
		i = ((steps - 1)*(value - min)) / (max - min);
		i *= draglen / (steps-1);
	} else {
		i = draglen - 1;
		i = (i * (value - min)) / (max - min);
	}

	if (vertical) {
		r0.left = r->left;
		r0.right = r->right;
		r0.top = r->bottom - 10 - i;
		r0.top = MAX(r0.top, r->top);
		r0.bottom = r0.top + 10;
		r0.bottom = MIN(r0.bottom, r->bottom);
	} else {
		r0.top = r->top;
		r0.bottom = r->bottom;
		r0.left = r->left + i;
		r0.left = MAX(r0.left, r->left);
		r0.right = r0.left + 10;
		r0.right = MIN(r0.right, r->right);
	}

	sgui_canvas_draw_box(canvas, &r0, sgui_default_skin.window_color,
				SGUI_RGB8);

	sgui_canvas_draw_line(canvas, r0.left, r0.top, SGUI_RECT_WIDTH(r0), 1,
				white, SGUI_RGB8);

	sgui_canvas_draw_line(canvas, r0.left, r0.top, SGUI_RECT_HEIGHT(r0), 0,
				white, SGUI_RGB8);

	sgui_canvas_draw_line(canvas, r0.right, r0.top, SGUI_RECT_HEIGHT(r0),
				0, black, SGUI_RGB8);

	sgui_canvas_draw_line(canvas, r0.left, r0.bottom, SGUI_RECT_WIDTH(r0),
				1, black, SGUI_RGB8);
}

static void default_get_skin_pixmap_size(sgui_skin* skin, unsigned int* width,
                                         unsigned int* height, int* format)
{
	(void)skin;
	*width = 128;
	*height = 128;
	*format = SGUI_RGBA8;
}

static void default_init_skin_pixmap(sgui_skin* skin, sgui_pixmap* pixmap)
{
	unsigned char buffer[PIXMAP_W * 2 * 4], *dptr;
	unsigned int a, b, x, y, count = 0;
	const unsigned char *iptr;
	(void)skin;

	iptr = pixmap_data;
	for (y = 0; y < PIXMAP_H; y += 2) {
		dptr = buffer;

		for (x = 0; x < (PIXMAP_W * 2); x += 2) {
			if (count) {
				--count;
			} else {
				count = (*iptr & 0x80) ?
					((*(iptr++) & 0x7F) - 1) :
					((*iptr & 0x40) ? 1 : 0);
				a = ((*iptr & 070) >> 3) * 4;
				b =  (*iptr & 007) * 4;
				++iptr;
			}

			*(dptr++) = colormap[a    ];
			*(dptr++) = colormap[a + 1];
			*(dptr++) = colormap[a + 2];
			*(dptr++) = colormap[a + 3];

			*(dptr++) = colormap[b    ];
			*(dptr++) = colormap[b + 1];
			*(dptr++) = colormap[b + 2];
			*(dptr++) = colormap[b + 3];
		}

		sgui_pixmap_load(pixmap, 0, y, buffer, 0, 0, PIXMAP_W, 2,
					PIXMAP_W, SGUI_RGBA8);
	}
}

/****************************************************************************/

sgui_skin sgui_default_skin = {
	.get_skin_pixmap_size = default_get_skin_pixmap_size,
	.init_skin_pixmap = default_init_skin_pixmap,
	.get_checkbox_extents = default_get_checkbox_extents,
	.get_radio_button_extents = default_get_checkbox_extents,
	.draw_checkbox = default_draw_checkbox,
	.draw_radio_button = default_draw_radio_button,
	.draw_button = default_draw_button,
	.draw_editbox = default_draw_editbox,
	.get_edit_box_height = default_get_edit_box_height,
	.draw_frame = default_draw_frame,
	.get_frame_border_width = default_get_frame_border_width,
	.draw_group_box = default_draw_group_box,
	.get_progess_bar_width = default_get_progess_bar_width,
	.draw_progress_bar = default_draw_progress_bar,
	.draw_scroll_bar = default_draw_scroll_bar,
	.get_scroll_bar_width = default_get_scroll_bar_width,
	.draw_tab_caption = default_draw_tab_caption,
	.draw_tab = default_draw_tab,
	.get_focus_box_width = default_get_focus_box_width,
	.draw_focus_box = default_draw_focus_box,
	.draw_slider = default_draw_slider,
	.get_slider_extents = default_get_slider_extents,
	.get_spin_buttons = default_get_spin_buttons,
	.get_edit_box_border_width = default_get_edit_box_border_width,
	.get_scroll_bar_button_extents = default_get_scroll_bar_button_extents,
	.get_tap_caption_extents = default_get_tap_caption_extents,
	.font_height = FONT_HEIGHT,
	.window_color = { 0x64, 0x64, 0x64, 0xFF },
	.font_color = { 0xFF, 0xFF, 0xFF, 0xFF },
};

void sgui_interal_skin_init_default(void)
{
	sgui_font *f;

	if (is_init)
		return;

	f = sgui_font_load(FONT, FONT_HEIGHT);
	if (!f)
		f = sgui_font_load(FONT_ALT_PATH FONT, FONT_HEIGHT);
	if (!f)
		f = sgui_font_load(FONT_ALT_PATH2 FONT, FONT_HEIGHT);

	sgui_default_skin.font_norm = f;

	f = sgui_font_load(FONT_ITAL, FONT_HEIGHT);
	if (!f)
		f = sgui_font_load(FONT_ALT_PATH FONT_ITAL, FONT_HEIGHT);
	if (!f)
		f = sgui_font_load(FONT_ALT_PATH2 FONT_ITAL, FONT_HEIGHT);

	sgui_default_skin.font_ital = f;

	f = sgui_font_load(FONT_BOLD, FONT_HEIGHT);
	if (!f)
		f = sgui_font_load(FONT_ALT_PATH FONT_BOLD, FONT_HEIGHT);
	if (!f)
		f = sgui_font_load(FONT_ALT_PATH2 FONT_BOLD, FONT_HEIGHT);

	sgui_default_skin.font_bold = f;

	f = sgui_font_load(FONT_BOLD_ITAL, FONT_HEIGHT);
	if (!f)
		f = sgui_font_load(FONT_ALT_PATH FONT_BOLD_ITAL, FONT_HEIGHT);
	if (!f)
		f = sgui_font_load(FONT_ALT_PATH2 FONT_BOLD_ITAL, FONT_HEIGHT);

	sgui_default_skin.font_boit = f;

	is_init = 1;
}

void sgui_interal_skin_deinit_default(void)
{
	sgui_font *f;

	if (is_init) {
		f = sgui_default_skin.font_bold;
		f->destroy(f);

		f = sgui_default_skin.font_ital;
		f->destroy(f);

		f = sgui_default_skin.font_boit;
		f->destroy(f);

		f = sgui_default_skin.font_norm;
		f->destroy(f);

		is_init = 0;
	}
}
