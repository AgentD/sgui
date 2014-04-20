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
#ifndef SGUI_MESSAGEBOX_H
#define SGUI_MESSAGEBOX_H



#include "sgui_predef.h"



#ifdef __cplusplus
extern "C" {
#endif



#define SGUI_MB_INFO     0
#define SGUI_MB_WARNING  1
#define SGUI_MB_CRITICAL 2
#define SGUI_MB_QUESTION 3



/**
 * \brief Create a message box
 *
 * \param icon    The icon to display (SGUI_MB_INFO, SGUI_MB_WARNING,
 *                SGUI_MB_CRITICAL or SGUI_MB_QUESTION).
 *                Defaults to SGUI_MB_INFO if invalid.
 * \param caption The caption to print onto the window title bar.
 * \param text    The text to write into the message box. (Can be multiline,
 *                and supports color/styling tags)
 * \param button1 The text on the first button or NULL for none.
 * \param button2 The text on the second button or NULL for none.
 * \param button3 The text on the third button or NULL for none.
 *
 * \return A pointer to a message box
 */
SGUI_DLL sgui_message_box* sgui_message_box_create( int icon,
                                                    const char* caption,
                                                    const char* text,
                                                    const char* button1,
                                                    const char* button2,
                                                    const char* button3 );

/**
 * \brief Destroy a message box and free all its memory
 *
 * \param mb A pointer to a message box
 */
SGUI_DLL void sgui_message_box_destroy( sgui_message_box* mb );

/**
 * \brief Display a message box in the center of the screen
 *
 * When the user presses one of the buttons, the message box window is closed
 * an event is generated (SGUI_MESSAGE_BOX_BUTTON1_EVENT,
 * SGUI_MESSAGE_BOX_BUTTON2_EVENT or SGUI_MESSAGE_BOX_BUTTON3_EVENT) with the
 * pointer to the message box set source window pointer.
 *
 * If the user closes the window without pressing a button,
 * SGUI_MESSAGE_BOX_CLOSED_EVENT is generated.
 *
 * \param mb A pointer to a message box
 */
SGUI_DLL void sgui_message_box_display( sgui_message_box* mb );



#ifdef __cplusplus
}
#endif

#endif /* SGUI_MESSAGEBOX_H */

