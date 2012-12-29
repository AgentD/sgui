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
#ifndef SGUI_KEYCODES_H
#define SGUI_KEYCODES_H



typedef enum
{
    SGUI_KC_UNKNOWN    = 0x00, /**< ANY key */
    SGUI_KC_BACK       = 0x08, /**< BACKSPACE key */
    SGUI_KC_TAB        = 0x09, /**< TAB key */
    SGUI_KC_CLEAR      = 0x0C, /**< CLEAR key */
    SGUI_KC_RETURN     = 0x0D, /**< ENTER key */
    SGUI_KC_SHIFT      = 0x10, /**< SHIFT key */
    SGUI_KC_CONTROL    = 0x11, /**< CTRL key */
    SGUI_KC_ALT        = 0x12, /**< ALT key */
    SGUI_KC_PAUSE      = 0x13, /**< PAUSE key */
    SGUI_KC_CAPITAL    = 0x14, /**< CAPS LOCK key */
    SGUI_KC_KANA       = 0x15, /**< IME Kana mode */
    SGUI_KC_HANGUEL    = 0x15, /**< IME Hanguel mode */
    SGUI_KC_HANGUL     = 0x15, /**< IME Hangul mode */
    SGUI_KC_JUNJA      = 0x17, /**< IME Junja mode */
    SGUI_KC_FINAL      = 0x18, /**< IME final mode */
    SGUI_KC_HANJA      = 0x19, /**< IME Hanja mode */
    SGUI_KC_KANJI      = 0x19, /**< IME Kanji mode */
    SGUI_KC_ESCAPE     = 0x1B, /**< ESC key */
    SGUI_KC_CONVERT    = 0x1C, /**< IME convert */
    SGUI_KC_NONCONVERT = 0x1D, /**< IME nonconvert */
    SGUI_KC_ACCEPT     = 0x1E, /**< IME accept */
    SGUI_KC_MODECHANGE = 0x1F, /**< IME mode change request */
    SGUI_KC_SPACE      = 0x20, /**< SPACEBAR */
    SGUI_KC_PRIOR      = 0x21, /**< PAGE UP key */
    SGUI_KC_NEXT       = 0x22, /**< PAGE DOWN key */
    SGUI_KC_END        = 0x23, /**< END key */
    SGUI_KC_HOME       = 0x24, /**< HOME key */
    SGUI_KC_LEFT       = 0x25, /**< LEFT ARROW key */
    SGUI_KC_UP         = 0x26, /**< UP ARROW key */
    SGUI_KC_RIGHT      = 0x27, /**< RIGHT ARROW key */
    SGUI_KC_DOWN       = 0x28, /**< DOWN ARROW key */
    SGUI_KC_SELECT     = 0x29, /**< SELECT key */
    SGUI_KC_PRINT      = 0x2A, /**< PRINT key */
    SGUI_KC_EXECUTE    = 0x2B, /**< EXECUTE key */
    SGUI_KC_SNAPSHOT   = 0x2C, /**< PRINT SCREEN key */
    SGUI_KC_INSERT     = 0x2D, /**< INS key */
    SGUI_KC_DELETE     = 0x2E, /**< DEL key */
    SGUI_KC_HELP       = 0x2F, /**< HELP key */
    SGUI_KC_0          = 0x30, /**< 0 key */
    SGUI_KC_1          = 0x31, /**< 1 key */
    SGUI_KC_2          = 0x32, /**< 2 key */
    SGUI_KC_3          = 0x33, /**< 3 key */
    SGUI_KC_4          = 0x34, /**< 4 key */
    SGUI_KC_5          = 0x35, /**< 5 key */
    SGUI_KC_6          = 0x36, /**< 6 key */
    SGUI_KC_7          = 0x37, /**< 7 key */
    SGUI_KC_8          = 0x38, /**< 8 key */
    SGUI_KC_9          = 0x39, /**< 9 key */
    SGUI_KC_A          = 0x41, /**< A key */
    SGUI_KC_B          = 0x42, /**< B key */
    SGUI_KC_C          = 0x43, /**< C key */
    SGUI_KC_D          = 0x44, /**< D key */
    SGUI_KC_E          = 0x45, /**< E key */
    SGUI_KC_F          = 0x46, /**< F key */
    SGUI_KC_G          = 0x47, /**< G key */
    SGUI_KC_H          = 0x48, /**< H key */
    SGUI_KC_I          = 0x49, /**< I key */
    SGUI_KC_J          = 0x4A, /**< J key */
    SGUI_KC_K          = 0x4B, /**< K key */
    SGUI_KC_L          = 0x4C, /**< L key */
    SGUI_KC_M          = 0x4D, /**< M key */
    SGUI_KC_N          = 0x4E, /**< N key */
    SGUI_KC_O          = 0x4F, /**< O key */
    SGUI_KC_P          = 0x50, /**< P key */
    SGUI_KC_Q          = 0x51, /**< Q key */
    SGUI_KC_R          = 0x52, /**< R key */
    SGUI_KC_S          = 0x53, /**< S key */
    SGUI_KC_T          = 0x54, /**< T key */
    SGUI_KC_U          = 0x55, /**< U key */
    SGUI_KC_V          = 0x56, /**< V key */
    SGUI_KC_W          = 0x57, /**< W key */
    SGUI_KC_X          = 0x58, /**< X key */
    SGUI_KC_Y          = 0x59, /**< Y key */
    SGUI_KC_Z          = 0x5A, /**< Z key */
    SGUI_KC_LSUPER     = 0x5B, /**< Left Super (Natural keyboard) */
    SGUI_KC_RSUPER     = 0x5C, /**< Right Super(Natural keyboard) */
    SGUI_KC_APPS       = 0x5D, /**< Applications key (Natural keyboard) */
    SGUI_KC_SLEEP      = 0x5F, /**< Computer Sleep key */
    SGUI_KC_NUM0       = 0x60, /**< Numpad 0 key */
    SGUI_KC_NUM1       = 0x61, /**< Numpad 1 key */
    SGUI_KC_NUM2       = 0x62, /**< Numpad 2 key */
    SGUI_KC_NUM3       = 0x63, /**< Numpad 3 key */
    SGUI_KC_NUM4       = 0x64, /**< Numpad 4 key */
    SGUI_KC_NUM5       = 0x65, /**< Numpad 5 key */
    SGUI_KC_NUM6       = 0x66, /**< Numpad 6 key */
    SGUI_KC_NUM7       = 0x67, /**< Numpad 7 key */
    SGUI_KC_NUM8       = 0x68, /**< Numpad 8 key */
    SGUI_KC_NUM9       = 0x69, /**< Numpad 9 key */
    SGUI_KC_MULTIPLY   = 0x6A, /**< Multiply key */
    SGUI_KC_ADD        = 0x6B, /**< Add key */
    SGUI_KC_SEPARATOR  = 0x6C, /**< Separator key */
    SGUI_KC_SUBTRACT   = 0x6D, /**< Subtract key */
    SGUI_KC_DECIMAL    = 0x6E, /**< Decimal key */
    SGUI_KC_DIVIDE     = 0x6F, /**< Divide key */
    SGUI_KC_F1         = 0x70, /**< F1 key */
    SGUI_KC_F2         = 0x71, /**< F2 key */
    SGUI_KC_F3         = 0x72, /**< F3 key */
    SGUI_KC_F4         = 0x73, /**< F4 key */
    SGUI_KC_F5         = 0x74, /**< F5 key */
    SGUI_KC_F6         = 0x75, /**< F6 key */
    SGUI_KC_F7         = 0x76, /**< F7 key */
    SGUI_KC_F8         = 0x77, /**< F8 key */
    SGUI_KC_F9         = 0x78, /**< F9 key */
    SGUI_KC_F10        = 0x79, /**< F10 key */
    SGUI_KC_F11        = 0x7A, /**< F11 key */
    SGUI_KC_F12        = 0x7B, /**< F12 key */
    SGUI_KC_F13        = 0x7C, /**< F13 key */
    SGUI_KC_F14        = 0x7D, /**< F14 key */
    SGUI_KC_F15        = 0x7E, /**< F15 key */
    SGUI_KC_F16        = 0x7F, /**< F16 key */
    SGUI_KC_F17        = 0x80, /**< F17 key */
    SGUI_KC_F18        = 0x81, /**< F18 key */
    SGUI_KC_F19        = 0x82, /**< F19 key */
    SGUI_KC_F20        = 0x83, /**< F20 key */
    SGUI_KC_F21        = 0x84, /**< F21 key */
    SGUI_KC_F22        = 0x85, /**< F22 key */
    SGUI_KC_F23        = 0x86, /**< F23 key */
    SGUI_KC_F24        = 0x87, /**< F24 key */
    SGUI_KC_NUMLOCK    = 0x90, /**< NUM LOCK key */
    SGUI_KC_SCROLL     = 0x91, /**< SCROLL LOCK key */
    SGUI_KC_LSHIFT     = 0xA0, /**< Left SHIFT key */
    SGUI_KC_RSHIFT     = 0xA1, /**< Right SHIFT key */
    SGUI_KC_LCONTROL   = 0xA2, /**< Left CONTROL key */
    SGUI_KC_RCONTROL   = 0xA3, /**< Right CONTROL key */
    SGUI_KC_LALT       = 0xA4, /**< Left ALT key */
    SGUI_KC_RALT       = 0xA5, /**< Right ALT key */
    SGUI_KC_OEM_PLUS   = 0xBB, /**< For any country/region, the '+' key */
    SGUI_KC_OEM_COMMA  = 0xBC, /**< For any country/region, the ',' key */
    SGUI_KC_OEM_MINUS  = 0xBD, /**< For any country/region, the '-' key */
    SGUI_KC_OEM_PERIOD = 0xBE, /**< For any country/region, the '.' key */
    SGUI_KC_ATTN       = 0xF6, /**< Attn key */
    SGUI_KC_CRSEL      = 0xF7, /**< CrSel key */
    SGUI_KC_EXSEL      = 0xF8, /**< ExSel key */
    SGUI_KC_EREOF      = 0xF9, /**< Erase EOF key */
    SGUI_KC_PLAY       = 0xFA, /**< Play key */
    SGUI_KC_ZOOM       = 0xFB, /**< Zoom key */
    SGUI_KC_PA1        = 0xFD, /**< PA1 key */
    SGUI_KC_OEM_CLEAR  = 0xFE  /**< Clear key */
}
SGUI_KEY_CODE;



#endif /* SGUI_KEYCODES_H */

