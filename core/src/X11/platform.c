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

#include <sys/select.h>
#include <pthread.h>



Display* dpy = NULL;
XIM im = 0;
Atom atom_wm_delete = 0;
FT_Library freetype = 0;
Window root = 0;
static sgui_font_cache* glyph_cache = NULL;
static pthread_mutex_t mutex;

static sgui_window_xlib* list = NULL;



static const char* wm_delete_window = "WM_DELETE_WINDOW";



/* Xlib error callback to prevent xlib from crashing our program on error */
static int xlib_swallow_errors( Display* display, XErrorEvent* event )
{
    (void)display; (void)event;
    return 0;
}

/* fetch and process the next Xlib event */
static void handle_events( void )
{
    sgui_window_xlib* i;
    XEvent e;

    while( XPending( dpy ) > 0 )
    {
        XNextEvent( dpy, &e );

        /* XFilterEvent filters out keyboard events needed for composing and
           returns True if it handled the event and we should ignore it */
        if( !XFilterEvent( &e, None ) )
        {
            /* route the event to it's window */
            for( i=list; i!=NULL && i->wnd!=e.xany.window; i=i->next );

            if( i!=NULL )
                handle_window_events( i, &e );
        }
    }
}

/* returns non-zero if there's at least 1 window still active */
static int have_active_windows( void )
{
    sgui_window_xlib* i;

    for( i=list; i!=NULL && !i->super.visible; i=i->next );

    return i!=NULL;
}

static void update_windows( void )
{
    sgui_window_xlib* i;

    for( i=list; i!=NULL; i=i->next )
        update_window( i );
}

/****************************************************************************/

void add_window( sgui_window_xlib* wnd )
{
    SGUI_ADD_TO_LIST( list, wnd );
}

void remove_window( sgui_window_xlib* wnd )
{
    sgui_window_xlib* i;

    SGUI_REMOVE_FROM_LIST( list, i, wnd );
}

sgui_font_cache* get_glyph_cache( void )
{
    sgui_pixmap* font_map;

    if( !glyph_cache )
    {
        font_map = xlib_pixmap_create( FONT_MAP_WIDTH, FONT_MAP_HEIGHT,
                                       SGUI_A8, root );

        if( font_map )
        {
            glyph_cache = sgui_font_cache_create( font_map );

            if( !glyph_cache )
                sgui_pixmap_destroy( font_map );
        }
    }

    return glyph_cache;
}

/****************************************************************************/

void sgui_internal_lock_mutex( void )
{
    pthread_mutex_lock( &mutex );
}

void sgui_internal_unlock_mutex( void )
{
    pthread_mutex_unlock( &mutex );
}

int sgui_init( void )
{
    /* create global mutex */
    pthread_mutexattr_t attr;

    pthread_mutexattr_init( &attr );
    pthread_mutexattr_settype( &attr, PTHREAD_MUTEX_RECURSIVE );
    pthread_mutex_init( &mutex, &attr );

    /* initialise freetype library */
    if( FT_Init_FreeType( &freetype ) )
        goto failure;

    /* open display connection */
    if( !(dpy = XOpenDisplay( 0 )) )
        goto failure;

    XSetErrorHandler( xlib_swallow_errors );

    /* create input method */
    if( !(im = XOpenIM( dpy, NULL, NULL, NULL )) )
        goto failure;

    /* get usefull constants */
    atom_wm_delete = XInternAtom( dpy, wm_delete_window, True );
    root = DefaultRootWindow( dpy );

    /* initialise keycode translation LUT */
    init_keycodes( );

    /* initialise default GUI skin */
    sgui_skin_set( NULL );
    return 1;
failure:
    sgui_deinit( );
    return 0;
}

void sgui_deinit( void )
{
    pthread_mutex_destroy( &mutex );
    sgui_font_cache_destroy( glyph_cache );

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
    handle_events( );
    update_windows( );
    XFlush( dpy );

    return have_active_windows( );
}

void sgui_main_loop( void )
{
    struct timeval tv;
    fd_set in_fds;
    int x11_fd;

    x11_fd = XConnectionNumber( dpy );

    while( have_active_windows( ) )
    {
        handle_events( );
        update_windows( );
        XFlush( dpy );

        /* wait for X11 events, one second time out */
        FD_ZERO( &in_fds );
        FD_SET( x11_fd, &in_fds );

        tv.tv_usec = 0;
        tv.tv_sec = 1;
        select( x11_fd+1, &in_fds, 0, 0, &tv );
    }
}

