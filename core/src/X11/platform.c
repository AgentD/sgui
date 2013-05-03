/*
 * platform.c
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
#include "sgui.h"
#include "internal.h"



Display* dpy = NULL;
XIM im = 0;
Atom atom_wm_delete = 0;
FT_Library freetype = 0;

static sgui_window_xlib* list = NULL;



static const char* wm_delete_window = "WM_DELETE_WINDOW";



/* Xlib error callback to prevent xlib from crashing our program on error */
static int xlib_swallow_errors( Display* display, XErrorEvent* event )
{
    (void)display;
    (void)event;
    return 0;
}



/****************************************************************************/

void add_window( sgui_window_xlib* wnd )
{
    wnd->next = list;
    list = wnd;
}

void remove_window( sgui_window_xlib* wnd )
{
    sgui_window_xlib* i;

    if( wnd==list )
    {
        list = list->next;
    }
    else
    {
        for( i=list; i->next; i=i->next )
        {
            if( i->next == wnd )
            {
                i->next = i->next->next;
                break;
            }
        }
    }
}

sgui_window_xlib* find_gl_window( void )
{
    sgui_window_xlib* i;

    for( i=list; i; i=i->next )
    {
        if( i->base.backend==SGUI_OPENGL_CORE ||
            i->base.backend==SGUI_OPENGL_COMPAT )
        {
            return i;
        }
    }

    return NULL;
}

/****************************************************************************/

int sgui_init( void )
{
    /* initialise freetype library */
    if( FT_Init_FreeType( &freetype ) )
    {
        sgui_deinit( );
        return 0;
    }

    /* open display connection */
    dpy = XOpenDisplay( 0 );

    if( !dpy )
    {
        sgui_deinit( );
        return 0;
    }

    XSetErrorHandler( xlib_swallow_errors );

    /* create input method */
    im = XOpenIM( dpy, NULL, NULL, NULL );

    if( !im )
    {
        sgui_deinit( );
        return 0;
    }

    /* get wm delete atom */
    atom_wm_delete = XInternAtom( dpy, wm_delete_window, True );

    /* initialise keycode translation LUT */
    init_keycodes( );

    return 1;
}

void sgui_deinit( void )
{
    if( im )
        XCloseIM( im );

    if( dpy )
        XCloseDisplay( dpy );

    if( freetype )
        FT_Done_FreeType( freetype );

    dpy = NULL;
    im = 0;
    freetype = 0;

    list = NULL;
}

int sgui_main_loop_step( void )
{
    sgui_window_xlib* i;
    XEvent e;

    if( XPending( dpy ) > 0 )
    {
        XNextEvent( dpy, &e );

        /* XFilterEvent filters out keyboard events needed for composing and
           returns True if it handled the event and we should ignore it */
        if( !XFilterEvent( &e, None ) )
        {
            /* route the event to it's window */
            for( i=list; i!=NULL; i=i->next )
            {
                if( i->wnd == e.xany.window )
                {
                    handle_window_events( i, &e );
                    break;
                }
            }
        }
    }

    /* check if there's at least 1 window still active */
    for( i=list; i!=NULL; i=i->next )
        if( i->base.visible )
            return 1;

    return 0;
}

void sgui_main_loop( void )
{
    sgui_window_xlib* i;
    int active;
    XEvent e;

    do
    {
        XNextEvent( dpy, &e );

        /* XFilterEvent filters out keyboard events needed for composing and
           returns True if it handled the event and we should ignore it */
        if( !XFilterEvent( &e, None ) )
        {
            /* route the event to it's window */
            for( i=list; i!=NULL; i=i->next )
            {
                if( i->wnd == e.xany.window )
                {
                    handle_window_events( i, &e );
                    break;
                }
            }
        }

        /* check if there's at least 1 window still active */
        for( i=list, active=0; i!=NULL && !active; i=i->next )
            active |= i->base.visible;
    }
    while( active );
}

