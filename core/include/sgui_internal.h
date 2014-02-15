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
#include "sgui_skin.h"



#define SGUI_ADD_TO_LIST( list, element )\
        (element)->next=(list); (list)=(element)

#define SGUI_REMOVE_FROM_LIST( list, iterator, element )\
        if( (list)==(element) )\
        {\
            (list) = (list)->next;\
        }\
        else\
        {\
            for( iterator=list; iterator->next; iterator=iterator->next )\
            {\
                if( iterator->next==element )\
                {\
                    iterator->next = iterator->next->next;\
                    break;\
                }\
            }\
        }

#ifndef MAX
    #define MAX( a, b ) (((a)>(b)) ? (a) : (b))
#endif

#ifndef MIN
    #define MIN( a, b ) (((a)<(b)) ? (a) : (b))
#endif


#define CANVAS_MAX_DIRTY 10



/* flags for the widget state change callback */
#define WIDGET_POSITION_CHANGED     0x01
#define WIDGET_VISIBILLITY_CHANGED  0x02
#define WIDGET_PARENT_CHANGED       0x04
#define WIDGET_CHILD_ADDED          0x08
#define WIDGET_CHILD_REMOVED        0x10
#define WIDGET_CANVAS_CHANGED       0x20



struct sgui_skin
{
    sgui_rect elements[ SGUI_SKIN_ELEMENTS ];

    unsigned int pixmap_width;  /**< \brief The width of the skin pixmap */
    unsigned int pixmap_height; /**< \brief The height of the skin pixmap */

    unsigned int font_height;   /**< \brief The pixel height of the font */

    unsigned char window_color[4];  /**< \brief The window background color */
    unsigned char font_color[4];    /**< \brief The font color */

    sgui_font* font_norm;   /**< \brief Font face for normal text */
    sgui_font* font_bold;   /**< \brief Font face for bold text */
    sgui_font* font_ital;   /**< \brief Font face for italic text */
    sgui_font* font_boit;   /**< \brief Font face for bold and italic text */

    void(* get_checkbox_extents )( sgui_skin* skin, sgui_rect* r );

    void(* get_radio_button_extents )( sgui_skin* skin, sgui_rect* r );

    unsigned int(* get_edit_box_height )( sgui_skin* skin );

    unsigned int(* get_edit_box_border_width )( sgui_skin* skin );

    unsigned int(* get_frame_border_width )( sgui_skin* skin );

    unsigned int(* get_progess_bar_width )( sgui_skin* skin );

    unsigned int(* get_scroll_bar_width )( sgui_skin* skin );

    void(* get_scroll_bar_button_extents )( sgui_skin* skin, sgui_rect* r );

    void(* draw_checkbox )( sgui_skin* skin, sgui_canvas* canvas,
                            int x, int y, int checked );

    void(* draw_radio_button )( sgui_skin* skin, sgui_canvas* canvas,
                                int x, int y, int checked );

    void(* draw_button )( sgui_skin* skin, sgui_canvas* canvas, sgui_rect* r,
                          int pressed );

    void(* draw_editbox )( sgui_skin* skin, sgui_canvas* canvas, sgui_rect* r,
                           const char* text, int offset, int cursor );

    void(* draw_frame )( sgui_skin* skin, sgui_canvas* canvas, sgui_rect* r );

    void(* draw_group_box )( sgui_skin* skin, sgui_canvas* canvas,
                             sgui_rect* r, const char* caption );

    void(* draw_progress_bar )( sgui_skin* skin, sgui_canvas* canvas, int x,
                                int y, unsigned int length, int vertical,
                                int percentage );

    void(* draw_progress_stippled )( sgui_skin* skin, sgui_canvas* canvas,
                                     int x, int y, unsigned int length,
                                     int vertical, int percentage );

    void(* draw_scroll_bar )( sgui_skin* skin, sgui_canvas* canvas,
                              int x, int y, unsigned int length, int vertical,
                              int pane_offset, unsigned int pane_length,
                              int decbutton, int incbutton );

    /**
     * \brief Load the required GUI elements into a pixmap
     *
     * \param skin   Pointer to the skin itself
     * \param pixmap Pointer to a pixmap
     */
    void(* load_to_pixmap )( sgui_skin* skin, sgui_pixmap* pixmap );
};

struct sgui_pixmap
{
    unsigned int width, height;     /**< \brief Size of the pixmap */

    /**
     * \brief Gets called by sgui_pixmap_destroy
     *
     * \param pixmap A pointer to the pixmap
     */
    void(* destroy )( sgui_pixmap* pixmap );

    /**
     * \brief Gets called by sgui_pixmap_load
     *
     * \param pixmap A pointer to the pixmap
     * \param dstx   Offset from the left of the pixmap
     * \param dsty   Offset from the top of the pixmap
     * \param data   A pointer to the first pixel
     * \param scan   The number of pixels to skip to get to the same pixel in
     *               the next row
     * \param width  The number of pixels per row to upload
     * \param height The number of rows to upload
     */
    void(* load )( sgui_pixmap* pixmap, int dstx, int dsty,
                   const unsigned char* data, unsigned int scan,
                   unsigned int width, unsigned int height, int format );
};

struct sgui_widget
{
    sgui_rect area;  /**< \brief The area occupied by a widget */

    int visible;     /**< \brief zero if the widget should not be rendered */

    /** \brief The canvas that the widget is attached to */
    sgui_canvas* canvas;

    /** \brief The next widget on the same level (linked list) */
    sgui_widget* next;

    /** \brief A pointer to the first widget in the children list */
    sgui_widget* children;

    /** \brief A pointer to the parent widget */
    sgui_widget* parent;

    /**
     * \brief Callback that is called to destroy a widget
     *
     * \param widget A pointer to the widget to destroy.
     */
    void (* destroy )( sgui_widget* widget );

    /**
     * \brief Callback that is called to draw a widget
     *
     * \param widget A pointer to the widget to draw.
     */
    void (* draw_callback )( sgui_widget* widget );

    /**
     * \brief Callback that is called to inject window events
     *
     * \param widget A pointer to the widget to update.
     * \param type   The event type.
     * \param event  The window event that occoured.
     */
    void (* window_event_callback )( sgui_widget* widget, int type,
                                     sgui_event* event );

    /**
     * \brief Callback that is called when the internal state of a widget
     *        changes(e.g. position, visibility, etc...)
     * 
     * \param widget A pointer to the widget that changed
     * \param change A combination of WIDGET_*_CHANGED flags that indicate
     *               what changed
     */
    void (* state_change_callback )( sgui_widget* widget, int change );
};

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

    sgui_rect dirty[ CANVAS_MAX_DIRTY ];
    unsigned int num_dirty;

    sgui_widget_callback fun;
    void* fun_user;

    sgui_pixmap* skin_pixmap;       /**< \brief The skin pixmap */

    /**
     * \brief Gets called by sgui_canvas_destroy
     *
     * \param canvas A pointer to the canvas.
     */
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
     * \brief Blend onto a canvas but stretch the source image
     *
     * This function does not have to be implemented. Set to NULL to use
     * the default implementation.
     *
     * \param canvas  A pointer to the canvas.
     * \param pixmap  The pixmap to blend.
     * \param srcrect A subrectangle of the pixmap to blend.
     * \param dstrect The destination rectangle on the canvas.
     * \param dx      Offset from the left on the pixmap
     * \param dy      Offset from the top on the pixmap
     */
    void(* stretch_blend )( sgui_canvas* canvas, sgui_pixmap* pixmap,
                            sgui_rect* srcrect, sgui_rect* dstrect,
                            int dx, int dy );

    /**
     * \brief Blit onto a canvas but stretch the source image
     *
     * This function does not have to be implemented. Set to NULL to use
     * the default implementation.
     *
     * \param canvas  A pointer to the canvas.
     * \param pixmap  The pixmap to blit.
     * \param srcrect A subrectangle of the pixmap to blit.
     * \param dstrect The destination rectangle on the canvas.
     * \param dx      Offset from the left on the pixmap
     * \param dy      Offset from the top on the pixmap
     */
    void(* stretch_blit )( sgui_canvas* canvas, sgui_pixmap* pixmap,
                           sgui_rect* srcrect, sgui_rect* dstrect,
                           int dx, int dy );

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

struct sgui_window
{
    sgui_canvas* canvas;        /**< \brief pointer to a canvas */

    sgui_window_callback event_fun; /**< \brief the window event callback */

    int x, y;                   /**< \brief position of the window */
    unsigned int w, h;          /**< \brief the size of the window */

    int visible;                /**< \brief Window visibility */

    int backend;                /**< \brief Window backend used */

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
 * \brief Perform common operations at the end of sgui_window_create
 *
 * This function stores the size and backend of a window in a window
 * structure.
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
 * \brief Propagate a window event
 *
 * \param wnd   The window that trigered the event
 * \param event The event that got triggered
 * \param e     A pointer ot a struct with additional information for an event
 */
SGUI_DLL void sgui_internal_window_fire_event( sgui_window* wnd, int event,
                                               sgui_event* e );

/**
 * \brief Initialise a skin structue with the default skin
 *
 * \param skin A pointer to a skin structure
 */
SGUI_DLL void sgui_interal_skin_init_default( sgui_skin* skin );

/**
 * \brief Instantiate an in memory implementation of a pixmap
 *
 * Used by the memory canvas implementation.
 *
 * \param width  The width of the pixmap in pixels
 * \param height The height of the pixmap in pixels
 * \param format The pixmap color format
 */
SGUI_DLL sgui_pixmap* sgui_internal_mem_pixmap_create( unsigned int width,
                                                       unsigned int height,
                                                       int format,
                                                       int swaprb );

/** \brief Get a pointer to the data of a memory pixmap */
SGUI_DLL unsigned char* sgui_internal_mem_pixmap_buffer( sgui_pixmap* pix );

/** \brief Get the color format of a memory pixmap */
SGUI_DLL int sgui_internal_mem_pixmap_format( sgui_pixmap* pix );

#ifdef __cplusplus
}
#endif


#endif /* SGUI_WIDGET_INTERNAL_H */

