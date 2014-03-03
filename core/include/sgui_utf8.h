/*
 * sgui_utf8.h
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
#ifndef SGUI_UTF8_H
#define SGUI_UTF8_H



#include "sgui_predef.h"



#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Get the unicode code point for a UTF8 encoded character
 *
 * \param utf8   A string holding the UTF8 character
 * \param length An optional pointer to an integer returning the length of the
 *               UTF8 encoded character in bytes
 *
 * \return The decoded unicode code point
 */
SGUI_DLL unsigned int sgui_utf8_decode( const char* utf8,
                                        unsigned int* length );

/**
 * \brief Get the UTF8 encoding of a unicode code point
 *
 * \param cp  The unicode code point
 * \param str A pointer to a buffer to write the UTF8 encoding to. Buffer
 *            must be able to hold at least 4 bytes.
 *
 * \return The number of bytes written to the buffer
 */
SGUI_DLL unsigned int sgui_utf8_encode( unsigned int cp, char* str );

/**
 * \brief Get the number of characters in an UTF8 encoded string
 *
 * In a string using UTF8 encoding, the number of bytes does not have to match
 * the number of characters, as characters can be up to 6 bytes long (unless
 * you use plain ASCII which is part of UTF8). Using the regular strlen
 * function on such a string yields the number of bytes, excluding the null
 * terminator, but not the number of actual characters.
 *
 * \param utf8 A string using UTF8 encoding
 *
 * \return The number of actual characters in the string
 */
SGUI_DLL unsigned int sgui_utf8_strlen( const char* utf8 );

/**
 * \brief Precompute the number of bytes required by a latin-1 string, after
 *        conversion to UTF8
 *
 * \param in A pointer to a latin-1 encoded string
 *
 * \return The number of bytes required to store the equivalent UTF8 string,
 *         excluding 0 terminator
 */
SGUI_DLL unsigned int sgui_utf8_from_latin1_length( const char* in );

/**
 * \brief Convert a latin-1 encoded string into UTF8 encoding
 *
 * \note Make sure that the destination buffer is large enought. Either use
 *       sgui_utf8_from_latin1_length do determine the exact length, or use
 *       twice the input size (maximum)
 *
 * \param out A pointer to the destination buffer
 * \param in A pointer to the input buffer
 */
SGUI_DLL void sgui_utf8_from_latin1( char* out, const char* in );

SGUI_DLL unsigned int sgui_utf8_strncpy( char* dst, const char* src,
                                         unsigned int buffersize,
                                         unsigned int characters );

#ifdef __cplusplus
}
#endif

#endif /* SGUI_UTF8_H */

