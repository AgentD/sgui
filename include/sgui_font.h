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



#include "sgui_canvas.h"



#ifdef __cplusplus
extern "C" {
#endif



typedef struct sgui_font sgui_font;



/**
 * \brief Load a font face from a file using STDIO
 *
 * \param filename The path to the font file
 *
 * \returns A font object
 */
sgui_font* sgui_font_load_from_file( const char* filename );

/**
 * \brief Load a font from a memory buffer
 *
 * This is usefull when loading fonts from other resources than the standard
 * file system (e.g. custom virtual filesystems like physicsFS et al.)
 *
 * \note Do NOT delete the buffer until the font gets destroyed.
 *
 * \param buffer     Pointer to a buffer containing the loaded font file data
 * \param buffersize The size of the buffer in bytes
 *
 * \returns A font object
 */
sgui_font* sgui_font_load_from_mem( void* buffer, unsigned int buffersize );

/** \brief Destroy a font object */
void sgui_font_destroy( sgui_font* font );

/**
 * \brief Get the with of a single line of text, in a single font face,
 *        in pixels
 *
 * \param font_face   The font face to use
 * \param font_height The height of the font in pixels
 * \param text        The UTF8 text to determine the rendered width of
 * \param length      The number of bytes to read
 */
unsigned int sgui_font_get_text_extents_plain( sgui_font* font_face,
                                               unsigned int font_height,
                                               const char* text,
                                               unsigned int length );

/**
 * \brief Get the with and height of a multi line text that uses html like
 *        tags to determine color and font face
 *
 * \see sgui_canvas_draw_text
 *
 * \param font_norm   The font face to use for normal text
 * \param font_bold   The font face to use for bold text
 * \param font_ital   The font face to use for italic text
 * \param font_boit   The font face to use for bold and italic text
 * \param font_height The height of the font in pixels
 * \param text        The UTF8 text to determine the rendered size of
 * \param width       Returns the width of the rendered text
 * \param height      Returns the height of the rendererd text
 */
void sgui_font_get_text_extents( sgui_font* font_norm, sgui_font* font_bold,
                                 sgui_font* font_ital, sgui_font* font_boit,
                                 unsigned int font_height, const char* text,
                                 unsigned int* width, unsigned int* height );

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
void sgui_font_draw_text_plain( sgui_canvas* canvas, int x, int y,
                                sgui_font* font_face, unsigned int height,
                                unsigned char* color,
                                const char* text, unsigned int length );

/**
 * \brief Render a multi line text that uses html like tags to switch color
 *        or font_face
 *
 * \param canvas      The canvas to draw to.
 * \param x           Distance from the left of the text to the left of the
 *                    canvas.
 * \param y           Distance from the top of the text to the top of the
 *                    canvas.
 * \param font_norm   The font face to use for normal text.
 * \param font_bold   The font face to use for bold text.
 * \param font_ital   The font face to use for italic text.
 * \param font_boit   The font face to use for bold and italic text.
 * \param font_height The height of the font faces in pixels.
 * \param color       The default RGB color to use.
 * \param text        The UTF8 text to print. The LF ('\n') character can be
 *                    used for line wraps, the \<b\> \</b\> and \<i\> \</i\>
 *                    for writing text bold or italic. A \<color="#RRGGBB"\>
 *                    tag can be used to switch text color, where the value
 *                    "default" for color switches back to default color.
 */
void sgui_font_draw_text( sgui_canvas* canvas, int x, int y,
                          sgui_font* font_norm, sgui_font* font_bold,
                          sgui_font* font_ital, sgui_font* font_boit,
                          unsigned int font_height, unsigned char* color,
                          const char* text );



#ifdef __cplusplus
}
#endif

#endif /* SGUI_FONT_H */

