/*
 * sgui_dll.h
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
#ifndef SGUI_DLL_H
#define SGUI_DLL_H



#ifndef SGUI_WINDOWS
    #if defined(MACHINE_OS_WINDOWS)
        #define SGUI_WINDOWS
    #elif defined(_WIN16) || defined(_WIN32) || defined(_WIN64)
        #define SGUI_WINDOWS
    #elif defined(__TOS_WIN__) || defined(__WINDOWS__) || defined(__WIN32__)
        #define SGUI_WINDOWS
    #endif
#endif



#ifdef SGUI_WINDOWS
    #ifdef SGUI_BUILDING_DLL
        #define SGUI_DLL __declspec(dllexport)
    #else
        #define SGUI_DLL __declspec(dllimport)
    #endif
#else
    #define SGUI_DLL
#endif



#endif /* SGUI_DLL_H */

