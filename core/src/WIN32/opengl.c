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
typedef int   (* WGLCHOOSEPIXELFORMATARBPROC )( HDC, const int*, const FLOAT*,
                                                UINT, int*, UINT* );

#define WGL_CONTEXT_MAJOR_VERSION_ARB   0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB   0x2092
#define WGL_CONTEXT_LAYER_PLANE_ARB     0x2093
#define WGL_CONTEXT_FLAGS_ARB           0x2094
#define WGL_CONTEXT_PROFILE_MASK_ARB    0x9126

#define WGL_CONTEXT_DEBUG_BIT_ARB               0x0001
#define WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB  0x0002

#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB          0x00000001
#define WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB 0x00000002



#define WGL_NUMBER_PIXEL_FORMATS_ARB            0x2000
#define WGL_DRAW_TO_WINDOW_ARB                  0x2001
#define WGL_DRAW_TO_BITMAP_ARB                  0x2002
#define WGL_ACCELERATION_ARB                    0x2003
#define WGL_NEED_PALETTE_ARB                    0x2004
#define WGL_NEED_SYSTEM_PALETTE_ARB             0x2005
#define WGL_SWAP_LAYER_BUFFERS_ARB              0x2006
#define WGL_SWAP_METHOD_ARB                     0x2007
#define WGL_NUMBER_OVERLAYS_ARB                 0x2008
#define WGL_NUMBER_UNDERLAYS_ARB                0x2009
#define WGL_TRANSPARENT_ARB                     0x200A
#define WGL_TRANSPARENT_RED_VALUE_ARB           0x2037
#define WGL_TRANSPARENT_GREEN_VALUE_ARB         0x2038
#define WGL_TRANSPARENT_BLUE_VALUE_ARB          0x2039
#define WGL_TRANSPARENT_ALPHA_VALUE_ARB         0x203A
#define WGL_TRANSPARENT_INDEX_VALUE_ARB         0x203B
#define WGL_SHARE_DEPTH_ARB                     0x200C
#define WGL_SHARE_STENCIL_ARB                   0x200D
#define WGL_SHARE_ACCUM_ARB                     0x200E
#define WGL_SUPPORT_GDI_ARB                     0x200F
#define WGL_SUPPORT_OPENGL_ARB                  0x2010
#define WGL_DOUBLE_BUFFER_ARB                   0x2011
#define WGL_STEREO_ARB                          0x2012
#define WGL_PIXEL_TYPE_ARB                      0x2013
#define WGL_COLOR_BITS_ARB                      0x2014
#define WGL_RED_BITS_ARB                        0x2015
#define WGL_RED_SHIFT_ARB                       0x2016
#define WGL_GREEN_BITS_ARB                      0x2017
#define WGL_GREEN_SHIFT_ARB                     0x2018
#define WGL_BLUE_BITS_ARB                       0x2019
#define WGL_BLUE_SHIFT_ARB                      0x201A
#define WGL_ALPHA_BITS_ARB                      0x201B
#define WGL_ALPHA_SHIFT_ARB                     0x201C
#define WGL_ACCUM_BITS_ARB                      0x201D
#define WGL_ACCUM_RED_BITS_ARB                  0x201E
#define WGL_ACCUM_GREEN_BITS_ARB                0x201F
#define WGL_ACCUM_BLUE_BITS_ARB                 0x2020
#define WGL_ACCUM_ALPHA_BITS_ARB                0x2021
#define WGL_DEPTH_BITS_ARB                      0x2022
#define WGL_STENCIL_BITS_ARB                    0x2023
#define WGL_AUX_BUFFERS_ARB                     0x2024
#define WGL_SAMPLE_BUFFERS_ARB                  0x2041
#define WGL_SAMPLES_ARB                         0x2042

#define WGL_NO_ACCELERATION_ARB                 0x2025
#define WGL_GENERIC_ACCELERATION_ARB            0x2026
#define WGL_FULL_ACCELERATION_ARB               0x2027

#define WGL_SWAP_EXCHANGE_ARB                   0x2028
#define WGL_SWAP_COPY_ARB                       0x2029
#define WGL_SWAP_UNDEFINED_ARB                  0x202A

#define WGL_TYPE_RGBA_ARB                       0x202B
#define WGL_TYPE_COLORINDEX_ARB                 0x202C



/*
    On Windows, the ChosePixelFormat / SetPixelFormat functions were used to
    get a pixel format for OpenGL rendering. The first function gets a pointer
    to a structure describing the format und returns a unique ID of the best
    matching format that then gets passed into the later function.

    When multisampling and other things were added, it turned out that this
    was not expandable. So a new function was introduced as an extension that
    accepts an array of key value pairs. So now if we want pixel format
    extensions like multisampling, we have to set up a dummy OpenGL context
    to get the extension, determine the unique identifier, and use the
    identifier in the actual window setup process. Since SetPixelFormat is
    only allowed ONCE per window, we even have to create a dummy window for
    the whole process.

    This is basically what this function does. It takes a new style pixel
    format attribute array as input, derives an old style structure from it,
    determines the pixel format, checks if it can determine the pixel format
    from the actual array, do that, return the identifier.
 */
int determine_pixel_format( int* pixel_attribs, int only_new )
{
    WGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB;
    int pixelformat, format, i, need_new = 0;
    PIXELFORMATDESCRIPTOR pfd;
    HWND tempwnd;
    HGLRC temprc;
    HDC tempdc;
    UINT numFormats;

    /* Generate an old style pixel format descriptor */
    ZeroMemory( &pfd, sizeof(PIXELFORMATDESCRIPTOR) );

    pfd.nSize    = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion = 1;

    for( i=0; pixel_attribs[i]; ++i )
    {
        switch( pixel_attribs[i] )
        {
        case WGL_DRAW_TO_WINDOW_ARB:   pfd.dwFlags = pixel_attribs[ ++i ] ? pfd.dwFlags | PFD_DRAW_TO_WINDOW : pfd.dwFlags & ~PFD_DRAW_TO_WINDOW; break;
        case WGL_DRAW_TO_BITMAP_ARB:   pfd.dwFlags = pixel_attribs[ ++i ] ? pfd.dwFlags | PFD_DRAW_TO_BITMAP : pfd.dwFlags & ~PFD_DRAW_TO_BITMAP; break;
        case WGL_SUPPORT_OPENGL_ARB:   pfd.dwFlags = pixel_attribs[ ++i ] ? pfd.dwFlags | PFD_SUPPORT_OPENGL : pfd.dwFlags & ~PFD_SUPPORT_OPENGL; break;
        case WGL_DOUBLE_BUFFER_ARB:    pfd.dwFlags = pixel_attribs[ ++i ] ? pfd.dwFlags | PFD_DOUBLEBUFFER   : pfd.dwFlags & ~PFD_DOUBLEBUFFER;   break;
        case WGL_SUPPORT_GDI_ARB:      pfd.dwFlags = pixel_attribs[ ++i ] ? pfd.dwFlags | PFD_SUPPORT_GDI    : pfd.dwFlags & ~PFD_SUPPORT_GDI;    break;
        case WGL_STEREO_ARB:           pfd.dwFlags = pixel_attribs[ ++i ] ? pfd.dwFlags | PFD_STEREO         : pfd.dwFlags & ~PFD_STEREO;         break;
        case WGL_PIXEL_TYPE_ARB:       pfd.iPixelType = pixel_attribs[ ++i ]==WGL_TYPE_RGBA_ARB ? PFD_TYPE_RGBA : PFD_TYPE_COLORINDEX; break;
        case WGL_COLOR_BITS_ARB:       pfd.cColorBits      = pixel_attribs[ ++i ]; break;
        case WGL_DEPTH_BITS_ARB:       pfd.cDepthBits      = pixel_attribs[ ++i ]; break;
        case WGL_STENCIL_BITS_ARB:     pfd.cStencilBits    = pixel_attribs[ ++i ]; break;
        case WGL_RED_BITS_ARB:         pfd.cRedBits        = pixel_attribs[ ++i ]; break;
        case WGL_RED_SHIFT_ARB:        pfd.cRedShift       = pixel_attribs[ ++i ]; break;
        case WGL_GREEN_BITS_ARB:       pfd.cGreenBits      = pixel_attribs[ ++i ]; break;
        case WGL_GREEN_SHIFT_ARB:      pfd.cGreenShift     = pixel_attribs[ ++i ]; break;
        case WGL_BLUE_BITS_ARB:        pfd.cBlueBits       = pixel_attribs[ ++i ]; break;
        case WGL_BLUE_SHIFT_ARB:       pfd.cBlueShift      = pixel_attribs[ ++i ]; break;
        case WGL_ALPHA_BITS_ARB:       pfd.cAlphaBits      = pixel_attribs[ ++i ]; break;
        case WGL_ALPHA_SHIFT_ARB:      pfd.cAlphaShift     = pixel_attribs[ ++i ]; break;
        case WGL_ACCUM_BITS_ARB:       pfd.cAccumBits      = pixel_attribs[ ++i ]; break;
        case WGL_ACCUM_RED_BITS_ARB:   pfd.cAccumRedBits   = pixel_attribs[ ++i ]; break;
        case WGL_ACCUM_GREEN_BITS_ARB: pfd.cAccumGreenBits = pixel_attribs[ ++i ]; break;
        case WGL_ACCUM_BLUE_BITS_ARB:  pfd.cAccumBlueBits  = pixel_attribs[ ++i ]; break;
        case WGL_ACCUM_ALPHA_BITS_ARB: pfd.cAccumAlphaBits = pixel_attribs[ ++i ]; break;
        case WGL_AUX_BUFFERS_ARB:      pfd.cAuxBuffers     = pixel_attribs[ ++i ]; break;
        default:
            need_new = 1;
        };
    }

    /* Create a dummy window */
    tempwnd = CreateWindow( wndclass, "", 0, 0, 0, 100, 100,
                            0, 0, hInstance, 0 );

    /* Create a dummy OpenGL context */
    tempdc = GetWindowDC( tempwnd );
    pixelformat = ChoosePixelFormat( tempdc, &pfd );
    SetPixelFormat( tempdc, pixelformat, NULL );
    temprc = wglCreateContext( tempdc );
    wglMakeCurrent( tempdc, temprc );

    /* Try to use the extension to determine the format from the array */
    wglChoosePixelFormatARB = (WGLCHOOSEPIXELFORMATARBPROC)
                               wglGetProcAddress( "wglChoosePixelFormatARB" );

    if( need_new )
    {
        if( wglChoosePixelFormatARB )
        {
            wglChoosePixelFormatARB( tempdc, pixel_attribs, NULL,
                                     1, &format, &numFormats );

            if( numFormats>=1 )
                pixelformat = format;
        }
        else if( only_new )
        {
            pixelformat = 0;
        }
    }

    /* Clean up */
    wglMakeCurrent( 0, 0 );
    wglDeleteContext( temprc );
    ReleaseDC( tempwnd, tempdc );
    DestroyWindow( tempwnd );

    return pixelformat;
}

void set_attributes( int* attr, int bpp, int depth, int stencil,
                     int doublebuffer, int samples )
{
    int i=0;

    if( bpp!=16 || bpp!=24 || bpp!=32 )
        bpp = 32;

    attr[ i++ ] = WGL_DRAW_TO_WINDOW_ARB;
    attr[ i++ ] = GL_TRUE;
    attr[ i++ ] = WGL_SUPPORT_OPENGL_ARB;
    attr[ i++ ] = GL_TRUE;
    attr[ i++ ] = WGL_COLOR_BITS_ARB;
    attr[ i++ ] = bpp;
    attr[ i++ ] = WGL_RED_BITS_ARB;
    attr[ i++ ] = bpp==16 ? 5 : 8;
    attr[ i++ ] = WGL_GREEN_BITS_ARB;
    attr[ i++ ] = bpp==16 ? 6 : 8;
    attr[ i++ ] = WGL_BLUE_BITS_ARB;
    attr[ i++ ] = bpp==16 ? 5 : 8;

    if( bpp==32 )
    {
        attr[ i++ ] = WGL_ALPHA_BITS_ARB;
        attr[ i++ ] = 8;
    }

    if( depth )
    {
        attr[ i++ ] = WGL_DEPTH_BITS_ARB;
        attr[ i++ ] = depth;
    }

    if( stencil )
    {
        attr[ i++ ] = WGL_STENCIL_BITS_ARB;
        attr[ i++ ] = stencil;
    }

    if( doublebuffer )
    {
        attr[ i++ ] = WGL_DOUBLE_BUFFER_ARB;
        attr[ i++ ] = GL_TRUE;
    }

    if( samples )
    {
        attr[ i++ ] = WGL_SAMPLE_BUFFERS_ARB;
        attr[ i++ ] = GL_TRUE;
        attr[ i++ ] = WGL_SAMPLES_ARB;
        attr[ i++ ] = samples;
    }

    attr[ i++ ] = 0;
}


int create_gl_context( sgui_window_w32* wnd, sgui_window_description* desc )
{
    WGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = NULL;
    int attribs[20], major, minor, samples, format;
    HGLRC temp, oldctx;
    HDC olddc;

    /* get a device context */
    wnd->hDC = GetDC( wnd->hWnd );

    if( !wnd->hDC )
        return 0;

    /* try to set a pixel format */
    samples = desc->samples;

    do
    {
        set_attributes( attribs, desc->bits_per_pixel, desc->depth_bits,
                        desc->stencil_bits, desc->doublebuffer, samples-- );

        format = determine_pixel_format( attribs, 1 );
    }
    while( samples>=0 && format<1 );

    if( !format )
        format = determine_pixel_format( attribs, 0 );

    if( !format )
    {
        ReleaseDC( wnd->hWnd, wnd->hDC );
        return 0;
    }

    SetPixelFormat( wnd->hDC, format, NULL );

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

        if( desc->backend==SGUI_OPENGL_COMPAT )
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
                wnd->hRC = wglCreateContextAttribsARB( wnd->hDC, 0, attribs );
            }
        }

        /* try to create 2.x context */
        for( minor=1; !wnd->hRC && minor>=0; --minor )
        {
            attribs[1] = 2;
            attribs[3] = minor;
            wnd->hRC = wglCreateContextAttribsARB( wnd->hDC, 0, attribs );
        }

        /* try to create 1.x context */
        for( minor=5; !wnd->hRC && minor>=0; --minor )
        {
            attribs[1] = 1;
            attribs[3] = minor;
            wnd->hRC = wglCreateContextAttribsARB( wnd->hDC, 0, attribs );
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
int create_gl_context( sgui_window_w32* wnd, sgui_window_description* desc )
{
    (void)wnd;
    (void)desc;
    return 0;
}

void destroy_gl_context( sgui_window_w32* wnd )
{
    (void)wnd;
}

void gl_swap_buffers( sgui_window* wnd )
{
    (void)wnd;
}

void gl_make_current( sgui_window_w32* wnd )
{
    (void)wnd;
}
#endif

