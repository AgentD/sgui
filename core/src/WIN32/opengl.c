/*
 * opengl.c
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
#include "sgui_opengl.h"
#include "sgui_event.h"
#include "internal.h"

#ifndef SGUI_NO_OPENGL
typedef HGLRC (* WGLCREATECONTEXTATTRIBSARBPROC )( HDC, HGLRC, const int* );

#define WGL_CONTEXT_MAJOR_VERSION_ARB   0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB   0x2092
#define WGL_CONTEXT_LAYER_PLANE_ARB     0x2093
#define WGL_CONTEXT_FLAGS_ARB           0x2094
#define WGL_CONTEXT_PROFILE_MASK_ARB    0x9126

#define WGL_CONTEXT_DEBUG_BIT_ARB               0x0001
#define WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB  0x0002

#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB          0x00000001
#define WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB 0x00000002



static int set_pixel_format( HDC hDC )
{
    PIXELFORMATDESCRIPTOR pfd;
    int format;

    memset( &pfd, 0, sizeof( pfd ) );

    pfd.nSize      = sizeof( pfd );
    pfd.nVersion   = 1;
    pfd.dwFlags    = PFD_DRAW_TO_WINDOW|PFD_SUPPORT_OPENGL|PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 24;
    pfd.cDepthBits = 24;
    pfd.iLayerType = PFD_MAIN_PLANE;

    format = ChoosePixelFormat( hDC, &pfd );

    return SetPixelFormat( hDC, format, &pfd );
}

int create_gl_context( sgui_window_w32* wnd, int compatibillity )
{
    WGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = NULL;
    sgui_window_w32* share_wnd;
    int attribs[10], major, minor;
    HGLRC temp, oldctx, share;
    HDC olddc;

    /* find an existing window with an OpenGL context */
    share_wnd = find_gl_window( );

    share = share_wnd ? share_wnd->hRC : 0;

    /* get a device context and set a pixel format */
    wnd->hDC = GetDC( wnd->hWnd );

    if( !wnd->hDC || !set_pixel_format( wnd->hDC ) )
        return 0;

    /* create an old fashioned OpenGL temporary context */
    temp = wglCreateContext( wnd->hDC );

    if( !temp )
        return 0;

    /* try to make it current */
    oldctx = wglGetCurrentContext( );
    olddc = wglGetCurrentDC( );

    if( !wglMakeCurrent( wnd->hDC, temp ) )
        return 0;

    /* load the new context creation function */
    wglCreateContextAttribsARB = (WGLCREATECONTEXTATTRIBSARBPROC)
    wglGetProcAddress( "wglCreateContextAttribsARB" );

    /* try to create a new context */
    wnd->hRC = 0;

    if( wglCreateContextAttribsARB )
    {
        /* fill attrib array */
        attribs[0] = WGL_CONTEXT_MAJOR_VERSION_ARB;
        attribs[1] = 0;
        attribs[2] = WGL_CONTEXT_MINOR_VERSION_ARB;
        attribs[3] = 0;
        attribs[4] = WGL_CONTEXT_PROFILE_MASK_ARB;

        if( compatibillity )
        {
            attribs[5] = WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB;
            attribs[6] = 0;
        }
        else
        {
            attribs[5] = WGL_CONTEXT_CORE_PROFILE_BIT_ARB;
            attribs[6] = WGL_CONTEXT_FLAGS_ARB;
            attribs[7] = WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB;
            attribs[8] = 0;
        }

        /* try to create 4.3 down to 3.0 context */
        for( major=4; !wnd->hRC && major>=3; --major )
        {
            for( minor=3; !wnd->hRC && minor>=0; --minor )
            {
                attribs[1] = major;
                attribs[3] = minor;
                wnd->hRC = wglCreateContextAttribsARB( wnd->hDC, share,
                                                       attribs );
            }
        }

        /* try to create 2.x context */
        for( minor=1; !wnd->hRC && minor>=0; --minor )
        {
            attribs[1] = 2;
            attribs[3] = minor;
            wnd->hRC = wglCreateContextAttribsARB( wnd->hDC, share, attribs );
        }

        /* try to create 1.x context */
        for( minor=5; !wnd->hRC && minor>=0; --minor )
        {
            attribs[1] = 1;
            attribs[3] = minor;
            wnd->hRC = wglCreateContextAttribsARB( wnd->hDC, share, attribs );
        }
    }

    /* restore the privous context */
    wglMakeCurrent( olddc, oldctx );

    /* delete the temporary context on success, use it instead otherwise */
    if( wnd->hRC )
    {
        wglDeleteContext( temp );
    }
    else
    {
        wnd->hRC = temp;

        if( share )
            wglShareLists( wnd->hRC, share );
    }

    return 1;
}

void destroy_gl_context( sgui_window_w32* wnd )
{
    if( wnd->hRC )
        wglDeleteContext( wnd->hRC );

    if( wnd->hDC )
        ReleaseDC( wnd->hWnd, wnd->hDC );
}

void gl_swap_buffers( sgui_window* wnd )
{
    SwapBuffers( ((sgui_window_w32*)wnd)->hDC );
}

void gl_make_current( sgui_window_w32* wnd )
{
    if( wnd )
        wglMakeCurrent( wnd->hDC, wnd->hRC );
    else
        wglMakeCurrent( NULL, NULL );
}
#else
int create_gl_context( sgui_window_w32* wnd, int compatibillity )
{
    (void)wnd;
    (void)compatibillity;
    return 0;
}

void destroy_gl_context( sgui_window_w32* wnd )
{
    (void)wnd;
}

void gl_swap_buffers( sgui_window_w32* wnd )
{
    (void)wnd;
}

void gl_make_current( sgui_window_w32* wnd )
{
    (void)wnd;
}
#endif

