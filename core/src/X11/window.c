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
#include "sgui_opengl.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>


void xlib_window_get_mouse_position( sgui_window* wnd, int* x, int* y )
{
    Window t1, t2;   /* values we are not interested */
    int t3, t4;      /* into but xlib does not accept */
    unsigned int t5; /* a NULL pointer for these */

    XQueryPointer( dpy, TO_X11(wnd)->wnd, &t1, &t2, &t3, &t4, x, y, &t5 );
}

void xlib_window_set_mouse_position( sgui_window* wnd, int x, int y )
{
    XWarpPointer( dpy, None, TO_X11(wnd)->wnd, 0, 0, wnd->w, wnd->h, x, y );
    XFlush( dpy );

    ++(TO_X11(wnd)->mouse_warped);  /* increment warp counter */
}

void xlib_window_set_visible( sgui_window* wnd, int visible )
{
    if( visible )
        XMapWindow( dpy, TO_X11(wnd)->wnd );
    else
        XUnmapWindow( dpy, TO_X11(wnd)->wnd );
}

void xlib_window_set_title( sgui_window* wnd, const char* title )
{
    XStoreName( dpy, TO_X11(wnd)->wnd, title );
}

void xlib_window_set_size( sgui_window* wnd,
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
}

void xlib_window_move_center( sgui_window* wnd )
{
    wnd->x = (DPY_WIDTH  >> 1) - (int)(wnd->w >> 1);
    wnd->y = (DPY_HEIGHT >> 1) - (int)(wnd->h >> 1);
    XMoveWindow( dpy, TO_X11(wnd)->wnd, wnd->x, wnd->y );
}

void xlib_window_move( sgui_window* wnd, int x, int y )
{
    XMoveWindow( dpy, TO_X11(wnd)->wnd, x, y );
}

void xlib_window_destroy( sgui_window* wnd )
{
    sgui_internal_window_deinit( wnd );

    if( TO_X11(wnd)->ic )
        XDestroyIC( TO_X11(wnd)->ic );

#ifndef SGUI_NO_OPENGL
    if( wnd->backend==SGUI_OPENGL_CORE || wnd->backend==SGUI_OPENGL_COMPAT )
    {
        if( TO_X11(wnd)->gl )
            glXDestroyContext( dpy, TO_X11(wnd)->gl );
    }
#endif

    if( TO_X11(wnd)->wnd )
        XDestroyWindow( dpy, TO_X11(wnd)->wnd );

    remove_window( TO_X11(wnd) );
    free( wnd );
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

        /* try to convert composed character to UTF8 string */
        Xutf8LookupString( wnd->ic, &e->xkey,
                           (char*)se.char_event.as_utf8_str,
                           sizeof(se.char_event.as_utf8_str),
                           &sym, &stat );

        /* send a char event if it worked */
        if( stat==XLookupChars || stat==XLookupBoth )
        {
            if( (se.char_event.as_utf8_str[0] & 0x80) ||
                !iscntrl( se.char_event.as_utf8_str[0] ) )
            {
                SEND_EVENT( wnd, SGUI_CHAR_EVENT, &se );
            }
        }

        /* send a key pressed event if we have a key sym */
        if( stat==XLookupKeySym || stat==XLookupBoth )
        {
            se.keyboard_event.code = key_entries_translate( sym );

            SEND_EVENT( wnd, SGUI_KEY_PRESSED_EVENT, &se );
        }
        break;
    case ButtonPress:
    case ButtonRelease:
        /* Button4 and Button5 are mouse wheel up and down */
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

            sgui_window_get_mouse_position( (sgui_window*)wnd,
                                            &se.mouse_press.x,
                                            &se.mouse_press.y );

            if( e->type==ButtonPress )
                SEND_EVENT( wnd, SGUI_MOUSE_PRESS_EVENT, &se );
            else
                SEND_EVENT( wnd, SGUI_MOUSE_RELEASE_EVENT, &se );
        }
        break;
    case MotionNotify:
        /* ignore mouse move event when the warp counter is positive */
        if( wnd->mouse_warped )
        {
            --(wnd->mouse_warped);  /* decrement warp counter */
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

        /* store the new position */
        wnd->base.x = e->xconfigure.x;
        wnd->base.y = e->xconfigure.y;

        /* do not accept zero size window */
        if( !se.size.new_width || !se.size.new_height )
            break;

        /* ignore if the size didn't change at all */
        if(se.size.new_width==wnd->base.w && se.size.new_height==wnd->base.h)
            break;

        /* store the new size */
        wnd->base.w = (unsigned int)e->xconfigure.width;
        wnd->base.h = (unsigned int)e->xconfigure.height;

        /* resize the back buffer image */
        sgui_canvas_resize( wnd->base.back_buffer, wnd->base.w, wnd->base.h );

        /* send a size change event */
        SEND_EVENT( wnd, SGUI_SIZE_CHANGE_EVENT, &se );

        /* redraw everything */
        if( wnd->base.backend==SGUI_NATIVE )
        {
            sgui_canvas_begin( wnd->base.back_buffer, NULL );
            sgui_canvas_clear( wnd->base.back_buffer, NULL );
            sgui_widget_manager_draw( wnd->base.mgr,
                                      wnd->base.back_buffer, NULL );
            sgui_canvas_end( wnd->base.back_buffer );
            sgui_widget_manager_clear_dirty_rects( wnd->base.mgr );
        }
        break;
    case DestroyNotify:
        wnd->base.visible = 0;
        wnd->wnd = 0;
        break;
    case UnmapNotify:
        if( e->xunmap.window==wnd->wnd && wnd->is_child )
        {
            wnd->base.visible = 0;
            SEND_EVENT( wnd, SGUI_USER_CLOSED_EVENT, NULL );
        }
        break;
    case ClientMessage:
        if( e->xclient.data.l[0] == (long)atom_wm_delete )
        {
            wnd->base.visible = 0;
            XUnmapWindow( dpy, wnd->wnd );
            XUnmapSubwindows( dpy, wnd->wnd );
            SEND_EVENT( wnd, SGUI_USER_CLOSED_EVENT, NULL );
        }
        break;
    case Expose:
        if( wnd->base.backend==SGUI_NATIVE )
        {
            sgui_rect_set_size( &se.expose_event, e->xexpose.x, e->xexpose.y,
                                e->xexpose.width, e->xexpose.height );

            SEND_EVENT( wnd, SGUI_EXPOSE_EVENT, &se );

            canvas_xlib_display( wnd->base.back_buffer,
                                 e->xexpose.x, e->xexpose.y,
                                 e->xexpose.width, e->xexpose.height );
        }
        else if( wnd->base.backend==SGUI_OPENGL_CORE ||
                 wnd->base.backend==SGUI_OPENGL_COMPAT )
        {
            sgui_window_make_current( (sgui_window*)wnd );

            sgui_canvas_begin( wnd->base.back_buffer, NULL );
            sgui_canvas_clear( wnd->base.back_buffer, NULL );
            sgui_canvas_end( wnd->base.back_buffer );

            SEND_EVENT( wnd, SGUI_EXPOSE_EVENT, &se );

            sgui_canvas_begin( wnd->base.back_buffer, NULL );
            sgui_widget_manager_draw( wnd->base.mgr,
                                      wnd->base.back_buffer, NULL );
            sgui_widget_manager_clear_dirty_rects( wnd->base.mgr );
            sgui_canvas_end( wnd->base.back_buffer );
            sgui_window_swap_buffers( (sgui_window*)wnd );
            sgui_window_make_current( NULL );
        }
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

    if( wnd->base.backend==SGUI_NATIVE )
    {
        for( i=0; i<num; ++i )
        {
            sgui_widget_manager_get_dirty_rect( wnd->base.mgr, &r, i );

            exp.x      = r.left;
            exp.y      = r.top;
            exp.width  = r.right  - r.left + 1;
            exp.height = r.bottom - r.top  + 1;

            XSendEvent( dpy, wnd->wnd, False, ExposureMask, (XEvent*)&exp );

            sgui_canvas_begin( wnd->base.back_buffer, &r );
            sgui_canvas_clear( wnd->base.back_buffer, &r );
            sgui_widget_manager_draw(wnd->base.mgr,wnd->base.back_buffer,&r);
            sgui_canvas_end( wnd->base.back_buffer );
        }

        sgui_widget_manager_clear_dirty_rects( wnd->base.mgr );
    }
    else if( wnd->base.backend==SGUI_OPENGL_CORE ||
             wnd->base.backend==SGUI_OPENGL_COMPAT )
    {
        if( num )
        {
            exp.x      = 0;
            exp.y      = 0;
            exp.width  = wnd->base.w;
            exp.height = wnd->base.h;

            XSendEvent( dpy, wnd->wnd, False, ExposureMask, (XEvent*)&exp );
            sgui_widget_manager_clear_dirty_rects( wnd->base.mgr );
        }
    }
}

/****************************************************************************/

sgui_window* sgui_window_create( sgui_window* parent, unsigned int width,
                                 unsigned int height, int resizeable,
                                 int backend )
{
    sgui_window_xlib* wnd;
    XSizeHints hints;
    XWindowAttributes attr;
#ifndef SGUI_NO_OPENGL
    XSetWindowAttributes swa;
    XVisualInfo* vi = NULL;
    GLXFBConfig fbc = NULL;
#endif
    unsigned long color = 0;
    unsigned char rgb[3];
    Window x_parent;

    if( !width || !height )
        return NULL;

#ifdef SGUI_NO_OPENGL
    if( backend==SGUI_OPENGL_CORE || backend==SGUI_OPENGL_COMPAT )
        return NULL;
#endif

    /********* allocate space for the window structure *********/
    wnd = malloc( sizeof(sgui_window_xlib) );

    if( !wnd )
        return NULL;

    memset( wnd, 0, sizeof(sgui_window_xlib) );

    if( !sgui_internal_window_init( (sgui_window*)wnd ) )
    {
        free( wnd );
        return NULL;
    }

    wnd->base.backend = backend;
    wnd->resizeable = resizeable;

    add_window( wnd );

    /******************** create the window ********************/
    x_parent = parent ? TO_X11(parent)->wnd : DefaultRootWindow(dpy);
    wnd->is_child = (parent!=NULL);

    if( backend==SGUI_OPENGL_CORE || backend==SGUI_OPENGL_COMPAT )
    {
#ifndef SGUI_NO_OPENGL
        /* Get an fbc (optional), a visual and a Colormap */
        if( !get_fbc_visual_cmap( &fbc, &vi, &swa.colormap ) )
        {
            xlib_window_destroy( (sgui_window*)wnd );
            return NULL;
        }

        /* create the window */
        swa.background_pixmap = None;
        swa.border_pixel      = 0;

        wnd->wnd = XCreateWindow( dpy, x_parent, 0, 0, width, height, 0,
                                  vi->depth, InputOutput, vi->visual,
                                  CWBorderPixel|CWColormap, &swa );

        if( !wnd->wnd )
            XFree( vi );
#endif
    }
    else
    {
        sgui_skin_get_window_background_color( rgb );

        color |= ((unsigned long)rgb[0]) << 16;
        color |= ((unsigned long)rgb[1]) << 8;
        color |= ((unsigned long)rgb[2]);

        wnd->wnd = XCreateSimpleWindow( dpy, x_parent, 0, 0, width, height,
                                        0, 0, color );
    }

    if( !wnd->wnd )
    {
        xlib_window_destroy( (sgui_window*)wnd );
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
                                 KeyPressMask | KeyReleaseMask |
                                 PointerMotionMask |
                                 ButtonPressMask | ButtonReleaseMask );

    XSetWMProtocols( dpy, wnd->wnd, &atom_wm_delete, 1 );

    XFlush( dpy );

    /* get the real geometry as the window manager is free to change it */
    XGetWindowAttributes( dpy, wnd->wnd, &attr );

    wnd->base.x = attr.x;
    wnd->base.y = attr.y;

    /********************** create canvas **********************/
    if( backend==SGUI_OPENGL_CORE || backend==SGUI_OPENGL_COMPAT )
    {
#ifndef SGUI_NO_OPENGL
        wnd->gl = create_context( fbc, vi, backend==SGUI_OPENGL_CORE );
        XFree( vi );

        if( !wnd->gl )
        {
            xlib_window_destroy( (sgui_window*)wnd );
            return NULL;
        }

        wnd->base.back_buffer = sgui_opengl_canvas_create( attr.width,
                                                           attr.height );
        wnd->base.swap_buffers = gl_swap_buffers;
#endif
    }
    else
    {
        wnd->base.back_buffer = canvas_xlib_create( wnd->wnd, attr.width,
                                                    attr.height );
    }

    if( !wnd->base.back_buffer )
    {
        xlib_window_destroy( (sgui_window*)wnd );
        return NULL;
    }

    /*********** Create an input context ************/
    wnd->ic = XCreateIC( im, XNInputStyle,
                         XIMPreeditNothing | XIMStatusNothing, XNClientWindow,
                         wnd->wnd, XNFocusWindow, wnd->wnd, NULL );

    if( !wnd->ic )
    {
        xlib_window_destroy( (sgui_window*)wnd );
        return NULL;
    }

    sgui_internal_window_post_init( (sgui_window*)wnd, attr.width,
                                    attr.height, backend );

    /* store entry points */
    wnd->base.get_mouse_position = xlib_window_get_mouse_position;
    wnd->base.set_mouse_position = xlib_window_set_mouse_position;
    wnd->base.set_visible        = xlib_window_set_visible;
    wnd->base.set_title          = xlib_window_set_title;
    wnd->base.set_size           = xlib_window_set_size;
    wnd->base.move_center        = xlib_window_move_center;
    wnd->base.move               = xlib_window_move;
    wnd->base.destroy            = xlib_window_destroy;

    return (sgui_window*)wnd;
}

void sgui_window_make_current( sgui_window* wnd )
{
#ifdef SGUI_NO_OPENGL
    (void)wnd;
#else
    if( wnd && (wnd->backend==SGUI_OPENGL_CORE ||
                wnd->backend==SGUI_OPENGL_COMPAT) )
    {
        glXMakeCurrent( dpy, TO_X11(wnd)->wnd, TO_X11(wnd)->gl );
    }
    else
    {
        glXMakeCurrent( dpy, 0, 0 );
    }
#endif
}

