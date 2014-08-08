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

/**
 * \file sgui_font.h
 *
 * This file contains the interface functions for the font rendering
 * abstraction, implemented directly in the backend.
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
 * \memberof sgui_font
 *
 * \param filename     The path to the font file
 * \param pixel_height The rendering height of the font face in pixels
 *
 * \returns A font object
 */
SGUI_DLL sgui_font* sgui_font_load( const char* filename,
                                    unsigned int pixel_height );

/**
 * \brief Load a font face from an array in memory
 *
 * \memberof sgui_font
 *
 * The given data is copied internally, so the pointer can be freed
 * immediately after calling the function.
 *
 * \param data         A pointer to a block of data, containing the font file
 * \param size         The number of bytes to read from the data block
 * \param pixel_height The rendering height of the font face in pixels
 *
 * \returns A font object
 */
SGUI_DLL sgui_font* sgui_font_load_memory( const void* data,
                                           unsigned long size,
                                           unsigned int pixel_height );

/**
 * \brief Destroy a font object
 *
 * \memberof sgui_font
 */
SGUI_DLL void sgui_font_destroy( sgui_font* font );

/**
 * \brief Get the rendering height of a font in pixels
 *
 * \memberof sgui_font
 */
SGUI_DLL unsigned int sgui_font_get_height( const sgui_font* font );

/**
 * \brief Get the kerning distance between to characters
 *
 * \memberof sgui_font
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
 * \memberof sgui_font
 *
 * \param codepoint The unicode code point for wich to load the coresponding
 *                  glyph.
 */
SGUI_DLL void sgui_font_load_glyph( sgui_font* font,
                                    unsigned int codepoint );

/**
 * \brief Get the dimensions of the currently loaded glyph of a font
 *
 * \memberof sgui_font
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
 * \memberof sgui_font
 *
 * \return A buffer holding grayscale values.
 */
SGUI_DLL unsigned char* sgui_font_get_glyph( sgui_font* font );

#ifdef __cplusplus
}
#endif

#endif /* SGUI_FONT_H */

