/*
 * sgui_skin.h
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
 * \file sgui_skin.h
 *
 * \brief Contains the declarations of the skinning subsystem.
 */
#ifndef SGUI_SKIN_H
#define SGUI_SKIN_H



#include "sgui_predef.h"
#include "sgui_color.h"
#include "sgui_rect.h"



#define SGUI_PROGRESS_BAR_DISCRETE    0x00
#define SGUI_PROGRESS_BAR_HORIZONTAL  0x00
#define SGUI_PROGRESS_BAR_CONTINUOUS  0x01
#define SGUI_PROGRESS_BAR_VERTICAL    0x02

#define SGUI_BUTTON                   0x00
#define SGUI_TOGGLE_BUTTON            0x01
#define SGUI_CHECKBOX                 0x02
#define SGUI_RADIO_BUTTON             0x03

/**
 * \enum SGUI_SKIN_ICON
 *
 * \brief Built-in icons that an \ref sgui_skin implementation is
 *        supposed to provide.
 */
typedef enum
{
	/** \brief Message box information icon */
	SGUI_ICON_MB_INFO = 0,

	/** \brief Message box warning icon */
	SGUI_ICON_MB_WARNING = 1,

	/** \brief Message box critical error icon */
	SGUI_ICON_MB_CRITICAL = 2,

	/** \brief Message box question icon */
	SGUI_ICON_MB_QUESTION = 3,

	/** \brief Not a valid icon ID but the number of icons there are */
	SGUI_SKIN_NUM_ICONS = 4
}
SGUI_SKIN_ICON;


/**
 * \struct sgui_skin
 *
 * \brief An abstract widget rendering facility
 */
struct sgui_skin
{
    unsigned int font_height;   /**< \brief The pixel height of the font */

    sgui_color window_color;  /**< \brief The window background color */
    sgui_color font_color;    /**< \brief The font color */

    sgui_font* font_norm;   /**< \brief Font face for normal text */
    sgui_font* font_bold;   /**< \brief Font face for bold text */
    sgui_font* font_ital;   /**< \brief Font face for italic text */
    sgui_font* font_boit;   /**< \brief Font face for bold and italic text */

    void(* get_skin_pixmap_size )( sgui_skin* skin, unsigned int* width,
                                   unsigned int* height, int* format);

    void(* init_skin_pixmap )( sgui_skin* skin, sgui_pixmap* pixmap );

    void(* get_icon_area )( sgui_skin* skin, sgui_rect* r, int icon );

    void(* get_button_extents )( sgui_skin* skin, int type, sgui_rect* r );

    unsigned int(* get_edit_box_height )( sgui_skin* skin );

    unsigned int(* get_edit_box_border_width )( sgui_skin* skin );

    unsigned int(* get_frame_border_width )( sgui_skin* skin );

    unsigned int(* get_progess_bar_width )( sgui_skin* skin );

    unsigned int(* get_scroll_bar_width )( sgui_skin* skin );

    unsigned int(* get_focus_box_width )( sgui_skin* skin );

    void(* get_scroll_bar_button_extents )( sgui_skin* skin, sgui_rect* r );

    void(* get_tap_caption_extents )( sgui_skin* skin, sgui_rect* r );

    void(* get_slider_extents )( sgui_skin* skin, sgui_rect* r,
                                 int vertical );

    void(* get_spin_buttons )( sgui_skin* skin, sgui_rect* up,
                               sgui_rect* down );

    void(* draw_focus_box )( sgui_skin* skin, sgui_canvas* canvas,
                             sgui_rect* r );

    void(* draw_button )( sgui_skin* skin, sgui_canvas* canvas, sgui_rect* r,
                          int type, int pressed );

    void(* draw_editbox )( sgui_skin* skin, sgui_canvas* canvas, sgui_rect* r,
                           const char* text, int offset, int cursor,
                           int selection, int numeric, int spinbuttons );

    void(* draw_frame )( sgui_skin* skin, sgui_canvas* canvas, sgui_rect* r );

    void(* draw_group_box )( sgui_skin* skin, sgui_canvas* canvas,
                             sgui_rect* r, const char* caption );

    void(* draw_progress_bar )( sgui_skin* skin, sgui_canvas* canvas, int x,
                                int y, unsigned int length, int flags,
                                unsigned int percentage );

    void(* draw_scroll_bar )( sgui_skin* skin, sgui_canvas* canvas,
                              int x, int y, unsigned int length, int vertical,
                              int pane_offset, unsigned int pane_length,
                              int decbutton, int incbutton );

    void(* draw_tab_caption )( sgui_skin* skin, sgui_canvas* canvas,
                               int x, int y, const char* caption,
                               unsigned int text_width );

    void(* draw_tab )( sgui_skin* skin, sgui_canvas* canvas, sgui_rect* r,
                       unsigned int gap, unsigned int gap_width );

    void(* draw_slider )( sgui_skin* skin, sgui_canvas* canvas, sgui_rect* r,
                          int vertical, int min, int max,
                          int value, int steps );
};



#ifdef __cplusplus
extern "C"
{
#endif

/**
 * \brief Retrieve a pointer to the currently set GUI skin renderer
 *
 * \memberof sgui_skin
 */
SGUI_DLL sgui_skin* sgui_skin_get( void );

/**
 * \brief Get the default font face
 *
 * \memberof sgui_skin
 *
 * \param bold   Nonzero to get the font face for bold text.
 * \param italic Nonzero to get the font face for italic text.
 *
 * \return The desired default font face
 */
SGUI_DLL sgui_font* sgui_skin_get_default_font( int bold, int italic );

/**
 * \brief Get the width (in pixels) of a string rendered with the default font
 *
 * \memberof sgui_skin
 *
 * \param text   An UTF8 string.
 * \param length The length of the string in bytes.
 * \param bold   Nonzero if the text should be rendered bold.
 * \param italic Nonzero if the text should be rendered italic.
 *
 * \return The width of the rendered text in pixels
 */
SGUI_DLL unsigned int sgui_skin_default_font_extents( const char* text,
                                                      unsigned int length,
                                                      int bold, int italic );

/**
 * \brief Get the with and height of a multi line text that uses html like
 *        tags to determine color and font face
 *
 * \memberof sgui_skin
 *
 * The functions uses the default fonts from the skinning system.
 *
 * \see sgui_canvas_draw_text
 *
 * \param text        The UTF8 text to determine the rendered size of
 * \param width       Returns the width of the rendered text
 * \param height      Returns the height of the rendererd text
 */
SGUI_DLL void sgui_skin_get_text_extents( const char* text, sgui_rect* r );

/**
 * \brief Render a multi line text that uses html like tags to switch color
 *        or font face, using the default fonts from the skinning system.
 *
 * \memberof sgui_skin
 *
 * \param canvas A pointer to the canvas object ot use for drawing.
 * \param x      Distance from the left of the text to the left of the canvas.
 * \param y      Distance from the top of the text to the top of the canvas.
 * \param text   The UTF8 text to print. The LF ('\n') character can be
 *               used for line wraps, the \<b\> \</b\> and \<i\> \</i\>
 *               for writing text bold or italic. A \<color="#RRGGBB"\>
 *               tag can be used to switch text color, where the value
 *               "default" for color switches back to default color.
 */
SGUI_DLL void sgui_skin_draw_text( sgui_canvas* canvas, int x, int y,
                                   const char* text );

#ifdef __cplusplus
}
#endif

#endif /* SGUI_SKIN_H */

