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
#include "platform.h"

#include <sys/select.h>
#include <pthread.h>
#include <time.h>



Display* dpy = NULL;
XIM im = 0;
Atom atom_wm_delete = 0;
Window root = 0;

static char* clipboard_buffer = NULL;
static unsigned int clipboard_size = 0;
static unsigned int clipboard_strlen = 0;

static Atom atom_targets = 0;
static Atom atom_text = 0;
static Atom atom_pty = 0;
static Atom atom_inc = 0;
static Atom atom_UTF8 = 0;
static Atom atom_clipboard = 0;

static sgui_window_xlib* clicked = NULL;
static unsigned long click_time = 0;

static pthread_mutex_t mutex;

static sgui_window_xlib* list = NULL;



/* used by wait_for_event to filter for a certain event type */
static Bool filter_event( Display* display, XEvent* event, XPointer arg )
{
    int type = *((int*)arg);
    (void)display; (void)arg;
    return event->type == type;
}

/* receives Xlib events and returns when a certain event is in the queue */
static void wait_for_event( XEvent* ret, int type )
{
    XIfEvent( dpy, ret, filter_event, (XPointer)&type );
}

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
    XEvent e, respond;
    long data[2];

    while( XPending( dpy ) > 0 )
    {
        XNextEvent( dpy, &e );

        /* process selection requests */
        if( e.type==SelectionRequest )
        {
            if( e.xselectionrequest.target == atom_UTF8 )
            {
                XChangeProperty( dpy, e.xselectionrequest.requestor,
                                 e.xselectionrequest.property,
                                 e.xselectionrequest.target, 8,
                                 PropModeReplace,
                                 (unsigned char*)clipboard_buffer,
                                 clipboard_strlen );
                respond.xselection.property = e.xselectionrequest.property;
            }
            else if( e.xselectionrequest.target == atom_targets )
            {
                data[0] = atom_text;
                data[1] = atom_UTF8;

                XChangeProperty( dpy, e.xselectionrequest.requestor,
                                 e.xselectionrequest.property,
                                 e.xselectionrequest.target, 8,
                                 PropModeReplace, (unsigned char*)&data,
                                 sizeof(data) );
                respond.xselection.property = e.xselectionrequest.property;
            }
            else
            {
                respond.xselection.property = None;
            }

            respond.xselection.type      = SelectionNotify;
            respond.xselection.display   = e.xselectionrequest.display;
            respond.xselection.requestor = e.xselectionrequest.requestor;
            respond.xselection.selection = e.xselectionrequest.selection;
            respond.xselection.target    = e.xselectionrequest.target;
            respond.xselection.time      = e.xselectionrequest.time;
            XSendEvent( dpy, e.xselectionrequest.requestor, 0, 0, &respond );
            XFlush( dpy );
            continue;
        }

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

    sgui_internal_lock_mutex( );
    for( i=list; i!=NULL && !i->super.visible; i=i->next );
    sgui_internal_unlock_mutex( );

    return i!=NULL;
}

static void update_windows( void )
{
    sgui_window_xlib* i;

    for( i=list; i!=NULL; i=i->next )
        update_window( i );
}

/****************************************************************************/

void add_window( sgui_window_xlib* this )
{
    SGUI_ADD_TO_LIST( list, this );
}

void remove_window( sgui_window_xlib* this )
{
    sgui_window_xlib* i;

    SGUI_REMOVE_FROM_LIST( list, i, this );
}

/****************************************************************************/

void xlib_window_clipboard_write( sgui_window* this, const char* text,
                                  unsigned int length )
{
    sgui_internal_lock_mutex( );

    clipboard_strlen = length;

    if( clipboard_size <= clipboard_strlen )
    {
        clipboard_size = clipboard_strlen+1;
        clipboard_buffer = realloc( clipboard_buffer, clipboard_size );
    }

    strncpy( clipboard_buffer, text, clipboard_strlen );
    clipboard_buffer[ clipboard_strlen ] = '\0';

    XSetSelectionOwner( dpy, atom_clipboard, TO_X11(this)->wnd, CurrentTime );
    XFlush( dpy );

    sgui_internal_unlock_mutex( );
}

const char* xlib_window_clipboard_read( sgui_window* this )
{
    Atom pty_type;
    int pty_format, convert = 0;
    unsigned char* buffer;
    unsigned long pty_size, pty_items;
    Atom target = atom_UTF8;
    Window owner;
    XEvent evt;

    sgui_internal_lock_mutex( );

    /* sanity check clipboard owner */
    owner = XGetSelectionOwner( dpy, atom_clipboard );

    if( owner==TO_X11(this)->wnd )
    {
        sgui_internal_unlock_mutex( );
        return clipboard_buffer;
    }

    if( owner==None )
    {
        sgui_internal_unlock_mutex( );
        return NULL;
    }

    /* try to convert the selection to an UTF8 string */
    XConvertSelection( dpy, atom_clipboard, target, atom_pty,
                       TO_X11(this)->wnd, CurrentTime );
    XFlush( dpy );

    wait_for_event( &evt, SelectionNotify );

    /* not possible? fallback to latin-1 string */
    if( evt.xselection.property==None )
    {
        target = XA_STRING;
        XConvertSelection( dpy, atom_clipboard, target, atom_pty,
                           TO_X11(this)->wnd, CurrentTime );
        XFlush( dpy );
        wait_for_event( &evt, SelectionNotify );
        convert = 1;
    }

    /* determine how to convert the selection */
    XGetWindowProperty( dpy, TO_X11(this)->wnd, atom_pty, 0, 0, False,
                        AnyPropertyType, &pty_type, &pty_format, &pty_items,
                        &pty_size, &buffer );
    XFree( buffer );
    clipboard_strlen = 0;

    /* increment method */
    if( pty_type == atom_inc )
    {
        XDeleteProperty( dpy, TO_X11(this)->wnd, atom_pty );
        XFlush( dpy );

	    while( 1 )
	    {
	        /* wait for next package */
            wait_for_event( &evt, PropertyNotify );

        	if( evt.xproperty.state!=PropertyNewValue )
                continue;

            /* get data format and size */
        	XGetWindowProperty( dpy, TO_X11(this)->wnd, atom_pty, 0, 0, False,
        	                    AnyPropertyType, &pty_type, &pty_format,
        	                    &pty_items, &pty_size, &buffer );

            if( pty_format != 8 )
            {
        	    XFree( buffer );
                XDeleteProperty( dpy, TO_X11(this)->wnd, atom_pty );
                continue;
            }

            if( pty_size == 0 )
            {
        	    XFree( buffer );
                XDeleteProperty( dpy, TO_X11(this)->wnd, atom_pty );
                break;
            }

            XFree( buffer );

            /* get data */
        	XGetWindowProperty( dpy, TO_X11(this)->wnd, atom_pty, 0, pty_size,
        	                    False, AnyPropertyType, &pty_type,
        	                    &pty_format, &pty_items, &pty_size, &buffer );

            /* resize clipboard buffer if neccessary */
            if( (clipboard_strlen + pty_items + 1) > clipboard_size )
            {
        	    clipboard_size += pty_items + 1;
        	    clipboard_buffer = realloc(clipboard_buffer, clipboard_size);
            }

            /* append to clipboard buffer */
        	memcpy( clipboard_buffer + clipboard_strlen, buffer, pty_items );
        	clipboard_strlen += pty_items;
        	clipboard_buffer[ clipboard_strlen ] = '\0';
        	XFree( buffer );

        	XDeleteProperty( dpy, TO_X11(this)->wnd, atom_pty );
        	XFlush( dpy );
	    }
    }
    else
    {
        if( pty_format != 8 )
        {
            sgui_internal_unlock_mutex( );
            return NULL;
        }

        XGetWindowProperty( dpy, TO_X11(this)->wnd, atom_pty, 0, pty_size,
                            False, AnyPropertyType, &pty_type, &pty_format,
                            &pty_items, &pty_size, &buffer );

        XDeleteProperty( dpy, TO_X11(this)->wnd, atom_pty );

        if( (pty_items + 1) > clipboard_size )
        {
    	    clipboard_size += pty_items + 1;
    	    clipboard_buffer = realloc(clipboard_buffer, clipboard_size);
        }

        clipboard_strlen = pty_items;
    	memcpy( clipboard_buffer, buffer, pty_items );
    	clipboard_buffer[ clipboard_strlen ] = '\0';
        XFree( buffer );
    }

    if( convert )
    {
        pty_size = sgui_utf8_from_latin1_length( clipboard_buffer );

        if( pty_size > clipboard_strlen )
        {
            buffer = malloc( pty_size + 1 );
            sgui_utf8_from_latin1( (char*)buffer, clipboard_buffer );
            free( clipboard_buffer );
            clipboard_buffer = (char*)buffer;
            clipboard_size = pty_size+1;
            clipboard_strlen = pty_size;
        }
    }

    sgui_internal_unlock_mutex( );
    return clipboard_buffer;
}

/****************************************************************************/

static unsigned long get_time_ms( void )
{
    struct timespec ts;

    #ifdef CLOCK_MONOTONIC_RAW
        clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    #else
        clock_gettime(CLOCK_MONOTONIC, &ts);
    #endif

    return ts.tv_sec*1000 + ts.tv_nsec/1000000;
}

int check_double_click( sgui_window_xlib* window )
{
    unsigned long delta, current;

    if( clicked == window )
    {
        current = get_time_ms( );
        delta = current >= click_time ? (current - click_time) :
                                        DOUBLE_CLICK_MS*64;

        if( delta <= DOUBLE_CLICK_MS )
            return 1;
    }

    clicked = window;
    click_time = get_time_ms( );
    return 0;
}

void interrupt_double_click( void )
{
    clicked = NULL;
    click_time = 0;
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

    XInitThreads( );

    if( !font_init( ) )
        goto failure;

    /* open display connection */
    if( !(dpy = XOpenDisplay( 0 )) )
        goto failure;

    XSetErrorHandler( xlib_swallow_errors );

    /* create input method */
    if( !(im = XOpenIM( dpy, NULL, NULL, NULL )) )
        goto failure;

    /* get usefull constants */
    atom_wm_delete = XInternAtom( dpy, "WM_DELETE_WINDOW", True );
    atom_pty       = XInternAtom( dpy, "SGUI_CLIP", False );
    atom_targets   = XInternAtom( dpy, "TARGETS", False );
    atom_text      = XInternAtom( dpy, "TEXT", False );
    atom_inc       = XInternAtom( dpy, "INCR", False );
    atom_UTF8      = XInternAtom( dpy, "UTF8_STRING", False );
    atom_clipboard = XInternAtom( dpy, "CLIPBOARD", False );
    root = DefaultRootWindow( dpy );

    /* initialise keycode translation LUT */
    init_keycodes( );

    /* initialise default GUI skin */
    sgui_skin_set( NULL );

    sgui_event_reset( );
    return 1;
failure:
    sgui_deinit( );
    return 0;
}

void sgui_deinit( void )
{
    sgui_event_reset( );

    sgui_interal_skin_deinit_default( );

    font_deinit( );

    pthread_mutex_destroy( &mutex );

    if( im )
        XCloseIM( im );

    if( dpy )
        XCloseDisplay( dpy );

    free( clipboard_buffer );

    clipboard_buffer = NULL;
    clipboard_size = 0;
    dpy = NULL;
    im = 0;
    list = NULL;
}

int sgui_main_loop_step( void )
{
    sgui_internal_lock_mutex( );
    handle_events( );
    update_windows( );
    XFlush( dpy );
    sgui_internal_unlock_mutex( );

    sgui_event_process( );

    return have_active_windows( ) || sgui_event_queued( );
}

void sgui_main_loop( void )
{
    struct timeval tv;
    fd_set in_fds;
    int x11_fd;

    x11_fd = XConnectionNumber( dpy );

    while( have_active_windows( ) )
    {
        sgui_internal_lock_mutex( );
        handle_events( );
        update_windows( );
        XFlush( dpy );
        sgui_internal_unlock_mutex( );

        sgui_event_process( );

        /* wait for X11 events, one second time out */
        FD_ZERO( &in_fds );
        FD_SET( x11_fd, &in_fds );

        tv.tv_usec = 0;
        tv.tv_sec = 1;
        select( x11_fd+1, &in_fds, 0, 0, &tv );
    }

    while( sgui_event_queued( ) )
        sgui_event_process( );
}

