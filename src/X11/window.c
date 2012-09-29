/*
 * window.c
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
#include "internal.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>


const char* wmDeleteWindow = "WM_DELETE_WINDOW";



sgui_window* sgui_window_create( unsigned int width, unsigned int height,
                                 int resizeable )
{
    sgui_window* wnd;
    XSizeHints hints;
    XWindowAttributes attr;
    unsigned long color = 0;
    unsigned char rgb[3];

    if( !width || !height )
        return NULL;

    /********* allocate space for the window structure *********/
    wnd = malloc( sizeof(sgui_window) );

    if( !wnd )
        return NULL;

    memset( wnd, 0, sizeof(sgui_window) );

    /***************** create a widget manager *****************/
    wnd->mgr = sgui_widget_manager_create( );

    if( !wnd->mgr )
    {
        free( wnd );
        return NULL;
    }

    /***************** connect to the X server *****************/
    wnd->dpy = XOpenDisplay( 0 );

    if( !wnd->dpy )
    {
        sgui_window_destroy( wnd );
        return NULL;
    }

    /* initialise key code translation LUT */
    init_keycodes( );

    /******************** create the window ********************/
    sgui_skin_get_window_background_color( rgb );

    color |= ((unsigned long)rgb[0]) << 16;
    color |= ((unsigned long)rgb[1]) << 8;
    color |= ((unsigned long)rgb[2]);

    wnd->wnd = XCreateSimpleWindow( wnd->dpy, DefaultRootWindow(wnd->dpy),
                                    0, 0, width, height, 0, 0, color );

    if( !wnd->wnd )
    {
        sgui_window_destroy( wnd );
        return NULL;
    }

    /* make the window non resizeable if required */
    if( !resizeable )
    {
        hints.flags      = PSize | PMinSize | PMaxSize;
        hints.min_width  = hints.max_width  = hints.base_width  = (int)width;
        hints.min_height = hints.max_height = hints.base_height = (int)height;

        XSetWMNormalHints( wnd->dpy, wnd->wnd, &hints );
    }

    /* tell X11 what events we will handle */
    XSelectInput( wnd->dpy, wnd->wnd, ExposureMask | StructureNotifyMask |
                                      SubstructureNotifyMask |
                                      KeyPressMask | KeyReleaseMask |
                                      PointerMotionMask |
                                      ButtonPressMask | ButtonReleaseMask );

    wnd->wmDelete = XInternAtom( wnd->dpy, wmDeleteWindow, True );
    XSetWMProtocols( wnd->dpy, wnd->wnd, &wnd->wmDelete, 1 );

    XFlush( wnd->dpy );

    /* get the real geometry as the window manager is free to change it */
    XGetWindowAttributes( wnd->dpy, wnd->wnd, &attr );

    wnd->x = attr.x;
    wnd->y = attr.y;
    wnd->w = (unsigned int)attr.width;
    wnd->h = (unsigned int)attr.height;

    /*********** Create an input method and context ************/
    wnd->im = XOpenIM( wnd->dpy, NULL, NULL, NULL );

    if( !wnd->im )
    {
        sgui_window_destroy( wnd );
        return NULL;
    }

    wnd->ic = XCreateIC( wnd->im,
                         XNInputStyle, XIMPreeditNothing | XIMStatusNothing,
                         XNClientWindow, wnd->wnd,
                         XNFocusWindow, wnd->wnd, NULL );

    if( !wnd->ic )
    {
        sgui_window_destroy( wnd );
        return NULL;
    }

    /************ create an image for local drawing ************/
    wnd->back_buffer = sgui_canvas_create( wnd->w, wnd->w, wnd->dpy );

    if( !wnd->back_buffer )
    {
        sgui_window_destroy( wnd );
        return NULL;
    }

    wnd->gc = XCreateGC( wnd->dpy, wnd->wnd, 0, 0 );

    if( !wnd->gc )
    {
        sgui_window_destroy( wnd );
        return NULL;
    }

    sgui_canvas_set_background_color( (sgui_canvas*)wnd->back_buffer, rgb );

    sgui_canvas_clear( (sgui_canvas*)wnd->back_buffer, NULL );

    /************* store the remaining information *************/
    wnd->resizeable = resizeable;
    wnd->mapped = 0;
    wnd->event_fun = NULL;

    return wnd;
}

void sgui_window_destroy( sgui_window* wnd )
{
    if( wnd )
    {
        SEND_EVENT( wnd, SGUI_API_DESTROY_EVENT, NULL );

        if( wnd->ic ) XDestroyIC( wnd->ic );
        if( wnd->im ) XCloseIM( wnd->im );

        if( wnd->back_buffer )
            sgui_canvas_destroy( wnd->back_buffer );

        if( wnd->gc  ) XFreeGC( wnd->dpy, wnd->gc );
        if( wnd->wnd ) XDestroyWindow( wnd->dpy, wnd->wnd );
        if( wnd->dpy ) XCloseDisplay( wnd->dpy );

        sgui_widget_manager_destroy( wnd->mgr );

        free( wnd );
    }
}

void sgui_window_get_mouse_position( sgui_window* wnd, int* x, int* y )
{
    Window t1, t2;
    int t3, t4;
    unsigned int t5;
    int X, Y;

    if( wnd )
    {
        XQueryPointer( wnd->dpy, wnd->wnd, &t1, &t2, &t3, &t4, &X, &Y, &t5 );

        if( x ) *x = X<0 ? 0 : (X>=(int)wnd->w ? ((int)wnd->w-1) : X);
        if( y ) *y = Y<0 ? 0 : (Y>=(int)wnd->h ? ((int)wnd->h-1) : Y);
    }
    else
    {
        if( x ) *x = 0;
        if( y ) *y = 0;
    }
}

void sgui_window_set_mouse_position( sgui_window* wnd, int x, int y,
                                     int send_event )
{
    sgui_event e;

    if( wnd )
    {
        x = x<0 ? 0 : (x>=(int)wnd->w ? ((int)wnd->w-1) : x);
        y = y<0 ? 0 : (y>=(int)wnd->h ? ((int)wnd->h-1) : y);

        XWarpPointer( wnd->dpy, None, wnd->wnd, 0, 0, wnd->w, wnd->h, x, y );
        XFlush( wnd->dpy );

        if( send_event )
        {
            e.mouse_move.x = x;
            e.mouse_move.y = y;
            SEND_EVENT( wnd, SGUI_MOUSE_MOVE_EVENT, &e );
        }

        ++(wnd->mouse_warped);
    }
}

void sgui_window_set_visible( sgui_window* wnd, int visible )
{
    if( wnd )
    {
        wnd->mapped = visible;

        if( visible )
        {
            XMapWindow( wnd->dpy, wnd->wnd );
        }
        else
        {
            XUnmapWindow( wnd->dpy, wnd->wnd );

            SEND_EVENT( wnd, SGUI_API_INVISIBLE_EVENT, NULL );
        }

        XFlush( wnd->dpy );
    }
}

int sgui_window_is_visible( sgui_window* wnd )
{
    return wnd ? wnd->mapped : 0;
}

void sgui_window_set_title( sgui_window* wnd, const char* title )
{
    if( wnd )
    {
        XStoreName( wnd->dpy, wnd->wnd, title );
        XFlush( wnd->dpy );
    }
}

void sgui_window_set_size( sgui_window* wnd,
                           unsigned int width, unsigned int height )
{
    XSizeHints hints;
    XWindowAttributes attr;

    if( !wnd || !width || !height )
        return;

    /* adjust the fixed size for nonresizeable windows */
    if( !wnd->resizeable )
    {
        hints.flags = PSize | PMinSize | PMaxSize;
        hints.min_width  = hints.base_width  = hints.max_width  = (int)width;
        hints.min_height = hints.base_height = hints.max_height = (int)height;

        XSetWMNormalHints( wnd->dpy, wnd->wnd, &hints );
    }

    /* resize the window */
    XResizeWindow( wnd->dpy, wnd->wnd, width, height );
    XFlush( wnd->dpy );

    /* get the real geometry as the window manager is free to change it */
    XGetWindowAttributes( wnd->dpy, wnd->wnd, &attr );
    wnd->w = (unsigned int)attr.width;
    wnd->h = (unsigned int)attr.height;

    /* resize the back buffer image */
    sgui_canvas_resize( wnd->back_buffer, wnd->w, wnd->h, wnd->dpy );

    /* redraw everything */
    sgui_canvas_clear( (sgui_canvas*)wnd->back_buffer, NULL );

    sgui_widget_manager_draw_all( wnd->mgr, (sgui_canvas*)wnd->back_buffer );
}

void sgui_window_get_size( sgui_window* wnd, unsigned int* width,
                           unsigned int* height )
{
    if( wnd )
    {
        if( width  ) *width  = wnd->w;
        if( height ) *height = wnd->h;
    }
}

void sgui_window_move_center( sgui_window* wnd )
{
    if( wnd )
    {
        wnd->x = (DPY_WIDTH  >> 1) - (int)(wnd->w >> 1);
        wnd->y = (DPY_HEIGHT >> 1) - (int)(wnd->h >> 1);
        XMoveWindow( wnd->dpy, wnd->wnd, wnd->x, wnd->y );
        XFlush( wnd->dpy );
    }
}

void sgui_window_move( sgui_window* wnd, int x, int y )
{
    if( wnd )
    {
        XMoveWindow( wnd->dpy, wnd->wnd, x, y );
        XFlush( wnd->dpy );
        wnd->x = x;
        wnd->y = y;
    }
}

void sgui_window_get_position( sgui_window* wnd, int* x, int* y )
{
    if( wnd )
    {
        if( x ) *x = wnd->x;
        if( y ) *y = wnd->y;
    }
}

int sgui_window_update( sgui_window* wnd )
{
    XEvent e, ne;
    char* atom;
    sgui_event se;
    XExposeEvent exp;
    Status stat;
    KeySym sym;
    unsigned int i, num;
    sgui_rect r;

    if( !wnd || !wnd->mapped )
        return 0;

    /* update the widgets, redraw window if there was any change */
    sgui_widget_manager_update( wnd->mgr );

    num = sgui_widget_manager_num_dirty_rects( wnd->mgr );

    exp.type       = Expose;
    exp.serial     = 0;
    exp.send_event = 1;
    exp.display    = wnd->dpy;
    exp.window     = wnd->wnd;
    exp.count      = 0;

    for( i=0; i<num; ++i )
    {
        sgui_widget_manager_get_dirty_rect( wnd->mgr, &r, i );

        exp.x      = r.left;
        exp.y      = r.top;
        exp.width  = r.right  - r.left + 1;
        exp.height = r.bottom - r.top  + 1;

        XSendEvent( wnd->dpy, wnd->wnd, False, ExposureMask, (XEvent*)&exp );
    }

    sgui_widget_manager_draw( wnd->mgr, (sgui_canvas*)wnd->back_buffer );
    sgui_widget_manager_clear_dirty_rects( wnd->mgr );

    /* message loop */
    while( XPending( wnd->dpy )>0 )
    {
        XNextEvent( wnd->dpy, &e );

        if( XFilterEvent( &e, wnd->wnd ) )
            continue;

        memset( &se, 0, sizeof(sgui_event) );

        switch( e.type )
        {
        case KeyRelease:
            /*
                On WinDOS, when holding a key pressed, a series of
                key-pressed events are generated, but X11 generates
                a series of key-pressed AND key-released events.
                Mimic the WinDOS behaviour and swallow the additional
                key-released events.
             */
            if( XPending( wnd->dpy ) > 0 )
            {
                XPeekEvent( wnd->dpy, &ne );

                if( ne.type==KeyPress && ne.xkey.keycode==e.xkey.keycode &&
                    (ne.xkey.time - e.xkey.time)<2 )
                {
                    break;
                }
            }

            sym = XLookupKeysym( &e.xkey, 0 );
            se.keyboard_event.code = key_entries_translate( sym );

            SEND_EVENT( wnd, SGUI_KEY_RELEASED_EVENT, &se );
            break;
        case KeyPress:
            Xutf8LookupString( wnd->ic, &e.xkey,
                               (char*)se.char_event.as_utf8_str,
                               sizeof(se.char_event.as_utf8_str),
                               &sym, &stat );

            if( stat==XLookupChars || stat==XLookupBoth )
            {
                if( (se.char_event.as_utf8_str[0] & 0x80) ||
                    !iscntrl( se.char_event.as_utf8_str[0] ) )
                {
                    SEND_EVENT( wnd, SGUI_CHAR_EVENT, &se );
                }
            }

            if( stat==XLookupKeySym || stat==XLookupBoth )
            {
                se.keyboard_event.code = key_entries_translate( sym );

                SEND_EVENT( wnd, SGUI_KEY_PRESSED_EVENT, &se );
            }
            break;
        case ButtonPress:
        case ButtonRelease:
            if( (e.xbutton.button==Button4||e.xbutton.button==Button5) &&
                e.type==ButtonPress )
            {
                se.mouse_wheel.direction = (e.xbutton.button==Button4)?1:-1;

                SEND_EVENT( wnd, SGUI_MOUSE_WHEEL_EVENT, &se );
            }
            else
            {
                if( e.xbutton.button == Button1 )
                    se.mouse_press.button = SGUI_MOUSE_BUTTON_LEFT;
                else if( e.xbutton.button == Button2 )
                    se.mouse_press.button = SGUI_MOUSE_BUTTON_MIDDLE;
                else if( e.xbutton.button == Button3 )
                    se.mouse_press.button = SGUI_MOUSE_BUTTON_RIGHT;
                else
                    break;

                sgui_window_get_mouse_position( wnd, &se.mouse_press.x,
                                                     &se.mouse_press.y );

                if( e.type==ButtonPress )
                {
                    SEND_EVENT( wnd, SGUI_MOUSE_PRESS_EVENT, &se );
                }
                else
                {
                    SEND_EVENT( wnd, SGUI_MOUSE_RELEASE_EVENT, &se );
                }
            }
            break;
        case MotionNotify:
            if( wnd->mouse_warped )
            {
                --(wnd->mouse_warped);
                break;
            }

            se.mouse_move.x = e.xmotion.x<0 ? 0 : e.xmotion.x;
            se.mouse_move.y = e.xmotion.y<0 ? 0 : e.xmotion.y;

            SEND_EVENT( wnd, SGUI_MOUSE_MOVE_EVENT, &se );
            break;
        case ConfigureNotify:
            if( ((int)wnd->w)!=e.xconfigure.width ||
                ((int)wnd->h)!=e.xconfigure.height )
            {
                se.size.new_width  = e.xconfigure.width;
                se.size.new_height = e.xconfigure.height;
            }

            if( !se.size.new_width || !se.size.new_height )
                break;

            wnd->x = e.xconfigure.x;
            wnd->y = e.xconfigure.y;
            wnd->w = (unsigned int)e.xconfigure.width;
            wnd->h = (unsigned int)e.xconfigure.height;

            /* resize the back buffer image */
            sgui_canvas_resize( wnd->back_buffer, wnd->w, wnd->h, wnd->dpy );

            /* send a size change event */
            SEND_EVENT( wnd, SGUI_SIZE_CHANGE_EVENT, &se );

            /* redraw everything */
            sgui_canvas_clear( (sgui_canvas*)wnd->back_buffer, NULL );

            sgui_widget_manager_draw_all( wnd->mgr,
                                          (sgui_canvas*)wnd->back_buffer );
            break;
        case ClientMessage:
            atom = XGetAtomName( wnd->dpy, e.xclient.message_type );

            if( *atom == *wmDeleteWindow )
                wnd->mapped = 0;

            XFree( atom );

            SEND_EVENT( wnd, SGUI_USER_CLOSED_EVENT, NULL );
            break;
        case Expose:
            XPutImage( wnd->dpy, wnd->wnd, wnd->gc, wnd->back_buffer->img,
                       e.xexpose.x, e.xexpose.y, e.xexpose.x, e.xexpose.y,
                       e.xexpose.width, e.xexpose.height );
            break;
        };
    }

    return wnd->mapped;
}

void sgui_window_on_event( sgui_window* wnd, sgui_window_callback fun )
{
    if( wnd )
        wnd->event_fun = fun;
}



void sgui_window_add_widget( sgui_window* wnd, sgui_widget* widget )
{
    if( wnd )
        sgui_widget_manager_add_widget( wnd->mgr, widget );
}

void sgui_window_remove_widget( sgui_window* wnd, sgui_widget* widget )
{
    if( wnd )
        sgui_widget_manager_remove_widget( wnd->mgr, widget );
}

void sgui_window_on_widget_event( sgui_window* wnd,
                                  sgui_widget_callback fun, void* user )
{
    if( wnd )
        sgui_widget_manager_on_event( wnd->mgr, fun, user );
}



sgui_canvas* sgui_window_get_canvas( sgui_window* wnd )
{
    return wnd ? (sgui_canvas*)wnd->back_buffer : NULL;
}

