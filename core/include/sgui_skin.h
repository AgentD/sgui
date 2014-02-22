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
#ifndef SGUI_SKIN_H
#define SGUI_SKIN_H



#include "sgui_predef.h"
#include "sgui_rect.h"



#ifdef __cplusplus
extern "C"
{
#endif

/**
 * \brief Set the skin to use for rendering UI elements
 *
 * The skin can only be set globally and should only be changed while no
 * window or widget exists.
 * 
 * GUI elements are only rerendered when required, changes would progress
 * slowly and look disturbing. Even if the skin is changed while no widgets
 * are visible, the widgets might look odd as they can use skin elements
 * of the previous skin for calculating their dimensions during
 * initialisation.
 *
 * This function is automatically called by sgui_init with a NULL argument.
 *
 * \param skin A pointer to a skin structure. NULL to reset to default
 */
SGUI_DLL void sgui_skin_set( sgui_skin* skin );

/**
 * \brief Override the default font for the skin
 *
 * \param normal      Font face for normal text.
 * \param bold        Font face for bold text.
 * \param italic      Font face for italic text.
 * \param bold_italic Font face for both bold and italic text.
 */
SGUI_DLL void sgui_skin_set_default_font( sgui_font* normal, sgui_font* bold,
                                          sgui_font* italic,
                                          sgui_font* bold_italic );

/** \brief Get the default window background color */
SGUI_DLL void sgui_skin_get_window_background_color( unsigned char* color );

/** \brief Get the default font color */
SGUI_DLL void sgui_skin_get_default_font_color( unsigned char* color );

/** \brief Get the default font height in pixels */
SGUI_DLL unsigned int sgui_skin_get_default_font_height( void );

/**
 * \brief Get the default font face
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
 * The functions uses  the default fonts from the skinning system.
 *
 * \see sgui_canvas_draw_text
 *
 * \param text        The UTF8 text to determine the rendered size of
 * \param width       Returns the width of the rendered text
 * \param height      Returns the height of the rendererd text
 */
SGUI_DLL void sgui_skin_get_text_extents( const char* text, sgui_rect* r );

SGUI_DLL void sgui_skin_get_checkbox_extents( sgui_rect* r );

SGUI_DLL void sgui_skin_get_radio_button_extents( sgui_rect* r );

SGUI_DLL unsigned int sgui_skin_get_edit_box_height( void );

SGUI_DLL unsigned int sgui_skin_get_edit_box_border_width( void );

SGUI_DLL unsigned int sgui_skin_get_frame_border_width( void );

SGUI_DLL unsigned int sgui_skin_get_progess_bar_width( void );

SGUI_DLL unsigned int sgui_skin_get_scroll_bar_width( void );

SGUI_DLL unsigned int sgui_skin_get_focus_box_width( void );

SGUI_DLL void sgui_skin_get_scroll_bar_button_extents( sgui_rect* r );

SGUI_DLL void sgui_skin_get_tap_caption_extents( sgui_rect* r );

SGUI_DLL void sgui_skin_draw_focus_box( sgui_canvas* canvas, sgui_rect* r );

SGUI_DLL void sgui_skin_draw_checkbox( sgui_canvas* canvas, int x, int y,
                                       int checked );

SGUI_DLL void sgui_skin_draw_radio_button( sgui_canvas* canvas, int x, int y,
                                           int checked );

SGUI_DLL void sgui_skin_draw_button( sgui_canvas* canvas, sgui_rect* r,
                                     int pressed );

SGUI_DLL void sgui_skin_draw_editbox( sgui_canvas* canvas, sgui_rect* r,
                                      const char* text, int offset,
                                      int cursor );

SGUI_DLL void sgui_skin_draw_frame( sgui_canvas* canvas, sgui_rect* r );

SGUI_DLL void sgui_skin_draw_group_box( sgui_canvas* canvas, sgui_rect* r,
                                        const char* caption );

SGUI_DLL void sgui_skin_draw_progress_bar( sgui_canvas* canvas, int x, int y,
                                           unsigned int length,
                                           int vertical, int percentage );

SGUI_DLL void sgui_skin_draw_progress_stippled( sgui_canvas* canvas,
                                                int x, int y,
                                                unsigned int length,
                                                int vertical,
                                                int percentage );

SGUI_DLL void sgui_skin_draw_scroll_bar( sgui_canvas* canvas, int x, int y,
                                         unsigned int length, int vertical,
                                         int pane_offset,
                                         unsigned int pane_length,
                                         int decbutton, int incbutton );

SGUI_DLL void sgui_skin_draw_tab_caption( sgui_canvas* canvas, int x, int y,
                                          const char* caption,
                                          unsigned int text_width );

SGUI_DLL void sgui_skin_draw_tab( sgui_canvas* canvas, sgui_rect* r,
                                  unsigned int gap, unsigned int gap_width );

#ifdef __cplusplus
}
#endif

#endif /* SGUI_SKIN_H */

