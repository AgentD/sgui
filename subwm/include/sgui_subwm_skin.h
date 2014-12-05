/*
 * sgui_subwm_skin.h
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
 * \file sgui_subwm_skin.h
 *
 * \brief Skinning functions for subwindows
 */
#ifndef SGUI_SUBWM_SKIN_H
#define SGUI_SUBWM_SKIN_H



#include "sgui_subwm_predef.h"



#define SGUI_WINDOW_BACKGROUND 0
#define SGUI_WINDOW_TOPMOST 1
#define SGUI_WINDOW_DRAGING 2

#define SGUI_WINDOW_TOP_LEFT 0
#define SGUI_WINDOW_TOP_RIGHT 1
#define SGUI_WINDOW_BOTTOM_LEFT 2
#define SGUI_WINDOW_BOTTOM_RIGHT 3



/**
 * \interface sgui_subwm_skin
 *
 * \brief Abstract representation of a window skin
 */
struct sgui_subwm_skin
{
    /**
     * \brief Get the dimensions of the skin texture for ctx windows
     *
     * \param skin   A pointer to the skin object
     * \param width  Returns the width of the texture
     * \param height Returns the height of the texture
     */
    void (* get_ctx_skin_texture_size )( sgui_subwm_skin* skin,
                                         unsigned int* width,
                                         unsigned int* height );

    /**
     * \brief Get a pointer to the static data to upload to the skin texture
     *        for ctx windows
     *
     * \param skin A pointer to the skin object
     *
     * \return A pointer to 8 bit per channel RGBA color data
     */
    const unsigned char* (* get_ctx_skin_texture )( sgui_subwm_skin* skin );

    /**
     * \brief Get the area of a window corner on the skin texture
     *
     * \param skin A pointer to the skin object
     * \param rect Returns the region on the window texture
     * \param type SGUI_WINDOW_TOP_LEFT for top left corner,
     *             SGUI_WINDOW_TOP_RIGHT for top right corner, and so on...
     */
    void (* get_ctx_window_corner )( sgui_subwm_skin* skin, sgui_rect* rect,
                                     int type );

    /**
     * \brief Get the desired alpha transparency for a type of window
     *
     * \param skin A pointer to the skin object
     * \param type Either SGUI_WINDOW_BACKGROUND for a window that is not
     *             top-most, SGUI_WINDOW_TOPMOST for the currently topmost
     *             window, or SGUI_WINDOW_DRAGING for a window that is
     *             currently being draged around by the user.
     *
     * \return An alpha value between 0 and 255
     */
    int (* get_window_transparency )( sgui_subwm_skin* skin, int type );

    /**
     * \brief Readraw a window title bar
     *
     * \param skin  A pointer to the skin object
     * \param cv    A pointer to a canvas to draw to
     * \param title The title to draw into the window
     */
    void (* draw_title_bar )( sgui_subwm_skin* skin, sgui_canvas* cv,
                              const char* title );

    /**
     * \brief Get the height of a window title bar in pixels
     *
     * \param skin  A pointer to the skin object
     *
     * \return The height of the title bar
     */
    unsigned int (* get_titlebar_height )( sgui_subwm_skin* skin );
};



#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Get a pointer to the currently set subwm skin
 *
 * \memberof sgui_subwm_skin
 *
 * \return A pointer to the currently set skin
 */
sgui_subwm_skin* sgui_subwm_skin_get( void );

/**
 * \brief Change the currently set subwm skin
 *
 * \memberof sgui_subwm_skin
 *
 * \param skin A pointer to the new skin, or NULL to restore to
 *             the built in default skin.
 */
void sgui_subwm_skin_set( sgui_subwm_skin* skin );

#ifdef __cplusplus
}
#endif

#endif /* SGUI_SUBWM_SKIN_H */

