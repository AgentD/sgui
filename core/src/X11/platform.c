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



struct x11_state x11;



static int resize_clipboard_buffer( unsigned int additional )
{
    char* new;

    new = realloc(x11.clipboard_buffer, x11.clipboard_size + additional);

    if( !new )
        return 0;

    x11.clipboard_buffer = new;
    x11.clipboard_size += additional;
    return 1;
}

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
    XIfEvent( x11.dpy, ret, filter_event, (XPointer)&type );
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

    while( XPending( x11.dpy ) > 0 )
    {
        XNextEvent( x11.dpy, &e );

        /* process selection requests */
        if( e.type==SelectionRequest )
        {
            if( e.xselectionrequest.target == x11.atom_UTF8 )
            {
                XChangeProperty( x11.dpy, e.xselectionrequest.requestor,
                                 e.xselectionrequest.property,
                                 e.xselectionrequest.target, 8,
                                 PropModeReplace,
                                 (unsigned char*)x11.clipboard_buffer,
                                 x11.clipboard_strlen );
                respond.xselection.property = e.xselectionrequest.property;
            }
            else if( e.xselectionrequest.target == x11.atom_targets )
            {
                data[0] = x11.atom_text;
                data[1] = x11.atom_UTF8;

                XChangeProperty( x11.dpy, e.xselectionrequest.requestor,
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
            XSendEvent(x11.dpy,e.xselectionrequest.requestor,0,0,&respond);
            XFlush( x11.dpy );
            continue;
        }

        /* XFilterEvent filters out keyboard events needed for composing and
           returns True if it handled the event and we should ignore it */
        if( !XFilterEvent( &e, None ) )
        {
            /* route the event to it's window */
            for( i=x11.list; i!=NULL && i->wnd!=e.xany.window; i=i->next );

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
    for( i=x11.list; i!=NULL && !(i->super.flags & SGUI_VISIBLE); i=i->next );
    sgui_internal_unlock_mutex( );

    return i!=NULL;
}

static void update_windows( void )
{
    sgui_window_xlib* i;

    for( i=x11.list; i!=NULL; i=i->next )
        update_window( (sgui_window*)i );
}

/****************************************************************************/

void add_window( sgui_window_xlib* this )
{
    SGUI_ADD_TO_LIST( x11.list, this );
}

void remove_window( sgui_window_xlib* this )
{
    sgui_window_xlib* i;

    SGUI_REMOVE_FROM_LIST( x11.list, i, this );
}

/****************************************************************************/

static unsigned int sgui_utf8_from_latin1_length( const char* in )
{
    unsigned int length = 0;

    while( *in )
    {
        length += (*(in++) & 0x80) ? 2 : 1;
    }

    return length;
}

static void sgui_utf8_from_latin1( char* out, const char* in )
{
    for( ; *in; ++in )
    {
        if( *in & 0x80 )
        {
            *out++ = 0xC2 + ((unsigned char)*in > 0xBF);
            *out++ = 0x80 + (*in & 0x3F);
        }
        else
        {
            *out++ = *in;
        }
    }

    *out = '\0';
}

void xlib_window_clipboard_write( sgui_window* this, const char* text,
                                  unsigned int length )
{
    sgui_internal_lock_mutex( );

    x11.clipboard_strlen = length;

    if( x11.clipboard_size <= x11.clipboard_strlen )
    {
        x11.clipboard_size=x11.clipboard_strlen+1;
        x11.clipboard_buffer=realloc(x11.clipboard_buffer,x11.clipboard_size);
    }

    strncpy( x11.clipboard_buffer, text, x11.clipboard_strlen );
    x11.clipboard_buffer[ x11.clipboard_strlen ] = '\0';

    XSetSelectionOwner( x11.dpy, x11.atom_clipboard,
                        TO_X11(this)->wnd, CurrentTime );
    XFlush( x11.dpy );

    sgui_internal_unlock_mutex( );
}

const char* xlib_window_clipboard_read( sgui_window* this )
{
    Atom pty_type;
    int pty_format, convert = 0;
    unsigned char* buffer;
    unsigned long pty_size, pty_items;
    Atom target = x11.atom_UTF8;
    Window owner;
    XEvent evt;

    sgui_internal_lock_mutex( );

    /* sanity check clipboard owner */
    owner = XGetSelectionOwner( x11.dpy, x11.atom_clipboard );

    if( owner==TO_X11(this)->wnd )
        goto done;

    if( owner==None )
        goto fail;

    /* try to convert the selection to an UTF8 string */
    XConvertSelection( x11.dpy, x11.atom_clipboard, target, x11.atom_pty,
                       TO_X11(this)->wnd, CurrentTime );
    XFlush( x11.dpy );

    wait_for_event( &evt, SelectionNotify );

    /* not possible? fallback to latin-1 string */
    if( evt.xselection.property==None )
    {
        target = XA_STRING;
        XConvertSelection( x11.dpy, x11.atom_clipboard, target, x11.atom_pty,
                           TO_X11(this)->wnd, CurrentTime );
        XFlush( x11.dpy );
        wait_for_event( &evt, SelectionNotify );
        convert = 1;
    }

    /* determine how to convert the selection */
    XGetWindowProperty( x11.dpy, TO_X11(this)->wnd, x11.atom_pty, 0, 0, False,
                        AnyPropertyType, &pty_type, &pty_format, &pty_items,
                        &pty_size, &buffer );
    XFree( buffer );
    x11.clipboard_strlen = 0;

    /* increment method */
    if( pty_type == x11.atom_inc )
    {
        XDeleteProperty( x11.dpy, TO_X11(this)->wnd, x11.atom_pty );
        XFlush( x11.dpy );

        while( 1 )
        {
            /* wait for next package */
            wait_for_event( &evt, PropertyNotify );

            if( evt.xproperty.state!=PropertyNewValue )
                continue;

            /* get data format and size */
            XGetWindowProperty( x11.dpy, TO_X11(this)->wnd, x11.atom_pty,
                                0, 0, False, AnyPropertyType, &pty_type,
                                &pty_format, &pty_items, &pty_size, &buffer );

            XFree( buffer );

            if( pty_format != 8 )
            {
                XDeleteProperty( x11.dpy, TO_X11(this)->wnd, x11.atom_pty );
                continue;
            }

            if( pty_size == 0 )
            {
                XDeleteProperty( x11.dpy, TO_X11(this)->wnd, x11.atom_pty );
                break;
            }

            /* get data */
            XGetWindowProperty( x11.dpy, TO_X11(this)->wnd, x11.atom_pty, 0,
                                pty_size, False, AnyPropertyType, &pty_type,
                                &pty_format, &pty_items, &pty_size, &buffer );

            /* resize clipboard buffer if neccessary */
            if( ((x11.clipboard_strlen+pty_items+1) > x11.clipboard_size) &&
                !resize_clipboard_buffer( pty_items + 1 ) )
            {
                goto fail;
            }

            /* append to clipboard buffer */
            memcpy(x11.clipboard_buffer+x11.clipboard_strlen,buffer,pty_items);
            x11.clipboard_strlen += pty_items;
            x11.clipboard_buffer[ x11.clipboard_strlen ] = '\0';
            XFree( buffer );

            XDeleteProperty( x11.dpy, TO_X11(this)->wnd, x11.atom_pty );
            XFlush( x11.dpy );
        }
    }
    else
    {
        if( pty_format != 8 )
            goto fail;

        XGetWindowProperty( x11.dpy, TO_X11(this)->wnd, x11.atom_pty, 0,
                            pty_size, False, AnyPropertyType, &pty_type,
                            &pty_format, &pty_items, &pty_size, &buffer );

        XDeleteProperty( x11.dpy, TO_X11(this)->wnd, x11.atom_pty );

        if( ((pty_items + 1) > x11.clipboard_size) &&
            !resize_clipboard_buffer( pty_items + 1 ) )
        {
            goto fail;
        }

        x11.clipboard_strlen = pty_items;
        memcpy( x11.clipboard_buffer, buffer, pty_items );
        x11.clipboard_buffer[ x11.clipboard_strlen ] = '\0';
        XFree( buffer );
    }

    if( convert )
    {
        pty_size = sgui_utf8_from_latin1_length( x11.clipboard_buffer );

        buffer = malloc( pty_size + 1 );
        if( !buffer )
            goto fail;

        sgui_utf8_from_latin1( (char*)buffer, x11.clipboard_buffer );
        free( x11.clipboard_buffer );
        x11.clipboard_buffer = (char*)buffer;
        x11.clipboard_size = pty_size+1;
        x11.clipboard_strlen = pty_size;
    }

done:
    sgui_internal_unlock_mutex( );
    return x11.clipboard_buffer;
fail:
    sgui_internal_unlock_mutex( );
    return NULL;
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

    if( x11.clicked == window )
    {
        current = get_time_ms( );
        delta = current >= x11.click_time ? (current - x11.click_time) :
                                             DOUBLE_CLICK_MS*64;

        if( delta <= DOUBLE_CLICK_MS )
            return 1;
    }

    x11.clicked = window;
    x11.click_time = get_time_ms( );
    return 0;
}

void interrupt_double_click( void )
{
    x11.clicked = NULL;
    x11.click_time = 0;
}

/****************************************************************************/

void sgui_internal_lock_mutex( void )
{
    pthread_mutex_lock( &x11.mutex );
}

void sgui_internal_unlock_mutex( void )
{
    pthread_mutex_unlock( &x11.mutex );
}

int sgui_init( void )
{
    pthread_mutexattr_t attr;

    memset( &x11, 0, sizeof(x11) );

    pthread_mutexattr_init( &attr );
    pthread_mutexattr_settype( &attr, PTHREAD_MUTEX_RECURSIVE );
    pthread_mutex_init( &x11.mutex, &attr );

    if( !font_init( ) )
        goto fail;

    /* open display connection */
    XInitThreads( );

    if( !(x11.dpy = XOpenDisplay( 0 )) )
        goto fail;

    XSetErrorHandler( xlib_swallow_errors );

    /* create input method */
    if( !(x11.im = XOpenIM( x11.dpy, NULL, NULL, NULL )) )
        goto fail;

    init_keycodes( );           /* initialise keycode translation LUT */
    sgui_skin_set( NULL );      /* initialise default GUI skin */
    sgui_event_reset( );        /* reset event system */

    x11.atom_wm_delete = XInternAtom( x11.dpy, "WM_DELETE_WINDOW", True );
    x11.atom_pty       = XInternAtom( x11.dpy, "SGUI_CLIP", False );
    x11.atom_targets   = XInternAtom( x11.dpy, "TARGETS", False );
    x11.atom_text      = XInternAtom( x11.dpy, "TEXT", False );
    x11.atom_inc       = XInternAtom( x11.dpy, "INCR", False );
    x11.atom_UTF8      = XInternAtom( x11.dpy, "UTF8_STRING", False );
    x11.atom_clipboard = XInternAtom( x11.dpy, "CLIPBOARD", False );
    x11.root           = DefaultRootWindow( x11.dpy );
    x11.screen         = DefaultScreen( x11.dpy );
    return 1;
fail:
    sgui_deinit( );
    return 0;
}

void sgui_deinit( void )
{
    sgui_event_reset( );                    /* clear event queue */
    sgui_interal_skin_deinit_default( );    /* reset skinning system */
    font_deinit( );                         /* reset font system */
    pthread_mutex_destroy( &x11.mutex );    /* destroy global mutex */

    if( x11.im )
        XCloseIM( x11.im );

    if( x11.dpy )
        XCloseDisplay( x11.dpy );

    free( x11.clipboard_buffer );
    memset( &x11, 0, sizeof(x11) );
}

int sgui_main_loop_step( void )
{
    sgui_internal_lock_mutex( );
    handle_events( );
    update_windows( );
    XFlush( x11.dpy );
    sgui_internal_unlock_mutex( );

    sgui_event_process( );

    return have_active_windows( ) || sgui_event_queued( );
}

void sgui_main_loop( void )
{
    int x11_fd = XConnectionNumber( x11.dpy );
    struct timeval tv;
    fd_set in_fds;

    while( have_active_windows( ) )
    {
        sgui_internal_lock_mutex( );
        handle_events( );
        update_windows( );
        XFlush( x11.dpy );
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

