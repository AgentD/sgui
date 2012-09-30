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


typedef struct
{
    sgui_window base;

    Display* dpy;
    Window wnd;
    Atom wmDelete;
    GC gc;
    XIM im;
    XIC ic;

    int resizeable;
    unsigned int mouse_warped;
}
sgui_window_xlib;


#define DPY_WIDTH\
        DisplayWidth( TO_X11(wnd)->dpy, DefaultScreen(TO_X11(wnd)->dpy) )

#define DPY_HEIGHT\
        DisplayHeight( TO_X11(wnd)->dpy, DefaultScreen(TO_X11(wnd)->dpy) )


const char* wmDeleteWindow = "WM_DELETE_WINDOW";


#define TO_X11( window ) ((sgui_window_xlib*)window)
#define SEND_EVENT( WND, ID, E ) sgui_internal_window_fire_event( WND, ID, E )


void window_x11_get_mouse_position( sgui_window* wnd, int* x, int* y )
{
    Window t1, t2;
    int t3, t4;
    unsigned int t5;
    sgui_window_xlib* w = (sgui_window_xlib*)wnd;

    XQueryPointer( w->dpy, w->wnd, &t1, &t2, &t3, &t4, x, y, &t5 );
}

void window_x11_set_mouse_position( sgui_window* wnd, int x, int y )
{
    sgui_window_xlib* w = (sgui_window_xlib*)wnd;

    XWarpPointer( w->dpy, None, w->wnd, 0, 0, wnd->w, wnd->h, x, y );
    XFlush( w->dpy );

    ++(w->mouse_warped);
}

void window_x11_set_visible( sgui_window* wnd, int visible )
{
    if( visible )
        XMapWindow( TO_X11(wnd)->dpy, TO_X11(wnd)->wnd );
    else
        XUnmapWindow( TO_X11(wnd)->dpy, TO_X11(wnd)->wnd );

    XFlush( TO_X11(wnd)->dpy );
}

void window_x11_set_title( sgui_window* wnd, const char* title )
{
    XStoreName( TO_X11(wnd)->dpy, TO_X11(wnd)->wnd, title );
    XFlush( TO_X11(wnd)->dpy );
}

void window_x11_set_size( sgui_window* wnd,
                          unsigned int width, unsigned int height )
{
    XSizeHints hints;
    XWindowAttributes attr;

    /* adjust the fixed size for nonresizeable windows */
    if( !TO_X11(wnd)->resizeable )
    {
        hints.flags = PSize | PMinSize | PMaxSize;
        hints.min_width  = hints.base_width  = hints.max_width  = (int)width;
        hints.min_height = hints.base_height = hints.max_height = (int)height;

        XSetWMNormalHints( TO_X11(wnd)->dpy, TO_X11(wnd)->wnd, &hints );
    }

    /* resize the window */
    XResizeWindow( TO_X11(wnd)->dpy, TO_X11(wnd)->wnd, width, height );
    XFlush( TO_X11(wnd)->dpy );

    /* get the real geometry as the window manager is free to change it */
    XGetWindowAttributes( TO_X11(wnd)->dpy, TO_X11(wnd)->wnd, &attr );
    wnd->w = (unsigned int)attr.width;
    wnd->h = (unsigned int)attr.height;

    /* resize the back buffer image */
    sgui_canvas_resize( (sgui_canvas_xlib*)wnd->back_buffer,
                        wnd->w, wnd->h, TO_X11(wnd)->dpy );
}

void window_x11_move_center( sgui_window* wnd )
{
    wnd->x = (DPY_WIDTH  >> 1) - (int)(wnd->w >> 1);
    wnd->y = (DPY_HEIGHT >> 1) - (int)(wnd->h >> 1);
    XMoveWindow( TO_X11(wnd)->dpy, TO_X11(wnd)->wnd, wnd->x, wnd->y );
    XFlush( TO_X11(wnd)->dpy );
}

void window_x11_move( sgui_window* wnd, int x, int y )
{
    XMoveWindow( TO_X11(wnd)->dpy, TO_X11(wnd)->wnd, x, y );
    XFlush( TO_X11(wnd)->dpy );
}

void window_x11_update( sgui_window* wnd )
{
    XEvent e, ne;
    char* atom;
    sgui_event se;
    XExposeEvent exp;
    Status stat;
    KeySym sym;
    unsigned int i, num;
    sgui_rect r;

    /* update the widgets, redraw window if there was any change */
    sgui_widget_manager_update( wnd->mgr );

    num = sgui_widget_manager_num_dirty_rects( wnd->mgr );

    exp.type       = Expose;
    exp.serial     = 0;
    exp.send_event = 1;
    exp.display    = TO_X11(wnd)->dpy;
    exp.window     = TO_X11(wnd)->wnd;
    exp.count      = 0;

    for( i=0; i<num; ++i )
    {
        sgui_widget_manager_get_dirty_rect( wnd->mgr, &r, i );

        exp.x      = r.left;
        exp.y      = r.top;
        exp.width  = r.right  - r.left + 1;
        exp.height = r.bottom - r.top  + 1;

        XSendEvent( TO_X11(wnd)->dpy, TO_X11(wnd)->wnd, False, ExposureMask,
                    (XEvent*)&exp );
    }

    sgui_widget_manager_draw( wnd->mgr, wnd->back_buffer );
    sgui_widget_manager_clear_dirty_rects( wnd->mgr );

    /* message loop */
    while( XPending( TO_X11(wnd)->dpy )>0 )
    {
        XNextEvent( TO_X11(wnd)->dpy, &e );

        if( XFilterEvent( &e, TO_X11(wnd)->wnd ) )
            continue;

        switch( e.type )
        {
        case KeyRelease:
            /*
                On WinDOS, holding a key down, causes a series of key-pressed
                events, but X11 generates a series of pressed AND released
                events. Mimic the WinDOS behaviour and swallow the additional
                key-released events.
             */
            if( XPending( TO_X11(wnd)->dpy ) > 0 )
            {
                XPeekEvent( TO_X11(wnd)->dpy, &ne );

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
            Xutf8LookupString( TO_X11(wnd)->ic, &e.xkey,
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

                window_x11_get_mouse_position( wnd, &se.mouse_press.x,
                                                    &se.mouse_press.y );

                if( e.type==ButtonPress )
                    SEND_EVENT( wnd, SGUI_MOUSE_PRESS_EVENT, &se );
                else
                    SEND_EVENT( wnd, SGUI_MOUSE_RELEASE_EVENT, &se );
            }
            break;
        case MotionNotify:
            if( TO_X11(wnd)->mouse_warped )
            {
                --(TO_X11(wnd)->mouse_warped);
            }
            else
            {
                se.mouse_move.x = e.xmotion.x<0 ? 0 : e.xmotion.x;
                se.mouse_move.y = e.xmotion.y<0 ? 0 : e.xmotion.y;
                SEND_EVENT( wnd, SGUI_MOUSE_MOVE_EVENT, &se );
            }
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
            sgui_canvas_resize( (sgui_canvas_xlib*)wnd->back_buffer,
                                wnd->w, wnd->h, TO_X11(wnd)->dpy );

            /* send a size change event */
            SEND_EVENT( wnd, SGUI_SIZE_CHANGE_EVENT, &se );

            /* redraw everything */
            sgui_canvas_clear( wnd->back_buffer, NULL );

            sgui_widget_manager_draw_all( wnd->mgr, wnd->back_buffer );
            break;
        case ClientMessage:
            atom = XGetAtomName( TO_X11(wnd)->dpy, e.xclient.message_type );

            if( *atom == *wmDeleteWindow )
                wnd->visible = 0;

            XFree( atom );

            SEND_EVENT( wnd, SGUI_USER_CLOSED_EVENT, NULL );
            break;
        case Expose:
            XPutImage( TO_X11(wnd)->dpy, TO_X11(wnd)->wnd, TO_X11(wnd)->gc,
                       ((sgui_canvas_xlib*)wnd->back_buffer)->img,
                       e.xexpose.x, e.xexpose.y, e.xexpose.x, e.xexpose.y,
                       e.xexpose.width, e.xexpose.height );
            break;
        };
    }
}

/****************************************************************************/

sgui_window* sgui_window_create( unsigned int width, unsigned int height,
                                 int resizeable )
{
    sgui_window_xlib* wnd;
    XSizeHints hints;
    XWindowAttributes attr;
    unsigned long color = 0;
    unsigned char rgb[3];

    if( !width || !height )
        return NULL;

    /********* allocate space for the window structure *********/
    wnd = malloc( sizeof(sgui_window_xlib) );

    if( !wnd )
        return NULL;

    memset( wnd, 0, sizeof(sgui_window_xlib) );

    /************** intitialise the base structure *************/
    if( !sgui_internal_window_init( (sgui_window*)wnd ) )
    {
        free( wnd );
        return NULL;
    }

    /***************** connect to the X server *****************/
    wnd->dpy = XOpenDisplay( 0 );

    if( !wnd->dpy )
    {
        sgui_window_destroy( (sgui_window*)wnd );
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
        sgui_window_destroy( (sgui_window*)wnd );
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

    wnd->base.x = attr.x;
    wnd->base.y = attr.y;
    wnd->base.w = (unsigned int)attr.width;
    wnd->base.h = (unsigned int)attr.height;

    /*********** Create an input method and context ************/
    wnd->im = XOpenIM( wnd->dpy, NULL, NULL, NULL );

    if( !wnd->im )
    {
        sgui_window_destroy( (sgui_window*)wnd );
        return NULL;
    }

    wnd->ic = XCreateIC( wnd->im, XNInputStyle,
                         XIMPreeditNothing | XIMStatusNothing, XNClientWindow,
                         wnd->wnd, XNFocusWindow, wnd->wnd, NULL );

    if( !wnd->ic )
    {
        sgui_window_destroy( (sgui_window*)wnd );
        return NULL;
    }

    /********************** create canvas **********************/
    wnd->base.back_buffer =
    (sgui_canvas*)sgui_canvas_create( wnd->base.w, wnd->base.h, wnd->dpy );

    if( !wnd->base.back_buffer )
    {
        sgui_window_destroy( (sgui_window*)wnd );
        return NULL;
    }

    wnd->gc = XCreateGC( wnd->dpy, wnd->wnd, 0, 0 );

    if( !wnd->gc )
    {
        sgui_window_destroy( (sgui_window*)wnd );
        return NULL;
    }

    sgui_canvas_set_background_color( wnd->base.back_buffer, rgb );

    sgui_canvas_clear( wnd->base.back_buffer, NULL );

    /************* store the remaining information *************/
    wnd->resizeable = resizeable;

    wnd->base.get_mouse_position = window_x11_get_mouse_position;
    wnd->base.set_mouse_position = window_x11_set_mouse_position;
    wnd->base.set_visible        = window_x11_set_visible;
    wnd->base.set_title          = window_x11_set_title;
    wnd->base.set_size           = window_x11_set_size;
    wnd->base.move_center        = window_x11_move_center;
    wnd->base.move               = window_x11_move;
    wnd->base.update             = window_x11_update;

    return (sgui_window*)wnd;
}

void sgui_window_destroy( sgui_window* wnd )
{
    if( !wnd )
        return;

    sgui_internal_window_fire_event( wnd, SGUI_API_DESTROY_EVENT, NULL );

    if( TO_X11(wnd)->ic ) XDestroyIC( TO_X11(wnd)->ic );
    if( TO_X11(wnd)->im ) XCloseIM( TO_X11(wnd)->im );

    if( wnd->back_buffer )
        sgui_canvas_destroy( (sgui_canvas_xlib*)wnd->back_buffer );

    if( TO_X11(wnd)->gc  ) XFreeGC( TO_X11(wnd)->dpy, TO_X11(wnd)->gc );
    if( TO_X11(wnd)->wnd ) XDestroyWindow(TO_X11(wnd)->dpy, TO_X11(wnd)->wnd);
    if( TO_X11(wnd)->dpy ) XCloseDisplay( TO_X11(wnd)->dpy );

    sgui_internal_window_deinit( wnd );

    free( wnd );
}

