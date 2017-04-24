
/*
 * sgui_messagebox.h
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
 * \file sgui_messagebox.h
 *
 * \brief Contains the declarations of the message box dialog implementation
 */
#ifndef SGUI_MESSAGEBOX_H
#define SGUI_MESSAGEBOX_H



#include "sgui_predef.h"
#include "sgui_skin.h"



/**
 * \struct sgui_message_box
 *
 * \extends sgui_dialog
 *
 * \brief A message box dialog window
 *
 * \image html messagebox.png "A message box with multi line text and an icon"
 */



#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Create a message box
 *
 * \memberof sgui_message_box
 *
 * \param lib     An \ref sgui_lib instance through which to create a window.
 * \param icon    The icon to display (SGUI_ICON_MB_INFO, SGUI_ICON_MB_WARNING,
 *                SGUI_ICON_MB_CRITICAL or SGUI_ICON_MB_QUESTION).
 * \param caption The caption to print onto the window title bar.
 * \param text    The text to write into the message box. (Can be multiline,
 *                and supports color/styling tags)
 * \param button1 The text on the first button or NULL for none.
 * \param button2 The text on the second button or NULL for none.
 * \param button3 The text on the third button or NULL for none.
 *
 * \return A pointer to a message box
 */
SGUI_DLL sgui_dialog* sgui_message_box_create( sgui_lib *lib, int icon,
                                               const char* caption,
                                               const char* text,
                                               const char* button1,
                                               const char* button2,
                                               const char* button3 );

/**
 * \brief Display a message box by directly using the platform functions
 *
 * \memberof sgui_message_box
 * \static
 *
 * Some platforms have a native window systems and offer a message box
 * function, others don't. This function directly uses the platform dependend
 * functionallity to display an error message box with a single "OK" button.
 * It is intended for situations where sgui_init( ) fails and other sgui
 * functions cannot be used.
 *
 * Since it has its on main loop and everything, the function blocks until the
 * user responds to the message box.
 *
 * \param caption A caption to display on the message box window title bar
 * \param text    A (possibly multi line) text to display inside the
 *                message box window
 */
SGUI_DLL void sgui_message_box_emergency( const char* caption,
                                          const char* text );



#ifdef __cplusplus
}
#endif

#endif /* SGUI_MESSAGEBOX_H */

