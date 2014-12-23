/*
 * sgui_dialog.h
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
 * \file sgui_dialog.h
 *
 * \brief Contains the sgui_dialog base dialog window structure
 */
#ifndef SGUI_DIALOG_H
#define SGUI_DIALOG_H



#include "sgui_predef.h"



/**
 * \struct sgui_dialog
 *
 * \brief A base structure for all dialog windows
 */
struct sgui_dialog
{
    /** \brief A pointer to the dialog window */
    sgui_window* window;

    /** \copydoc sgui_dialog_destroy */
    void(* destroy )( sgui_dialog* dialog );
};



#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Destroy a dialog and free all its resources
 *
 * \memberof sgui_dialog
 *
 * \param dialog A pointer to a dialog object
 */
SGUI_DLL void sgui_dialog_destroy( sgui_dialog* dialog );

/**
 * \brief Display a dialog window
 *
 * \memberof sgui_dialog
 *
 * \param dialog A pointer to a dialog object
 */
SGUI_DLL void sgui_dialog_display( sgui_dialog* dialog );

#ifdef __cplusplus
}
#endif

#endif /* SGUI_DIALOG_H */

