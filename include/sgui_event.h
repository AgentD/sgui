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



enum
{
    SGUI_MOUSE_BUTTON_LEFT = 0,
    SGUI_MOUSE_BUTTON_MIDDLE,
    SGUI_MOUSE_BUTTON_RIGHT
};

enum
{
    /* window events */
    SGUI_USER_CLOSED_EVENT = 0,
    SGUI_API_INVISIBLE_EVENT,
    SGUI_API_DESTROY_EVENT,
    SGUI_SIZE_CHANGE_EVENT,

    SGUI_MOUSE_MOVE_EVENT,
    SGUI_MOUSE_PRESS_EVENT,
    SGUI_MOUSE_RELEASE_EVENT,
    SGUI_MOUSE_WHEEL_EVENT,
    SGUI_MOUSE_ENTER_EVENT,
    SGUI_MOUSE_LEAVE_EVENT,

    SGUI_KEY_PRESSED_EVENT,
    SGUI_KEY_RELEASED_EVENT,
    SGUI_CHAR_EVENT,

    /* widget manager events */
    SGUI_FOCUS_EVENT,
    SGUI_FOCUS_LOSE_EVENT,

    /* button events */
    SGUI_BUTTON_CLICK_EVENT,

    SGUI_RADIO_BUTTON_SELECT_EVENT,

    SGUI_CHECKBOX_CHECK_EVENT,
    SGUI_CHECKBOX_UNCHECK_EVENT
};



union sgui_event
{
    struct { unsigned int new_width, new_height; } size;

    struct { int x, y; } mouse_move;

    struct { int button, x, y; } mouse_press;

    struct { int direction; } mouse_wheel;

    struct { SGUI_KEY_CODE code; } keyboard_event;

    struct { char as_utf8_str[8]; } char_event;
};

#endif /* SGUI_EVENT_H */

