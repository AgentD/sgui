/*
 * canvas_gl_tex.c
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
#include "internal_gl.h"



#ifndef SGUI_NO_OPENGL
static sgui_pixmap* canvas_gl_create_pixmap( sgui_canvas* canvas,
                                             unsigned int width,
                                             unsigned int height, int format )
{
    (void)canvas;

    return gl_pixmap_create( width, height, format );
}



sgui_canvas* sgui_opengl_canvas_create( unsigned int width,
                                        unsigned int height,
                                        int compatibillity )
{
    sgui_canvas* cv;

    /* create the propper canvas */
    if( compatibillity )
    {
        cv = gl_canvas_create_compat( width, height );
    }
    else
    {
        cv = gl_canvas_create_core( width, height );
    }

    /* set common function pointers */
    if( cv )
    {
        cv->create_pixmap = canvas_gl_create_pixmap;
    }

    return (sgui_canvas*)cv;
}
#else
sgui_canvas* sgui_opengl_canvas_create( unsigned int width,
                                        unsigned int height )
{
    (void)width;
    (void)height;
    return NULL;
}
#endif

