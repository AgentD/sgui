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

static sgui_window_xlib** windows = NULL;
static unsigned int num_windows = 0;
static unsigned int used_windows = 0;



static const char* wm_delete_window = "WM_DELETE_WINDOW";



/* Xlib error callback to prevent xlib from crashing our program on error */
static int xlib_swallow_errors( Display* display, XErrorEvent* event )
{
    (void)display;
    (void)event;
    return 0;
}



/****************************************************************************/

sgui_window_xlib* add_window( void )
{
    sgui_window_xlib** new_windows;

    /* reallocate list if we need more entries */
    if( used_windows == num_windows )
    {
        num_windows += 10;
        new_windows = realloc( windows,
                               sizeof(sgui_window_xlib*) * num_windows );

        if( !new_windows )
            return NULL;

        windows = new_windows;
    }

    /* allocate a new window structure */
    windows[ used_windows ] = malloc( sizeof(sgui_window_xlib) );

    if( !windows[ used_windows ] )
        return NULL;

    /* initialise it */
    memset( windows[ used_windows ], 0, sizeof(sgui_window_xlib) );

    if( !sgui_internal_window_init( (sgui_window*)windows[ used_windows ] ) )
    {
        free( windows[ used_windows ] );
        return NULL;
    }

    return windows[ used_windows++ ];
}

void remove_window( sgui_window_xlib* wnd )
{
    unsigned int i;

    for( i=0; i<used_windows; ++i )
    {
        if( windows[ i ] == wnd )
        {
            windows[ i ] = windows[ used_windows - 1 ];
            --used_windows;

            sgui_internal_window_deinit( (sgui_window*)wnd );
            free( wnd );
            break;
        }
    }
}

/****************************************************************************/

int sgui_init( void )
{
    windows = malloc( 10 * sizeof(sgui_window_xlib*) );

    if( !windows )
    {
        sgui_deinit( );
        return 0;
    }

    num_windows = 10;
    used_windows = 0;

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
    unsigned int i;

    if( windows )
    {
        for( i=0; i<used_windows; ++i )
            free( windows[ i ] );

        free( windows );
    }

    if( im )
        XCloseIM( im );

    if( dpy )
        XCloseDisplay( dpy );

    dpy = NULL;
    im = 0;

    windows = NULL;
    num_windows = 0;
    used_windows = 0;
}

int sgui_main_loop_step( void )
{
    unsigned int i;
    XEvent e;

    if( XPending( dpy ) > 0 )
    {
        XNextEvent( dpy, &e );

        /* XFilterEvent filters out keyboard events needed for composing and
           returns True if it handled the event and we should ignore it */
        if( !XFilterEvent( &e, None ) )
        {
            /* route the event to it's window */
            for( i=0; i<used_windows; ++i )
            {
                if( windows[ i ]->wnd == e.xany.window )
                {
                    handle_window_events( windows[ i ], &e );
                    break;
                }
            }
        }
    }

    /* check if there's at least 1 window still active */
    for( i=0; i<used_windows; ++i )
        if( windows[ i ]->base.visible )
            return 1;

    return 0;
}

void sgui_main_loop( void )
{
    unsigned int i, active;
    XEvent e;

    do
    {
        XNextEvent( dpy, &e );

        /* XFilterEvent filters out keyboard events needed for composing and
           returns True if it handled the event and we should ignore it */
        if( !XFilterEvent( &e, None ) )
        {
            /* route the event to it's window */
            for( i=0; i<used_windows; ++i )
            {
                if( windows[ i ]->wnd == e.xany.window )
                {
                    handle_window_events( windows[ i ], &e );
                    break;
                }
            }
        }

        /* check if there's at least 1 window still active */
        for( i=0, active=0; i<used_windows && !active; ++i )
            active |= windows[ i ]->base.visible;
    }
    while( active );
}

