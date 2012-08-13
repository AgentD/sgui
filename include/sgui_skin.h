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



void sgui_skin_init( void );

void sgui_skin_deinit( void );



/**
 * \brief Override the default font for the skin
 *
 * \param normal      Font face for normal text.
 * \param bold        Font face for bold text.
 * \param italic      Font face for italic text.
 * \param bold_italic Font face for both bold and italic text.
 * \param height      Default font height in pixels.
 */
void sgui_skin_set_default_font( sgui_font* normal, sgui_font* bold,
                                 sgui_font* italic, sgui_font* bold_italic,
                                 unsigned int height );



/**
 * \brief Get a the default window background color
 */
void sgui_skin_get_window_background_color( unsigned char* color );

/**
 * \brief Predict the size of a button when rendered
 *
 * \param text   The UTF8 text to display on the button
 * \param width  Returns the width of the button in pixels
 * \param height Returns the height of the button in pixels
 * \param text_w Returns the width of the text on the button in pixels
 */
void sgui_skin_get_button_extents( const unsigned char* text,
                                   unsigned int* width,
                                   unsigned int* height,
                                   unsigned int* text_w );

/**
 * \brief Predict the extents of a text drawn onto a window
 *
 * \param text   The UTF8 text to print. The LF ('\n') character can be used
 *               for line wraps, the \<b\> \</b\> and \<i\> \</i\> for writing
 *               text bold or italic. A \<color="#RRGGBB"\> tag can be used to
 *               switch text color, where the value "default" for color
 *               switches back to default color.
 * \param width  Returns the width of the rendered text.
 * \param height Returns the height of the rendered text.
 */
void sgui_skin_get_text_extents( const unsigned char* text,
                                 unsigned int* width, unsigned int* height );

/**
 * \brief Predict the size of a checkbox when rendered
 *
 * \param text   The UTF8 text to print onto the checkbox.
 * \param width  Returns the width of the checkbox text.
 * \param height Returns the height of the checkbox text.
 * \param text_w Returns the width of the text on the button in pixels
 */
void sgui_skin_get_checkbox_extents( const unsigned char* text,
                                     unsigned int* width,
                                     unsigned int* height,
                                     unsigned int* text_w );

/**
 * \brief Predict the size of a radio menu when rendered
 *
 * \param text      The UTF8 options of the menu.
 * \param num_lines The number of options.
 * \param width     Returns the width of the menu.
 * \param height    Returns the height of the menu.
 */
void sgui_skin_get_radio_menu_extents( const unsigned char** text,
                                       unsigned int num_lines,
                                       unsigned int* width,
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
void sgui_skin_get_progress_bar_extents( unsigned int length, int style,
                                         int vertical, unsigned int* width,
                                         unsigned int* height );


/**
 * \brief Gets a radio menu option index from a point
 *
 * \param y Distance from the top of the radio menu
 *
 * \return The index of the radion menu option at that distance
 */
unsigned int sgui_skin_get_radio_menu_option_from_point( int y );


/**
 * \brief Draw a progress bar onto a window
 *
 * \param wnd      The window to draw to.
 * \param x        The distance from the left side of the bar to the left of
 *                 the window.
 * \param y        The distance from the top of the bar to the top of
 *                 the window.
 * \param length   The length of the bar.
 * \param vertical Non-zero for vertical bars, zero for horizontal.
 * \param style    The style of the bar (stippled or continuous).
 * \param value    The progress value to indicate (value between 0.0 and 1.0).
 */
void sgui_skin_draw_progress_bar( sgui_window* wnd, int x, int y,
                                  unsigned int length, int vertical,
                                  int style, float value );

/**
 * \brief Draw a button widget onto a window
 *
 * \param wnd    The window to draw to.
 * \param x      The distance from the left side of the button to the left of
 *               the window.
 * \param y      The distance from the top of the button to the top of
 *               the window.
 * \param width  Width of the button area.
 * \param text_w Width of the text string in pixels.
 * \param height Height of the button area.
 * \param state  Zero if the button is in default state, non-zero if it is
 *               pressed.
 * \param text   UTF8 text to print onto the button.
 */
void sgui_skin_draw_button( sgui_window* wnd, int x, int y, int state,
                            unsigned int width, unsigned int text_w,
                            unsigned int height, const unsigned char* text );

/**
 * \brief Draw a text onto a window
 *
 * \param wnd   The window to draw to.
 * \param x     Distance from the left of the text to the left of the window.
 * \param y     Distance from the top of the text to the top of the window.
 * \param width Width of the longest line in pixels.
 * \param text  The UTF8 text to print. The LF ('\n') character can be used
 *              for line wraps, the \<b\> \</b\> and \<i\> \</i\> for writing
 *              text bold or italic. A \<color="#RRGGBB"\> tag can be used to
 *              switch text color, where the value "default" for color switches
 *              back to default color.
 */
void sgui_skin_draw_text( sgui_window* wnd, int x, int y,
                          unsigned int width, const unsigned char* text );

/**
 * \brief Draw a text onto a window
 *
 * \param wnd    The window to draw to.
 * \param x      Distance from the left of the box to the left of the window.
 * \param y      Distance from the top of the box to the top of the window.
 * \param text_w Width of the text string in pixels.
 * \param text   The UTF8 text to print onto the checkbox.
 * \param state  Non-zero if the checkbox is checked.
 */
void sgui_skin_draw_checkbox( sgui_window* wnd, int x, int y,
                              unsigned int text_w, const unsigned char* text,
                              int state );

/**
 * \brief Predict the size of a radio menu when rendered
 *
 * \param wnd       The window to draw to.
 * \param x         Distance from the left of the menu to the left of the
 *                  window.
 * \param y         Distance from the top of the menu to the top of the
 *                  window. 
 * \param text      The UTF8 options of the menu.
 * \param num_lines The number of options.
 * \param selected  The option selected.
 * \param width     The width of the menu.
 * \param height    The height of the menu.
 */
void sgui_skin_draw_radio_menu( sgui_window* wnd, int x, int y,
                                const unsigned char** text,
                                unsigned int num_lines, unsigned int selected,
                                unsigned int width, unsigned int height );


#ifdef __cplusplus
}
#endif

#endif /* SGUI_SKIN_H */

