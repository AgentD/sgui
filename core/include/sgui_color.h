/*
 * sgui_color.h
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

/**
 * \file sgui_color.h
 *
 * \brief Contains forward the \ref sgui_color type
 */
#ifndef SGUI_COLOR_H
#define SGUI_COLOR_H


#include "sgui_predef.h"


#define SGUI_A8    0
#define SGUI_RGB8  1
#define SGUI_RGBA8 2


union sgui_color
{
    struct
    {
        sgui_u8 r;
        sgui_u8 g;
        sgui_u8 b;
        sgui_u8 a;
    } c;

    sgui_u8 v[4];

    sgui_u32 uival;
};

static SGUI_INLINE sgui_color sgui_color_load3(const unsigned char *ptr)
{
	sgui_color ret;
	ret.v[0] = ptr[0];
	ret.v[1] = ptr[1];
	ret.v[2] = ptr[2];
	ret.v[3] = 0xFF;
	return ret;
}

static SGUI_INLINE void sgui_color_store3(unsigned char *ptr,
						const sgui_color col)
{
	ptr[0] = col.v[0];
	ptr[1] = col.v[1];
	ptr[2] = col.v[2];
}

static SGUI_CONST_INLINE sgui_color sgui_color_set(int r, int g, int b, int a)
{
	sgui_color ret;
	ret.v[0] = r;
	ret.v[1] = g;
	ret.v[2] = b;
	ret.v[3] = a;
	return ret;
}

static SGUI_CONST_INLINE sgui_color sgui_color_mix(const sgui_color lower,
							const sgui_color upper,
							const int alpha)
{
	sgui_color ret;

	ret.c.r = (upper.c.r * alpha + lower.c.r * (0xFF - alpha)) >> 8;
	ret.c.g = (upper.c.g * alpha + lower.c.g * (0xFF - alpha)) >> 8;
	ret.c.b = (upper.c.b * alpha + lower.c.b * (0xFF - alpha)) >> 8;
	ret.c.a = (upper.c.a * alpha + lower.c.a * (0xFF - alpha)) >> 8;

	return ret;
}

#endif /* SGUI_COLOR_H */

