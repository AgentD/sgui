/*
 * keycode_translate.c
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
#include "internal.h"



/************************* key symbol lookup table *************************/
static struct
{
    KeySym key;
    SGUI_KEY_CODE code;
}
key_entries[ 200 ];

static unsigned int num_key_entries = 0;



void key_entries_add( KeySym key, SGUI_KEY_CODE code )
{
    unsigned int i, j;

    for( i=0; i<num_key_entries; ++i )
    {
        if( key_entries[ i ].key > key )
        {
            for( j=num_key_entries; j>i; --j )
            {
                key_entries[ j ].key  = key_entries[ j-1 ].key;
                key_entries[ j ].code = key_entries[ j-1 ].code;
            }
            key_entries[ i ].key  = key;
            key_entries[ i ].code = code;
            ++num_key_entries;
            return;
        }
    }

    key_entries[ num_key_entries ].key  = key;
    key_entries[ num_key_entries ].code = code;
    ++num_key_entries;
}

SGUI_KEY_CODE key_entries_translate( KeySym key )
{
    unsigned int i, l = 0, u = num_key_entries;

    while( l < u )
    {
        i = (l + u) >> 1;

        if( key < key_entries[ i ].key )
            u = i;
        else if( key > key_entries[ i ].key )
            l = i + 1;
        else
            return key_entries[ i ].code;
    }

    return SGUI_KC_UNKNOWN;
}

/***************************************************************************/

void init_keycodes( )
{
    if( !num_key_entries )
    {
        key_entries_add( XK_BackSpace,        SGUI_KC_BACK       );
        key_entries_add( XK_Tab,              SGUI_KC_TAB        );
        key_entries_add( XK_Linefeed,         SGUI_KC_RETURN     );
        key_entries_add( XK_Clear,            SGUI_KC_CLEAR      );
        key_entries_add( XK_Return,           SGUI_KC_RETURN     );
        key_entries_add( XK_Pause,            SGUI_KC_PAUSE      );
        key_entries_add( XK_Scroll_Lock,      SGUI_KC_SCROLL     );
        key_entries_add( XK_Sys_Req,          SGUI_KC_SNAPSHOT   );
        key_entries_add( XK_Escape,           SGUI_KC_ESCAPE     );
        key_entries_add( XK_Delete,           SGUI_KC_DELETE     );
        key_entries_add( XK_Home,             SGUI_KC_HOME       );
        key_entries_add( XK_Left,             SGUI_KC_LEFT       );
        key_entries_add( XK_Up,               SGUI_KC_UP         );
        key_entries_add( XK_Right,            SGUI_KC_RIGHT      );
        key_entries_add( XK_Down,             SGUI_KC_DOWN       );
        key_entries_add( XK_Prior,            SGUI_KC_PRIOR      );
        key_entries_add( XK_Page_Up,          SGUI_KC_PRIOR      );
        key_entries_add( XK_Next,             SGUI_KC_NEXT       );
        key_entries_add( XK_Page_Down,        SGUI_KC_NEXT       );
        key_entries_add( XK_End,              SGUI_KC_END        );
        key_entries_add( XK_Begin,            SGUI_KC_HOME       );
        key_entries_add( XK_Print,            SGUI_KC_PRINT      );
        key_entries_add( XK_Execute,          SGUI_KC_EXECUTE    );
        key_entries_add( XK_Insert,           SGUI_KC_INSERT     );
        key_entries_add( XK_Menu,             SGUI_KC_ALT        );
        key_entries_add( XK_Num_Lock,         SGUI_KC_NUMLOCK    );
        key_entries_add( XK_KP_Space,         SGUI_KC_SPACE      );
        key_entries_add( XK_KP_Tab,           SGUI_KC_TAB        );
        key_entries_add( XK_KP_Enter,         SGUI_KC_RETURN     );
        key_entries_add( XK_KP_F1,            SGUI_KC_F1         );
        key_entries_add( XK_KP_F2,            SGUI_KC_F2         );
        key_entries_add( XK_KP_F3,            SGUI_KC_F3         );
        key_entries_add( XK_KP_F4,            SGUI_KC_F4         );
        key_entries_add( XK_KP_Home,          SGUI_KC_HOME       );
        key_entries_add( XK_KP_Left,          SGUI_KC_LEFT       );
        key_entries_add( XK_KP_Up,            SGUI_KC_UP         );
        key_entries_add( XK_KP_Right,         SGUI_KC_RIGHT      );
        key_entries_add( XK_KP_Down,          SGUI_KC_DOWN       );
        key_entries_add( XK_KP_Prior,         SGUI_KC_PRIOR      );
        key_entries_add( XK_KP_Page_Up,       SGUI_KC_PRIOR      );
        key_entries_add( XK_KP_Next,          SGUI_KC_NEXT       );
        key_entries_add( XK_KP_Page_Down,     SGUI_KC_NEXT       );
        key_entries_add( XK_KP_End,           SGUI_KC_END        );
        key_entries_add( XK_KP_Begin,         SGUI_KC_HOME       );
        key_entries_add( XK_KP_Insert,        SGUI_KC_INSERT     );
        key_entries_add( XK_KP_Delete,        SGUI_KC_DELETE     );
        key_entries_add( XK_KP_Multiply,      SGUI_KC_MULTIPLY   );
        key_entries_add( XK_KP_Add,           SGUI_KC_ADD        );
        key_entries_add( XK_KP_Separator,     SGUI_KC_SEPARATOR  );
        key_entries_add( XK_KP_Subtract,      SGUI_KC_SUBTRACT   );
        key_entries_add( XK_KP_Decimal,       SGUI_KC_DECIMAL    );
        key_entries_add( XK_KP_Divide,        SGUI_KC_DIVIDE     );
        key_entries_add( XK_KP_0,             SGUI_KC_NUM0       );
        key_entries_add( XK_KP_1,             SGUI_KC_NUM1       );
        key_entries_add( XK_KP_2,             SGUI_KC_NUM2       );
        key_entries_add( XK_KP_3,             SGUI_KC_NUM3       );
        key_entries_add( XK_KP_4,             SGUI_KC_NUM4       );
        key_entries_add( XK_KP_5,             SGUI_KC_NUM5       );
        key_entries_add( XK_KP_6,             SGUI_KC_NUM6       );
        key_entries_add( XK_KP_7,             SGUI_KC_NUM7       );
        key_entries_add( XK_KP_8,             SGUI_KC_NUM8       );
        key_entries_add( XK_KP_9,             SGUI_KC_NUM9       );
        key_entries_add( XK_F1,               SGUI_KC_F1         );
        key_entries_add( XK_F2,               SGUI_KC_F2         );
        key_entries_add( XK_F3,               SGUI_KC_F3         );
        key_entries_add( XK_F4,               SGUI_KC_F4         );
        key_entries_add( XK_F5,               SGUI_KC_F5         );
        key_entries_add( XK_F6,               SGUI_KC_F6         );
        key_entries_add( XK_F7,               SGUI_KC_F7         );
        key_entries_add( XK_F8,               SGUI_KC_F8         );
        key_entries_add( XK_F9,               SGUI_KC_F9         );
        key_entries_add( XK_F10,              SGUI_KC_F10        );
        key_entries_add( XK_F11,              SGUI_KC_F11        );
        key_entries_add( XK_F12,              SGUI_KC_F12        );
        key_entries_add( XK_F13,              SGUI_KC_F13        );
        key_entries_add( XK_F14,              SGUI_KC_F14        );
        key_entries_add( XK_F15,              SGUI_KC_F15        );
        key_entries_add( XK_F16,              SGUI_KC_F16        );
        key_entries_add( XK_F17,              SGUI_KC_F17        );
        key_entries_add( XK_F18,              SGUI_KC_F18        );
        key_entries_add( XK_F19,              SGUI_KC_F19        );
        key_entries_add( XK_F20,              SGUI_KC_F20        );
        key_entries_add( XK_F21,              SGUI_KC_F21        );
        key_entries_add( XK_F22,              SGUI_KC_F22        );
        key_entries_add( XK_F23,              SGUI_KC_F23        );
        key_entries_add( XK_F24,              SGUI_KC_F24        );
        key_entries_add( XK_Shift_L,          SGUI_KC_LSHIFT     );
        key_entries_add( XK_Shift_R,          SGUI_KC_RSHIFT     );
        key_entries_add( XK_Control_L,        SGUI_KC_LCONTROL   );
        key_entries_add( XK_Control_R,        SGUI_KC_RCONTROL   );
        key_entries_add( XK_Caps_Lock,        SGUI_KC_CAPITAL    );
        key_entries_add( XK_Shift_Lock,       SGUI_KC_CAPITAL    );
        key_entries_add( XK_Alt_L,            SGUI_KC_LALT       );
        key_entries_add( XK_Alt_R,            SGUI_KC_RALT       );
        key_entries_add( XK_Super_L,          SGUI_KC_LSUPER     );
        key_entries_add( XK_Super_R,          SGUI_KC_RSUPER     );
        key_entries_add( XK_ISO_Level3_Shift, SGUI_KC_RALT       );
        key_entries_add( XK_space,            SGUI_KC_SPACE      );
        key_entries_add( XK_plus,             SGUI_KC_OEM_PLUS   );
        key_entries_add( XK_comma,            SGUI_KC_OEM_COMMA  );
        key_entries_add( XK_minus,            SGUI_KC_OEM_MINUS  );
        key_entries_add( XK_period,           SGUI_KC_OEM_PERIOD );
        key_entries_add( XK_0,                SGUI_KC_0          );
        key_entries_add( XK_1,                SGUI_KC_1          );
        key_entries_add( XK_2,                SGUI_KC_2          );
        key_entries_add( XK_3,                SGUI_KC_3          );
        key_entries_add( XK_4,                SGUI_KC_4          );
        key_entries_add( XK_5,                SGUI_KC_5          );
        key_entries_add( XK_6,                SGUI_KC_6          );
        key_entries_add( XK_7,                SGUI_KC_7          );
        key_entries_add( XK_8,                SGUI_KC_8          );
        key_entries_add( XK_9,                SGUI_KC_9          );
        key_entries_add( XK_A,                SGUI_KC_A          );
        key_entries_add( XK_B,                SGUI_KC_B          );
        key_entries_add( XK_C,                SGUI_KC_C          );
        key_entries_add( XK_D,                SGUI_KC_D          );
        key_entries_add( XK_E,                SGUI_KC_E          );
        key_entries_add( XK_F,                SGUI_KC_F          );
        key_entries_add( XK_G,                SGUI_KC_G          );
        key_entries_add( XK_H,                SGUI_KC_H          );
        key_entries_add( XK_I,                SGUI_KC_I          );
        key_entries_add( XK_J,                SGUI_KC_J          );
        key_entries_add( XK_K,                SGUI_KC_K          );
        key_entries_add( XK_L,                SGUI_KC_L          );
        key_entries_add( XK_M,                SGUI_KC_M          );
        key_entries_add( XK_N,                SGUI_KC_N          );
        key_entries_add( XK_O,                SGUI_KC_O          );
        key_entries_add( XK_P,                SGUI_KC_P          );
        key_entries_add( XK_Q,                SGUI_KC_Q          );
        key_entries_add( XK_R,                SGUI_KC_R          );
        key_entries_add( XK_S,                SGUI_KC_S          );
        key_entries_add( XK_T,                SGUI_KC_T          );
        key_entries_add( XK_U,                SGUI_KC_U          );
        key_entries_add( XK_V,                SGUI_KC_V          );
        key_entries_add( XK_W,                SGUI_KC_W          );
        key_entries_add( XK_X,                SGUI_KC_X          );
        key_entries_add( XK_Y,                SGUI_KC_Y          );
        key_entries_add( XK_Z,                SGUI_KC_Z          );
        key_entries_add( XK_a,                SGUI_KC_A          );
        key_entries_add( XK_b,                SGUI_KC_B          );
        key_entries_add( XK_c,                SGUI_KC_C          );
        key_entries_add( XK_d,                SGUI_KC_D          );
        key_entries_add( XK_e,                SGUI_KC_E          );
        key_entries_add( XK_f,                SGUI_KC_F          );
        key_entries_add( XK_g,                SGUI_KC_G          );
        key_entries_add( XK_h,                SGUI_KC_H          );
        key_entries_add( XK_i,                SGUI_KC_I          );
        key_entries_add( XK_j,                SGUI_KC_J          );
        key_entries_add( XK_k,                SGUI_KC_K          );
        key_entries_add( XK_l,                SGUI_KC_L          );
        key_entries_add( XK_m,                SGUI_KC_M          );
        key_entries_add( XK_n,                SGUI_KC_N          );
        key_entries_add( XK_o,                SGUI_KC_O          );
        key_entries_add( XK_p,                SGUI_KC_P          );
        key_entries_add( XK_q,                SGUI_KC_Q          );
        key_entries_add( XK_r,                SGUI_KC_R          );
        key_entries_add( XK_s,                SGUI_KC_S          );
        key_entries_add( XK_t,                SGUI_KC_T          );
        key_entries_add( XK_u,                SGUI_KC_U          );
        key_entries_add( XK_v,                SGUI_KC_V          );
        key_entries_add( XK_w,                SGUI_KC_W          );
        key_entries_add( XK_x,                SGUI_KC_X          );
        key_entries_add( XK_y,                SGUI_KC_Y          );
        key_entries_add( XK_z,                SGUI_KC_Z          );
    }
}

SGUI_KEY_CODE keycode_from_XKeyEvent( XKeyEvent* xkey )
{
    KeySym sym;

    sym = XLookupKeysym( xkey, 0 );

    return key_entries_translate( sym );
}

