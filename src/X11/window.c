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


const char* wmDeleteWindow = "WM_DELETE_WINDOW";



void clear( sgui_window* wnd, unsigned long color )
{
    unsigned int X, Y;

    for( Y=0; Y<wnd->h; ++Y )
        for( X=0; X<wnd->w; ++X )
            XPutPixel( wnd->back_buffer, X, Y, color );
}

void draw_box( sgui_window* wnd, int x, int y,
                                 unsigned int w, unsigned int h,
                                 unsigned long color )
{
    int lr_x = x + (int)w;
    int lr_y = y + (int)h;
    int X, Y;

    x = x<0 ? 0 : x;
    y = y<0 ? 0 : y;

    if( x >= (int)wnd->w || y >= (int)wnd->h || lr_x < 0 || lr_y < 0 )
        return;

    if( lr_x >= (int)wnd->w )
        lr_x = (int)wnd->w - 1;

    if( lr_y >= (int)wnd->h )
        lr_y = (int)wnd->h - 1;

    for( Y=y; Y!=lr_y; ++Y )
        for( X=x; X!=lr_x; ++X )
            XPutPixel( wnd->back_buffer, X, Y, color );
}

void draw_line( sgui_window* wnd, int x, int y, int length, int horizontal,
                                  unsigned long color )
{
    int i, endi;

    if( horizontal )
    {
        if( (y<0) || (y>=(int)wnd->h) )
            return;

        if( length < 0 )
        {
            length = -length;
            x -= length;
        }

        if( (x >= (int)wnd->w) || ((x+length)<0) )
            return;

        i = x < 0 ? 0 : x;
        endi = x+length+1;

        if( endi >= (int)wnd->w )
            endi = wnd->w - 1;

        for( ; i!=endi; ++i )
            XPutPixel( wnd->back_buffer, i, y, color );
    }
    else
    {
        if( (x<0) || (x>=(int)wnd->w) )
            return;

        if( length < 0 )
        {
            length = -length;
            y -= length;
        }

        if( (y >= (int)wnd->h) || ((y+length)<0) )
            return;

        i = y < 0 ? 0 : y;
        endi = y+length+1;

        if( endi >= (int)wnd->h )
            endi = wnd->h - 1;

        for( ; i!=endi; ++i )
            XPutPixel( wnd->back_buffer, x, i, color );
    }
}





sgui_window* sgui_window_create( unsigned int width, unsigned int height,
                                 int resizeable )
{
    sgui_window* wnd;
    XSizeHints hints;
    XWindowAttributes attr;

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

    /******************** create the window ********************/
    wnd->wnd = XCreateSimpleWindow( wnd->dpy, DefaultRootWindow(wnd->dpy),
                                    0, 0, width, height, 0,
                                    SGUI_WINDOW_COLOR, SGUI_WINDOW_COLOR );

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

    clear( wnd, SGUI_WINDOW_COLOR );

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

    clear( wnd, SGUI_WINDOW_COLOR );

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
    XEvent e;
    char* atom;
    sgui_event se;
    XExposeEvent exp;

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

        memset( &se, 0, sizeof(sgui_event) );

        switch( e.type )
        {
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

            clear( wnd, SGUI_WINDOW_COLOR );

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

void sgui_window_draw_box( sgui_window* wnd, int x, int y,
                           unsigned int width, unsigned int height,
                           unsigned long bgcolor, int inset )
{
    draw_box( wnd, x, y, width, height, bgcolor );

    if( inset>0 )
    {
        draw_line( wnd, x,       y,        width,  1, SGUI_INSET_COLOR  );
        draw_line( wnd, x,       y,        height, 0, SGUI_INSET_COLOR  );

        draw_line( wnd, x,       y+height, width,  1, SGUI_OUTSET_COLOR );
        draw_line( wnd, x+width, y,        height, 0, SGUI_OUTSET_COLOR );
    }
    else if( inset<0 )
    {
        draw_line( wnd, x,       y,        width,  1, SGUI_OUTSET_COLOR );
        draw_line( wnd, x,       y,        height, 0, SGUI_OUTSET_COLOR );

        draw_line( wnd, x,       y+height, width,  1, SGUI_INSET_COLOR  );
        draw_line( wnd, x+width, y,        height, 0, SGUI_INSET_COLOR  );
    }
}

void sgui_window_draw_fancy_lines( sgui_window* wnd, int x, int y,
                                   int* length, unsigned int num_lines,
                                   int start_horizontal )
{
    unsigned int i;
    int h, oldx = x, oldy = y;

    for( h=start_horizontal, i=0; i<num_lines; ++i, h=!h )
    {
        if( h )
        {
            draw_line( wnd, x, y+1, length[i]+1, 1, SGUI_OUTSET_COLOR );
            x += length[i];
        }
        else
        {
            draw_line( wnd, x+1, y, length[i], 0, SGUI_OUTSET_COLOR );
            y += length[i];
        }
    }

    for( x=oldx, y=oldy, h=start_horizontal, i=0; i<num_lines; ++i, h=!h )
    {
        draw_line( wnd, x, y, length[i], h, SGUI_INSET_COLOR );

        if( h )
            x += length[i];
        else
            y += length[i];
    }
}

