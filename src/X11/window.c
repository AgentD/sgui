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
#define SGUI_BUILDING_DLL
#include "internal.h"
#include "sgui_event.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>


void window_x11_get_mouse_position( sgui_window* wnd, int* x, int* y )
{
    Window t1, t2;
    int t3, t4;
    unsigned int t5;

    XQueryPointer( dpy, TO_X11(wnd)->wnd, &t1, &t2, &t3, &t4, x, y, &t5 );
}

void window_x11_set_mouse_position( sgui_window* wnd, int x, int y )
{
    XWarpPointer( dpy, None, TO_X11(wnd)->wnd, 0, 0, wnd->w, wnd->h, x, y );
    XFlush( dpy );

    ++(TO_X11(wnd)->mouse_warped);
}

void window_x11_set_visible( sgui_window* wnd, int visible )
{
    if( visible )
        XMapWindow( dpy, TO_X11(wnd)->wnd );
    else
        XUnmapWindow( dpy, TO_X11(wnd)->wnd );
}

void window_x11_set_title( sgui_window* wnd, const char* title )
{
    XStoreName( dpy, TO_X11(wnd)->wnd, title );
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

        XSetWMNormalHints( dpy, TO_X11(wnd)->wnd, &hints );
    }

    /* resize the window */
    XResizeWindow( dpy, TO_X11(wnd)->wnd, width, height );
    XFlush( dpy );

    /* get the real geometry as the window manager is free to change it */
    XGetWindowAttributes( dpy, TO_X11(wnd)->wnd, &attr );
    wnd->w = (unsigned int)attr.width;
    wnd->h = (unsigned int)attr.height;

    /* resize the back buffer image */
    sgui_canvas_resize( wnd->back_buffer, wnd->w, wnd->h );
}

void window_x11_move_center( sgui_window* wnd )
{
    wnd->x = (DPY_WIDTH  >> 1) - (int)(wnd->w >> 1);
    wnd->y = (DPY_HEIGHT >> 1) - (int)(wnd->h >> 1);
    XMoveWindow( dpy, TO_X11(wnd)->wnd, wnd->x, wnd->y );
}

void window_x11_move( sgui_window* wnd, int x, int y )
{
    XMoveWindow( dpy, TO_X11(wnd)->wnd, x, y );
}

/****************************************************************************/

void handle_window_events( sgui_window_xlib* wnd, XEvent* e )
{
    unsigned int i, num;
    XExposeEvent exp;
    sgui_event se;
    sgui_rect r;
    Status stat;
    KeySym sym;

    switch( e->type )
    {
    case KeyRelease:
        sym = XLookupKeysym( &e->xkey, 0 );
        se.keyboard_event.code = key_entries_translate( sym );

        SEND_EVENT( wnd, SGUI_KEY_RELEASED_EVENT, &se );
        break;
    case KeyPress:
        memset( se.char_event.as_utf8_str, 0,
                sizeof(se.char_event.as_utf8_str) );

        Xutf8LookupString( wnd->ic, &e->xkey,
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
        if( (e->xbutton.button==Button4||e->xbutton.button==Button5) &&
            e->type==ButtonPress )
        {
            se.mouse_wheel.direction = (e->xbutton.button==Button4)?1:-1;

            SEND_EVENT( wnd, SGUI_MOUSE_WHEEL_EVENT, &se );
        }
        else
        {
            if( e->xbutton.button == Button1 )
                se.mouse_press.button = SGUI_MOUSE_BUTTON_LEFT;
            else if( e->xbutton.button == Button2 )
                se.mouse_press.button = SGUI_MOUSE_BUTTON_MIDDLE;
            else if( e->xbutton.button == Button3 )
                se.mouse_press.button = SGUI_MOUSE_BUTTON_RIGHT;
            else
                break;

            window_x11_get_mouse_position( (sgui_window*)wnd,
                                           &se.mouse_press.x,
                                           &se.mouse_press.y );

            if( e->type==ButtonPress )
                SEND_EVENT( wnd, SGUI_MOUSE_PRESS_EVENT, &se );
            else
                SEND_EVENT( wnd, SGUI_MOUSE_RELEASE_EVENT, &se );
        }
        break;
    case MotionNotify:
        if( wnd->mouse_warped )
        {
            --(wnd->mouse_warped);
        }
        else
        {
            se.mouse_move.x = e->xmotion.x<0 ? 0 : e->xmotion.x;
            se.mouse_move.y = e->xmotion.y<0 ? 0 : e->xmotion.y;
            SEND_EVENT( wnd, SGUI_MOUSE_MOVE_EVENT, &se );
        }
        break;
    case ConfigureNotify:
        se.size.new_width  = e->xconfigure.width;
        se.size.new_height = e->xconfigure.height;

        if( !se.size.new_width || !se.size.new_height )
            break;

        if(se.size.new_width==wnd->base.w && se.size.new_height==wnd->base.h)
            break;

        wnd->base.x = e->xconfigure.x;
        wnd->base.y = e->xconfigure.y;
        wnd->base.w = (unsigned int)e->xconfigure.width;
        wnd->base.h = (unsigned int)e->xconfigure.height;

        /* resize the back buffer image */
        sgui_canvas_resize( wnd->base.back_buffer, wnd->base.w, wnd->base.h );

        /* send a size change event */
        SEND_EVENT( wnd, SGUI_SIZE_CHANGE_EVENT, &se );

        /* redraw everything */
        sgui_canvas_clear( wnd->base.back_buffer, NULL );

        sgui_widget_manager_draw_all( wnd->base.mgr, wnd->base.back_buffer );
        break;
    case ClientMessage:
        if( e->xclient.data.l[0] == (long)atom_wm_delete )
        {
            wnd->base.visible = 0;
            XUnmapWindow( dpy, wnd->wnd );
            SEND_EVENT( wnd, SGUI_USER_CLOSED_EVENT, NULL );
        }
        break;
    case Expose:
        display_canvas( wnd->wnd, wnd->context.xlib, wnd->base.back_buffer,
                        e->xexpose.x, e->xexpose.y,
                        e->xexpose.width, e->xexpose.height );
        break;
    };

    /* generate expose events for dirty rectangles */
    num = sgui_widget_manager_num_dirty_rects( wnd->base.mgr );

    exp.type       = Expose;
    exp.serial     = 0;
    exp.send_event = 1;
    exp.display    = dpy;
    exp.window     = wnd->wnd;
    exp.count      = 0;

    for( i=0; i<num; ++i )
    {
        sgui_widget_manager_get_dirty_rect( wnd->base.mgr, &r, i );

        exp.x      = r.left;
        exp.y      = r.top;
        exp.width  = r.right  - r.left + 1;
        exp.height = r.bottom - r.top  + 1;

        XSendEvent( dpy, wnd->wnd, False, ExposureMask, (XEvent*)&exp );
    }

    sgui_widget_manager_draw( wnd->base.mgr, wnd->base.back_buffer );
    sgui_widget_manager_clear_dirty_rects( wnd->base.mgr );
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
    wnd = add_window( );

    if( !wnd )
        return NULL;

    /******************** create the window ********************/
    sgui_skin_get_window_background_color( rgb );

    color |= ((unsigned long)rgb[0]) << 16;
    color |= ((unsigned long)rgb[1]) << 8;
    color |= ((unsigned long)rgb[2]);

    wnd->wnd = XCreateSimpleWindow( dpy, DefaultRootWindow(dpy),
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

        XSetWMNormalHints( dpy, wnd->wnd, &hints );
    }

    /* tell X11 what events we will handle */
    XSelectInput( dpy, wnd->wnd, ExposureMask | StructureNotifyMask |
                                 SubstructureNotifyMask |
                                 KeyPressMask | KeyReleaseMask |
                                 PointerMotionMask |
                                 ButtonPressMask | ButtonReleaseMask );

    XSetWMProtocols( dpy, wnd->wnd, &atom_wm_delete, 1 );

    XFlush( dpy );

    /* get the real geometry as the window manager is free to change it */
    XGetWindowAttributes( dpy, wnd->wnd, &attr );

    wnd->base.x = attr.x;
    wnd->base.y = attr.y;
    wnd->base.w = (unsigned int)attr.width;
    wnd->base.h = (unsigned int)attr.height;

    /*********** Create an input method and context ************/
    wnd->ic = XCreateIC( im, XNInputStyle,
                         XIMPreeditNothing | XIMStatusNothing, XNClientWindow,
                         wnd->wnd, XNFocusWindow, wnd->wnd, NULL );

    if( !wnd->ic )
    {
        sgui_window_destroy( (sgui_window*)wnd );
        return NULL;
    }

    /********************** create canvas **********************/
    wnd->base.back_buffer = sgui_canvas_create( wnd->base.w, wnd->base.h );

    if( !wnd->base.back_buffer )
    {
        sgui_window_destroy( (sgui_window*)wnd );
        return NULL;
    }

    wnd->context.xlib = XCreateGC( dpy, wnd->wnd, 0, 0 );

    if( !wnd->context.xlib )
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

    return (sgui_window*)wnd;
}

void sgui_window_destroy( sgui_window* wnd )
{
    if( !wnd )
        return;

    sgui_internal_window_fire_event( wnd, SGUI_API_DESTROY_EVENT, NULL );

    if( wnd->back_buffer )
        sgui_canvas_destroy( wnd->back_buffer );

    if( TO_X11(wnd)->ic )
        XDestroyIC( TO_X11(wnd)->ic );

    if( TO_X11(wnd)->context.xlib )
        XFreeGC( dpy, TO_X11(wnd)->context.xlib );

    if( TO_X11(wnd)->wnd )
        XDestroyWindow( dpy, TO_X11(wnd)->wnd );

    remove_window( TO_X11(wnd) );
}

