/*
 * pixmap.c
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
#include "sgui_pixmap.h"

#include "internal.h"

#include <stdlib.h>
#include <string.h>


#ifndef SGUI_NO_MEM_CANVAS
typedef struct {
	sgui_pixmap super;

	unsigned char *buffer;
	int format, swaprb;
} mem_pixmap;


static void mem_pixmap_load_rgb8(sgui_pixmap *super, int dstx, int dsty,
				const unsigned char *data, unsigned int scan,
				unsigned int width, unsigned int height,
				int format)
{
	mem_pixmap *this = (mem_pixmap *)super;
	unsigned char temp, *dst, *dstrow;
	const unsigned char *src, *row;
	unsigned int i, j;

	dst = this->buffer + (dstx + dsty * super->width) * 3;
	src = data;

	if (format == SGUI_A8) {
		for (j = 0; j < height; ++j) {
			dstrow = dst;
			row = src;
			for (i = 0; i < width; ++i) {
				dstrow[0] = dstrow[1] = dstrow[2] = *(row++);
				dstrow += 3;
			}
			src += scan;
			dst += super->width * 3;
		}
	} else if (format == SGUI_RGB8) {
		for (j = 0; j < height; ++j) {
			memcpy(dst, src, width * 3);
			src += scan * 3;
			dst += super->width * 3;
		}
	} else {
		for (j = 0; j < height; ++j) {
			dstrow = dst;
			row = src;

			for (i = 0; i < width; ++i) {
				dstrow[0] = row[0];
				dstrow[1] = row[1];
				dstrow[2] = row[2];
				row += 4;
				dstrow += 3;
			}
			src += scan * 4;
			dst += super->width * 3;
		}
	}

	if (this->swaprb && format != SGUI_A8) {
		dst = this->buffer + (dstx + dsty * super->width) * 3;

		for (j = 0; j < height; ++j) {
			dstrow = dst;
			for (i = 0; i < width; ++i) {
				temp = dstrow[0];
				dstrow[0] = dstrow[2];
				dstrow[2] = temp;
				dstrow += 3;
			}
			dst += super->width * 3;
		}
	}
}

static void mem_pixmap_load_rgba8(sgui_pixmap *super, int dstx, int dsty,
				const unsigned char *data,
				unsigned int scan, unsigned int width,
				unsigned int height, int format)
{
	mem_pixmap *this = (mem_pixmap *)super;
	sgui_color *dst, *dstrow;
	const unsigned char *src, *row;
	unsigned int i, j;
	int val;

	dst = (sgui_color *)(this->buffer + (dstx + dsty * super->width) * 4);
	src = data;

	if (format == SGUI_A8) {
        	for (j = 0; j < height; ++j) {
        		dstrow = dst;
        		row = src;
			for (i = 0; i < width; ++i, ++dstrow) {
				val = *(row++);
				*dstrow = sgui_color_set(val, val, val, val);
			}
			src += scan;
			dst += super->width;
		}
	} else if (format == SGUI_RGB8) {
		for (j = 0; j<height; ++j) {
			dstrow = dst;
			row = src;
			for (i = 0; i < width; ++i) {
				*(dstrow++) = sgui_color_load3(row);
				row += 3;
			}
			src += scan * 3;
			dst += super->width;
		}
	} else {
		for (j = 0; j < height; ++j) {
			memcpy(dst, src, width * 4);

			src += scan * 4;
			dst += super->width;
		}
	}

	if (this->swaprb && format != SGUI_A8) {
		dst = (sgui_color *)this->buffer + dstx + dsty * super->width;

		for (j = 0; j < height; ++j) {
			dstrow = dst;
			for (i = 0; i < width; ++i) {
				val = dstrow->c.r;
				dstrow->c.r = dstrow->c.b;
				dstrow->c.b = val;
				++dstrow;
			}
			dst += super->width;
		}
	}
}

static void mem_pixmap_load_a8(sgui_pixmap *super, int dstx, int dsty,
				const unsigned char *data,
				unsigned int scan, unsigned int width,
				unsigned int height, int format)
{
	mem_pixmap *this = (mem_pixmap *)super;
	unsigned char *dst, *dstrow;
	const unsigned char *src, *row;
	unsigned int i, j;

	dst = this->buffer + (dstx + dsty * super->width);
	src = data;

	if (format == SGUI_A8) {
		for (j = 0; j < height; ++j) {
			memcpy(dst, src, width);
			src += scan;
			dst += super->width;
		}
	} else if (format == SGUI_RGB8) {
		for (j = 0; j < height; ++j) {
			dstrow = dst;
			row = src;
			for (i=0; i < width; ++i) {
				*(dstrow++) = *row;
				row += 3;
			}
			src += scan;
			dst += super->width;
		}
	} else {
		for (j = 0; j < height; ++j) {
			dstrow = dst;
			row = src;
			for (i = 0; i < width; ++i) {
				*(dstrow++) = row[3];
				row += 4;
			}
			src += scan;
			dst += super->width;
		}
	}
}

static void mem_pixmap_destroy(sgui_pixmap *super)
{
	mem_pixmap *this = (mem_pixmap *)super;

	free(this->buffer);
	free(this);
}

/****************************************************************************/

sgui_pixmap *mem_pixmap_create(unsigned int width, unsigned int height,
				int format, int swaprb)
{
	sgui_pixmap *super;
	mem_pixmap *this;

	if (!width || !height)
		return NULL;

	this = calloc(1, sizeof(*this));
	super = (sgui_pixmap *)this;

	if (!this)
		return NULL;

	this->buffer = malloc(width * height * (format == SGUI_RGB8 ? 3 : 
						format == SGUI_RGBA8 ? 4 : 1));

	if (!this->buffer) {
		free(this);
		return NULL;
	}

	this->format = format;
	this->swaprb = swaprb;
	super->width = width;
	super->height = height;
	super->destroy = mem_pixmap_destroy;
	super->load = (format == SGUI_RGBA8) ? mem_pixmap_load_rgba8 :
			(format == SGUI_RGB8) ? mem_pixmap_load_rgb8 :
						mem_pixmap_load_a8;
	return (sgui_pixmap *)this;
}

const unsigned char *mem_pixmap_buffer(const sgui_pixmap *this)
{
	return ((mem_pixmap *)this)->buffer;
}

int mem_pixmap_format(const sgui_pixmap *this)
{
	return ((mem_pixmap *)this)->format;
}
#endif
