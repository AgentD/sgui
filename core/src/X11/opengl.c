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
#include "sgui_opengl.h"
#include "sgui_event.h"

#ifndef SGUI_NO_OPENGL
GLXFBConfig get_fb_config( void )
{
    GLXFBConfig fbc, *fbl;
    CHOOSEFBCFGPROC ChooseFBConfig;
    int fbcount;

    int attr[] =
    {
        GLX_X_RENDERABLE,   True,
        GLX_X_VISUAL_TYPE,  GLX_TRUE_COLOR,
        GLX_RED_SIZE,       8,
        GLX_GREEN_SIZE,     8,
        GLX_BLUE_SIZE,      8,
        GLX_ALPHA_SIZE,     8,
        GLX_DEPTH_SIZE,     24,
        GLX_STENCIL_SIZE,   8,
        GLX_DOUBLEBUFFER,   True,
        None
    };

    ChooseFBConfig = (CHOOSEFBCFGPROC)LOAD_GLFUN( "glXChooseFBConfig" );

    if( !ChooseFBConfig )
        return NULL;

    fbl = ChooseFBConfig( dpy, DefaultScreen(dpy), attr, &fbcount );

    if( !fbl )
        return NULL;

    fbc = fbl[0];
    XFree( fbl );

    return fbc;
}

XVisualInfo* get_visual_from_fbc( GLXFBConfig fbc )
{
    GETVISUALFROMFBCPROC GetVisualFromFBConfig;

    GetVisualFromFBConfig = (GETVISUALFROMFBCPROC)
    LOAD_GLFUN( "glXGetVisualFromFBConfig" );

    if( !GetVisualFromFBConfig )
        return NULL;

    return GetVisualFromFBConfig( dpy, fbc );
}

XVisualInfo* get_visual_old( void )
{
    int attr[] =
    {
        GLX_RGBA,
        GLX_DOUBLEBUFFER,
        GLX_RED_SIZE,     8,
        GLX_GREEN_SIZE,   8,
        GLX_BLUE_SIZE,    8,
        GLX_ALPHA_SIZE,   8,
        GLX_DEPTH_SIZE,  24,
        GLX_STENCIL_SIZE, 8,
        None
    };

    return glXChooseVisual( dpy, DefaultScreen(dpy), attr );
}

GLXContext create_context( GLXFBConfig cfg )
{
    CREATECONTEXTATTRIBSPROC CreateContextAttribs;
    GLXContext ctx = 0;
    int attribs[10], major, minor;

    /* try to load context creation function */
    CreateContextAttribs = (CREATECONTEXTATTRIBSPROC)
    LOAD_GLFUN( "glXCreateContextAttribsARB" );

    if( !CreateContextAttribs )
        return NULL;

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
    for( major=4; !ctx && major>=3; --major )
    {
        for( minor=3; !ctx && minor>=0; --minor )
        {
            attribs[1] = major;
            attribs[3] = minor;
            ctx = CreateContextAttribs( dpy, cfg, 0, True, attribs );
        }
    }

    /* try to create 2.x context */
    for( minor=1; !ctx && minor>=0; --minor )
    {
        attribs[1] = 2;
        attribs[3] = minor;
        ctx = CreateContextAttribs( dpy, cfg, 0, True, attribs );
    }

    /* try to create 1.x context */
    for( minor=5; !ctx && minor>=0; --minor )
    {
        attribs[1] = 1;
        attribs[3] = minor;
        ctx = CreateContextAttribs( dpy, cfg, 0, True, attribs );
    }

    return ctx;
}
#endif

