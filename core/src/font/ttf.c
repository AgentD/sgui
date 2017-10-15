/*
 * ttf.c
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
#include "sgui_font.h"
#include "sgui_internal.h"

#include <ft2build.h>
#include FT_FREETYPE_H

#ifdef SGUI_UNIX
	#define SYS_FONT_PATH "/usr/share/fonts/TTF/"
#elif defined SGUI_WINDOWS
	#define SYS_FONT_PATH "C:\\Windows\\Fonts\\"
#else
	#define NO_SYS_FONT_PATH 1
#endif

typedef struct {
	sgui_font super;
	FT_Face face;
	void *buffer;
	unsigned int current_glyph;
} sgui_ttf_font;

static FT_Library freetype = 0;
static unsigned int refcount = 0;

static int freetype_grab(void)
{
	sgui_internal_lock_mutex();
	if (refcount == 0) {
		if (FT_Init_FreeType(&freetype)) {
			sgui_internal_unlock_mutex();
			return -1;
		}
	}

	++refcount;
	sgui_internal_unlock_mutex();
	return 0;
}

static void freetype_drop(void)
{
	sgui_internal_lock_mutex();

	if (refcount) {
		--refcount;

		if (refcount == 0) {
			FT_Done_FreeType(freetype);
			freetype = 0;
		}
	}

	sgui_internal_unlock_mutex();
}

static void font_destroy(sgui_font *this)
{
	FT_Done_Face(((sgui_ttf_font *)this)->face);

	free(((sgui_ttf_font *)this)->buffer);
	free(this);

	freetype_drop();
}

static void font_load_glyph(sgui_font *super, unsigned int codepoint)
{
	sgui_ttf_font *this = (sgui_ttf_font *)super;
	FT_UInt i;

	this->current_glyph = codepoint;

	i = FT_Get_Char_Index(this->face, codepoint);

	FT_Load_Glyph(this->face, i, FT_LOAD_DEFAULT);
	FT_Render_Glyph(this->face->glyph, FT_RENDER_MODE_NORMAL);
}

static int font_get_kerning_distance(sgui_font *super, unsigned int first,
				     unsigned int second)
{
	sgui_ttf_font *this = (sgui_ttf_font *)super;
	FT_UInt index_a, index_b;
	FT_Vector delta;

	if (!FT_HAS_KERNING(this->face))
		return 0;

	index_a = FT_Get_Char_Index(this->face, first);
	index_b = FT_Get_Char_Index(this->face, second);

	FT_Get_Kerning(this->face, index_a, index_b,
			FT_KERNING_DEFAULT, &delta);

	return -((delta.x < 0 ? -delta.x : delta.x) >> 6);
}

static void font_get_glyph_metrics(sgui_font *super, unsigned int *width,
				   unsigned int *height, int *bearing)
{
	sgui_ttf_font *this = (sgui_ttf_font *)super;
	unsigned int w = 0, h = 0;
	int b = 0;

	if (this->current_glyph == ' ') {
		w = super->height / 3;
		h = super->height;
		b = 0;
	} else {
		w = this->face->glyph->bitmap.width;
		h = this->face->glyph->bitmap.rows;
		b = super->height - this->face->glyph->bitmap_top;
	}

	if (width)
		*width = w;
	if (height)
		*height = h;
	if (bearing)
		*bearing = b;
}

static unsigned char *font_get_glyph(sgui_font *this)
{
	if (((sgui_ttf_font *)this)->current_glyph != ' ')
		return ((sgui_ttf_font *)this)->face->glyph->bitmap.buffer;

	return NULL;
}

static sgui_ttf_font *font_load_common(unsigned int pixel_height)
{
	sgui_ttf_font *this = calloc(1, sizeof(*this));
	sgui_font *super = (sgui_font *)this;

	if (!this)
		return NULL;

	super->height = pixel_height;
	super->destroy = font_destroy;
	super->load_glyph = font_load_glyph;
	super->get_kerning_distance = font_get_kerning_distance;
	super->get_glyph_metrics = font_get_glyph_metrics;
	super->get_glyph = font_get_glyph;
	return this;
}

sgui_font *sgui_font_load(const char *filename, unsigned int pixel_height)
{
	sgui_ttf_font *this;
	char buffer[512];

	if (freetype_grab())
		return NULL;

	this = font_load_common(pixel_height);
	if (!this) {
		freetype_drop();
		return NULL;
	}

	if (!FT_New_Face(freetype, filename, 0, &this->face))
		goto cont;

#ifndef NO_SYS_FONT_PATH
	sprintf(buffer, "%s%s", SYS_FONT_PATH, filename);

	if (!FT_New_Face(freetype, buffer, 0, &this->face))
		goto cont;
#endif

	free(this);
	freetype_drop();
	return NULL;
cont:
	FT_Set_Pixel_Sizes(this->face, 0, pixel_height);
	return (sgui_font *)this;
}

sgui_font *sgui_font_load_memory(const void *data, unsigned long size,
				unsigned int pixel_height)
{
	sgui_ttf_font *this;

	if (freetype_grab())
		return NULL;

	this = font_load_common(pixel_height);
	if (!this) {
		freetype_drop();
		return NULL;
	}

	this->buffer = malloc(size);

	if (!this->buffer) {
		free(this);
		freetype_drop();
		return NULL;
	}

	memcpy(this->buffer, data, size);

	if (FT_New_Memory_Face(freetype, this->buffer, size, 0, &this->face)) {
		free(this->buffer);
		free(this);
		freetype_drop();
		return NULL;
	}

	FT_Set_Pixel_Sizes(this->face, 0, pixel_height);
	return (sgui_font *)this;
}
