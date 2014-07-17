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

/**
 * \file sgui_canvas.h
 *
 * This file contains the interface functions for the canvas datatype.
 */
#ifndef SGUI_CANVAS_H
#define SGUI_CANVAS_H



#include "sgui_predef.h"
#include "sgui_widget.h"
#include "sgui_rect.h"



#define CANVAS_MAX_DIRTY 10



struct sgui_canvas
{
    int ox, oy;                     /**< \brief current offset */
    unsigned int width, height;     /**< \brief Size of the canvas */
    sgui_rect sc;                   /**< \brief current scissor rect */
    int began;
    sgui_widget root;               /**< \brief The dummy root widget */
    sgui_widget* mouse_over;        /**< \brief The widget under the mouse
                                                cursor */
    sgui_widget* focus;             /**< \brief The widget with keyboad
                                                focus */

    int draw_focus;     /**< \brief Non-zero if focus box should be drawn */

    sgui_rect dirty[ CANVAS_MAX_DIRTY ];
    unsigned int num_dirty;

    /** \copydoc sgui_canvas_destroy */
    void(* destroy )( sgui_canvas* canvas );

    /**
     * \brief Gets called by sgui_canvas_resize
     *
     * Can be set to NULL if not needed by the implementation.
     *
     * \param canvas A pointer to the canvas.
     * \param width  The new width of the canvas
     * \param height The new height of the canvas
     */
    void(* resize )( sgui_canvas* canvas, unsigned int width,
                     unsigned int height );

    /**
     * \brief Gets called by sgui_canvas_create_pixmap
     *
     * \param canvas A pointer to the canvas
     * \param width  The width of the pixmap
     * \param height The height of the pixmap
     * \param format The color format of the pixmap
     */
    sgui_pixmap* (* create_pixmap )( sgui_canvas* canvas, unsigned int width,
                                     unsigned int height, int format );

    /**
     * \brief Gets called by sgui_canvas_begin
     *
     * Can be set to NULL if not needed by the implementation.
     *
     * \param canvas A pointer to the canvas.
     * \param r      The rectangle to redraw (already clamped to the canvas)
     */
    void(* begin )( sgui_canvas* canvas, sgui_rect* r );

    /**
     * \brief Gets called by sgui_canvas_end
     *
     * Can be set to NULL if not needed by the implementation.
     *
     * \param canvas A pointer to the canvas.
     */
    void(* end )( sgui_canvas* canvas );

    /**
     * \brief Clear a portion of a canvas
     *
     * \param canvas A pointer to the canvas.
     * \param r      The region to clear, with ofset applied and cliped with
     *               the scissor rect
     */
    void(* clear )( sgui_canvas* canvas, sgui_rect* r );

    /**
     * \brief Draw a box onto a canvas
     *
     * \param canvas  A pointer to the canvas.
     * \param r       The rect to draw (offset applied and clipped)
     * \param color   The color to draw the rect in
     * \param format  The format of the color
     */
    void(* draw_box )( sgui_canvas* canvas, sgui_rect* r,
                       unsigned char* color, int format );

    /**
     * \brief Blit onto a canvas
     *
     * \param canvas  A pointer to the canvas.
     * \param x       Distance from the left of the canvas
     * \param y       Distance from the top of the canvas
     * \param pixmap  The pixmap to blend.
     * \param srcrect A subrectangle of the pixmap to blit.
     */
    void(* blit )( sgui_canvas* canvas, int x, int y, sgui_pixmap* pixmap,
                   sgui_rect* srcrect );

    /**
     * \brief Blend onto a canvas
     *
     * \param canvas  A pointer to the canvas.
     * \param x       Distance from the left of the canvas
     * \param y       Distance from the top of the canvas
     * \param pixmap  The pixmap to blend.
     * \param srcrect A subrectangle of the pixmap to blend.
     */
    void(* blend )( sgui_canvas* canvas, int x, int y, sgui_pixmap* pixmap,
                    sgui_rect* srcrect );

    /**
     * \brief Blend a constant color onto a canvas, use alpha from pixmap
     *
     * This method is only used internally by the glyph cache.
     *
     * \param canvas  A pointer to the canvas.
     * \param x       Distance from the left of the canvas
     * \param y       Distance from the top of the canvas
     * \param pixmap  The pixmap to blend.
     * \param srcrect A subrectangle of the pixmap to blend.
     * \param color   The constant RGB color.
     */
    void (* blend_glyph )( sgui_canvas* canvas, int x, int y,
                           sgui_pixmap* pixmap, sgui_rect* r,
                           unsigned char* color );

    /**
     * \brief Draw a string of text onto a canvas
     *
     * \param canvas A pointer to the canvas.
     * \param x      The distance from the left of the canvas
     * \param y      The distance from the top of the canvas
     * \param font   The font face to use for rendering
     * \param color  The RGB color to use for rendering
     * \param text   The UTF8 string to render
     * \param length The number of bytes to read from the string
     *
     * \return The length of the rendered string in pixels.
     */
    int(* draw_string )( sgui_canvas* canvas, int x, int y, sgui_font* font,
                         unsigned char* color, const char* text,
                         unsigned int length );
};



#define SGUI_A8    0
#define SGUI_RGB8  1
#define SGUI_RGBA8 2



#ifdef __cplusplus
extern "C"
{
#endif

/**
 * \brief Used by canvas implementations to initialise the canvas base struct
 *
 * \param cv     A pointer to the canvas structure
 * \param width  The width of the canvas
 * \param height The height of the canvas
 */
SGUI_DLL void sgui_canvas_init( sgui_canvas* cv, unsigned int width,
                                unsigned int height );

/** \brief Destroy a canvas */
SGUI_DLL void sgui_canvas_destroy( sgui_canvas* canvas );

/**
 * \brief Get the root node of a canvas
 *
 * \param canvas A pointer to the canvas
 *
 * \return A pointer to the root widget
 */
SGUI_DLL sgui_widget* sgui_canvas_get_root( const sgui_canvas* canvas );

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
SGUI_DLL unsigned int sgui_canvas_num_dirty_rects(const sgui_canvas* canvas);

/**
 * \brief Get a dirty rectangle from a canvas by index
 *
 * \param canvas The canvas
 * \param rect   A pointer to a rectangle to write to
 * \param i      The index of the dirty rectangle
 */
SGUI_DLL void sgui_canvas_get_dirty_rect( const sgui_canvas* canvas,
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
                                             const sgui_event* e );

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
SGUI_DLL void sgui_canvas_get_size( const sgui_canvas* canvas,
                                    unsigned int* width,
                                    unsigned int* height );

/**
 * \brief Create a pixmap
 *
 * This function creates a pixmap. A pixmap is a server side memory area that
 * can hold a picture wich can be displayed on a canvas.
 * A pixmap can only be createad by a canvas and is only valid for the canvas
 * it was created for.
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
 * \brief Get a pointer to the currently used drawing scissor rectangle
 *
 * \param canvas The canvas to get the scissor rectangle from
 * \param r      Returns the scissor rectangle
 */
SGUI_DLL void sgui_canvas_get_scissor_rect( const sgui_canvas* canvas,
                                            sgui_rect* r );

/**
 * \brief Set the currently used drawing scissor rectangle of a canvas
 *
 * \param canvas The canvas of which to set the scissor rectangle
 * \param r      A pointer to the new scissor rectangle
 */
SGUI_DLL void sgui_canvas_set_scissor_rect( sgui_canvas* canvas,
                                            const sgui_rect* r );

/**
 * \brief Get the currently used drawing offset from a canvas
 *
 * \param canvas The canvas of which to get the drawing offset
 * \param x      Returns the horizontal offset
 * \param y      Returns the vertical offset
 */
SGUI_DLL void sgui_canvas_get_offset( const sgui_canvas* canvas,
                                      int* x, int* y );

/**
 * \brief Set an offset added to all positions of canvas drawing functions
 *
 * \param canvas The canvas of which to set the drawing offset
 * \param x      The horizontal offset
 * \param y      The vertical offset
 */
SGUI_DLL void sgui_canvas_set_offset( sgui_canvas* canvas, int x, int y );

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
SGUI_DLL void sgui_canvas_begin( sgui_canvas* canvas, const sgui_rect* r );

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
 * \brief Draw an image onto a canvas
 *
 * \param x      Distance from the left of the canvas to the left of the image
 * \param y      Distance from the top of the canvas to the top of the image
 * \param pixmap The pixmap to draw onto the canvas
 * \param srcrect A sub rect within the source image, or NULL to use the
 *                uncropped source image.
 * \param blend  Non-zero to blend the image, zero for simple blitting
 */
SGUI_DLL void sgui_canvas_draw_pixmap( sgui_canvas* canvas, int x, int y,
                                       sgui_pixmap* pixmap,
                                       sgui_rect* srcrect, int blend );

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

