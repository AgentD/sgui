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



#define SGUI_MOUSE_BUTTON_LEFT   0
#define SGUI_MOUSE_BUTTON_MIDDLE 1
#define SGUI_MOUSE_BUTTON_RIGHT  2



#define SGUI_USER_CLOSED_EVENT        0
#define SGUI_API_INVISIBLE_EVENT      1
#define SGUI_API_DESTROY_EVENT        2
#define SGUI_SIZE_CHANGE_EVENT        3

#define SGUI_MOUSE_MOVE_EVENT         4
#define SGUI_MOUSE_PRESS_EVENT        5
#define SGUI_MOUSE_RELEASE_EVENT      6
#define SGUI_MOUSE_WHEEL_EVENT        7
#define SGUI_MOUSE_ENTER_EVENT        8
#define SGUI_MOUSE_LEAVE_EVENT        9

#define SGUI_KEY_PRESSED_EVENT       10
#define SGUI_KEY_RELEASED_EVENT      11
#define SGUI_CHAR_EVENT              12

#define SGUI_FOCUS_EVENT             13
#define SGUI_FOCUS_LOSE_EVENT        14



typedef union
{
    struct { unsigned int new_width, new_height; } size;

    struct { int x, y; } mouse_move;

    struct { int button, x, y; } mouse_press;

    struct { int direction; } mouse_wheel;

    struct { SGUI_KEY_CODE code; } keyboard_event;

    struct { unsigned char as_utf8_str[8]; } char_event;
}
sgui_event;



#endif /* SGUI_EVENT_H */

