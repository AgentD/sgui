/*
 * messagebox_x11.c
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
#include "sgui_messagebox.h"
#include "sgui_internal.h"

#ifdef MACHINE_OS_UNIX
#include <stdio.h>

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>



void sgui_message_box_emergency( const char* caption, const char* text )
{
    int black, white, length, height, direction, ascent, descent, X, Y, W, H;
    int okX1, okY1, okX2, okY2, okBaseX, okBaseY, okWidth, okHeight, run=1;
    int buttonFocus=0, offset, i, j, lines;
    Atom atom_wm_delete;
    Display* dpy=NULL;
    Window wnd=0;
    GC gc=0;
    XFontStruct* font;
    XCharStruct overall;
    XSizeHints hints;
    XEvent e;

    /* Open a display */
    if( !(dpy = XOpenDisplay( 0 )) )
        return;

    atom_wm_delete = XInternAtom( dpy, "WM_DELETE_WINDOW", True );
    black = BlackPixel( dpy, DefaultScreen(dpy) );
    white = WhitePixel( dpy, DefaultScreen(dpy) );

    /* Create a window with the specified caption */
    wnd = XCreateSimpleWindow( dpy, DefaultRootWindow(dpy), 0, 0, 100, 100,
                               0, black, black );

    if( !wnd )
        goto fail;

    XSelectInput( dpy, wnd, ExposureMask | StructureNotifyMask |
                            KeyReleaseMask | PointerMotionMask |
                            ButtonPressMask | ButtonReleaseMask );

    XStoreName( dpy, wnd, caption );
    XSetWMProtocols( dpy, wnd, &atom_wm_delete, 1 );
    XFlush( dpy );

    /* Create a graphics context for the window */
    gc = XCreateGC( dpy, wnd, 0, 0 );

    if( !gc )
        goto fail;

    XSetForeground( dpy, gc, white );
    XSetBackground( dpy, gc, black );

    /* Determine the size of the text */
    if( !(font = XQueryFont( dpy, XGContextFromGC(gc) )) )
        goto fail;

    for( lines=1, i=0, j=0, length=0, height=0; text[j]; i=j+1, ++lines )
    {
        for( j=i; text[j] && text[j]!='\n'; ++j ) { }

        XTextExtents( font, text+i, j-i,
                      &direction, &ascent, &descent, &overall );

        length = MAX(overall.width,    length);
        height = MAX((ascent+descent), height);
    }

    /* Resize and position the window accordingly */
    X = DisplayWidth (dpy, DefaultScreen(dpy))/2 - length/2 - 10;
    Y = DisplayHeight(dpy, DefaultScreen(dpy))/2 - lines*height/2 - height-10;
    W = length + 20;
    H = lines*height + height + 40;

    hints.flags      = PSize | PMinSize | PMaxSize;
    hints.min_width  = hints.max_width  = hints.base_width  = W;
    hints.min_height = hints.max_height = hints.base_height = H;

    XMoveResizeWindow( dpy, wnd, X, Y, W, H );
    XSetWMNormalHints( dpy, wnd, &hints );

    /* Determine the size of the OK button */
    XTextExtents( font, "OK", 2, &direction, &ascent, &descent, &overall );

    okWidth = overall.width;
    okHeight = ascent + descent;

    okX1 = W/2 - okWidth/2 - 15;
    okY1 = (lines*height + 20) + 5;
    okX2 = W/2 + okWidth/2 + 15;
    okY2 = okY1 + 4 + okHeight;
    okBaseX = okX1 + 15;
    okBaseY = okY1 + 2 + okHeight;

    XFreeFontInfo( NULL, font, 1 );

    /* Event loop */
    XMapRaised( dpy, wnd );
    XFlush( dpy );

    while( run )
    {
        XNextEvent( dpy, &e );
        offset = 0;

        if( e.type == MotionNotify )
        {
            if( e.xmotion.x>=okX1 && e.xmotion.x<=okX2 &&
                e.xmotion.y>=okY1 && e.xmotion.y<=okY2 )
            {
                if( !buttonFocus )
                    e.type = Expose;

                buttonFocus = 1;
            }
            else
            {
                if( buttonFocus )
                    e.type = Expose;

                buttonFocus = 0;
            }
        }

        switch( e.type )
        {
        case ButtonPress:
        case ButtonRelease:
            if( e.xbutton.button!=Button1 )
                break;

            if( buttonFocus )
            {
                offset = e.type==ButtonPress ? 1 : 0;

                if( !offset )
                    run = 0;
            }
        case Expose:
        case MapNotify:
            XClearWindow( dpy, wnd );

            for( Y=10+height, i=0, j=0; text[j]; i=j+1, Y+=height )
            {
                for( j=i; text[j] && text[j]!='\n'; ++j ) { }

                XDrawString( dpy, wnd, gc, 10, Y, text+i, j-i );
            }

            /* Draw OK button */
            if( buttonFocus )
            {
                XFillRectangle( dpy, wnd, gc, offset+okX1, offset+okY1,
                                              okX2-okX1, okY2-okY1 );
                XSetForeground( dpy, gc, black );
            }
            else
            {
                XDrawLine( dpy, wnd, gc, okX1, okY1, okX2, okY1 );
                XDrawLine( dpy, wnd, gc, okX1, okY2, okX2, okY2 );
                XDrawLine( dpy, wnd, gc, okX1, okY1, okX1, okY2 );
                XDrawLine( dpy, wnd, gc, okX2, okY1, okX2, okY2 );
            }

            XDrawString( dpy, wnd, gc, offset+okBaseX, offset+okBaseY,
                         "OK", 2 );

            XSetForeground( dpy, gc, white );
            XFlush( dpy );
            break;
        case KeyRelease:
            if( XLookupKeysym( &e.xkey, 0 ) == XK_Escape )
                run = 0;
            break;
        case ClientMessage:
            if( e.xclient.data.l[0] == (long)atom_wm_delete )
                run = 0;
            break;
        };
    }

    /* Clean up */
    XFreeGC( dpy, gc );
    XDestroyWindow( dpy, wnd );
    XCloseDisplay( dpy );
    return;
fail:
    if( gc  ) XFreeGC( dpy, gc );
    if( wnd ) XDestroyWindow( dpy, wnd );
    XCloseDisplay( dpy );

    /* last chance: print to error stream */
    fprintf( stderr, "***** %s *****\n%s\n\n", caption, text );
}
#endif /* MACHINE_OS_UNIX */

