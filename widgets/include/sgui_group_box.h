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



#include "sgui_predef.h"



#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Create a group box
 *
 * A group box is a very simple widget that may group other widgets
 * (e.g. radio buttons for a radio button menu). A group box has a border that
 * seperates the contained widgets and a caption.
 *
 * \param x       The horizontal component of the group box position
 * \param y       The vertical component of the group box position
 * \param width   The width of the group box
 * \param height  The height of the group box
 * \param caption The caption of the group box
 *
 * \return A pointer to a new group box widget
 */
SGUI_DLL sgui_widget* sgui_group_box_create( int x, int y,
                                             unsigned int width,
                                             unsigned int height,
                                             const char* caption );

#ifdef __cplusplus
}
#endif

#endif /* SGUI_GROUP_BOX_H */

