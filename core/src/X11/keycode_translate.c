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
#define SGUI_BUILDING_DLL
#include "platform.h"



typedef struct
{
    unsigned int key;
    int code;
}
key_entry;



static key_entry key_entries[ 164 ] =
{
    { XK_BackSpace,        SGUI_KC_BACK         },
    { XK_Tab,              SGUI_KC_TAB          },
    { XK_Linefeed,         SGUI_KC_RETURN       },
    { XK_Clear,            SGUI_KC_CLEAR        },
    { XK_Return,           SGUI_KC_RETURN       },
    { XK_Pause,            SGUI_KC_PAUSE        },
    { XK_Scroll_Lock,      SGUI_KC_SCROLL       },
    { XK_Sys_Req,          SGUI_KC_SNAPSHOT     },
    { XK_Escape,           SGUI_KC_ESCAPE       },
    { XK_Delete,           SGUI_KC_DELETE       },
    { XK_Home,             SGUI_KC_HOME         },
    { XK_Left,             SGUI_KC_LEFT         },
    { XK_Up,               SGUI_KC_UP           },
    { XK_Right,            SGUI_KC_RIGHT        },
    { XK_Down,             SGUI_KC_DOWN         },
    { XK_Prior,            SGUI_KC_PRIOR        },
    { XK_Page_Up,          SGUI_KC_PRIOR        },
    { XK_Next,             SGUI_KC_NEXT         },
    { XK_Page_Down,        SGUI_KC_NEXT         },
    { XK_End,              SGUI_KC_END          },
    { XK_Begin,            SGUI_KC_HOME         },
    { XK_Print,            SGUI_KC_PRINT        },
    { XK_Execute,          SGUI_KC_EXECUTE      },
    { XK_Insert,           SGUI_KC_INSERT       },
    { XK_Menu,             SGUI_KC_ALT          },
    { XK_Num_Lock,         SGUI_KC_NUMLOCK      },
    { XK_KP_Space,         SGUI_KC_SPACE        },
    { XK_KP_Tab,           SGUI_KC_TAB          },
    { XK_KP_Enter,         SGUI_KC_RETURN       },
    { XK_KP_F1,            SGUI_KC_F1           },
    { XK_KP_F2,            SGUI_KC_F2           },
    { XK_KP_F3,            SGUI_KC_F3           },
    { XK_KP_F4,            SGUI_KC_F4           },
    { XK_KP_Home,          SGUI_KC_HOME         },
    { XK_KP_Left,          SGUI_KC_LEFT         },
    { XK_KP_Up,            SGUI_KC_UP           },
    { XK_KP_Right,         SGUI_KC_RIGHT        },
    { XK_KP_Down,          SGUI_KC_DOWN         },
    { XK_KP_Prior,         SGUI_KC_PRIOR        },
    { XK_KP_Page_Up,       SGUI_KC_PRIOR        },
    { XK_KP_Next,          SGUI_KC_NEXT         },
    { XK_KP_Page_Down,     SGUI_KC_NEXT         },
    { XK_KP_End,           SGUI_KC_END          },
    { XK_KP_Begin,         SGUI_KC_HOME         },
    { XK_KP_Insert,        SGUI_KC_INSERT       },
    { XK_KP_Delete,        SGUI_KC_DELETE       },
    { XK_KP_Multiply,      SGUI_KC_MULTIPLY     },
    { XK_KP_Add,           SGUI_KC_ADD          },
    { XK_KP_Separator,     SGUI_KC_SEPARATOR    },
    { XK_KP_Subtract,      SGUI_KC_SUBTRACT     },
    { XK_KP_Decimal,       SGUI_KC_DECIMAL      },
    { XK_KP_Divide,        SGUI_KC_DIVIDE       },
    { XK_KP_0,             SGUI_KC_NUM0         },
    { XK_KP_1,             SGUI_KC_NUM1         },
    { XK_KP_2,             SGUI_KC_NUM2         },
    { XK_KP_3,             SGUI_KC_NUM3         },
    { XK_KP_4,             SGUI_KC_NUM4         },
    { XK_KP_5,             SGUI_KC_NUM5         },
    { XK_KP_6,             SGUI_KC_NUM6         },
    { XK_KP_7,             SGUI_KC_NUM7         },
    { XK_KP_8,             SGUI_KC_NUM8         },
    { XK_KP_9,             SGUI_KC_NUM9         },
    { XK_F1,               SGUI_KC_F1           },
    { XK_F2,               SGUI_KC_F2           },
    { XK_F3,               SGUI_KC_F3           },
    { XK_F4,               SGUI_KC_F4           },
    { XK_F5,               SGUI_KC_F5           },
    { XK_F6,               SGUI_KC_F6           },
    { XK_F7,               SGUI_KC_F7           },
    { XK_F8,               SGUI_KC_F8           },
    { XK_F9,               SGUI_KC_F9           },
    { XK_F10,              SGUI_KC_F10          },
    { XK_F11,              SGUI_KC_F11          },
    { XK_F12,              SGUI_KC_F12          },
    { XK_F13,              SGUI_KC_F13          },
    { XK_F14,              SGUI_KC_F14          },
    { XK_F15,              SGUI_KC_F15          },
    { XK_F16,              SGUI_KC_F16          },
    { XK_F17,              SGUI_KC_F17          },
    { XK_F18,              SGUI_KC_F18          },
    { XK_F19,              SGUI_KC_F19          },
    { XK_F20,              SGUI_KC_F20          },
    { XK_F21,              SGUI_KC_F21          },
    { XK_F22,              SGUI_KC_F22          },
    { XK_F23,              SGUI_KC_F23          },
    { XK_F24,              SGUI_KC_F24          },
    { XK_Shift_L,          SGUI_KC_LSHIFT       },
    { XK_Shift_R,          SGUI_KC_RSHIFT       },
    { XK_Control_L,        SGUI_KC_LCONTROL     },
    { XK_Control_R,        SGUI_KC_RCONTROL     },
    { XK_Caps_Lock,        SGUI_KC_CAPITAL      },
    { XK_Shift_Lock,       SGUI_KC_CAPITAL      },
    { XK_Alt_L,            SGUI_KC_LALT         },
    { XK_Alt_R,            SGUI_KC_RALT         },
    { XK_Super_L,          SGUI_KC_LSUPER       },
    { XK_Super_R,          SGUI_KC_RSUPER       },
    { XK_ISO_Level3_Shift, SGUI_KC_RALT         },
    { XK_space,            SGUI_KC_SPACE        },
    { XK_plus,             SGUI_KC_OEM_PLUS     },
    { XK_comma,            SGUI_KC_OEM_COMMA    },
    { XK_minus,            SGUI_KC_OEM_MINUS    },
    { XK_period,           SGUI_KC_OEM_PERIOD   },
    { XK_0,                SGUI_KC_0            },
    { XK_1,                SGUI_KC_1            },
    { XK_2,                SGUI_KC_2            },
    { XK_3,                SGUI_KC_3            },
    { XK_4,                SGUI_KC_4            },
    { XK_5,                SGUI_KC_5            },
    { XK_6,                SGUI_KC_6            },
    { XK_7,                SGUI_KC_7            },
    { XK_8,                SGUI_KC_8            },
    { XK_9,                SGUI_KC_9            },
    { XK_A,                SGUI_KC_A            },
    { XK_B,                SGUI_KC_B            },
    { XK_C,                SGUI_KC_C            },
    { XK_D,                SGUI_KC_D            },
    { XK_E,                SGUI_KC_E            },
    { XK_F,                SGUI_KC_F            },
    { XK_G,                SGUI_KC_G            },
    { XK_H,                SGUI_KC_H            },
    { XK_I,                SGUI_KC_I            },
    { XK_J,                SGUI_KC_J            },
    { XK_K,                SGUI_KC_K            },
    { XK_L,                SGUI_KC_L            },
    { XK_M,                SGUI_KC_M            },
    { XK_N,                SGUI_KC_N            },
    { XK_O,                SGUI_KC_O            },
    { XK_P,                SGUI_KC_P            },
    { XK_Q,                SGUI_KC_Q            },
    { XK_R,                SGUI_KC_R            },
    { XK_S,                SGUI_KC_S            },
    { XK_T,                SGUI_KC_T            },
    { XK_U,                SGUI_KC_U            },
    { XK_V,                SGUI_KC_V            },
    { XK_W,                SGUI_KC_W            },
    { XK_X,                SGUI_KC_X            },
    { XK_Y,                SGUI_KC_Y            },
    { XK_Z,                SGUI_KC_Z            },
    { XK_a,                SGUI_KC_A            },
    { XK_b,                SGUI_KC_B            },
    { XK_c,                SGUI_KC_C            },
    { XK_d,                SGUI_KC_D            },
    { XK_e,                SGUI_KC_E            },
    { XK_f,                SGUI_KC_F            },
    { XK_g,                SGUI_KC_G            },
    { XK_h,                SGUI_KC_H            },
    { XK_i,                SGUI_KC_I            },
    { XK_j,                SGUI_KC_J            },
    { XK_k,                SGUI_KC_K            },
    { XK_l,                SGUI_KC_L            },
    { XK_m,                SGUI_KC_M            },
    { XK_n,                SGUI_KC_N            },
    { XK_o,                SGUI_KC_O            },
    { XK_p,                SGUI_KC_P            },
    { XK_q,                SGUI_KC_Q            },
    { XK_r,                SGUI_KC_R            },
    { XK_s,                SGUI_KC_S            },
    { XK_t,                SGUI_KC_T            },
    { XK_u,                SGUI_KC_U            },
    { XK_v,                SGUI_KC_V            },
    { XK_w,                SGUI_KC_W            },
    { XK_x,                SGUI_KC_X            },
    { XK_y,                SGUI_KC_Y            },
    { XK_z,                SGUI_KC_Z            }
};



static int key_entries_compare( const void* a, const void* b )
{
    const key_entry* A = a;
    const key_entry* B = b;
    return (int)A->key - (int)B->key;
}

/*
    search for the coresponding sgui key code for a given KeySym. Performs
    hand coded binary search instead of using bsearch. Needs to be fast;
    called every time a user presses or releases a key.
 */
int key_entries_translate( KeySym key )
{
    unsigned int i, l = 0, u = sizeof(key_entries)/sizeof(key_entries[0]);

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

/*
    Sort the key code lookup table to allow binary search.
    Called only once at library init, so it doesn't need to be that fast and
    can rely on qsort( ).
 */
void init_keycodes( void )
{
    static int is_init = 0;

    if( !is_init )
    {
        qsort( key_entries, sizeof(key_entries)/sizeof(key_entries[0]),
               sizeof(key_entries[0]), key_entries_compare );

        is_init = 1;
    }
}

