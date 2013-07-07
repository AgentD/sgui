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




#define SGUI_TAB_CAPTION                12



#define SGUI_PBAR_H_STIPPLED_START        0x00
#define SGUI_PBAR_H_STIPPLED_EMPTY        0x01
#define SGUI_PBAR_H_STIPPLED_FILLED       0x02
#define SGUI_PBAR_H_STIPPLED_END          0x03
#define SGUI_PBAR_V_STIPPLED_START        0x04
#define SGUI_PBAR_V_STIPPLED_EMPTY        0x05
#define SGUI_PBAR_V_STIPPLED_FILLED       0x06
#define SGUI_PBAR_V_STIPPLED_END          0x07
#define SGUI_PBAR_H_FILLED_START          0x08
#define SGUI_PBAR_H_FILLED_EMPTY          0x09
#define SGUI_PBAR_H_FILLED_FILLED         0x0A
#define SGUI_PBAR_H_FILLED_END            0x0B
#define SGUI_PBAR_V_FILLED_START          0x0C
#define SGUI_PBAR_V_FILLED_EMPTY          0x0D
#define SGUI_PBAR_V_FILLED_FILLED         0x0E
#define SGUI_PBAR_V_FILLED_END            0x0F
#define SGUI_CHECKBOX                     0x10
#define SGUI_CHECKBOX_SELECTED            0x11
#define SGUI_RADIO_BUTTON                 0x12
#define SGUI_RADIO_BUTTON_SELECTED        0x13
#define SGUI_BUTTON_LEFT_TOP              0x14
#define SGUI_BUTTON_RIGHT_TOP             0x15
#define SGUI_BUTTON_LEFT_BOTTOM           0x16
#define SGUI_BUTTON_RIGHT_BOTTOM          0x17
#define SGUI_BUTTON_LEFT                  0x18
#define SGUI_BUTTON_RIGHT                 0x19
#define SGUI_BUTTON_TOP                   0x1A
#define SGUI_BUTTON_BOTTOM                0x1B
#define SGUI_BUTTON_FILL                  0x1C
#define SGUI_BUTTON_IN_LEFT_TOP           0x1D
#define SGUI_BUTTON_IN_RIGHT_TOP          0x1E
#define SGUI_BUTTON_IN_LEFT_BOTTOM        0x1F
#define SGUI_BUTTON_IN_RIGHT_BOTTOM       0x20
#define SGUI_BUTTON_IN_LEFT               0x21
#define SGUI_BUTTON_IN_RIGHT              0x22
#define SGUI_BUTTON_IN_TOP                0x23
#define SGUI_BUTTON_IN_BOTTOM             0x24
#define SGUI_BUTTON_IN_FILL               0x25
#define SGUI_SCROLL_BAR_H_PANE_LEFT       0x26
#define SGUI_SCROLL_BAR_H_PANE_CENTER     0x27
#define SGUI_SCROLL_BAR_H_PANE_RIGHT      0x28
#define SGUI_SCROLL_BAR_V_PANE_TOP        0x29
#define SGUI_SCROLL_BAR_V_PANE_CENTER     0x2A
#define SGUI_SCROLL_BAR_V_PANE_BOTTOM     0x2B
#define SGUI_SCROLL_BAR_V_BACKGROUND      0x2C
#define SGUI_SCROLL_BAR_H_BACKGROUND      0x2D
#define SGUI_SCROLL_BAR_BUTTON_UP         0x2E
#define SGUI_SCROLL_BAR_BUTTON_DOWN       0x2F
#define SGUI_SCROLL_BAR_BUTTON_LEFT       0x30
#define SGUI_SCROLL_BAR_BUTTON_RIGHT      0x31
#define SGUI_SCROLL_BAR_BUTTON_UP_IN      0x32
#define SGUI_SCROLL_BAR_BUTTON_DOWN_IN    0x33
#define SGUI_SCROLL_BAR_BUTTON_LEFT_IN    0x34
#define SGUI_SCROLL_BAR_BUTTON_RIGHT_IN   0x35
#define SGUI_EDIT_BOX_LEFT                0x36
#define SGUI_EDIT_BOX_CENTER              0x37
#define SGUI_EDIT_BOX_RIGHT               0x38
#define SGUI_EDIT_BOX_CURSOR              0x39
#define SGUI_FRAME_LEFT_TOP               0x3A
#define SGUI_FRAME_RIGHT_TOP              0x3B
#define SGUI_FRAME_LEFT_BOTTOM            0x3C
#define SGUI_FRAME_RIGHT_BOTTOM           0x3D
#define SGUI_FRAME_LEFT                   0x3E
#define SGUI_FRAME_RIGHT                  0x3F
#define SGUI_FRAME_TOP                    0x40
#define SGUI_FRAME_BOTTOM                 0x41
#define SGUI_FRAME_CENTER                 0x42
#define SGUI_FRAME_BORDER                 0x43



#ifdef __cplusplus
extern "C"
{
#endif

/**
 * \brief Get the minum size a pixmap needs to hold the current skin elements
 *
 * \param width  Returns the width of the pixmap
 * \param height Returns the height of the pixmap
 */
SGUI_DLL void sgui_skin_get_pixmap_size( unsigned int* width,
                                         unsigned int* height );

/** \brief Fill a given pixmap with the default GUI skin */
SGUI_DLL void sgui_skin_to_pixmap( sgui_pixmap* pixmap );

/**
 * \brief Get the region on the skin pixmap where a given element is stored
 *
 * \param element The element in question (e.g SGUI_CHECKBOX_SELECTED)
 * \param r       Returns the region of the element
 */
SGUI_DLL void sgui_skin_get_element( int element, sgui_rect* r );

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

/**
 * \brief Get the dimensions of a widget type
 *
 * \param type The type of widget
 * \param r    Returns the dimensions of the widget
 */
SGUI_DLL void sgui_skin_get_widget_extents( int type, sgui_rect* r );

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
SGUI_DLL void sgui_skin_draw_group_box( sgui_canvas* cv, sgui_rect* area,
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

