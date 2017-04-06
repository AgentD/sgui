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
#include <string.h>

#if defined(_MSC_VER) || defined(__MINGW32__)
#include <malloc.h>
#else
#include <alloca.h>
#endif


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

#define MBOX_ICON_W 32
#define MBOX_ICON_H 32

#define MBOX_ICON_TOP 32

/* COLORMAP 8 */
static const unsigned char colormap[4*8] = {
	0x00, 0x00, 0x00, 0x00,	/* completely transparent */
	0x00, 0x00, 0x00, 0xFF,	/* black */
	0xFF, 0xFF, 0xFF, 0xFF,	/* white */
	0x00, 0x00, 0x00, 0x80,	/* semi transparent black */
	0xFF, 0xFF, 0xFF, 0x80, /* semi transparent white */
	0x00, 0x00, 0xFF, 0xFF,	/* blue */
	0xFF, 0xFF, 0x00, 0xFF,	/* yellow */
	0xFF, 0x00, 0x00, 0xFF,	/* red */
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

/* SIZE 32x32 */
static const unsigned char info[255] = {
0x85,0,1,0x83,011,010,0x8A,0,011,012,0x83,022,021,011,0x88,0,011,0x87,022,011,
0x86,0,1,0x89,022,010,0x85,0,012,0x83,022,025,055,052,0x83,022,021,0x84,0,1,
0x84,022,0x83,055,0x84,022,010,0x83,0,012,0x84,022,0x83,055,0x84,022,021,0x40,
1,0x85,022,025,055,052,0x85,022,010,0,1,0x8D,022,014,0,012,0x8D,022,021,040,
012,0x84,022,025,0x83,055,0x85,022,021,040,012,0x85,022,025,0x6D,0x85,022,021,
044,012,0x85,022,025,0x6D,0x85,022,021,044,012,0x85,022,025,0x6D,0x85,022,021,
044,012,0x85,022,025,0x6D,0x85,022,021,044,012,0x85,022,025,0x6D,0x85,022,021,
044,1,0x85,022,025,0x6D,0x85,022,014,044,1,0x85,022,025,0x6D,0x85,022,014,044,
0,012,0x84,022,025,0x6D,0x84,022,021,044,040,0,1,0x83,022,025,0x84,055,0x83,
022,014,044,040,0x40,012,0x89,022,021,0x64,0x83,0,1,0x89,022,014,044,040,0x84,
0,011,0x87,022,011,0x64,0x85,0,4,011,012,0x83,022,021,011,0x64,040,0x86,0,044,
041,011,0x52,014,0x83,044,0x88,0,0x64,012,022,014,0x64,0x8A,0,4,012,022,014,
040,0x8C,0,1,022,014,040,0x8D,0,012,014,040,0x8D,0,1,014,040,0x8E,0,044,040,
0x8E,0,4,040,0x85,0
};

/* SIZE 32x32 */
static const unsigned char warning[248] = {
0x86,0,1,011,0x8E,0,016,066,010,0x8C,0,1,0x76,061,040,0x8B,0,1,0x76,061,044,
0x8B,0,016,0x83,066,014,040,0x8A,0,016,0x83,066,014,040,0x89,0,1,0x84,066,061,
044,0x89,0,1,0x84,066,061,044,0x89,0,016,0x85,066,014,040,0x88,0,016,066,061,
011,0x76,014,040,0x87,0,1,0x76,0x49,016,066,061,044,0x87,0,1,0x76,0x49,016,
066,061,044,0x87,0,016,0x76,0x49,016,0x76,014,040,0x86,0,016,0x76,0x49,016,
0x76,014,040,0x85,0,1,0x83,066,0x49,016,0x76,061,044,0x85,0,1,0x83,066,0x49,
016,0x76,061,044,0x85,0,016,0x83,066,061,011,0x84,066,014,040,0x84,0,016,0x83,
066,061,011,0x84,066,014,040,0x83,0,1,0x84,066,061,011,0x84,066,061,044,0x83,
0,1,0x85,066,016,0x84,066,061,044,0x83,0,016,0x85,066,016,0x85,066,014,040,
0x40,016,0x8B,066,014,040,0,1,0x86,066,011,0x85,066,061,044,0,1,0x85,066,061,
011,016,0x84,066,061,044,0,016,0x85,066,061,011,016,0x85,066,014,040,016,0x86,
066,011,0x86,066,014,040,016,0x8D,066,014,044,016,0x8D,066,014,044,1,0x8C,066,
061,0x64,0,0x8C,011,014,0x64,0x40,0x8D,044,040,0x40,4,0x8C,044,0
};

/* SIZE 32x32 */
static const unsigned char critical[291] = {
0x85,0,1,0x83,011,010,0x8A,0,1,017,0x83,077,071,010,0x88,0,1,017,0x85,077,071,
010,0x87,0,017,0x87,077,071,0x86,0,1,0x89,077,014,0x85,0,017,0x89,077,071,040,
0x83,0,1,0x8B,077,014,0x83,0,017,0x7F,072,0x85,077,027,0x7F,071,040,0x40,017,
0x7F,022,027,0x83,077,072,022,0x7F,071,040,0,1,0x7F,072,0x52,0x83,077,0x52,
027,0x7F,014,0,1,0x83,077,0x52,027,077,072,0x52,0x83,077,014,0,017,0x83,077,
072,0x52,077,0x52,027,0x83,077,071,040,017,0x84,077,0x85,022,0x84,077,071,040,
017,0x84,077,072,0x83,022,027,0x84,077,071,044,017,0x85,077,0x83,022,0x85,077,
071,044,017,0x85,077,0x83,022,0x85,077,071,044,017,0x84,077,072,0x83,022,027,
0x84,077,071,044,017,0x84,077,0x85,022,0x84,077,071,044,017,0x83,077,072,0x52,
077,0x52,027,0x83,077,071,044,1,0x83,077,0x52,027,077,072,0x52,0x83,077,014,
040,1,0x7F,072,0x52,0x83,077,0x52,027,0x7F,014,040,0,017,0x7F,022,027,0x83,
077,072,022,0x7F,071,044,040,0,017,0x7F,072,0x85,077,027,0x7F,071,044,0x40,
1,0x8B,077,014,044,0x83,0,017,0x89,077,071,044,040,0x83,0,041,0x89,077,014,
044,0x84,0,4,017,0x87,077,071,044,040,0x85,0,041,017,0x86,077,014,044,0x86,
0,4,041,017,0x83,077,071,014,044,040,0x87,0,4,041,0x83,011,014,0x64,0x89,0,
4,0x85,044,0x8B,0,4,0x83,044,0x86,0
};

/* SIZE 32x32 */
static const unsigned char question[257] = {
0x85,0,1,0x83,011,010,0x8A,0,011,012,0x83,022,021,011,0x88,0,011,0x87,022,011,
0x86,0,1,0x89,022,010,0x85,0,012,0x89,022,021,0x84,0,1,0x84,022,0x83,055,0x84,
022,010,0x83,0,012,0x83,022,025,022,025,055,052,0x83,022,021,0x40,1,0x84,022,
055,0x52,0x6D,0x84,022,010,0,1,0x84,022,0x6D,022,0x6D,0x84,022,014,0,012,0x84,
022,0x6D,022,0x6D,0x84,022,021,040,012,0x84,022,025,052,025,055,052,0x84,022,
021,040,012,0x86,022,0x6D,0x85,022,021,044,012,0x86,022,055,052,0x85,022,021,
044,012,0x86,022,055,0x86,022,021,044,012,0x86,022,055,0x86,022,021,044,012,
0x8D,022,021,044,1,0x86,022,055,0x86,022,014,044,1,0x85,022,025,055,052,0x85,
022,014,044,0,012,0x84,022,025,055,052,0x84,022,021,044,040,0,1,0x85,022,055,
0x85,022,014,044,040,0x40,012,0x89,022,021,0x64,0x83,0,1,0x89,022,014,044,040,
0x84,0,011,0x87,022,011,0x64,0x85,0,4,011,012,0x83,022,021,011,0x64,040,0x86,
0,044,041,011,0x52,014,0x83,044,0x88,0,0x64,012,022,014,0x64,0x8A,0,4,012,022,
014,040,0x8C,0,1,022,014,040,0x8D,0,012,014,040,0x8D,0,1,014,040,0x8E,0,044,
040,0x8E,0,4,040,0x85,0
};

static const sgui_rect default_icons[] = {
	{ 0, 32,  31, 63},
	{32, 32,  63, 63},
	{64, 32,  95, 63},
	{96, 32, 127, 63},
};

static int is_init = 0;
static const sgui_color black = { .c = { 0x00, 0x00, 0x00, 0xFF } };
static const sgui_color white = { .c = { 0xFF, 0xFF, 0xFF, 0xFF } };
static const sgui_color darkoverlay = { .c = { 0x00, 0x00, 0x00, 0x80 } };
static const sgui_color focusbox = { .c = { 0xFF, 0x80, 0x25, 0xFF } };
static const sgui_color yellow = { .c = { 0xFF, 0xFF, 0x00, 0xFF } };


static void decode_pixmap(sgui_pixmap *pixmap, int x, int y,
			const unsigned char *src, size_t size,
			unsigned int width, unsigned int height)
{
	union { sgui_u32 c32[2]; sgui_u64 c64; } col;
	const unsigned char *iptr, *end;
	unsigned int cw, count = 0;
	unsigned char *buffer;
	sgui_u64 *dptr64;
	size_t pixels;

	buffer = alloca(width * height * 4);

	iptr = src;
	end = src + size;
	pixels = width * height;
	dptr64 = (sgui_u64 *)buffer;

	while (iptr < end && pixels) {
		cw = *(iptr++);

		if (cw & 0x80) {
			if (iptr == end)
				break;
			count = cw & 0x7F;
			cw = *(iptr++);
		} else if (cw & 0x40) {
			count = 2;
		} else {
			count = 1;
		}

		col.c32[0] = ((sgui_u32 *)colormap)[(cw & 070) >> 3];
		col.c32[1] = ((sgui_u32 *)colormap)[ cw & 007      ];

		while (count && pixels >= 2) {
			*(dptr64++) = col.c64;
			--count;
			pixels -= 2;
		}
		if (count) {
			*((sgui_u32 *)dptr64) = col.c32[0];
			break;
		}
	}

	sgui_pixmap_load(pixmap, x, y, buffer, 0, 0, width, height,
			width, SGUI_RGBA8);
}

static void default_get_icon_area(sgui_skin *skin, sgui_rect *r, int icon)
{
	(void)skin;

	if (icon < 0 || icon >= SGUI_SKIN_NUM_ICONS) {
		memset(r, 0, sizeof(*r));
	} else {
		*r = default_icons[icon];
	}
}

static void default_get_button_extents(sgui_skin *this, int type, sgui_rect *r)
{
	(void)this;
	memset(r, 0, sizeof(*r));
	if (type == SGUI_CHECKBOX || type == SGUI_RADIO_BUTTON)
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

static void default_draw_button(sgui_skin *skin, sgui_canvas *canvas,
				sgui_rect *r, int type, int pressed)
{
	int w, h, ph, x = r->left, y = r->top;
	const sgui_pixmap *skin_pixmap;
	sgui_color lt, rb;
	sgui_rect *pic;
	(void)skin;

	switch (type) {
	case SGUI_RADIO_BUTTON:
		pic = pressed ? &radio_ticked : &radio;
		goto out_pic;
	case SGUI_CHECKBOX:
		pic = pressed ? &chkbox_ticked : &chkbox;
		goto out_pic;
	case SGUI_BUTTON:
	case SGUI_TOGGLE_BUTTON:
		w = SGUI_RECT_WIDTH_V(r);
		h = SGUI_RECT_HEIGHT_V(r);

		lt = pressed ? black : white;
		rb = pressed ? white : black;

		sgui_canvas_draw_line(canvas, x, y, w, 1, lt, 0);
		sgui_canvas_draw_line(canvas, x, y, h, 0, lt, 0);
		sgui_canvas_draw_line(canvas, x, y + h - 1, w, 1, rb, 0);
		sgui_canvas_draw_line(canvas, x + w - 1, y, h, 0, rb, 0);
		break;
	}
	return;
out_pic:
	h = SGUI_RECT_HEIGHT_V(r);
	ph = SGUI_RECT_HEIGHT_V(pic);

	if (h > ph)
		y += (h - ph) / 2;

	skin_pixmap = canvas->get_skin_pixmap(canvas);
	sgui_canvas_draw_pixmap(canvas, x, y, skin_pixmap, pic,
				SGUI_CANVAS_BLEND);
}

static void default_draw_editbox(sgui_skin *this, sgui_canvas *canvas,
				sgui_rect *r, const char *text, int offset,
				int cursor, int selection, int numeric,
				int spinbuttons)
{
	sgui_color selcolor = { .c = { 0xFF, 0x80, 0x25, 0xFF } };
	sgui_color cur = { .c = { 0x7F, 0x7F, 0x7F, 0xFF } };
	int x = r->left, y = r->top;
	int w=SGUI_RECT_WIDTH_V(r), h = SGUI_RECT_HEIGHT_V(r);
	const sgui_pixmap *skin_pixmap;
	int cx, dx = 0;
	sgui_rect r0;
	(void)this;

	sgui_canvas_draw_box(canvas, r, darkoverlay, SGUI_CANVAS_BLEND);
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
		sgui_canvas_draw_box(canvas, &r0, selcolor, 0);
	}

	/* draw text */
	sgui_canvas_draw_text_plain(canvas, x + 2 + dx, y + 4, 0, 0,
						white, text, -1);

	/* draw cursor */
	if (cursor >= 0) {
		cx = sgui_skin_default_font_extents(text, cursor, 0, 0) + 2;
		sgui_canvas_draw_line(canvas, dx + x + cx, y + 5, h - 10, 0,
					cur, 0);
	}

	/* draw borders */
	sgui_canvas_draw_line(canvas, x, y, w, 1, black, 0);
	sgui_canvas_draw_line(canvas, x, y, h, 0, black, 0);
	sgui_canvas_draw_line(canvas, x, y + h - 1, w, 1, white, 0);
	sgui_canvas_draw_line(canvas, x + w - 1, y, h, 0, white, 0);

	if (spinbuttons) {
		skin_pixmap = canvas->get_skin_pixmap(canvas);

		sgui_canvas_draw_pixmap(canvas, x + 9, y + 7, skin_pixmap,
					&spin_up, SGUI_CANVAS_BLEND);
		sgui_canvas_draw_pixmap(canvas, x + 9, y + 18, skin_pixmap,
					&spin_dn, SGUI_CANVAS_BLEND);
	}
}

static void default_draw_frame(sgui_skin *this, sgui_canvas *canvas,
				sgui_rect *r)
{
	int w = SGUI_RECT_WIDTH_V(r), h = SGUI_RECT_HEIGHT_V(r);
	int x = r->left, y = r->top;
	(void)this;

	sgui_canvas_draw_box(canvas, r, darkoverlay, SGUI_CANVAS_BLEND);

	sgui_canvas_draw_line(canvas, x, y, w, 1, black, 0);
	sgui_canvas_draw_line(canvas, x, y, h, 0, black, 0);
	sgui_canvas_draw_line(canvas, x, y + h - 1, w, 1, white, 0);
	sgui_canvas_draw_line(canvas, x + w - 1, y, h, 0, white, 0);
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

	sgui_canvas_draw_line(canvas, x + 1, y + 1, h - 1, 0, white, 0);
	sgui_canvas_draw_line(canvas, x + 1, y + h - 1, w - 1, 1, white, 0);
	sgui_canvas_draw_line(canvas, x + w - 1, y + 1, h - 1, 0, white, 0);
	sgui_canvas_draw_line(canvas, x + 1, y + 1, 12, 1, white, 0);

	sgui_canvas_draw_line(canvas, x, y, h - 1, 0, black, 0);
	sgui_canvas_draw_line(canvas, x, y + h - 2, w - 2, 1, black, 0);
	sgui_canvas_draw_line(canvas, x + w - 2, y, h - 1, 0, black, 0);
	sgui_canvas_draw_line(canvas, x, y, 13, 1, black, 0);

	sgui_canvas_draw_line(canvas, x + 18 + txw, y + 1, w - txw - 20, 1,
				white, 0);
	sgui_canvas_draw_line(canvas, x + 18 + txw, y, w - txw - 20, 1,
				black, 0);

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
		sgui_canvas_draw_box(canvas, &r, yellow, 0);
	} else {
		if (flags & SGUI_PROGRESS_BAR_VERTICAL) {
			for (i = 5; i < bar; i += 12) {
				sgui_rect_set_size(&r, x + 5,
							y + length - 7 - i,
							20, 7);
				sgui_canvas_draw_box(canvas, &r, white, 0);
			}
		} else {
			for (i = 5; i < bar; i += 12) {
				sgui_rect_set_size(&r, x + i, y + 5, 7, 20);
				sgui_canvas_draw_box(canvas, &r, white, 0);
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
		sgui_canvas_draw_box(canvas, &r,
					sgui_default_skin.window_color, 0);

		/* upper button */
		sgui_rect_set_size(&r, x, y, 20, 20);
		default_draw_button(this, canvas, &r, SGUI_BUTTON, incbutton);

		sgui_canvas_draw_pixmap(canvas, x + 5 - incbutton,
						y + 7 - incbutton,
						skin_pixmap, &arrow_up,
						SGUI_CANVAS_BLEND);

		/* lower button */
		sgui_rect_set_size(&r, x, y+length-20, 20, 20);
		default_draw_button(this, canvas, &r, SGUI_BUTTON, decbutton);

		sgui_canvas_draw_pixmap(canvas, x + 5 - decbutton,
						y + length - 12 - decbutton,
						skin_pixmap, &arrow_dn,
						SGUI_CANVAS_BLEND);

		/* pane */
		sgui_rect_set_size(&r, x, y+20+pane_offset, 20, pane_length);
		default_draw_button(this, canvas, &r, SGUI_BUTTON, 0);
	} else {
		/* background */
		sgui_rect_set_size( &r, x, y, length, 20 );
		sgui_canvas_draw_box(canvas, &r,
					sgui_default_skin.window_color, 0);

		/* left button */
		sgui_rect_set_size( &r, x, y, 20, 20 );
		default_draw_button(this, canvas, &r, SGUI_BUTTON, incbutton);

		sgui_canvas_draw_pixmap(canvas, x + 7 - incbutton,
						y + 5 - incbutton,
						skin_pixmap, &arrow_l,
						SGUI_CANVAS_BLEND);

		/* right button */
		sgui_rect_set_size( &r, x+length-20, y, 20, 20 );
		default_draw_button(this, canvas, &r, SGUI_BUTTON, decbutton);

		sgui_canvas_draw_pixmap(canvas, x + length - 13 - decbutton,
						y + 5 - decbutton,
						skin_pixmap, &arrow_r,
						SGUI_CANVAS_BLEND);

		/* pane */
		sgui_rect_set_size( &r, x+20+pane_offset, y, pane_length, 20 );
		default_draw_button(this, canvas, &r, SGUI_BUTTON, 0);
	}
}

static void default_draw_tab_caption(sgui_skin *this, sgui_canvas *canvas,
					int x, int y, const char *caption,
					unsigned int text_width)
{
	(void)this;
	sgui_canvas_draw_line(canvas, x, y, text_width, 1, white, 0);
	sgui_canvas_draw_line(canvas, x, y, 25, 0, white, 0);
	sgui_canvas_draw_line(canvas, x + text_width - 1, y, 25, 0,
				black, 0);
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

	sgui_canvas_draw_line(canvas, x, y, h, 0, white, 0);
	sgui_canvas_draw_line(canvas, x, y, gap - x + 1, 1, white, 0);
	sgui_canvas_draw_line(canvas, gap + gap_width - 1, y,
				w - gap_width - gap, 1, white, 0);

	sgui_canvas_draw_line(canvas, x, y + h - 1, w, 1, black, 0);
	sgui_canvas_draw_line(canvas, x + w - 1, y, h, 0, black, 0);
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
					focusbox, 0);
		sgui_canvas_draw_line(canvas, x + 1 + i, y + h - 2, l, 1,
					focusbox, 0);
	}

	for (i = 0; i < h - 2; i += 6) {
		l = h - 2 - i;
		l = l >= 6 ? 3 : l;
		sgui_canvas_draw_line(canvas, x + 1, y + 1 + i, 3, 0,
					focusbox, 0);
		sgui_canvas_draw_line(canvas, x + w - 2, y + 1 + i, 3, 0,
					focusbox, 0);
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
							white, 0);
			}
		} else {
			x = r->left + 5;
			y = r->bottom - 6;

			for (i = 0; i < steps; ++i, x += delta) {
				sgui_canvas_draw_line(canvas, x, y, 6, 0,
							white, 0);
			}
		}
	}

	/* sliding area */
	if (vertical) {
		x = (r->left + r->right) / 2;
		y = r->top;

		sgui_canvas_draw_line(canvas, x, y + 3, draglen + 6, 0,
					black, 0);
		sgui_canvas_draw_line(canvas, x + 1, y + 3, draglen + 6, 0,
					white, 0);
	} else {
		x = r->left;
		y = (r->top + r->bottom) / 2;

		sgui_canvas_draw_line(canvas, x + 3, y, draglen + 6, 1,
					black, 0);
		sgui_canvas_draw_line(canvas, x + 3, y + 1, draglen + 6, 1,
					white, 0);
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

	sgui_canvas_draw_box(canvas, &r0, sgui_default_skin.window_color, 0);

	sgui_canvas_draw_line(canvas, r0.left, r0.top, SGUI_RECT_WIDTH(r0), 1,
				white, 0);

	sgui_canvas_draw_line(canvas, r0.left, r0.top, SGUI_RECT_HEIGHT(r0), 0,
				white, 0);

	sgui_canvas_draw_line(canvas, r0.right, r0.top, SGUI_RECT_HEIGHT(r0),
				0, black, 0);

	sgui_canvas_draw_line(canvas, r0.left, r0.bottom, SGUI_RECT_WIDTH(r0),
				1, black, 0);
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
	unsigned int w, h;
	int x, y;
	(void)skin;

	decode_pixmap(pixmap, 0, 0, pixmap_data, sizeof(pixmap_data),
			PIXMAP_W, PIXMAP_H);

	x = 0;
	y = MBOX_ICON_TOP;
	w = MBOX_ICON_W;
	h = MBOX_ICON_H;


	decode_pixmap(pixmap, x, y, info, sizeof(info), w, h);
	x += MBOX_ICON_W;

	decode_pixmap(pixmap, x, y, warning, sizeof(warning), w, h);
	x += MBOX_ICON_W;

	decode_pixmap(pixmap, x, y, critical, sizeof(critical), w, h);
	x += MBOX_ICON_W;

	decode_pixmap(pixmap, x, y, question, sizeof(pixmap_data), w, h);
}

/****************************************************************************/

sgui_skin sgui_default_skin = {
	.get_skin_pixmap_size = default_get_skin_pixmap_size,
	.init_skin_pixmap = default_init_skin_pixmap,
	.get_button_extents = default_get_button_extents,
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
	.get_icon_area = default_get_icon_area,
	.font_height = FONT_HEIGHT,
	.window_color = { .c = { 0x64, 0x64, 0x64, 0xFF } },
	.font_color = { .c = { 0xFF, 0xFF, 0xFF, 0xFF } },
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
