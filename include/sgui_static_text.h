/*
 * sgui_static_text.h
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
#ifndef SGUI_STATIC_TEXT_H
#define SGUI_STATIC_TEXT_H



#include "sgui_predef.h"



#ifdef __cplusplus
extern "C"
{
#endif



/**
 * \brief Create a static text widget
 *
 * \param x    The x component of the widget position.
 * \param y    The y component of the widget position.
 * \param text The UTF8 text to print. The LF ('\n') character can be used
 *             for line wraps, the \<b\> \</b\> and \<i\> \</i\> for writing
 *             text bold or italic. A \<color="#RRGGBB"\> tag can be used to
 *             switch text color. The default text color is SGUI_DEFAULT_TEXT.
 * \param norm The font face to use for rendering the text.
 * \param bold The font face to use for rendering bold parts of the text.
 * \param ital The font face to use for rendering italic parts of the text.
 * \param boit The font face to use for rendering bold and italic parts of
 *             the text.
 * \praam size The font size in pixels.
 */
sgui_widget* sgui_static_text_create( int x, int y, const unsigned char* text,
                                      sgui_font* norm, sgui_font* bold,
                                      sgui_font* ital, sgui_font* boit,
                                      unsigned int size );

/** \brief Destroy a static text widget */
void sgui_static_text_destroy( sgui_widget* widget );



#ifdef __cplusplus
}
#endif

#endif /* SGUI_STATIC_TEXT_H */

