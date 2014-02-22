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



#define SGUI_A8    0
#define SGUI_RGB8  1
#define SGUI_RGBA8 2



/** \brief Destroy a canvas */
SGUI_DLL void sgui_canvas_destroy( sgui_canvas* canvas );

/**
 * \brief Get the root node of a canvas
 *
 * \param canvas A pointer to the canvas
 *
 * \return A pointer to the root widget
 */
SGUI_DLL sgui_widget* sgui_canvas_get_root( sgui_canvas* canvas );

/**
 * \brief Override the widget that currently has keyboard focus
 *
 * \param canvas A pointer to a canvas object
 * \param widget A pointer to a widget that should receive focus instead, or
 *               NULL to drop focus alltogether
 */
SGUI_DLL void sgui_canvas_set_focus( sgui_canvas* canvas,
                                     sgui_widget* widget );

/**
 * \brief Add a dirty rect (area that needs redraw) to a canvas
 *
 * \param canvas The canvas
 * \param r      The dirty rectangle
 */
SGUI_DLL void sgui_canvas_add_dirty_rect( sgui_canvas* canvas, sgui_rect* r );

/**
 * \brief Get the number of dirty rectangles from a canvas
 *
 * \param canvas The canvas
 *
 * \return The number of dirty rectangles
 */
SGUI_DLL unsigned int sgui_canvas_num_dirty_rects( sgui_canvas* canvas );

/**
 * \brief Get a dirty rectangle from a canvas by index
 *
 * \param canvas The canvas
 * \param rect   A pointer to a rectangle to write to
 * \param i      The index of the dirty rectangle
 */
SGUI_DLL void sgui_canvas_get_dirty_rect( sgui_canvas* canvas,
                                          sgui_rect* rect, unsigned int i );

/**
 * \brief Clear the dirty rects of a canvas
 *
 * \param canvas The canvas
 */
SGUI_DLL void sgui_canvas_clear_dirty_rects( sgui_canvas* canvas );

/**
 * \brief Redraw all visible widgets of a canvas that are within areas flaged
 *        as dirty.
 *
 * The dirty areas are cleared after a call to this function. The function can
 * be called outside a begin-end block and calls begin-end itself if required.
 *
 * \param canvas The canvas
 * \param clear  If non-zero the given area is cleared before redrawing
 */
SGUI_DLL void sgui_canvas_redraw_widgets( sgui_canvas* canvas, int clear );

/**
 * \brief Redraw all visible widgets of a canvas.
 *
 * The dirty areas are cleared after a call to this function. The function can
 * calls be called outside a begin-end block and calls begin-end itself if
 * required.
 *
 * \param canvas The canvas
 * \param clear  If non-zero the canvas is cleared before drawing
 */
SGUI_DLL void sgui_canvas_draw_widgets( sgui_canvas* canvas, int clear );

/**
 * \brief Send a window event to all widgets held by a canvas
 *
 * The canvas automatically manages keyboard focus and generates mouse
 * enter, mouse leave, focus and focus lost events for the widgets it holds.
 *
 * \param canvas The canvas
 * \param e      The event data to send
 */
SGUI_DLL void sgui_canvas_send_window_event( sgui_canvas* canvas,
                                             sgui_event* e );

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
 * \brief Create a pixmap
 *
 * This function creates a pixmap. A pixmap is a server side memory area that
 * can hold a picture wich can be displayed on a canvas.
 * A pixmap can only be createad by a canvas and is only valid for the canvas
 * it was created for.
 *
 * \note Creating a pixmap for an OpenGL backend requires that a context is
 *       made current.
 *
 * \param canvas  The canvas for which to create the pixmap
 * \param width   The width of the pixmap in pixels
 * \param height  The height of the pixmap in pixels
 * \param format  The color format used by the pixmap (SGUI_RGB8, SGUI_RGBA8,
 *                etc...)
 *
 * \return A pointer to a new pixmap object on success, NULL on error.
 */
SGUI_DLL sgui_pixmap* sgui_canvas_create_pixmap( sgui_canvas* canvas,
                                                 unsigned int width,
                                                 unsigned int height,
                                                 int format );


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
 * \brief Clear a rectangular region on the canvas
 *
 * \param r The area to clear
 */
SGUI_DLL void sgui_canvas_clear( sgui_canvas* canvas, sgui_rect* r );

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




/**
 * \brief Create an instance of a reference implementation of a canvas that
 *        uses a memory buffer
 *
 * \param buffer A pointer to a buffer to draw to
 * \param width  The width of the memory buffer
 * \param height The height of the memory buffer
 * \param format The color format of the memory buffer
 * \param swaprb If non-zero (TRUE), red and blue are swapped
 *
 * \return A pointer to a canvas on success, NULL on failure
 */
SGUI_DLL sgui_canvas* sgui_memory_canvas_create( unsigned char* buffer,
                                                 unsigned int width,
                                                 unsigned int height,
                                                 int format,
                                                 int swaprb );

/**
 * \brief Set a new buffer pointer for a memory canvas
 *
 * \param canvas A pointer to a memory canvas object
 * \param buffer A pointer to a buffer to draw to
 */
SGUI_DLL void sgui_memory_canvas_set_buffer( sgui_canvas* canvas,
                                             unsigned char* buffer );

#ifdef __cplusplus
}
#endif

#endif /* SGUI_CANVAS_H */

