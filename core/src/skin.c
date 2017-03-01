/*
 * skin.c
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
#include "sgui_internal.h"
#include "sgui_skin.h"
#include "sgui_font.h"
#include "sgui_utf8.h"

#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

static sgui_skin* skin;

#define ITALIC 0x01
#define BOLD 0x02

static struct {
	const char *entity;
	const char *subst;
} entities[] = {
	{ "&lt;",  "<" },
	{ "&gt;",  ">" },
	{ "&amp;", "&" },
};

struct text_state {
	unsigned char col[3];
	int longest;
	int X, Y;
	int f;
};

static void process_section(struct text_state *s, int draw, const char *text,
				sgui_canvas *canvas, unsigned int i,
				int x, int y)
{
	if (draw) {
		s->X += sgui_canvas_draw_text_plain(canvas, x + s->X,
						y + s->Y, s->f & BOLD,
						s->f & ITALIC, s->col,
						text, i);
	} else {
		s->X += sgui_skin_default_font_extents(text, i, s->f & BOLD,
							s->f & ITALIC);
	}
}

static void process_tag(struct text_state *s, const char *text)
{
	unsigned int c;
	char *end;

	if (!strncmp(text, "<color=\"default\">", 17)) {
		memcpy(s->col, skin->font_color, 3);
		return;
	} else if(!strncmp(text, "<color=\"#", 9)) {
		c = strtol(text + 9, &end, 16);

		if (!strncmp(end, "\">", 2) && (end - (text + 9)) == 6) {
			s->col[0] = (c>>16) & 0xFF;
			s->col[1] = (c>>8 ) & 0xFF;
			s->col[2] =  c      & 0xFF;
		}
	} else if(!strncmp(text, "<b>", 3)) {
		s->f |= BOLD;
	} else if (!strncmp(text, "<i>", 3)) {
		s->f |= ITALIC;
	} else if (!strncmp(text, "</b>", 4)) {
		s->f &= ~BOLD;
	} else if (!strncmp(text, "</i>", 4)) {
		s->f &= ~ITALIC;
	}
}

static void process_entity(struct text_state *s, const char *text, int draw,
				sgui_canvas *canvas, int x, int y)
{
	const char *subst = NULL;
	char buffer[8];
	size_t i, len;

	for (i = 0; i < sizeof(entities) / sizeof(entities[0]); ++i) {
		len = strlen(entities[i].entity);

		if (!strncmp(text, entities[i].entity, len)) {
			subst = entities[i].subst;
			break;
		}
	}

	memset(buffer, 0, sizeof(buffer));
	subst = buffer;

	if (!subst && !strncmp(text, "&#h", 3)) {
		sgui_utf8_encode(strtol(text + 3, NULL, 16), buffer);
	} else if(!subst && !strncmp(text, "&#", 2)) {
		sgui_utf8_encode(strtol(text + 2, NULL, 10), buffer);
	} else {
		return;
	}

	if (draw) {
		s->X += sgui_canvas_draw_text_plain(canvas, x + s->X,
							y + s->Y, s->f & BOLD,
							s->f & ITALIC,
							s->col, subst, -1);
	} else {
		s->X += sgui_skin_default_font_extents(subst, -1, s->f & BOLD,
							s->f & ITALIC);
	}
}

/*
	text:text string to process
	canvas: canvas for drawing if "draw" is non-zero
	x: offset from the left if drawing text
	y: offset from the topy if drawing text
	r: if "draw" is zero, a pointer to a rect returning the text outlines
	draw: non-zero to draw the text, zero to measure the outlines
*/
static void process_text(const char *text, sgui_canvas *canvas, int x, int y,
			sgui_rect *r, int draw)
{
	struct text_state state;
	size_t i;

	memset(&state, 0, sizeof(state));
	memcpy(state.col, skin->font_color, 3);

	while (text && *text) {
		/* count chars until tag, entity, line break or terminator */
		for (i = 0; text[i] && !strchr("<&\n", text[i]); ++i)
			;

		process_section(&state, draw, text, canvas, i, x, y);

		if (text[i] == '<') {
			process_tag(&state, text + i);

			while (text[i] && text[i] != '>')
				++i;
		} else if(text[i] == '&') {
			process_entity(&state, text + i, 1, canvas, x, y);

			while (text[i] && text[i] != ';')
				++i;
		} else if (text[i] == '\n') {
			state.longest = state.X > state.longest ?
					state.X : state.longest;
			state.X = 0;			/* carriage return */
			state.Y += skin->font_height;	/* line feed */
		}

		text += text[i] ? (i + 1) : i;
	}

	/* account for last line */
	state.longest = state.X > state.longest ? state.X : state.longest;
	state.Y += skin->font_height;

	/* HACK: Add font height/2 because characters can peek below the line */
	if (!draw) {
		sgui_rect_set_size(r, x, y, state.longest,
					state.Y + skin->font_height / 2);
	}
}

/****************************************************************************/

void sgui_skin_set(sgui_skin *ui_skin)
{
	sgui_interal_skin_init_default();
	skin = ui_skin ? ui_skin : &sgui_default_skin;
}

sgui_skin *sgui_skin_get(void)
{
	return skin ? skin : &sgui_default_skin;
}

sgui_font* sgui_skin_get_default_font(int bold, int italic)
{
	if (bold && italic)
		return skin->font_boit;
	if (bold)
		return skin->font_bold;
	if (italic)
		return skin->font_ital;
	return skin->font_norm;
}

unsigned int sgui_skin_default_font_extents(const char *text,
						unsigned int length,
						int bold, int italic)
{
	sgui_font *font_face = sgui_skin_get_default_font(bold, italic);
	unsigned int x = 0, w, len = 0, i = 0;
	unsigned long character, previous = 0;

	while (i < length && (*text) && (*text != '\n')) {
		character = sgui_utf8_decode(text, &len);
		font_face->load_glyph(font_face, character);

		x += font_face->get_kerning_distance(font_face, previous,
								character);

		font_face->get_glyph_metrics(font_face, &w, NULL, NULL);
		x += w + 1;

		previous = character;
		text += len;
		i += len;
	}

	return x;
}

void sgui_skin_get_text_extents(const char *text, sgui_rect *r)
{
	process_text(text, NULL, 0, 0, r, 0);
}

void sgui_skin_draw_text(sgui_canvas *canvas, int x, int y,
			const char *text)
{
	process_text(text, canvas, x, y, NULL, 1);
}
