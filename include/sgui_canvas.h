/*
 * sgui_canvas.h
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
#ifndef SGUI_CANVAS_H
#define SGUI_CANVAS_H



#include "sgui_predef.h"



#ifdef __cplusplus
extern "C"
{
#endif



typedef enum
{
    SCF_RGB8 = 0,
    SCF_RGBA8
}
SGUI_COLOR_FORMAT;



/**
 * \brief Create a canvas object
 *
 * \param width  The width of the canvas
 * \param height The height of the canvas
 *
 * \return A pointer to a new canvas object
 */
sgui_canvas* sgui_canvas_create( unsigned int width, unsigned int height );

/** \brief Destroy a canvas object */
void sgui_canvas_destroy( sgui_canvas* canvas );

/**
 * \brief Change the size of a canvas
 *
 * \param width  The new width of the canvas
 * \param height The new height of the canvas
 */
void sgui_canvas_resize( sgui_canvas* canvas, unsigned int width,
                         unsigned int height );




/**
 * \brief Get the size of a canvas
 *
 * \param canvas The canvas to report the size of
 * \param width  Returns the width of the canvas
 * \param height Returns the height of the canvas
 */
void sgui_canvas_get_size( sgui_canvas* canvas, unsigned int* width,
                           unsigned int* height );

/**
 * \brief Set the background color of the canvas
 *
 * \param color The background color
 */
void sgui_canvas_set_background_color( sgui_canvas* canvas,
                                       unsigned char* color );



/**
 * \brief Call to start drawing to an area on the canvas
 *
 * A call to this function enables redrawing of a certain portion of a canvas.
 * Call sgui_canvas_end( ) when you are done drawing. Drawing outside begin
 * and end is not possible (except clearing).
 *
 * To support nested widget managers, multiple succeeding begin calls are
 * ignored, but the end function must be called for each begin call.
 *
 * The begin function resets the drawing state of the canvas (i.e. scissor
 * rect and offset, stacks are cleared). The scissor rect is immediately set
 * to the given rect.
 *
 * \param r The rectangle to draw to
 */
void sgui_canvas_begin( sgui_canvas* canvas, sgui_rect* r );

/**
 * \brief Call when you are done with drawing
 *
 * \see sgui_canvas_begin
 */
void sgui_canvas_end( sgui_canvas* canvas );



/**
 * \brief Clear a rectangular region on the canvas to the background color
 *
 * \param r The area to clear
 */
void sgui_canvas_clear( sgui_canvas* canvas, sgui_rect* r );


/**
 * \brief Set the scissor rect of a canvas
 *
 * Rendering to a canvas is only allowed inside the scissor rect, everything
 * outside will be clipped.
 *
 * Scissor rects are stackable, a new rect will be merged with the current set
 * and the current stored on a stack, to restore the current one, simply set
 * all parameters to zero.
 *
 * \param r The scissor rect to apply or NULL to restore the previous one
 */
void sgui_canvas_set_scissor_rect( sgui_canvas* canvas, sgui_rect* r );

/** \brief Enable or disable background clearing */
void sgui_canvas_allow_clear( sgui_canvas* canvas, int clear );

/** \brief Returns non-zero if background clearing is enabled */
int sgui_canvas_is_clear_allowed( sgui_canvas* canvas );

/**
 * \brief Set an offset added to all drawing and scissor positions
 *
 * Offsets are stackable, a new offset will be added to the current one
 * and the current stored on a stack, to restore the current one, simply call
 * sgui_canvas_restore_offset.
 *
 * \param x The distance from the left of the canvas.
 * \param y The distance from the top of the canvas.
 */
void sgui_canvas_set_offset( sgui_canvas* canvas, int x, int y );

/** \brief Restores the previous offset of a canvas */
void sgui_canvas_restore_offset( sgui_canvas* canvas );

/**
 * \brief Blit an image onto a canvas
 *
 * \param x      Distance from the left of the canvas to the left of the image
 * \param y      Distance from the top of the canvas to the top of the image
 * \param width  Width of the image
 * \param height Width of the image
 * \param format Color format of the image
 * \param data   Pointer to the image data
 */
void sgui_canvas_blit( sgui_canvas* canvas, int x, int y, unsigned int width,
                       unsigned int height, SGUI_COLOR_FORMAT format,
                       const void* data );

/**
 * \brief Blend an image onto a canvas
 *
 * \param x      Distance from the left of the canvas to the left of the image
 * \param y      Distance from the top of the canvas to the top of the image
 * \param width  Width of the image
 * \param height Width of the image
 * \param format Color format of the image (must be a format with an alpha
 *               channel)
 * \param data   Pointer to the image data
 */
void sgui_canvas_blend( sgui_canvas* canvas, int x, int y, unsigned int width,
                        unsigned int height, SGUI_COLOR_FORMAT format,
                        const void* data );

/**
 * \brief Draw a rectangle onto a canvas
 *
 * \param r      The geometry of the box to draw
 * \param color  The color to draw the box in
 * \param format The color format stored in the color array
 */
void sgui_canvas_draw_box( sgui_canvas* canvas, sgui_rect* r,
                           unsigned char* color, SGUI_COLOR_FORMAT format );

/**
 * \brief Draw a line onto a canvas
 *
 * \param x          Distance from the left of the canvas to the start of the
 *                   line.
 * \param y          Distance from the top of the canvas to the start of the
 *                   line.
 * \param length     The length of the line in the desired direction
 * \param horizontal Non-zero to draw a horizontal line, zero to draw a
 *                   vertical line.
 * \param color      The color to draw the line in
 * \param format     The color format stored in the color array
 */
void sgui_canvas_draw_line( sgui_canvas* canvas, int x, int y,
                            unsigned int length, int horizontal,
                            unsigned char* color, SGUI_COLOR_FORMAT format );

/**
 * \brief Perform a stencil blend on a canvas (e.g. glyph rendering)
 *
 * \param buffer A monochrome buffer holding coverage values that are used to
 *               blend the given RGB color onto the canvas
 * \param x      Distance from the left of the canvas to the left of the
 *               object
 * \param y      Distance from the top of the canvas to the top of the object
 * \param w      The width of the buffer
 * \param h      The height of the buffer
 * \param color  The color to draw
 *
 * \return A negative value if the target was to the left or above the drawing
 *         area, a positive value if it was to the right or below and zero
 *         otherwise.
 */
int sgui_canvas_blend_stencil( sgui_canvas* canvas, unsigned char* buffer,
                               int x, int y, unsigned int w, unsigned int h,
                               unsigned char* color );

#ifdef __cplusplus
}
#endif

#endif /* SGUI_CANVAS_H */

