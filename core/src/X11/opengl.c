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
#include "internal.h"
#include "sgui_event.h"

#ifndef SGUI_NO_OPENGL
#ifndef GLX_ARB_create_context
    #define GLX_CONTEXT_MAJOR_VERSION_ARB             0x2091
    #define GLX_CONTEXT_MINOR_VERSION_ARB             0x2092
    #define GLX_CONTEXT_FLAGS_ARB                     0x2094

    #define GLX_CONTEXT_DEBUG_BIT_ARB                 0x0001
    #define GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB    0x0002
#endif

#ifndef GLX_ARB_create_context_profile
    #define GLX_CONTEXT_PROFILE_MASK_ARB              0x9126
    #define GLX_CONTEXT_CORE_PROFILE_BIT_ARB          0x00000001
    #define GLX_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB 0x00000002
#endif


typedef GLXContext (* CREATECONTEXTATTRIBSPROC )( Display*, GLXFBConfig,
                                                  GLXContext, Bool,
                                                  const int* );


void set_attributes( int* attr, int bpp, int depth, int stencil,
                     int doublebuffer, int samples )
{
    int i=0;

    if( bpp!=16 || bpp!=24 || bpp!=32 )
        bpp = 32;

    attr[ i++ ] = GLX_BUFFER_SIZE;
    attr[ i++ ] = bpp;
    attr[ i++ ] = GLX_RED_SIZE;
    attr[ i++ ] = bpp==16 ? 5 : 8;
    attr[ i++ ] = GLX_GREEN_SIZE;
    attr[ i++ ] = bpp==16 ? 6 : 8;
    attr[ i++ ] = GLX_BLUE_SIZE;
    attr[ i++ ] = bpp==16 ? 5 : 8;

    if( bpp==32 )
    {
        attr[ i++ ] = GLX_ALPHA_SIZE;
        attr[ i++ ] = 8;
    }

    if( depth )
    {
        attr[ i++ ] = GLX_DEPTH_SIZE;
        attr[ i++ ] = depth;
    }

    if( stencil )
    {
        attr[ i++ ] = GLX_STENCIL_SIZE;
        attr[ i++ ] = stencil;
    }

    if( doublebuffer )
    {
        attr[ i++ ] = GLX_DOUBLEBUFFER;
        attr[ i++ ] = True;
    }

    if( samples )
    {
        attr[ i++ ] = GLX_SAMPLE_BUFFERS;
        attr[ i++ ] = 1;
        attr[ i++ ] = GLX_SAMPLES;
        attr[ i++ ] = samples;
    }

    attr[ i++ ] = None;
}

int get_fbc_visual_cmap( GLXFBConfig* fbc, XVisualInfo** vi, Colormap* cmap,
                         sgui_window_description* desc )
{
    GLXFBConfig* fbl;
    int fbcount, attr[20], samples;

    *fbc  = NULL;
    *vi   = NULL;
    *cmap = 0;

    samples = desc->samples;

    do
    {
        set_attributes( attr, desc->bits_per_pixel, desc->depth_bits,
                        desc->stencil_bits, desc->doublebuffer, samples-- );

        fbl = glXChooseFBConfig( dpy, DefaultScreen(dpy), attr, &fbcount );
    }
    while( !fbl && samples>=0 );

    if( fbl && fbcount )
    {
        *fbc = fbl[0];
        *vi = glXGetVisualFromFBConfig( dpy, fbl[0] );
        XFree( fbl );
    }

    if( !(*vi) )
        return 0;

    /* get a color map for the visual */
    *cmap = XCreateColormap( dpy, RootWindow(dpy, (*vi)->screen),
                             (*vi)->visual, AllocNone );

    if( !(*cmap) )
    {
        XFree( *vi );
        *vi = NULL;
        return 0;
    }

    return 1;
}

int create_context( GLXFBConfig cfg, int core, sgui_window_xlib* wnd )
{
    CREATECONTEXTATTRIBSPROC CreateContextAttribs;
    int attribs[10], major, minor;
    wnd->gl = 0;

    /* try to load context creation function */
    CreateContextAttribs = (CREATECONTEXTATTRIBSPROC)
    LOAD_GLFUN( "glXCreateContextAttribsARB" );

    /* try to create core context */
    if( core && CreateContextAttribs )
    {
        /* fill attribute array */
        attribs[0] = GLX_CONTEXT_MAJOR_VERSION_ARB;
        attribs[1] = 0;
        attribs[2] = GLX_CONTEXT_MINOR_VERSION_ARB;
        attribs[3] = 0;
        attribs[4] = GLX_CONTEXT_PROFILE_MASK_ARB;
        attribs[5] = GLX_CONTEXT_CORE_PROFILE_BIT_ARB;
        attribs[6] = GLX_CONTEXT_FLAGS_ARB;
        attribs[7] = GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB;
        attribs[8] = None;

        /* try to create 4.x down to 3.x context */
        for( major=4; !wnd->gl && major>=3; --major )
        {
            for( minor=3; !wnd->gl && minor>=0; --minor )
            {
                attribs[1] = major;
                attribs[3] = minor;
                wnd->gl = CreateContextAttribs( dpy, cfg, 0, True, attribs );
            }
        }
    }

    /* fallback: old context creation function */
    if( !wnd->gl )
        wnd->gl = glXCreateNewContext(dpy, cfg, GLX_RGBA_TYPE, NULL, GL_TRUE);

    return (wnd->gl!=0);
}

void gl_swap_buffers( sgui_window* wnd )
{
    glXSwapBuffers( dpy, TO_X11(wnd)->wnd );
}
#else
int get_fbc_visual_cmap( GLXFBConfig* fbc, XVisualInfo** vi, Colormap* cmap,
                         sgui_window_description* desc )
{
    (void)fbc; (void)vi; (void)cmap; (void)desc;
    return 0;
}

int create_context( GLXFBConfig cfg, int core, sgui_window_xlib* wnd )
{
    (void)cfg; (void)core; (void)wnd;
    return 0;
}

void gl_swap_buffers( sgui_window* wnd )
{
    (void)wnd;
}
#endif

