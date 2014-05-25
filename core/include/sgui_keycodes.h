/*
 * sgui_keycodes.h
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
 * \file sgui_keycodes.h
 *
 * This file contains the definitions of all sgui keyboard codes.
 */
#ifndef SGUI_KEYCODES_H
#define SGUI_KEYCODES_H



#define SGUI_MOD_SHIFT     0x01 /**< Flag for any kind of SHIFT key */
#define SGUI_MOD_CTRL      0x02 /**< Flag for any kind of CTRL key */
#define SGUI_MOD_ALT       0x04 /**< Flag for any kind of ALT key */
#define SGUI_MOD_SUPER     0x08 /**< Flag for any kind of SUPER key */

#define SGUI_KC_SELECT_ALL 0x0100   /**< select-all composition */
#define SGUI_KC_COPY       0x0101   /**< copy composition */
#define SGUI_KC_PASTE      0x0102   /**< paste composition */
#define SGUI_KC_CUT        0x0103   /**< cut composition */

#define SGUI_KC_UNKNOWN    0x00 /**< ANY key */
#define SGUI_KC_BACK       0x08 /**< BACKSPACE key */
#define SGUI_KC_TAB        0x09 /**< TAB key */
#define SGUI_KC_CLEAR      0x0C /**< CLEAR key */
#define SGUI_KC_RETURN     0x0D /**< ENTER key */
#define SGUI_KC_SHIFT      0x10 /**< SHIFT key */
#define SGUI_KC_CONTROL    0x11 /**< CTRL key */
#define SGUI_KC_ALT        0x12 /**< ALT key */
#define SGUI_KC_PAUSE      0x13 /**< PAUSE key */
#define SGUI_KC_CAPITAL    0x14 /**< CAPS LOCK key */
#define SGUI_KC_KANA       0x15 /**< IME Kana mode */
#define SGUI_KC_HANGUEL    0x15 /**< IME Hanguel mode */
#define SGUI_KC_HANGUL     0x15 /**< IME Hangul mode */
#define SGUI_KC_JUNJA      0x17 /**< IME Junja mode */
#define SGUI_KC_FINAL      0x18 /**< IME final mode */
#define SGUI_KC_HANJA      0x19 /**< IME Hanja mode */
#define SGUI_KC_KANJI      0x19 /**< IME Kanji mode */
#define SGUI_KC_ESCAPE     0x1B /**< ESC key */
#define SGUI_KC_CONVERT    0x1C /**< IME convert */
#define SGUI_KC_NONCONVERT 0x1D /**< IME nonconvert */
#define SGUI_KC_ACCEPT     0x1E /**< IME accept */
#define SGUI_KC_MODECHANGE 0x1F /**< IME mode change request */
#define SGUI_KC_SPACE      0x20 /**< SPACEBAR */
#define SGUI_KC_PRIOR      0x21 /**< PAGE UP key */
#define SGUI_KC_NEXT       0x22 /**< PAGE DOWN key */
#define SGUI_KC_END        0x23 /**< END key */
#define SGUI_KC_HOME       0x24 /**< HOME key */
#define SGUI_KC_LEFT       0x25 /**< LEFT ARROW key */
#define SGUI_KC_UP         0x26 /**< UP ARROW key */
#define SGUI_KC_RIGHT      0x27 /**< RIGHT ARROW key */
#define SGUI_KC_DOWN       0x28 /**< DOWN ARROW key */
#define SGUI_KC_SELECT     0x29 /**< SELECT key */
#define SGUI_KC_PRINT      0x2A /**< PRINT key */
#define SGUI_KC_EXECUTE    0x2B /**< EXECUTE key */
#define SGUI_KC_SNAPSHOT   0x2C /**< PRINT SCREEN key */
#define SGUI_KC_INSERT     0x2D /**< INS key */
#define SGUI_KC_DELETE     0x2E /**< DEL key */
#define SGUI_KC_HELP       0x2F /**< HELP key */
#define SGUI_KC_0          0x30 /**< 0 key */
#define SGUI_KC_1          0x31 /**< 1 key */
#define SGUI_KC_2          0x32 /**< 2 key */
#define SGUI_KC_3          0x33 /**< 3 key */
#define SGUI_KC_4          0x34 /**< 4 key */
#define SGUI_KC_5          0x35 /**< 5 key */
#define SGUI_KC_6          0x36 /**< 6 key */
#define SGUI_KC_7          0x37 /**< 7 key */
#define SGUI_KC_8          0x38 /**< 8 key */
#define SGUI_KC_9          0x39 /**< 9 key */
#define SGUI_KC_A          0x41 /**< A key */
#define SGUI_KC_B          0x42 /**< B key */
#define SGUI_KC_C          0x43 /**< C key */
#define SGUI_KC_D          0x44 /**< D key */
#define SGUI_KC_E          0x45 /**< E key */
#define SGUI_KC_F          0x46 /**< F key */
#define SGUI_KC_G          0x47 /**< G key */
#define SGUI_KC_H          0x48 /**< H key */
#define SGUI_KC_I          0x49 /**< I key */
#define SGUI_KC_J          0x4A /**< J key */
#define SGUI_KC_K          0x4B /**< K key */
#define SGUI_KC_L          0x4C /**< L key */
#define SGUI_KC_M          0x4D /**< M key */
#define SGUI_KC_N          0x4E /**< N key */
#define SGUI_KC_O          0x4F /**< O key */
#define SGUI_KC_P          0x50 /**< P key */
#define SGUI_KC_Q          0x51 /**< Q key */
#define SGUI_KC_R          0x52 /**< R key */
#define SGUI_KC_S          0x53 /**< S key */
#define SGUI_KC_T          0x54 /**< T key */
#define SGUI_KC_U          0x55 /**< U key */
#define SGUI_KC_V          0x56 /**< V key */
#define SGUI_KC_W          0x57 /**< W key */
#define SGUI_KC_X          0x58 /**< X key */
#define SGUI_KC_Y          0x59 /**< Y key */
#define SGUI_KC_Z          0x5A /**< Z key */
#define SGUI_KC_LSUPER     0x5B /**< Left Super (Natural keyboard) */
#define SGUI_KC_RSUPER     0x5C /**< Right Super(Natural keyboard) */
#define SGUI_KC_APPS       0x5D /**< Applications key (Natural keyboard) */
#define SGUI_KC_SLEEP      0x5F /**< Computer Sleep key */
#define SGUI_KC_NUM0       0x60 /**< Numpad 0 key */
#define SGUI_KC_NUM1       0x61 /**< Numpad 1 key */
#define SGUI_KC_NUM2       0x62 /**< Numpad 2 key */
#define SGUI_KC_NUM3       0x63 /**< Numpad 3 key */
#define SGUI_KC_NUM4       0x64 /**< Numpad 4 key */
#define SGUI_KC_NUM5       0x65 /**< Numpad 5 key */
#define SGUI_KC_NUM6       0x66 /**< Numpad 6 key */
#define SGUI_KC_NUM7       0x67 /**< Numpad 7 key */
#define SGUI_KC_NUM8       0x68 /**< Numpad 8 key */
#define SGUI_KC_NUM9       0x69 /**< Numpad 9 key */
#define SGUI_KC_MULTIPLY   0x6A /**< Multiply key */
#define SGUI_KC_ADD        0x6B /**< Add key */
#define SGUI_KC_SEPARATOR  0x6C /**< Separator key */
#define SGUI_KC_SUBTRACT   0x6D /**< Subtract key */
#define SGUI_KC_DECIMAL    0x6E /**< Decimal key */
#define SGUI_KC_DIVIDE     0x6F /**< Divide key */
#define SGUI_KC_F1         0x70 /**< F1 key */
#define SGUI_KC_F2         0x71 /**< F2 key */
#define SGUI_KC_F3         0x72 /**< F3 key */
#define SGUI_KC_F4         0x73 /**< F4 key */
#define SGUI_KC_F5         0x74 /**< F5 key */
#define SGUI_KC_F6         0x75 /**< F6 key */
#define SGUI_KC_F7         0x76 /**< F7 key */
#define SGUI_KC_F8         0x77 /**< F8 key */
#define SGUI_KC_F9         0x78 /**< F9 key */
#define SGUI_KC_F10        0x79 /**< F10 key */
#define SGUI_KC_F11        0x7A /**< F11 key */
#define SGUI_KC_F12        0x7B /**< F12 key */
#define SGUI_KC_F13        0x7C /**< F13 key */
#define SGUI_KC_F14        0x7D /**< F14 key */
#define SGUI_KC_F15        0x7E /**< F15 key */
#define SGUI_KC_F16        0x7F /**< F16 key */
#define SGUI_KC_F17        0x80 /**< F17 key */
#define SGUI_KC_F18        0x81 /**< F18 key */
#define SGUI_KC_F19        0x82 /**< F19 key */
#define SGUI_KC_F20        0x83 /**< F20 key */
#define SGUI_KC_F21        0x84 /**< F21 key */
#define SGUI_KC_F22        0x85 /**< F22 key */
#define SGUI_KC_F23        0x86 /**< F23 key */
#define SGUI_KC_F24        0x87 /**< F24 key */
#define SGUI_KC_NUMLOCK    0x90 /**< NUM LOCK key */
#define SGUI_KC_SCROLL     0x91 /**< SCROLL LOCK key */
#define SGUI_KC_LSHIFT     0xA0 /**< Left SHIFT key */
#define SGUI_KC_RSHIFT     0xA1 /**< Right SHIFT key */
#define SGUI_KC_LCONTROL   0xA2 /**< Left CONTROL key */
#define SGUI_KC_RCONTROL   0xA3 /**< Right CONTROL key */
#define SGUI_KC_LALT       0xA4 /**< Left ALT key */
#define SGUI_KC_RALT       0xA5 /**< Right ALT key */
#define SGUI_KC_OEM_PLUS   0xBB /**< For any country/region, the '+' key */
#define SGUI_KC_OEM_COMMA  0xBC /**< For any country/region, the ',' key */
#define SGUI_KC_OEM_MINUS  0xBD /**< For any country/region, the '-' key */
#define SGUI_KC_OEM_PERIOD 0xBE /**< For any country/region, the '.' key */
#define SGUI_KC_ATTN       0xF6 /**< Attn key */
#define SGUI_KC_CRSEL      0xF7 /**< CrSel key */
#define SGUI_KC_EXSEL      0xF8 /**< ExSel key */
#define SGUI_KC_EREOF      0xF9 /**< Erase EOF key */
#define SGUI_KC_PLAY       0xFA /**< Play key */
#define SGUI_KC_ZOOM       0xFB /**< Zoom key */
#define SGUI_KC_PA1        0xFD /**< PA1 key */
#define SGUI_KC_OEM_CLEAR  0xFE /**< Clear key */



#endif /* SGUI_KEYCODES_H */

