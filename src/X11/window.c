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

    /** **/
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
    wnd->back_buffer_data = malloc( wnd->w*wnd->h*4 );

    if( !wnd->back_buffer_data )
    {
        sgui_window_destroy( wnd );
        return NULL;
    }

    wnd->gc = XCreateGC( wnd->dpy, wnd->wnd, 0, 0 );

    wnd->back_buffer = XCreateImage( wnd->dpy, CopyFromParent, 24, ZPixmap, 0,
                                     (char*)wnd->back_buffer_data,
                                     wnd->w, wnd->h, 32, 0 );

    if( !wnd->back_buffer || !wnd->gc )
    {
        sgui_window_destroy( wnd );
        return NULL;
    }

    sgui_window_clear( wnd, 0, 0, wnd->w, wnd->h );

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

        if( wnd->ic          ) XDestroyIC( wnd->ic );
        if( wnd->im          ) XCloseIM( wnd->im );
        if( wnd->back_buffer ) XDestroyImage( wnd->back_buffer );
        if( wnd->gc          ) XFreeGC( wnd->dpy, wnd->gc );
        if( wnd->wnd         ) XDestroyWindow( wnd->dpy, wnd->wnd );
        if( wnd->dpy         ) XCloseDisplay( wnd->dpy );

        sgui_widget_manager_destroy( wnd->mgr );

        free( wnd );
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

    /* create the back buffer image */
    XDestroyImage( wnd->back_buffer );

    wnd->back_buffer_data = malloc( wnd->w*wnd->h*4 );

    wnd->back_buffer = XCreateImage( wnd->dpy, CopyFromParent, 24, ZPixmap, 0,
                                     (char*)wnd->back_buffer_data,
                                     wnd->w, wnd->h, 32, 0 );

    sgui_window_clear( wnd, 0, 0, wnd->w, wnd->h );

    /* redraw everything */
    SEND_EVENT( wnd, SGUI_DRAW_EVENT, NULL );
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

    if( !wnd || !wnd->mapped )
        return 0;

    /* update the widgets, redraw window if there was any change */
    if( sgui_widget_manager_update( wnd->mgr, wnd ) )
    {
        exp.type       = Expose;
        exp.serial     = 0;
        exp.send_event = 1;
        exp.display    = wnd->dpy;
        exp.window     = wnd->wnd;
        exp.x          = 0;
        exp.y          = 0;
        exp.width      = (int)wnd->w;
        exp.height     = (int)wnd->h;
        exp.count      = 0;

        XSendEvent( wnd->dpy, wnd->wnd, False, ExposureMask, (XEvent*)&exp );
    }

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
                se.mouse_press.pressed = (e.type==ButtonPress);

                if( e.xbutton.button == Button1 )
                    se.mouse_press.button = SGUI_MOUSE_BUTTON_LEFT;
                else if( e.xbutton.button == Button2 )
                    se.mouse_press.button = SGUI_MOUSE_BUTTON_MIDDLE;
                else if( e.xbutton.button == Button3 )
                    se.mouse_press.button = SGUI_MOUSE_BUTTON_RIGHT;
                else
                    break;

                SEND_EVENT( wnd, SGUI_MOUSE_PRESS_EVENT, &se );
            }
            break;
        case MotionNotify:
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
            XDestroyImage( wnd->back_buffer );

            wnd->back_buffer_data = malloc( wnd->w*wnd->h*4 );

            wnd->back_buffer = XCreateImage( wnd->dpy, CopyFromParent, 24,
                                             ZPixmap, 0,
                                             (char*)wnd->back_buffer_data,
                                             wnd->w, wnd->h, 32, 0 );

            sgui_window_clear( wnd, 0, 0, wnd->w, wnd->h );

            SEND_EVENT( wnd, SGUI_SIZE_CHANGE_EVENT, &se );

            /* redraw everything */
            SEND_EVENT( wnd, SGUI_DRAW_EVENT, NULL );
            break;
        case ClientMessage:
            atom = XGetAtomName( wnd->dpy, e.xclient.message_type );

            if( *atom == *wmDeleteWindow )
                wnd->mapped = 0;

            XFree( atom );

            SEND_EVENT( wnd, SGUI_USER_CLOSED_EVENT, NULL );
            break;
        case Expose:
            XPutImage( wnd->dpy, wnd->wnd, wnd->gc, wnd->back_buffer,
                       0, 0, 0, 0, wnd->w, wnd->h );
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



void sgui_window_clear( sgui_window* wnd, int x, int y,
                        unsigned int width, unsigned int height )
{
    unsigned int X, Y;
    unsigned long color = 0;
    unsigned char rgb[3];

    sgui_skin_get_window_background_color( rgb );

    color |= ((unsigned long)rgb[0]) << 16;
    color |= ((unsigned long)rgb[1]) << 8;
    color |= ((unsigned long)rgb[2]);

    for( Y=0; Y<height; ++Y )
    {
        if( ((int)Y+y)<0 || ((int)Y+y)>=(int)wnd->h )
            continue;

        for( X=0; X<width; ++X )
        {
            if( ((int)X+x)<0 || ((int)X+x)>=(int)wnd->w )
                continue;

            XPutPixel( wnd->back_buffer, X+x, Y+y, color );
        }
    }
}

void sgui_window_blit_image( sgui_window* wnd, int x, int y,
                             unsigned int width, unsigned int height,
                             unsigned char* image, int has_a )
{
    unsigned int i, j, bpp;
    unsigned long color, R, G, B;

    if( x>=(int)wnd->w || y>=(int)wnd->h )
        return;

    if( (x+(int)width)<0 || (y+(int)width)<0 )
        return;

    bpp = has_a ? 4 : 3;

    for( j=0; j<height; ++j )
    {
        for( i=0; i<width; ++i, image+=bpp )
        {
            R = image[0];
            G = image[1];
            B = image[2];

            if( (x+(int)i)>0 && (y+(int)j)>0 &&
                (x+(int)i)<(int)wnd->w && (y+(int)j)<(int)wnd->h )
            {
                color = R<<16 | G<<8 | B;

                XPutPixel( wnd->back_buffer, x+i, y+j, color );
            }
        }
    }
}

void sgui_window_blend_image( sgui_window* wnd, int x, int y,
                              unsigned int width, unsigned int height,
                              unsigned char* image )
{
    unsigned int i, j;
    unsigned long color, R, G, B, srcR, srcG, srcB;
    float A;

    if( x>=(int)wnd->w || y>=(int)wnd->h )
        return;

    if( (x+(int)width)<0 || (y+(int)width)<0 )
        return;

    for( j=0; j<height; ++j )
    {
        for( i=0; i<width; ++i )
        {
            R = *(image++);
            G = *(image++);
            B = *(image++);
            A = (float)(*(image++)) / 255.0f;

            if( (x+(int)i)>0 && (y+(int)j)>0 &&
                (x+(int)i)<(int)wnd->w && (y+(int)j)<(int)wnd->h )
            {
                color = XGetPixel( wnd->back_buffer, x+i, y+j );

                srcR = (color>>16) & 0xFF;
                srcG = (color>>8 ) & 0xFF;
                srcB =  color      & 0xFF;

                R = R * A + srcR * (1.0f-A);
                G = G * A + srcG * (1.0f-A);
                B = B * A + srcB * (1.0f-A);

                color = R<<16 | G<<8 | B;

                XPutPixel( wnd->back_buffer, x+i, y+j, color );
            }
        }
    }
}

