/*
 * sgui_scroll_bar.h
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
#ifndef SGUI_SCROLL_BAR_H
#define SGUI_SCROLL_BAR_H



#include "sgui_predef.h"



#define SGUI_SCROLL_BAR_HORIZONTAL 1
#define SGUI_SCROLL_BAR_VERTICAL   0



#ifdef __cplusplus
extern "C"
{
#endif

/**
 * \brief Create a scroll bar widget
 *
 * \param x                  The horizontal component of the bars position.
 * \param y                  The vertical component of the bars position.
 * \param horizontal         Non-zero for a horizontal bar, zero for a
 *                           vertical bar.
 * \param length             The length of the scroll bar in pixels.
 * \param scroll_area_length The length of the scrolled area in pixels.
 * \param disp_area_length   The length of the displayable area in pixels.
 */
sgui_widget* sgui_scroll_bar_create( int x, int y, int horizontal,
                                     unsigned int length,
                                     unsigned int scroll_area_length,
                                     unsigned int disp_area_length );

/** \brief Destroy a scroll bar widget */
void sgui_scroll_bar_destroy( sgui_widget* bar );

/**
 * \brief Set the scroll area offset in pixels of a scroll bar
 *
 * \param bar    The scroll bar
 * \param offset The offset of the scroll area in pixels
 */
void sgui_scroll_bar_set_offset( sgui_widget* bar, unsigned int offset );

/** \brief Get the scroll area offset in pixels from a scroll bar */
unsigned int sgui_scroll_bar_get_offset( sgui_widget* bar );

/**
 * \brief Inform the scrollbar of a change in the scroll area size
 *
 * \param bar                The scroll bar
 * \param scroll_area_length The length of the scrolled area in pixels.
 * \param disp_area_length   The length of the displayable area in pixels.
 */
void sgui_scroll_bar_set_area( sgui_widget* bar,
                               unsigned int scroll_area_length,
                               unsigned int disp_area_length );

#ifdef __cplusplus
}
#endif

#endif /* SGUI_SCROLL_BAR_H */

