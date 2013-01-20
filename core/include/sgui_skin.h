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



#ifdef __cplusplus
extern "C"
{
#endif



/**
 * \brief Override the default font for the skin
 *
 * \param normal      Font face for normal text.
 * \param bold        Font face for bold text.
 * \param italic      Font face for italic text.
 * \param bold_italic Font face for both bold and italic text.
 * \param height      Default font height in pixels.
 */
SGUI_DLL void sgui_skin_set_default_font( sgui_font* normal, sgui_font* bold,
                                          sgui_font* italic,
                                          sgui_font* bold_italic,
                                          unsigned int height );

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
 * \brief Predict the size of a checkbox when rendered
 *
 * \param width  Returns the width of the checkbox.
 * \param height Returns the height of the checkbox.
 */
SGUI_DLL void sgui_skin_get_checkbox_extents( unsigned int* width,
                                              unsigned int* height );

/**
 * \brief Predict the size of a radio button when rendered
 *
 * \param width  Returns the width of the radio button.
 * \param height Returns the height of the radio button.
 */
SGUI_DLL void sgui_skin_get_radio_button_extents( unsigned int* width,
                                                  unsigned int* height );

/**
 * \brief Predict the size of a radio menu when rendered
 *
 * \param length    The proposed length of the bar.
 * \param style     The style of the bar (either SGUI_PROGRESS_BAR_CONTINUOUS,
 *                  or SGUI_PROGRESS_BAR_STIPPLED).
 * \param vertical  Non-zero if the bar is vertical.
 * \param width     Returns the width of the bar.
 * \param height    Returns the height of the bar.
 */
SGUI_DLL void sgui_skin_get_progress_bar_extents( unsigned int length,
                                                  int style, int vertical,
                                                  unsigned int* width,
                                                  unsigned int* height );

/** \brief Get the default height of an edit box */
SGUI_DLL unsigned int sgui_skin_get_edit_box_height( void );

/**
 * \brief Get the dimensions of a scroll bar
 *
 * \param horizontal Whether the bar is horizontal (non-zero) or
 *                   vertical (zero).
 * \param length     The length of the scroll bar in pixels.
 * \param width      Returns the width of the bar
 * \param height     Returns the height of the bar
 * \param bw         Returns the width of the buttons attached to the bar
 * \param bh         Returns the height of the buttons attached to the bar
 */
SGUI_DLL void sgui_skin_get_scroll_bar_extents( int horizontal,
                                                unsigned int length,
                                                unsigned int* width,
                                                unsigned int* height,
                                                unsigned int* bw,
                                                unsigned int* bh );

/** \brief Get the width of the border around a frame */
SGUI_DLL unsigned int sgui_skin_get_frame_border_width( void );


/** \brief Get the width of a tab caption */
SGUI_DLL unsigned int sgui_skin_get_tab_caption_width( const char* caption );

/** \brief Get the default height of a tab caption */
SGUI_DLL unsigned int sgui_skin_get_tab_caption_height( void );

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
SGUI_DLL void sgui_skin_get_text_extents( const char* text,
                                          unsigned int* width,
                                          unsigned int* height );


/**
 * \brief Draw a progress bar onto a canvas
 *
 * \param cv       The canvas to draw to.
 * \param x        The distance from the left side of the bar to the left of
 *                 the window.
 * \param y        The distance from the top of the bar to the top of
 *                 the window.
 * \param length   The length of the bar.
 * \param vertical Non-zero for vertical bars, zero for horizontal.
 * \param style    The style of the bar (stippled or continuous).
 * \param value    The progress value to indicate (value between 0 and 100).
 */
SGUI_DLL void sgui_skin_draw_progress_bar( sgui_canvas* cv, int x, int y,
                                           unsigned int length, int vertical,
                                           int style, unsigned int value );

/**
 * \brief Draw a button widget onto a canvas
 *
 * \param cv     The canvas to draw to.
 * \param r      The button area.
 * \param state  Zero if the button is in default state, non-zero if it is
 *               pressed.
 */
SGUI_DLL void sgui_skin_draw_button( sgui_canvas* cv, sgui_rect* r,
                                     int state );

/**
 * \brief Draw a checkbox onto a canvas
 *
 * \param cv     The canvas to draw to.
 * \param x      Distance from the left of the box to the left of the window.
 * \param y      Distance from the top of the box to the top of the window.
 * \param state  Non-zero if the checkbox is checked.
 */
SGUI_DLL void sgui_skin_draw_checkbox( sgui_canvas* cv, int x, int y,
                                       int state );

/**
 * \brief Draw a radio menu onto a canvas
 *
 * \param cv       The canvas to draw to.
 * \param x        Distance from the left of the radio button to the left of
 *                 the canvas.
 * \param y        Distance from the top of the radio button to the top of
 *                 the canvas.
 * \param selected Whether the radio button is selected or not
 */
SGUI_DLL void sgui_skin_draw_radio_button( sgui_canvas* cv, int x, int y,
                                           int selected );

/**
 * \brief Draw an edit box onto a canvas
 *
 * \param cv     The canvas to draw to.
 * \param x      Distance from the left of the box to the left of the window.
 * \param y      Distance from the top of the box to the top of the window.
 * \param text   The text to render into the edit box
 * \param width  The width of the edit box.
 * \param cursor Character index (UTF8!!) of the cursor or -1 to not draw a
 *               cursor at all.
 */
SGUI_DLL void sgui_skin_draw_edit_box( sgui_canvas* cv, int x, int y,
                                       const char* text,
                                       unsigned int width, int cursor );

/**
 * \brief Draw a frame onto a canvas
 *
 * \param cv     The canvas to draw to
 * \param x      Distance from the left of the frame to the left of the canvas
 * \param y      Distance from the top of the frame to the top of the canvas
 * \param width  The width of the frame
 * \param height The height of the frame
 */
SGUI_DLL void sgui_skin_draw_frame( sgui_canvas* cv, int x, int y, unsigned int width,
                                    unsigned int height );

/**
 * \brief Draw a scroll bar onto a canvas
 *
 * \param cv               The canvas to draw to
 * \param x                Distance from the left of the canvas
 * \param y                Distance from the top of the canvas
 * \param horizontal       Whether the bar is a horizontal or vertical bar
 * \param length           Length of the scroll bar
 * \param p_offset         Offset of the scroll bar pane
 * \param p_length         Length of the scroll bar pane
 * \param inc_button_state State of the button that increments the bar value
 * \param dec_button_state State of the button that decrements the bar value
 */
SGUI_DLL void sgui_skin_draw_scroll_bar( sgui_canvas* cv, int x, int y,
                                         int horizontal, unsigned int length,
                                         unsigned int p_offset,
                                         unsigned int p_length,
                                         int inc_button_state,
                                         int dec_button_state );

/**
 * \brief Draw a happy little group box onto a canvas
 *
 * \param cv      The canvas to draw to
 * \param x       Distance from the left of the canvas
 * \param y       Distance from the top of the canvas
 * \param width   The width of the group box
 * \param height  The height of the group box
 * \param caption The caption of the group box
 */
SGUI_DLL void sgui_skin_draw_group_box( sgui_canvas* cv, int x, int y,
                                        unsigned int width,
                                        unsigned int height,
                                        const char* caption );

/**
 * \brief Draw the caption of a tab onto a canvas
 *
 * \param cv      The canvas to draw to
 * \param x       Distance from the left of the canvas
 * \param y       Distance from the top of the canvas
 * \param width   The width of the tab caption
 * \param caption The caption text
 */
SGUI_DLL void sgui_skin_draw_tab_caption( sgui_canvas* cv, int x, int y,
                                          unsigned int width,
                                          const char* caption );

/**
 * \brif Draw a tab onto onto a canvas
 *
 * \param cv        The canvas to draw to
 * \param x         Distance from the left of the canvas
 * \param y         Distance from the top of the canvas
 * \param width     The width of the tab
 * \param height    The height of the tab
 * \param gap       The offset at witch to insert a gap for the caption
 * \param gap_width The width of the gap
 */
SGUI_DLL void sgui_skin_draw_tab( sgui_canvas* cv, int x, int y,
                                  unsigned int width, unsigned int height,
                                  unsigned int gap, unsigned int gap_width );


#ifdef __cplusplus
}
#endif

#endif /* SGUI_SKIN_H */

