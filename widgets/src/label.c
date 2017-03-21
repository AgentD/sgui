/*
 * label.c
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
#include "sgui_label.h"
#include "sgui_canvas.h"
#include "sgui_skin.h"
#include "sgui_font.h"
#include "sgui_internal.h"
#include "sgui_utf8.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>


typedef struct {
	sgui_widget super;

	char *text;
} sgui_label;


static void label_draw(sgui_widget *super)
{
	sgui_label *this = (sgui_label *)super;

	sgui_skin_draw_text(super->canvas, super->area.left, super->area.top,
				this->text);
}

static void label_destroy(sgui_widget *this)
{
	free(((sgui_label *)this)->text);
	free(this);
}

sgui_widget *sgui_label_create(int x, int y, const char *text)
{
	sgui_label *this = calloc(1, sizeof(*this));
	sgui_widget *super = (sgui_widget *)this;

	if (!this)
		return NULL;

	this->text = sgui_strdup(text);
	if (!this->text) {
		free(this);
		return NULL;
	}

	sgui_widget_init((sgui_widget *)this, 0, 0, 0, 0);
	super->draw = label_draw;
	super->destroy = label_destroy;
	super->flags = SGUI_WIDGET_VISIBLE;

	sgui_skin_get_text_extents(text, &super->area);
	sgui_rect_set_position(&super->area, x, y);
	return (sgui_widget*)this;
}
