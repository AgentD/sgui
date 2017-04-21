/*
 * sgui_predef.h
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
 * \file sgui_predef.h
 *
 * \brief Contains forward declarations of all datatypes.
 */
#ifndef SGUI_PREDEF_H
#define SGUI_PREDEF_H



#include "sgui_config.h"
#include <stddef.h>

#ifndef _MSC_VER
    #include <stdint.h>
#endif


#if defined(SGUI_WINDOWS) && !defined(__TINYC__)
    #ifdef SGUI_BUILDING_DLL
        #define SGUI_DLL __declspec(dllexport)
    #else
        #define SGUI_DLL __declspec(dllimport)
    #endif
#else
    #define SGUI_DLL
#endif

#ifdef _MSC_VER
    #define SGUI_INLINE __forceinline
    #define SGUI_CONST_INLINE
    #define sgui_strdup _strdup

    typedef unsigned __int8 sgui_u8;
    typedef unsigned __int32 sgui_u32;
    typedef unsigned __int64 sgui_u64;
#else
    #define SGUI_INLINE __inline__ __attribute__((always_inline))
    #define SGUI_CONST_INLINE __attribute__((const)) __inline__
    #define sgui_strdup strdup

    typedef uint8_t sgui_u8;
    typedef uint32_t sgui_u32;
    typedef uint64_t sgui_u64;
#endif



typedef struct sgui_icon sgui_icon;
typedef struct sgui_icon_cache sgui_icon_cache;
typedef struct sgui_font sgui_font;
typedef struct sgui_rect sgui_rect;
typedef struct sgui_canvas sgui_canvas;
typedef struct sgui_widget sgui_widget;
typedef struct sgui_window sgui_window;
typedef struct sgui_pixmap sgui_pixmap;
typedef struct sgui_skin sgui_skin;
typedef struct sgui_context sgui_context;
typedef struct sgui_event sgui_event;
typedef struct sgui_window_description sgui_window_description;
typedef struct sgui_model sgui_model;
typedef struct sgui_item sgui_item;
typedef struct sgui_dialog sgui_dialog;
typedef struct sgui_lib sgui_lib;
typedef union sgui_color sgui_color;

typedef void(* sgui_funptr )( );



#endif /* SGUI_PREDEF_H */

