/*
 * sgui_event.h
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
#ifndef SGUI_EVENT_H
#define SGUI_EVENT_H



#include "sgui_keycodes.h"
#include "sgui_predef.h"
#include "sgui_rect.h"



#define SGUI_MOUSE_BUTTON_LEFT   0
#define SGUI_MOUSE_BUTTON_MIDDLE 1
#define SGUI_MOUSE_BUTTON_RIGHT  2


/**************** window events ****************/
#define SGUI_USER_CLOSED_EVENT          0x0000
#define SGUI_API_INVISIBLE_EVENT        0x0001
#define SGUI_API_DESTROY_EVENT          0x0002
#define SGUI_SIZE_CHANGE_EVENT          0x0003

#define SGUI_MOUSE_MOVE_EVENT           0x0004
#define SGUI_MOUSE_PRESS_EVENT          0x0005
#define SGUI_MOUSE_RELEASE_EVENT        0x0006
#define SGUI_MOUSE_WHEEL_EVENT          0x0007
#define SGUI_MOUSE_ENTER_EVENT          0x0008
#define SGUI_MOUSE_LEAVE_EVENT          0x0009

#define SGUI_KEY_PRESSED_EVENT          0x000A
#define SGUI_KEY_RELEASED_EVENT         0x000B
#define SGUI_CHAR_EVENT                 0x000C

#define SGUI_EXPOSE_EVENT               0x000D

/************ widget manager events ************/
#define SGUI_FOCUS_EVENT                0x0010
#define SGUI_FOCUS_LOSE_EVENT           0x0011

/* button events */
#define SGUI_BUTTON_CLICK_EVENT         0x0020

#define SGUI_RADIO_BUTTON_SELECT_EVENT  0x0021

#define SGUI_CHECKBOX_CHECK_EVENT       0x0022
#define SGUI_CHECKBOX_UNCHECK_EVENT     0x0023

/*************** edit box events ***************/
/* text got modified */
#define SGUI_EDIT_BOX_TEXT_CHANGED      0x0030

/* text got modified, user pressed enter */
#define SGUI_EDIT_BOX_TEXT_ENTERED      0x0031



union sgui_event
{
    struct { unsigned int new_width, new_height; } size;

    struct { int x, y; } mouse_move;

    struct { int button, x, y; } mouse_press;

    struct { int direction; } mouse_wheel;

    struct { int code; } keyboard_event;

    struct { char as_utf8_str[8]; } char_event;

    sgui_rect expose_event;
};

#endif /* SGUI_EVENT_H */

