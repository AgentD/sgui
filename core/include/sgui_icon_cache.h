/*
 * sgui_icon_cache.h
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
#ifndef SGUI_ICON_CACHE_H
#define SGUI_ICON_CACHE_H



#include "sgui_predef.h"



#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Create an icon cache object
 *
 * \param canvas The canvas that is supposed to own the icon pixmap
 * \param width  The width of the icon pixmap
 * \param height The height of the icon pixmap
 * \param alpha  Non-zero if the underlying pixmap should have an alpha
 *               channel, zero for RGB only.
 *
 * \return On success, a pointer to an icon pixmap object, NULL on failure
 */
SGUI_DLL sgui_icon_cache* sgui_icon_cache_create( sgui_canvas* canvas,
                                                  unsigned int width,
                                                  unsigned int height,
                                                  int alpha );

/**
 * \brief Destroy an icon cache object
 *
 * \param cache A pointer to an icon cache object
 */
SGUI_DLL void sgui_icon_cache_destroy( sgui_icon_cache* cache );

/**
 * \brief Add an icon to an icon cache
 *
 * \param cache  A pointer to an icon cache object
 * \param id     A unique id to asociate with the icon
 * \param width  The width of the icon
 * \param height The height of the icon
 *
 * \return Non-zero on success, zero on failure (e.g. out of space on pixmap)
 */
SGUI_DLL int sgui_icon_cache_add_icon( sgui_icon_cache* cache,
                                       unsigned int id,
                                       unsigned int width,
                                       unsigned int height );

/**
 * \brief Load icon data into an icon cache
 *
 * \param cache  A pointer to an icon cache object
 * \param id     The unique id of the icon
 * \param data   A pointer to the image data to load
 * \param scan   Number of pixels to skip to get to the first pixel in the
 *               next line of the image.
 * \param format The color format of the image data.
 */
SGUI_DLL void sgui_icon_cache_load_icon( sgui_icon_cache* cache,
                                         unsigned int id,
                                         unsigned char* data,
                                         unsigned int scan,
                                         int format );

/**
 * \brief Draw an icon from an icon cache onto the coresponding canvas
 *
 * \param cache A pointer to an icon cache object
 * \param id    The unique id of the canvas
 * \param x     Distance of the left of the icon from the left of the canvas
 * \param y     Distance of the top of the icon from the top of the canvas
 */
SGUI_DLL void sgui_icon_cache_draw_icon( const sgui_icon_cache* cache,
                                         unsigned int id, int x, int y );

/**
 * \brief Get the area occupied by an icon on the icon pixmap
 *
 * \param cache A pointer to the icon cache object
 * \param id    The id of the icon
 * \param out   Returns the area of the icon on the pixmaü
 *
 * \return Non-zero on success, zero on failure (e.g. no icon with that id)
 */
SGUI_DLL int sgui_icon_cache_get_icon_area( const sgui_icon_cache* cache,
                                            unsigned int id,
                                            sgui_rect* out );

#ifdef __cplusplus
}
#endif

#endif /* SGUI_ICON_CACHE_H */

