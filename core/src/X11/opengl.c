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
#define LOAD_GLFUN( name ) glXGetProcAddress( (const GLubyte*)(name) )

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

#ifndef GLX_VERSION_1_3
    typedef struct GLXFBConfigRec* GLXFBConfig;

    #define GLX_X_RENDERABLE  0x8012
    #define GLX_X_VISUAL_TYPE 0x22
    #define GLX_TRUE_COLOR    0x8002
#endif

typedef GLXFBConfig* (* CHOOSEFBCFGPROC )(Display*,int,const int*,int*);
typedef XVisualInfo* (* GETVISUALFROMFBCPROC )(Display*, GLXFBConfig);
typedef GLXContext   (* CREATECONTEXTATTRIBSPROC )( Display*, GLXFBConfig,
                                                    GLXContext, Bool,
                                                    const int* );


int get_fbc_visual_cmap( GLXFBConfig* fbc, XVisualInfo** vi, Colormap* cmap,
                         sgui_window_description* desc )
{
    GLXFBConfig* fbl;
    CHOOSEFBCFGPROC ChooseFBConfig;
    GETVISUALFROMFBCPROC GetVisualFromFBConfig;
    int fbcount, i;

    int attr[20] =
    {
        GLX_X_RENDERABLE, True,
        GLX_X_VISUAL_TYPE, GLX_TRUE_COLOR,
        None, 0
    };

    int attr_old[20] = { GLX_RGBA, None, 0 };

    /* set up new descriptor */
    for( i=0; attr[i]!=None; ++i );

    attr[ i++ ] = GLX_RED_SIZE;
    attr[ i++ ] = desc->bits_per_pixel==16 ? 5 : 8;
    attr[ i++ ] = GLX_GREEN_SIZE;
    attr[ i++ ] = desc->bits_per_pixel==16 ? 6 : 8;
    attr[ i++ ] = GLX_BLUE_SIZE;
    attr[ i++ ] = desc->bits_per_pixel==16 ? 5 : 8;

    if( desc->bits_per_pixel>24 )
    {
        attr[ i++ ] = GLX_ALPHA_SIZE;
        attr[ i++ ] = 8;
    }

    if( desc->depth_bits )
    {
        attr[ i++ ] = GLX_DEPTH_SIZE;
        attr[ i++ ] = desc->depth_bits;
    }

    if( desc->stencil_bits )
    {
        attr[ i++ ] = GLX_STENCIL_SIZE;
        attr[ i++ ] = desc->stencil_bits;
    }

    if( desc->doublebuffer )
    {
        attr[ i++ ] = GLX_DOUBLEBUFFER;
        attr[ i++ ] = True;
    }

    attr[ i++ ] = None;

    /* set up old descriptor */
    for( i=0; attr_old[i]!=None; ++i );

    if( desc->doublebuffer )
        attr_old[ i++ ] = GLX_DOUBLEBUFFER;

    attr_old[ i++ ] = GLX_RED_SIZE;
    attr_old[ i++ ] = desc->bits_per_pixel==16 ? 5 : 8;
    attr_old[ i++ ] = GLX_GREEN_SIZE;
    attr_old[ i++ ] = desc->bits_per_pixel==16 ? 6 : 8;
    attr_old[ i++ ] = GLX_BLUE_SIZE;
    attr_old[ i++ ] = desc->bits_per_pixel==16 ? 5 : 8;

    if( desc->bits_per_pixel>24 )
    {
        attr_old[ i++ ] = GLX_ALPHA_SIZE;
        attr_old[ i++ ] = 8;
    }

    if( desc->depth_bits )
    {
        attr_old[ i++ ] = GLX_DEPTH_SIZE;
        attr_old[ i++ ] = desc->depth_bits;
    }

    if( desc->stencil_bits )
    {
        attr_old[ i++ ] = GLX_STENCIL_SIZE;
        attr_old[ i++ ] = desc->stencil_bits;
    }

    attr_old[ i++ ] = None;

    /* */
    ChooseFBConfig = (CHOOSEFBCFGPROC)LOAD_GLFUN( "glXChooseFBConfig" );
    GetVisualFromFBConfig = (GETVISUALFROMFBCPROC)
                            LOAD_GLFUN( "glXGetVisualFromFBConfig" );

    *fbc = NULL;
    *vi  = NULL;

    if( ChooseFBConfig && GetVisualFromFBConfig )
    {
        fbl = ChooseFBConfig( dpy, DefaultScreen(dpy), attr, &fbcount );

        if( fbl )
        {
            *fbc = fbl[0];
            *vi = GetVisualFromFBConfig( dpy, fbl[0] );
            XFree( fbl );
        }
    }
    else
    {
        *vi = glXChooseVisual( dpy, DefaultScreen(dpy), attr_old );
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

GLXContext create_context( GLXFBConfig cfg, XVisualInfo* vi, int core )
{
    CREATECONTEXTATTRIBSPROC CreateContextAttribs;
    int attribs[10], major, minor;
    GLXContext ctx = 0;

    if( core && cfg )
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

        /* try to load context creation function */
        CreateContextAttribs = (CREATECONTEXTATTRIBSPROC)
        LOAD_GLFUN( "glXCreateContextAttribsARB" );

        if( CreateContextAttribs )
        {
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
        }
    }

    if( !ctx && vi )
        ctx = glXCreateContext( dpy, vi, 0, GL_TRUE );

    return ctx;
}

void gl_swap_buffers( sgui_window* wnd )
{
    glXSwapBuffers( dpy, TO_X11(wnd)->wnd );
}
#endif

