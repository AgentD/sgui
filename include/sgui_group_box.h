/*
 * sgui_group_box.h
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
#ifndef SGUI_GROUP_BOX_H
#define SGUI_GROUP_BOX_H



#include "sgui_widget.h"



#ifdef __cplusplus
extern "C" {
#endif

sgui_widget* sgui_group_box_create( int x, int y,
                                    unsigned int width, unsigned int height,
                                    const char* caption );

void sgui_group_box_destroy( sgui_widget* box );

void sgui_group_box_add_widget( sgui_widget* box, sgui_widget* w );

void sgui_group_box_remove_widget( sgui_widget* box, sgui_widget* w );

#ifdef __cplusplus
}
#endif

#endif /* SGUI_GROUP_BOX_H */

