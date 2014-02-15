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
#define SGUI_GROUPBOX_LEFT_TOP            0x44
#define SGUI_GROUPBOX_RIGHT_TOP           0x45
#define SGUI_GROUPBOX_LEFT_BOTTOM         0x46
#define SGUI_GROUPBOX_RIGHT_BOTTOM        0x47
#define SGUI_GROUPBOX_LEFT                0x48
#define SGUI_GROUPBOX_RIGHT               0x49
#define SGUI_GROUPBOX_TOP                 0x4A
#define SGUI_GROUPBOX_BOTTOM              0x4B
#define SGUI_TAB_CAP_LEFT                 0x4C
#define SGUI_TAB_CAP_CENTER               0x4D
#define SGUI_TAB_CAP_RIGHT                0x4E
#define SGUI_TAB_LEFT_TOP                 0x4F
#define SGUI_TAB_RIGHT_TOP                0x50
#define SGUI_TAB_LEFT_BOTTOM              0x51
#define SGUI_TAB_RIGHT_BOTTOM             0x52
#define SGUI_TAB_LEFT                     0x53
#define SGUI_TAB_RIGHT                    0x54
#define SGUI_TAB_TOP                      0x55
#define SGUI_TAB_BOTTOM                   0x56
#define SGUI_TAB_GAP_LEFT                 0x57
#define SGUI_TAB_GAP_RIGHT                0x58
#define SGUI_SKIN_ELEMENTS                0x59



#ifdef __cplusplus
extern "C"
{
#endif

/**
 * \brief Load the skin to use for rendering UI elements from a file
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
 * This function reads a configuration file containing skin element positions
 * on a pixmap in the fasssion "elementname = (x,y,w,h)". For element names,
 * see the sample files provided with sgui (or the source of this function).
 * The skin pixmap can be loaded from a TGA image file, plain RGB or RBGA
 * data, no RLE compression, origin in the upper left (to keep the loader
 * simple).
 *
 * \param configfile A path to a configuration file to load the skin
 *                   information from
 * \param fs         A pointer to a filesystem abstraction layer to use for
 *                   accessing files. Use NULL for standard I/O
 */
SGUI_DLL void sgui_skin_load( const char* configfile, sgui_filesystem* fs );

/**
 * \brief Reset the skinning system internals and free memory
 *
 * This function is automatically called by sgui_deinit and does things
 * like freeing memory used by a skin loaded from a file.
 */
SGUI_DLL void sgui_skin_unload( void );

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
 * \param skin A pointer to a skin structure to copy over the current skin
 */
SGUI_DLL void sgui_skin_set( sgui_skin* skin );

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
SGUI_DLL void sgui_skin_get_element( unsigned int element, sgui_rect* r );

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

#ifdef __cplusplus
}
#endif

#endif /* SGUI_SKIN_H */

