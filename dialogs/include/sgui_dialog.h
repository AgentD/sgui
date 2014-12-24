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
    sgui_window* window;    /**< \brief A pointer to the dialog window */
    sgui_widget* b0;        /**< \brief First dialog button */
    sgui_widget* b1;        /**< \brief Second dialog button */
    sgui_widget* b2;        /**< \brief Third dialog button */

    /**
     * \copydoc sgui_dialog_destroy
     *
     * Before this function is called, the buttons and the window are
     * disconnected an destroyed.
     */
    void(* destroy )( sgui_dialog* dialog );

    /**
     * \brief Gets conntected by sgui_dialog_init to the button row events
     *
     * \param dialog A pointer to a dialog object
     * \param index  0 for the first button, 1 for the second button,
     *               2 for the third button, or a negative index if the
     *               window got closed.
     */
    void(* handle_button )( sgui_dialog* dialog, int index );
};



#define SGUI_LEFT 0
#define SGUI_CENTER 1
#define SGUI_RIGHT 2



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

/**
 * \brief Initialize dialog connections and button row
 *
 * \memberof sgui_dialog
 * \protected
 *
 * This function is used internally by sgui_dialog implementations to add the
 * default button row to the bottom of the dialog and make the default
 * connections.
 *
 * \param dialog     A pointer to a dialog object
 * \param button0    The text on the first button, or NULL if not used
 * \param button1    The text on the first button, or NULL if not used
 * \param button2    The text on the first button, or NULL if not used
 * \param allignment SGUI_LEFT for left adjusted buttons, SGUI_CENTER for
 *                   button row in the middle, SGUI_RIGHT for right adjusted
 *                   buttons
 *
 * \return Non-zero on success, zero on failure
 */
SGUI_DLL int sgui_dialog_init( sgui_dialog* dialog,
                               const char* button0, const char* button1,
                               const char* button2, int allignment );

#ifdef __cplusplus
}
#endif

#endif /* SGUI_DIALOG_H */

