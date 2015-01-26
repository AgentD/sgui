/*
 * sgui_d3d9.h
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
 * \file sgui_d3d9.h
 *
 * \brief Contains the Direct3D 9 context implementation.
 */
#ifndef SGUI_D3D9_H
#define SGUI_D3D9_H



#include "sgui_context.h"
#include "sgui_window.h"

#ifndef SGUI_DOXYGEN
    #define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <d3d9.h>



/**
 * \struct sgui_d3d9_context
 *
 * \implements sgui_context
 *
 * \brief A Direct3D 9 context implementation
 */
typedef struct
{
#ifndef SGUI_DOXYGEN
    sgui_context super;
#endif

    sgui_window* wnd;

    /** \brief A pointer to the Direct3D device structure */
	IDirect3DDevice9* device;

	/** \brief The present parameters used */
    D3DPRESENT_PARAMETERS present;
}
sgui_d3d9_context;



#endif /* SGUI_D3D9_H */

