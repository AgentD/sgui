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


static void xlib_window_get_mouse_position(sgui_window* this, int* x, int* y)
{
    Window t1, t2;   /* values we are not interested */
    int t3, t4;      /* into but xlib does not accept */
    unsigned int t5; /* a NULL pointer for these */

    sgui_internal_lock_mutex( );
    XQueryPointer( dpy, TO_X11(this)->wnd, &t1, &t2, &t3, &t4, x, y, &t5 );
    sgui_internal_unlock_mutex( );
}

static void xlib_window_set_mouse_position( sgui_window* this, int x, int y )
{
    sgui_internal_lock_mutex( );
    XWarpPointer(dpy, None, TO_X11(this)->wnd, 0, 0, this->w, this->h, x, y);
    XFlush( dpy );

    ++(TO_X11(this)->mouse_warped);  /* increment warp counter */
    sgui_internal_unlock_mutex( );
}

static void xlib_window_set_visible( sgui_window* this, int visible )
{
    sgui_internal_lock_mutex( );

    if( visible )
        XMapWindow( dpy, TO_X11(this)->wnd );
    else
        XUnmapWindow( dpy, TO_X11(this)->wnd );

    sgui_internal_unlock_mutex( );
}

static void xlib_window_set_title( sgui_window* this, const char* title )
{
    sgui_internal_lock_mutex( );
    XStoreName( dpy, TO_X11(this)->wnd, title );
    sgui_internal_unlock_mutex( );
}

static void xlib_window_set_size( sgui_window* this,
                                  unsigned int width, unsigned int height )
{
    XSizeHints hints;
    XWindowAttributes attr;

    sgui_internal_lock_mutex( );

    /* adjust the fixed size for nonresizeable windows */
    if( !TO_X11(this)->resizeable )
    {
        hints.flags = PSize | PMinSize | PMaxSize;
        hints.min_width  = hints.base_width  = hints.max_width  = (int)width;
        hints.min_height = hints.base_height = hints.max_height = (int)height;

        XSetWMNormalHints( dpy, TO_X11(this)->wnd, &hints );
    }

    /* resize the window */
    XResizeWindow( dpy, TO_X11(this)->wnd, width, height );
    XFlush( dpy );

    /* get the real geometry as the window manager is free to change it */
    XGetWindowAttributes( dpy, TO_X11(this)->wnd, &attr );
    this->w = (unsigned int)attr.width;
    this->h = (unsigned int)attr.height;

    sgui_internal_unlock_mutex( );
}

static void xlib_window_move_center( sgui_window* this )
{
    this->x = (DPY_WIDTH  >> 1) - (int)(this->w >> 1);
    this->y = (DPY_HEIGHT >> 1) - (int)(this->h >> 1);

    sgui_internal_lock_mutex( );
    XMoveWindow( dpy, TO_X11(this)->wnd, this->x, this->y );
    sgui_internal_unlock_mutex( );
}

static void xlib_window_move( sgui_window* this, int x, int y )
{
    sgui_internal_lock_mutex( );
    XMoveWindow( dpy, TO_X11(this)->wnd, x, y );
    sgui_internal_unlock_mutex( );
}

static void xlib_window_force_redraw( sgui_window* this, sgui_rect* r )
{
    XExposeEvent exp;

    exp.type       = Expose;
    exp.serial     = 0;
    exp.send_event = 1;
    exp.display    = dpy;
    exp.window     = TO_X11(this)->wnd;
    exp.count      = 0;
    exp.x          = r->left;
    exp.y          = r->top;
    exp.width      = r->right  - r->left + 1;
    exp.height     = r->bottom - r->top  + 1;

    sgui_internal_lock_mutex( );
    XSendEvent( dpy, TO_X11(this)->wnd, False, ExposureMask, (XEvent*)&exp );
    sgui_internal_unlock_mutex( );
}

static void xlib_window_destroy( sgui_window* this )
{
    sgui_internal_lock_mutex( );

    if( TO_X11(this)->ic )
        XDestroyIC( TO_X11(this)->ic );

    if( this->backend==SGUI_OPENGL_CORE || this->backend==SGUI_OPENGL_COMPAT )
    {
#ifndef SGUI_NO_OPENGL
        if( this->ctx.gl )
            sgui_gl_context_destroy( this->ctx.gl );
#endif
    }
    else if( this->backend==SGUI_NATIVE && this->ctx.canvas )
    {
        sgui_canvas_destroy( this->ctx.canvas );
    }

    if( TO_X11(this)->wnd )
        XDestroyWindow( dpy, TO_X11(this)->wnd );

    remove_window( TO_X11(this) );
    sgui_internal_unlock_mutex( );

    free( this );
}

#ifndef SGUI_NO_OPENGL
static void xlib_window_set_vsync( sgui_window* this, int interval )
{
    if( this->backend==SGUI_OPENGL_CORE || this->backend==SGUI_OPENGL_COMPAT )
    {
        void(* SwapIntervalEXT )( Display*, GLXDrawable, int );

        sgui_internal_lock_mutex( );

        SwapIntervalEXT = (void(*)(Display*,GLXDrawable,int))
                          LOAD_GLFUN( "glXSwapIntervalEXT" );

        if( SwapIntervalEXT )
            SwapIntervalEXT( dpy, TO_X11(this)->wnd, interval );

        sgui_internal_unlock_mutex( );
    }
}
#endif

static void xlib_window_get_platform_data( const sgui_window* this,
                                           void* window )
{
    *((Window*)window) = TO_X11(this)->wnd;
}

/****************************************************************************/

void update_window( sgui_window_xlib* this )
{
    sgui_window* super = (sgui_window*)this;
    unsigned int i, num;
    XExposeEvent exp;
    sgui_rect r;

    if( super->backend==SGUI_NATIVE )
    {
        num = sgui_canvas_num_dirty_rects( super->ctx.canvas );

        exp.type       = Expose;
        exp.serial     = 0;
        exp.send_event = 1;
        exp.display    = dpy;
        exp.window     = this->wnd;
        exp.count      = 0;

        for( i=0; i<num; ++i )
        {
            sgui_canvas_get_dirty_rect( super->ctx.canvas, &r, i );

            exp.x      = r.left;
            exp.y      = r.top;
            exp.width  = r.right  - r.left + 1;
            exp.height = r.bottom - r.top  + 1;

            XSendEvent( dpy, this->wnd, False, ExposureMask, (XEvent*)&exp );
        }

        sgui_canvas_redraw_widgets( super->ctx.canvas, 1 );
    }
}

void handle_window_events( sgui_window_xlib* this, XEvent* e )
{
    sgui_window* super = (sgui_window*)this;
    sgui_event se;
    Status stat;
    KeySym sym;

    se.window = super;

    switch( e->type )
    {
    case KeyRelease:
        sym = XLookupKeysym( &e->xkey, 0 );
        se.arg.i = key_entries_translate( sym );
        se.type = SGUI_KEY_RELEASED_EVENT;
        sgui_internal_window_fire_event( super, &se );
        break;
    case KeyPress:
        memset( se.arg.utf8, 0, sizeof(se.arg.utf8) );

        /* try to convert composed character to UTF8 string */
        Xutf8LookupString( this->ic, &e->xkey,
                           (char*)se.arg.utf8, sizeof(se.arg.utf8),
                           &sym, &stat );

        /* send a char event if it worked */
        if( stat==XLookupChars || stat==XLookupBoth )
        {
            if( (se.arg.utf8[0] & 0x80) || !iscntrl( se.arg.utf8[0] ) )
            {
                se.type = SGUI_CHAR_EVENT;
                sgui_internal_window_fire_event( super, &se );
            }
        }

        /* send a key pressed event if we have a key sym */
        if( stat==XLookupKeySym || stat==XLookupBoth )
        {
            se.arg.i = key_entries_translate( sym );
            se.type = SGUI_KEY_PRESSED_EVENT;
            sgui_internal_window_fire_event( super, &se );
        }
        break;
    case ButtonPress:
    case ButtonRelease:
        /* Button4 and Button5 are mouse wheel up and down */
        if( e->xbutton.button==Button4 || e->xbutton.button==Button5 )
        {
            if( e->type==ButtonPress )
            {
                se.type = SGUI_MOUSE_WHEEL_EVENT;
                se.arg.i = (e->xbutton.button==Button4) ? 1 : -1;
                sgui_internal_window_fire_event( super, &se );
            }
        }
        else
        {
            switch( e->xbutton.button )
            {
            case Button1: se.arg.i3.z = SGUI_MOUSE_BUTTON_LEFT;   break;
            case Button2: se.arg.i3.z = SGUI_MOUSE_BUTTON_MIDDLE; break;
            case Button3: se.arg.i3.z = SGUI_MOUSE_BUTTON_RIGHT;  break;
            }

            se.arg.i3.x = e->xbutton.x;
            se.arg.i3.y = e->xbutton.y;
            se.type = e->type==ButtonPress ? SGUI_MOUSE_PRESS_EVENT :
                                             SGUI_MOUSE_RELEASE_EVENT;

            sgui_internal_window_fire_event( super, &se );
        }
        break;
    case MotionNotify:
        /* ignore mouse move event when the warp counter is positive */
        if( this->mouse_warped )
        {
            --(this->mouse_warped);  /* decrement warp counter */
        }
        else
        {
            se.arg.i2.x = e->xmotion.x<0 ? 0 : e->xmotion.x;
            se.arg.i2.y = e->xmotion.y<0 ? 0 : e->xmotion.y;
            se.type = SGUI_MOUSE_MOVE_EVENT;
            sgui_internal_window_fire_event( super, &se );
        }
        break;
    case ConfigureNotify:
        se.arg.ui2.x = e->xconfigure.width;
        se.arg.ui2.y = e->xconfigure.height;
        se.type = SGUI_SIZE_CHANGE_EVENT;

        /* store the new position */
        super->x = e->xconfigure.x;
        super->y = e->xconfigure.y;

        /* do not accept zero size window */
        if( !se.arg.ui2.x || !se.arg.ui2.y )
            break;

        /* ignore if the size didn't change at all */
        if( se.arg.ui2.x==super->w && se.arg.ui2.y==super->h )
            break;

        /* store the new size */
        super->w = (unsigned int)e->xconfigure.width;
        super->h = (unsigned int)e->xconfigure.height;

        /* resize the back buffer image */
        if( super->backend==SGUI_NATIVE )
            sgui_canvas_resize( super->ctx.canvas, super->w, super->h );

        /* send a size change event */
        sgui_internal_window_fire_event( super, &se );

        /* redraw everything */
        if( super->backend==SGUI_NATIVE )
            sgui_canvas_draw_widgets( super->ctx.canvas, 1 );
        break;
    case DestroyNotify:
        super->visible = 0;
        this->wnd = 0;
        break;
    case MapNotify:
        if( super->backend==SGUI_OPENGL_CORE ||
            super->backend==SGUI_OPENGL_COMPAT )
        {
            sgui_rect_set_size( &se.arg.rect, 0, 0, super->w, super->h );
            se.type = SGUI_EXPOSE_EVENT;
            sgui_internal_window_fire_event( super, &se );
        }
        break;
    case UnmapNotify:
        if( e->xunmap.window==this->wnd && this->is_child )
        {
            se.type = SGUI_USER_CLOSED_EVENT;
            super->visible = 0;
            sgui_internal_window_fire_event( super, &se );
        }
        break;
    case ClientMessage:
        if( e->xclient.data.l[0] == (long)atom_wm_delete )
        {
            se.type = SGUI_USER_CLOSED_EVENT;
            super->visible = 0;
            XUnmapWindow( dpy, this->wnd );
            XUnmapSubwindows( dpy, this->wnd );
            sgui_internal_window_fire_event( super, &se );
        }
        break;
    case Expose:
        if( super->backend==SGUI_NATIVE )
        {
            canvas_xlib_display(super->ctx.canvas, e->xexpose.x, e->xexpose.y,
                                e->xexpose.width, e->xexpose.height);
        }
        else if( super->backend==SGUI_OPENGL_CORE ||
                 super->backend==SGUI_OPENGL_COMPAT )
        {
            se.type = SGUI_EXPOSE_EVENT;
            sgui_rect_set_size( &se.arg.rect, 0, 0, super->w, super->h );
            sgui_internal_window_fire_event( super, &se );
        }
        break;
    };
}

/****************************************************************************/

sgui_window* sgui_window_create_desc( const sgui_window_description* desc )
{
    sgui_window_xlib* this;
    sgui_window* super;
    XSizeHints hints;
    XWindowAttributes attr;
#ifndef SGUI_NO_OPENGL
    XSetWindowAttributes swa;
    XVisualInfo* vi = NULL;
#endif
    unsigned long color = 0;
    unsigned char rgb[3];
    Window x_parent;

    if( !desc || !desc->width || !desc->height )
        return NULL;

#ifdef SGUI_NO_OPENGL
    if( desc->backend==SGUI_OPENGL_CORE || desc->backend==SGUI_OPENGL_COMPAT )
        return NULL;
#endif

    /********* allocate space for the window structure *********/
    this = malloc( sizeof(sgui_window_xlib) );
    super = (sgui_window*)this;

    if( !this )
        return NULL;

    memset( this, 0, sizeof(sgui_window_xlib) );

    sgui_internal_lock_mutex( );
    add_window( this );

    /******************** create the window ********************/
    x_parent = desc->parent ? TO_X11(desc->parent)->wnd : root;
    this->is_child = (desc->parent!=NULL);

    if( desc->backend==SGUI_OPENGL_CORE || desc->backend==SGUI_OPENGL_COMPAT )
    {
#ifndef SGUI_NO_OPENGL
        /* Get an fbc, a visual and a Colormap */
        if( !get_fbc_visual_cmap( &this->cfg, &vi, &swa.colormap, desc ) )
            goto failure;

        /* create the window */
        swa.border_pixel = 0;

        this->wnd = XCreateWindow( dpy, x_parent, 0, 0,
                                   desc->width, desc->height, 0,
                                   vi->depth, InputOutput, vi->visual,
                                   CWBorderPixel|CWColormap, &swa );

        XFree( vi );
#endif
    }
    else
    {
        sgui_skin_get_window_background_color( rgb );

        color = (rgb[0] << 16) | (rgb[1] << 8) | (rgb[2]);

        this->wnd = XCreateSimpleWindow( dpy, x_parent, 0, 0,
                                         desc->width, desc->height,
                                         0, 0, color );
    }

    if( !this->wnd )
        goto failure;

    /* make the window non resizeable if required */
    if( !desc->resizeable )
    {
        hints.flags = PSize | PMinSize | PMaxSize;
        hints.min_width = hints.max_width =
        hints.base_width = (int)desc->width;
        hints.min_height = hints.max_height =
        hints.base_height = (int)desc->height;

        XSetWMNormalHints( dpy, this->wnd, &hints );
    }

    /* tell X11 what events we will handle */
    XSelectInput( dpy, this->wnd, ExposureMask | StructureNotifyMask |
                                  KeyPressMask | KeyReleaseMask |
                                  PointerMotionMask | PropertyChangeMask |
                                  ButtonPressMask | ButtonReleaseMask );

    XSetWMProtocols( dpy, this->wnd, &atom_wm_delete, 1 );

    XFlush( dpy );

    /* get the real geometry as the window manager is free to change it */
    XGetWindowAttributes( dpy, this->wnd, &attr );

    super->x = attr.x;
    super->y = attr.y;

    /********************** create canvas **********************/
    if( desc->backend==SGUI_OPENGL_CORE || desc->backend==SGUI_OPENGL_COMPAT )
    {
#ifndef SGUI_NO_OPENGL
        super->backend = desc->backend;
        super->ctx.gl =
        sgui_gl_context_create( super, desc->share?desc->share->ctx.gl:NULL,
                                desc->backend==SGUI_OPENGL_CORE );

        if( !super->ctx.gl )
            goto failure;

        super->swap_buffers = gl_swap_buffers;
#endif
    }
    else
    {
        super->ctx.canvas = canvas_xlib_create( this->wnd, attr.width,
                                                attr.height );

        if( !super->ctx.canvas )
            goto failure;

        super->swap_buffers = NULL;
    }

    /*********** Create an input context ************/
    this->ic = XCreateIC( im, XNInputStyle,
                          XIMPreeditNothing|XIMStatusNothing, XNClientWindow,
                          this->wnd, XNFocusWindow, this->wnd, NULL );

    if( !this->ic )
        goto failure;

    sgui_internal_window_post_init( super, attr.width, attr.height,
                                    desc->backend );

    this->resizeable = desc->resizeable;

    /* store entry points */
    super->get_mouse_position = xlib_window_get_mouse_position;
    super->set_mouse_position = xlib_window_set_mouse_position;
    super->set_visible        = xlib_window_set_visible;
    super->set_title          = xlib_window_set_title;
    super->set_size           = xlib_window_set_size;
    super->move_center        = xlib_window_move_center;
    super->move               = xlib_window_move;
    super->force_redraw       = xlib_window_force_redraw;
    super->get_platform_data  = xlib_window_get_platform_data;
    super->write_clipboard    = xlib_window_clipboard_write;
    super->read_clipboard     = xlib_window_clipboard_read;
    super->destroy            = xlib_window_destroy;

#ifdef SGUI_NO_OPENGL
    super->set_vsync = NULL;
#else
    super->set_vsync = xlib_window_set_vsync;
#endif

    sgui_internal_unlock_mutex( );
    return (sgui_window*)this;
failure:
    sgui_internal_unlock_mutex( );
    xlib_window_destroy( super );
    return NULL;
}

