/*
 * sgui_editbox.h
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
#ifndef SGUI_EDIT_BOX_H
#define SGUI_EDIT_BOX_H



#include "sgui_widget.h"



#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Create an edit box
 *
 * \param x         Distance from the left of the window.
 * \param y         Distance from the top of the window.
 * \param width     The width of the edit box.
 * \param max_chars The maximum number of characters that can be entered.
 */
sgui_widget* sgui_edit_box_create( int x, int y, unsigned int width,
                                   unsigned int max_chars );

/**
 * \brief Destroy an edit box widget
 *
 * \param box The edit box widget to destroy
 */
void sgui_edit_box_destroy( sgui_widget* box );

#ifdef __cplusplus
}
#endif

#endif /* SGUI_EDIT_BOX_H */

