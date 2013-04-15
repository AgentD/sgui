/*
 * sgui_internal.h
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
#ifndef SGUI_INTERNAL_H
#define SGUI_INTERNAL_H



#include "sgui_predef.h"
#include "sgui_rect.h"
#include "sgui_window.h"
#include "sgui_canvas.h"



struct sgui_canvas
{
    int ox, oy;                     /**< \brief current offset */

    unsigned int width, height;     /**< \brief Size of the canvas */

    sgui_rect sc;                   /**< \brief current scissor rect */

    int began;

    unsigned char bg_color[3];      /**< \brief RGB8 background color */


    /**
     * \brief Gets called by sgui_canvas_destroy
     *
     * For C++ people: This is the destructur (but it also has to free up the
     * memory used).
     *
     * \param canvas A pointer to the canvas (for C++ people: practically a
     *               this pointer).
     */
    void(* destroy )( sgui_canvas* canvas );

    /**
     * \brief Gets called by sgui_canvas_resize
     *
     * Can be set to NULL if not needed by the implementation.
     *
     * \param canvas A pointer to the canvas (for C++ people: practically a
     *               this pointer).
     * \param width  The new width of the canvas
     * \param height The new height of the canvas
     */
    void(* resize )( sgui_canvas* canvas, unsigned int width,
                     unsigned int height );

    /**
     * \brief Gets called by sgui_canvas_begin
     *
     * Can be set to NULL if not needed by the implementation.
     *
     * \param canvas A pointer to the canvas (for C++ people: practically a
     *               this pointer).
     * \param r      The rectangle to redraw (already clamped to the canvas)
     */
    void(* begin )( sgui_canvas* canvas, sgui_rect* r );

    /**
     * \brief Gets called by sgui_canvas_end
     *
     * Can be set to NULL if not needed by the implementation.
     *
     * \param canvas A pointer to the canvas (for C++ people: practically a
     *               this pointer).
     */
    void(* end )( sgui_canvas* canvas );

    /**
     * \brief Clear a portion of a canvas to the background color
     *
     * \param canvas A pointer to the canvas (for C++ people: practically a
     *               this pointer).
     * \param r      The region to clear, with ofset applied and cliped with
     *               the scissor rect
     */
    void(* clear )( sgui_canvas* canvas, sgui_rect* r );

    /**
     * \brief Blit onto a canvas
     *
     * \param canvas          A pointer to the canvas (for C++
     *                        people: practically a this pointer).
     * \param x               Distance from the left of the canvas
     * \param y               Distance from the top of the canvas
     * \param width           The number of pixels per scanline after clipping
     * \param height          The number of scanlines to blit after clipping
     * \param scanline_length The actual length of one scanline of the image
     * \param format          The color format of the pixels
     * \param data            The data to blit. Already advanced to the actual
     *                        first pixel after clipping.
     */
    void(* blit )( sgui_canvas* canvas, int x, int y, sgui_pixmap* pixmap,
                   sgui_rect* srcrect );

    /**
     * \brief Blend onto a canvas
     *
     * \param canvas          A pointer to the canvas (for C++
     *                        people: practically a this pointer).
     * \param x               Distance from the left of the canvas
     * \param y               Distance from the top of the canvas
     * \param width           The number of pixels per scanline after clipping
     * \param height          The number of scanlines to blit after clipping
     * \param scanline_length The actual length of one scanline of the image
     * \param data            The data to blit. Already advanced to the actual
     *                        first pixel after clipping.
     */
    void(* blend )( sgui_canvas* canvas, int x, int y, sgui_pixmap* pixmap,
                    sgui_rect* srcrect );

    /**
     * \brief Draw a box onto a canvas
     *
     * \param canvas  A pointer to the canvas (for C++ people: practically a
     *                this pointer).
     * \param r       The rect to draw (offset applied and clipped)
     * \param color   The color to draw the rect in
     * \param format  The format of the color
     */
    void(* draw_box )( sgui_canvas* canvas, sgui_rect* r,
                       unsigned char* color, int format );

    /**
     * \brief Draw a string of text onto a canvas
     *
     * \param canvas A pointer to the canvas (for C++ people: practically a
     *               this pointer).
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

struct sgui_widget
{
    sgui_rect area;         /**< \brief The area occupied by a widget */

    int visible;            /**< \brief zero if the widget should not be
                                        rendered */

    /** \brief The widget manager responsible for that widget */
    sgui_widget_manager* mgr;

    /**
     * \brief The next widget on the same level (linked list)
     *
     * Used by widget manager. Do not alter in widget code!
     */
    sgui_widget* next;

    /**
     * \brief A pointer to the first widget in the children list
     *
     * Used by widget manager. Do not alter in widget code!
     */
    sgui_widget* children;

    /**
     * \brief A pointer to the parent widget
     *
     * Used by widget manager. Do not alter in widget code!
     */
    sgui_widget* parent;

    /**
     * \brief Callback that is called to destroy a widget
     *
     * \param widget A pointer to the widget to destroy (for C++
     *               people: practically a this pointer).
     */
    void (* destroy )( sgui_widget* widget );

    /**
     * \brief Callback that is called to draw a widget
     *
     * \param widget A pointer to the widget to draw (for C++
     *               people: practically a this pointer).
     * \param cv     A pointer to a canvas to draw to.
     */
    void (* draw_callback )( sgui_widget* widget, sgui_canvas* cv );

    /**
     * \brief Callback that is called to inject window events
     *
     * \param widget A pointer to the widget to update (for C++
     *               people: practically a this pointer).
     * \param type   The event type.
     * \param event  The window event that occoured.
     */
    void (* window_event_callback )( sgui_widget* widget, int type,
                                     sgui_event* event );

    /**
     * \brief Callback that is called when the internal state of a widget
     *        changes(e.g. position, visibility, etc...)
     * 
     */
    void (* state_change_callback )( sgui_widget* widget );
};

struct sgui_window
{
    sgui_widget_manager* mgr;   /**< \brief pointer to a widget manager */
    sgui_canvas* back_buffer;   /**< \brief pointer to a canvas */

    sgui_window_callback event_fun; /**< \brief the window event callback */

    int x, y;                   /**< \brief position of the window */
    unsigned int w, h;          /**< \brief the size of the window */

    int visible;                /**< \brief Window visibility */

    int backend;                /**< \breif Window backend used */

    void* userptr;


    /**
     * \brief Called by sgui_window_get_mouse_position
     *
     * \param wnd Pointer to the window itself
     * \param x   Pointer to the x return value, never NULL, adjusted to
     *            window dimensions after return
     * \param y   Pointer to the y return value, never NULL, adjusted to
     *            window dimensions after return
     */
    void (* get_mouse_position )( sgui_window* wnd, int* x, int* y );

    /**
     * \brief Called by sgui_window_set_mouse_position
     *
     * \param wnd Pointer to the window itself
     * \param x   The horizontal component of the position, already
     *            adjusted to the window size
     * \param y   The vertical component of the position, already
     *            adjusted to the window size
     */
    void (* set_mouse_position )( sgui_window* wnd, int x, int y );

    /**
     * \brief Called by sgui_window_set_visible
     *
     * \param wnd     Pointer to the window itself
     * \param visible Non-zero to show window, zero to hide
     */
    void (* set_visible )( sgui_window* wnd, int visible );

    /**
     * \brief Called by sgui_window_set_visible
     *
     * \param wnd   Pointer to the window itself
     * \param title Pointer to the new title string (never a NULL pointer)
     */
    void (* set_title )( sgui_window* wnd, const char* title );

    /**
     * \brief Called by sgui_window_set_size
     *
     * The sgui_window_set_size function takes care of also resizing the
     * canvas and redrawing the widget, if the backend is SGUI_NATIVE.
     *
     * \param wnd    Pointer to the window itself
     * \param width  New width of the window
     * \param height New height of the window
     */
    void (* set_size )( sgui_window* wnd,
                        unsigned int width, unsigned int height );

    /**
     * \brief Called by sgui_window_move_center
     *
     * \param wnd Pointer to the window itself
     */
    void (* move_center )( sgui_window* wnd );

    /**
     * \brief Called by sgui_window_move
     *
     * \param wnd    Pointer to the window itself
     * \param x      New position horizontal component
     * \param y      New position vertical component
     */
    void (* move )( sgui_window* wnd, int x, int y );

    /**
     * \brief Called by sgui_window_swap_buffers
     *
     * Can be set to NULL if not needed by the implementation.
     *
     * \param wnd Pointer to the window itself
     */
    void (* swap_buffers )( sgui_window* wnd );

    /**
     * \brief Called by sgui_window_destroy
     *
     * \param wnd Pointer to the window itself
     */
    void (* destroy )( sgui_window* wnd );
};



#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Initialise a widget structure
 *
 * \param widget A pointer to the widget structure
 * \param x      The horizontal component of the widgets position
 * \param y      The vertical component of the widgets position
 * \param width  The width of the widget
 * \param height The height of the widget
 */
SGUI_DLL void sgui_internal_widget_init( sgui_widget* widget, int x, int y,
                                         unsigned int width,
                                         unsigned int height );

/**
 * \brief Initialise a canvas structure
 *
 * \param cv     A pointer to the canvas structure
 * \param width  The width of the canvas
 * \param height The height of the canvas
 */
SGUI_DLL void sgui_internal_canvas_init( sgui_canvas* cv, unsigned int width,
                                         unsigned int height );

/**
 * \brief Initialise a window structure
 *
 * This function also creates a widget manager for the window.
 *
 * \param window A pointer to the window structure
 *
 * \return Non-zero on success, zero on failure
 */
SGUI_DLL int sgui_internal_window_init( sgui_window* window );

/**
 * \brief Perform common operations at the end of sgui_window_create
 *
 * This function stores the size and backend of a window in a window
 * structure, sets the window canvas background color to default window
 * color and clears it.
 *
 * \param window  A pointer to a window structure
 * \param width   The width to store in the structure
 * \param height  The height to store in the structure
 * \param backend The backend to store in the structure
 */
SGUI_DLL void sgui_internal_window_post_init( sgui_window* window,
                                              unsigned int width,
                                              unsigned int height,
                                              int backend );

/**
 * \brief Perform the reverse oprations of sgui_internal_window_init
 *
 * This function takes care of destroying the widget manager and
 * canvas of the window.
 *
 * \param window A pointer to a window structure
 */
SGUI_DLL void sgui_internal_window_deinit( sgui_window* window );

/**
 * \brief Propagate a window event
 *
 * \param wnd   The window that trigered the event
 * \param event The event that got triggered
 * \param e     A pointer ot a struct with additional information for an event
 */
SGUI_DLL void sgui_internal_window_fire_event( sgui_window* wnd, int event,
                                               sgui_event* e );

#ifdef __cplusplus
}
#endif


#endif /* SGUI_WIDGET_INTERNAL_H */

