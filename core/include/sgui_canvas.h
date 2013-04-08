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



#define SGUI_RGB8  0
#define SGUI_RGBA8 1



/** \brief Destroy a canvas */
SGUI_DLL void sgui_canvas_destroy( sgui_canvas* canvas );

/**
 * \brief Change the size of a canvas
 *
 * \param canvas The canvas to resize
 * \param width  The width of the canvas
 * \param height The height of the canvas
 */
SGUI_DLL void sgui_canvas_resize( sgui_canvas* canvas, unsigned int width,
                                  unsigned int height );



/**
 * \brief Get the size of a canvas
 *
 * \param canvas The canvas to report the size of
 * \param width  Returns the width of the canvas
 * \param height Returns the height of the canvas
 */
SGUI_DLL void sgui_canvas_get_size( sgui_canvas* canvas, unsigned int* width,
                                    unsigned int* height );

/**
 * \brief Set the background color of the canvas
 *
 * \param color The background color
 */
SGUI_DLL void sgui_canvas_set_background_color( sgui_canvas* canvas,
                                                unsigned char* color );



/**
 * \brief Call to start drawing to an area on the canvas
 *
 * A call to this function enables redrawing of a certain portion of a canvas.
 * Call sgui_canvas_end( ) when you are done drawing. Drawing outside begin
 * and end is not possible.
 *
 * Calling the begin function more than once has no effect. One call to end is
 * required, regardless of how often begin has been called.
 *
 * The begin function resets the drawing state of the canvas (i.e. scissor
 * rect and offset, stacks are cleared). The scissor rect is immediately set
 * to the given rect.
 *
 * \param r The rectangle to draw to
 */
SGUI_DLL void sgui_canvas_begin( sgui_canvas* canvas, sgui_rect* r );

/**
 * \brief Call when you are done with drawing
 *
 * \see sgui_canvas_begin
 */
SGUI_DLL void sgui_canvas_end( sgui_canvas* canvas );



/**
 * \brief Clear a rectangular region on the canvas to the background color
 *
 * \param r The area to clear
 */
SGUI_DLL void sgui_canvas_clear( sgui_canvas* canvas, sgui_rect* r );


/**
 * \brief Set the current scissor rect of a canvas in absolute coordinates
 *
 * Rendering to a canvas is only allowed inside the scissor rect, everything
 * outside will be clipped.
 *
 * This function sets the current scissor rect in absolute coordinates
 * (i.e. relative to the upper left corner, no offset).
 *
 * \param r A pointer to the rect to set as new scissor rect
 */
SGUI_DLL void sgui_canvas_set_scissor_rect( sgui_canvas* canvas,
                                            sgui_rect* r );

/**
 * \brief Get the current scissor rect of a canvas in absolute coordinates
 *
 * Rendering to a canvas is only allowed inside the scissor rect, everything
 * outside will be clipped.
 *
 * This function returns the current scissor rect in absolute coordinates
 * (i.e. relative to the upper left corner, no offset).
 *
 * \param r A pointer to the rect that returns the current scissor rect
 */
SGUI_DLL void sgui_canvas_get_scissor_rect( sgui_canvas* canvas,
                                            sgui_rect* r );

/**
 * \brief Merge the current scissor rect of a canvas with a given rect
 *
 * Rendering to a canvas is only allowed inside the scissor rect, everything
 * outside will be clipped.
 *
 * This function takes a rectangle, shifts it by the current offset of the
 * canvas and then sets the scissor rect of the canvas to the intersection
 * of the rect with the current scissor rect.
 *
 * \param r A pointer to the rect that returns the current scissor rect
 */
SGUI_DLL void sgui_canvas_merge_scissor_rect( sgui_canvas* canvas,
                                              sgui_rect* r );

/**
 * \brief Set an offset added to all drawing and scissor positions
 *
 * \param x The distance from the left of the canvas.
 * \param y The distance from the top of the canvas.
 */
SGUI_DLL void sgui_canvas_set_offset( sgui_canvas* canvas, int x, int y );

/**
 * \brief Add an offset to the current offset added to all drawing and
 *        scissor positions
 *
 * \param x A horizontal offset to add
 * \param y A vertical offset to add
 */
SGUI_DLL void sgui_canvas_add_offset( sgui_canvas* canvas, int x, int y );

/**
 * \brief Get the current offset added to all drawing and scissor positions
 *
 * \param x Returns the distance from the left of the canvas.
 * \param y Returns the distance from the top of the canvas.
 */
SGUI_DLL void sgui_canvas_get_offset( sgui_canvas* canvas, int* x, int* y );

/**
 * \brief Blit an image onto a canvas
 *
 * \param x      Distance from the left of the canvas to the left of the image
 * \param y      Distance from the top of the canvas to the top of the image
 * \param pixmap The pixmap to blit onto the canvas
 * \param srcrect A sub rect within the source image, or NULL to use the
 *                uncropped source image.
 */
SGUI_DLL void sgui_canvas_blit( sgui_canvas* canvas, int x, int y,
                                sgui_pixmap* pixmap, sgui_rect* srcrect );

/**
 * \brief Blend an image onto a canvas
 *
 * \param x      Distance from the left of the canvas to the left of the image
 * \param y      Distance from the top of the canvas to the top of the image
 * \param pixmap The pixmap to blend onto the canvas
 * \param srcrect A sub rect within the source image, or NULL to use the
 *                uncropped source image.
 */
SGUI_DLL void sgui_canvas_blend( sgui_canvas* canvas, int x, int y,
                                 sgui_pixmap* pixmap, sgui_rect* srcrect );

/**
 * \brief Draw a rectangle onto a canvas
 *
 * \param r      The geometry of the box to draw
 * \param color  The color to draw the box in
 * \param format The color format stored in the color array (SGUI_RGB8, ...)
 */
SGUI_DLL void sgui_canvas_draw_box( sgui_canvas* canvas, sgui_rect* r,
                                    unsigned char* color,
                                    int format );

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
 * \param format     The color format stored in the color array(SGUI_RGB8, ...)
 */
SGUI_DLL void sgui_canvas_draw_line( sgui_canvas* canvas, int x, int y,
                                     unsigned int length, int horizontal,
                                     unsigned char* color, int format );

/**
 * \brief Render one line of text in a single font face
 *
 * \param x         Distance from the left of the canvas to the left of the
 *                  text.
 * \param y         Distance from the top of the canvas to the top of the
 *                  text.
 * \param bold      Whether to draw the text bold.
 * \param italic    Whether to draw the text italic.
 * \param color     The RGB color to draw the text in.
 * \param text      The text to draw.
 * \param length    The number of bytes to read from the text.
 *
 * \return The horizontal extents of the rendered text in pixels
 */
SGUI_DLL int sgui_canvas_draw_text_plain( sgui_canvas* canvas, int x, int y,
                                          int bold, int italic,
                                          unsigned char* color,
                                          const char* text,
                                          unsigned int length );

/**
 * \brief Render a multi line text that uses html like tags to switch color
 *        or font face, using the default fonts from the skinning system.
 *
 * \param x           Distance from the left of the text to the left of the
 *                    canvas.
 * \param y           Distance from the top of the text to the top of the
 *                    canvas.
 * \param text        The UTF8 text to print. The LF ('\n') character can be
 *                    used for line wraps, the \<b\> \</b\> and \<i\> \</i\>
 *                    for writing text bold or italic. A \<color="#RRGGBB"\>
 *                    tag can be used to switch text color, where the value
 *                    "default" for color switches back to default color.
 */
SGUI_DLL void sgui_canvas_draw_text( sgui_canvas* canvas, int x, int y,
                                     const char* text );

#ifdef __cplusplus
}
#endif

#endif /* SGUI_CANVAS_H */

