/*
 * sgui_font.h
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
#ifndef SGUI_FONT_H
#define SGUI_FONT_H



#include "sgui_predef.h"



#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Load a font face from a file
 *
 * \param fs       The filesystem driver to use for accessing the font file.
 *                 Pass NULL to use the default stdio filesystem driver.
 * \param filename The path to the font file
 *
 * \returns A font object
 */
SGUI_DLL sgui_font* sgui_font_load( const sgui_filesystem* fs,
                                    const char* filename );

/** \brief Destroy a font object */
SGUI_DLL void sgui_font_destroy( sgui_font* font );

/**
 * \brief Set the rendering height of a font in pixels
 *
 * \param pixel_height The height of the font in pixels
 */
SGUI_DLL void sgui_font_set_height( sgui_font* font,
                                    unsigned int pixel_height );

/**
 * \brief Get the kerning distance between to characters
 *
 * \param first  The unicode code point of the first character.
 * \param second The unicode code point of the second character
 *               (following the first character).
 *
 * \return A negative value, giving the amount of pixels the cursor has to
 *         move to render the second glyph with kerning applied.
 */
SGUI_DLL int sgui_font_get_kerning_distance( sgui_font* font,
                                             unsigned int first,
                                             unsigned int second );

/**
 * \brief Load a specific glyph for a font
 *
 * \param codepoint The unicode code point for wich to load the coresponding
 *                  glyph.
 */
SGUI_DLL void sgui_font_load_glyph( sgui_font* font,
                                    unsigned int codepoint );

/**
 * \brief Get the dimensions of the currently loaded glyph of a font
 *
 * \param width   If used, returns the horizontal extents of the glyph.
 * \param width   If used, returns the vertical extents of the glyph.
 * \param bearing If used, returns the distance from the top of the line
 *                to the top of the rendered glyph bitmap.
 */
SGUI_DLL void sgui_font_get_glyph_metrics( sgui_font* font,
                                           unsigned int* width,
                                           unsigned int* height,
                                           int* bearing );

/**
 * \brief Get a buffer holding a rendering of the currently loaded
 *        glyph of a font
 *
 * \return A buffer holding grayscale values.
 */
SGUI_DLL unsigned char* sgui_font_get_glyph( sgui_font* font );





/**
 * \brief Get the with of a single line of text, in a single font face,
 *        in pixels
 *
 * \param font_face   The font face to use
 * \param text        The UTF8 text to determine the rendered width of
 * \param length      The number of bytes to read
 */
SGUI_DLL unsigned int sgui_font_get_text_extents_plain( sgui_font* font_face,
                                                        const char* text,
                                                        unsigned int length );

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
SGUI_DLL void sgui_font_get_text_extents( const char* text,
                                          unsigned int* width,
                                          unsigned int* height );

/**
 * \brief Render one line of text in a single font face
 *
 * \param canvas    A canvas to draw to.
 * \param x         Distance from the left of the canvas to the left of the
 *                  text.
 * \param y         Distance from the top of the canvas to the top of the
 *                  text.
 * \param font_face The font face to use.
 * \param height    The height of the text in pixels.
 * \param color     The RGB color to draw the text in.
 * \param text      The text to draw.
 * \param length    The number of bytes to read from the text.
 */
SGUI_DLL void sgui_font_draw_text_plain( sgui_canvas* canvas, int x, int y,
                                         sgui_font* font_face,
                                         unsigned char* color,
                                         const char* text,
                                         unsigned int length );

/**
 * \brief Render a multi line text that uses html like tags to switch color
 *        or font face, using the default fonts from the skinning system.
 *
 * \param canvas      The canvas to draw to.
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
SGUI_DLL void sgui_font_draw_text( sgui_canvas* canvas, int x, int y,
                                   const char* text );

#ifdef __cplusplus
}
#endif

#endif /* SGUI_FONT_H */

