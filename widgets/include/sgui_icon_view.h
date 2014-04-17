/*
 * sgui_icon_view.h
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
#ifndef SGUI_ICON_VIEW_H
#define SGUI_ICON_VIEW_H



#include "sgui_predef.h"



#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Create an icon view widget
 *
 * \param x          The distance of the view area from the left of the canvas
 * \param y          The distance of the view area from the top of the canvas
 * \param width      The horizontal length of the icon view area
 * \param height     The vertical length of the icon view area
 * \param cache      An icon cache object holding icons
 * \param background Non-zero to draw a frame widget style background around
 *                   the icon view area, zero to disable
 *
 * \return A pointer to an icon view widget
 */
SGUI_DLL sgui_widget* sgui_icon_view_create( int x, int y, unsigned width,
                                             unsigned int height,
                                             sgui_icon_cache* cache,
                                             int background );

/**
 * \brief Add an icon to an icon view widget
 *
 * \param 
 */
SGUI_DLL void sgui_icon_view_add_icon( sgui_widget* view, int x, int y,
                                       const char* subtext,
                                       unsigned int id );

#ifdef __cplusplus
}
#endif

#endif /* SGUI_ICON_VIEW_H */

