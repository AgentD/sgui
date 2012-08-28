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



#include "sgui_font_manager.h"



#ifdef __cplusplus
extern "C"
{
#endif



typedef enum
{
    SCF_RGB8 = 0,
    SCF_RGBA8,

    SCF_BGR8,
    SCF_BGRA8
}
SGUI_COLOR_FORMAT;



typedef struct sgui_canvas sgui_canvas;



/**
 * \brief Create a canvas object
 *
 * \param width  The widht of the canvas
 * \param height The height of the canvas
 * \param format The color format of the canvas
 *
 * \return A pointer to a new canvas object
 */
sgui_canvas* sgui_canvas_create( unsigned int width, unsigned int height,
                                 SGUI_COLOR_FORMAT format );

/**
 * \brief Create a canvas object that uses a memory buffer, ownership of the
 *        buffer remains with the caller
 *
 * \param buffer The buffer to use
 * \param width  The widht of the canvas
 * \param height The height of the canvas
 * \param format The color format of the buffer
 *
 * \return A pointer to a new canvas object
 */
sgui_canvas* sgui_canvas_create_use_buffer( void* buffer,
                                            unsigned int width,
                                            unsigned int height,
                                            SGUI_COLOR_FORMAT format );

/** \brief Destroy a canvas object */
void sgui_canvas_destroy( sgui_canvas* canvas );

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
 * \brief Resize a canvas that does not use an external buffer, losing all
 *        previous contents of the canvas.
 *
 * \param width  The new width of the canvas
 * \param height The new height of the canvas
 */
void sgui_canvas_resize( sgui_canvas* canvas, unsigned int width,
                         unsigned int height );

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
 * \param x      Distance from the left of the canvas to the left of the box
 * \param y      Distance from the top of the canvas to the top of the box
 * \param width  The width of the box
 * \param height The height of the box
 * \param color  The color to draw the box in
 * \param format The color format stored in the color array
 */
void sgui_canvas_draw_box( sgui_canvas* canvas, int x, int y,
                           unsigned int width, unsigned int height,
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

void sgui_canvas_draw_text_plain( sgui_canvas* canvas, int x, int y,
                                  sgui_font* font_face, unsigned int height,
                                  unsigned char* color,
                                  SGUI_COLOR_FORMAT format,
                                  const unsigned char* text,
                                  unsigned int length,
                                  unsigned int max_width );

void sgui_canvas_draw_text( sgui_canvas* canvas, int x, int y,
                            sgui_font* font_norm, sgui_font* font_bold,
                            sgui_font* font_ital, sgui_font* font_boit,
                            unsigned int font_height, unsigned char* color,
                            SGUI_COLOR_FORMAT format,
                            const unsigned char* text );

void* sgui_canvas_get_raw_data( sgui_canvas* canvas );

void sgui_canvas_set_raw_data( sgui_canvas* canvas, SGUI_COLOR_FORMAT format,
                               unsigned int width, unsigned int height,
                               void* data );



#ifdef __cplusplus
}
#endif

#endif /* SGUI_CANVAS_H */

