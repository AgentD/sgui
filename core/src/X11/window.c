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

    XQueryPointer( dpy, TO_X11(this)->wnd, &t1, &t2, &t3, &t4, x, y, &t5 );
}

static void xlib_window_set_mouse_position( sgui_window* this, int x, int y )
{
    XWarpPointer(dpy, None, TO_X11(this)->wnd, 0, 0, this->w, this->h, x, y);
    XFlush( dpy );

    ++(TO_X11(this)->mouse_warped);  /* increment warp counter */
}

static void xlib_window_set_visible( sgui_window* this, int visible )
{
    if( visible )
        XMapWindow( dpy, TO_X11(this)->wnd );
    else
        XUnmapWindow( dpy, TO_X11(this)->wnd );
}

static void xlib_window_set_title( sgui_window* this, const char* title )
{
    XStoreName( dpy, TO_X11(this)->wnd, title );
}

static void xlib_window_set_size( sgui_window* this,
                                  unsigned int width, unsigned int height )
{
    XSizeHints hints;
    XWindowAttributes attr;

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
}

static void xlib_window_move_center( sgui_window* this )
{
    this->x = (DPY_WIDTH  >> 1) - (int)(this->w >> 1);
    this->y = (DPY_HEIGHT >> 1) - (int)(this->h >> 1);
    XMoveWindow( dpy, TO_X11(this)->wnd, this->x, this->y );
}

static void xlib_window_move( sgui_window* this, int x, int y )
{
    XMoveWindow( dpy, TO_X11(this)->wnd, x, y );
}

static void xlib_window_destroy( sgui_window* this )
{
    if( this->canvas )
        sgui_canvas_destroy( this->canvas );

    if( TO_X11(this)->ic )
        XDestroyIC( TO_X11(this)->ic );

#ifndef SGUI_NO_OPENGL
    if( this->backend==SGUI_OPENGL_CORE || this->backend==SGUI_OPENGL_COMPAT )
    {
        if( TO_X11(this)->gl )
            glXDestroyContext( dpy, TO_X11(this)->gl );
    }
#endif

    if( TO_X11(this)->wnd )
        XDestroyWindow( dpy, TO_X11(this)->wnd );

    remove_window( TO_X11(this) );
    free( this );
}

/****************************************************************************/

void handle_window_events( sgui_window_xlib* this, XEvent* e )
{
    unsigned int i, num;
    sgui_window* super;
    XExposeEvent exp;
    sgui_event se;
    sgui_rect r;
    Status stat;
    KeySym sym;

    super = (sgui_window*)this;

    switch( e->type )
    {
    case KeyRelease:
        sym = XLookupKeysym( &e->xkey, 0 );
        se.keyboard_event.code = key_entries_translate( sym );

        SEND_EVENT( this, SGUI_KEY_RELEASED_EVENT, &se );
        break;
    case KeyPress:
        memset( se.char_event.as_utf8_str, 0,
                sizeof(se.char_event.as_utf8_str) );

        /* try to convert composed character to UTF8 string */
        Xutf8LookupString( this->ic, &e->xkey,
                           (char*)se.char_event.as_utf8_str,
                           sizeof(se.char_event.as_utf8_str),
                           &sym, &stat );

        /* send a char event if it worked */
        if( stat==XLookupChars || stat==XLookupBoth )
        {
            if( (se.char_event.as_utf8_str[0] & 0x80) ||
                !iscntrl( se.char_event.as_utf8_str[0] ) )
            {
                SEND_EVENT( this, SGUI_CHAR_EVENT, &se );
            }
        }

        /* send a key pressed event if we have a key sym */
        if( stat==XLookupKeySym || stat==XLookupBoth )
        {
            se.keyboard_event.code = key_entries_translate( sym );

            SEND_EVENT( this, SGUI_KEY_PRESSED_EVENT, &se );
        }
        break;
    case ButtonPress:
    case ButtonRelease:
        /* Button4 and Button5 are mouse wheel up and down */
        if( (e->xbutton.button==Button4||e->xbutton.button==Button5) &&
            e->type==ButtonPress )
        {
            se.mouse_wheel.direction = (e->xbutton.button==Button4)?1:-1;

            SEND_EVENT( this, SGUI_MOUSE_WHEEL_EVENT, &se );
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

            sgui_window_get_mouse_position( (sgui_window*)this,
                                            &se.mouse_press.x,
                                            &se.mouse_press.y );

            if( e->type==ButtonPress )
                SEND_EVENT( this, SGUI_MOUSE_PRESS_EVENT, &se );
            else
                SEND_EVENT( this, SGUI_MOUSE_RELEASE_EVENT, &se );
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
            se.mouse_move.x = e->xmotion.x<0 ? 0 : e->xmotion.x;
            se.mouse_move.y = e->xmotion.y<0 ? 0 : e->xmotion.y;
            SEND_EVENT( this, SGUI_MOUSE_MOVE_EVENT, &se );
        }
        break;
    case ConfigureNotify:
        se.size.new_width  = e->xconfigure.width;
        se.size.new_height = e->xconfigure.height;

        /* store the new position */
        super->x = e->xconfigure.x;
        super->y = e->xconfigure.y;

        /* do not accept zero size window */
        if( !se.size.new_width || !se.size.new_height )
            break;

        /* ignore if the size didn't change at all */
        if( se.size.new_width==super->w && se.size.new_height==super->h )
            break;

        /* store the new size */
        super->w = (unsigned int)e->xconfigure.width;
        super->h = (unsigned int)e->xconfigure.height;

        /* resize the back buffer image */
        sgui_canvas_resize( super->canvas, super->w, super->h );

        /* send a size change event */
        SEND_EVENT( this, SGUI_SIZE_CHANGE_EVENT, &se );

        /* redraw everything */
        sgui_canvas_draw_widgets( super->canvas, 1 );
        break;
    case DestroyNotify:
        super->visible = 0;
        this->wnd = 0;
        break;
    case MapNotify:
        if( super->backend==SGUI_OPENGL_CORE ||
            super->backend==SGUI_OPENGL_COMPAT )
        {
            sgui_rect_set_size( &se.expose_event, 0, 0,
                                super->w, super->h );
            SEND_EVENT( this, SGUI_EXPOSE_EVENT, &se );
        }
        break;
    case UnmapNotify:
        if( e->xunmap.window==this->wnd && this->is_child )
        {
            super->visible = 0;
            SEND_EVENT( this, SGUI_USER_CLOSED_EVENT, NULL );
        }
        break;
    case ClientMessage:
        if( e->xclient.data.l[0] == (long)atom_wm_delete )
        {
            super->visible = 0;
            XUnmapWindow( dpy, this->wnd );
            XUnmapSubwindows( dpy, this->wnd );
            SEND_EVENT( this, SGUI_USER_CLOSED_EVENT, NULL );
        }
        break;
    case Expose:
        if( super->canvas )
        {
            canvas_xlib_display( super->canvas,
                                 e->xexpose.x, e->xexpose.y,
                                 e->xexpose.width, e->xexpose.height );
        }

        if( super->backend==SGUI_OPENGL_CORE ||
            super->backend==SGUI_OPENGL_COMPAT )
        {
            sgui_rect_set_size( &se.expose_event, 0, 0,
                                super->w, super->h );
            SEND_EVENT( this, SGUI_EXPOSE_EVENT, &se );
        }
        break;
    };

    /* generate expose events for dirty rectangles */
    if( super->canvas )
    {
        num = sgui_canvas_num_dirty_rects( super->canvas );

        exp.type       = Expose;
        exp.serial     = 0;
        exp.send_event = 1;
        exp.display    = dpy;
        exp.window     = this->wnd;
        exp.count      = 0;

        for( i=0; i<num; ++i )
        {
            sgui_canvas_get_dirty_rect( super->canvas, &r, i );

            exp.x      = r.left;
            exp.y      = r.top;
            exp.width  = r.right  - r.left + 1;
            exp.height = r.bottom - r.top  + 1;

            XSendEvent( dpy, this->wnd, False, ExposureMask, (XEvent*)&exp );
        }

        sgui_canvas_redraw_widgets( super->canvas, 1 );
    }
}

/****************************************************************************/

sgui_window* sgui_window_create_desc( sgui_window_description* desc )
{
    sgui_window_xlib* this;
    sgui_window* super;
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

    add_window( this );

    /******************** create the window ********************/
    x_parent = desc->parent ? TO_X11(desc->parent)->wnd :
                              DefaultRootWindow(dpy);
    this->is_child = (desc->parent!=NULL);

    if( desc->backend==SGUI_OPENGL_CORE || desc->backend==SGUI_OPENGL_COMPAT )
    {
#ifndef SGUI_NO_OPENGL
        /* Get an fbc (optional), a visual and a Colormap */
        if( !get_fbc_visual_cmap( &fbc, &vi, &swa.colormap, desc ) )
        {
            xlib_window_destroy( super );
            return NULL;
        }

        /* create the window */
        swa.background_pixmap = None;
        swa.border_pixel      = 0;

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

        color |= ((unsigned long)rgb[0]) << 16;
        color |= ((unsigned long)rgb[1]) << 8;
        color |= ((unsigned long)rgb[2]);

        this->wnd = XCreateSimpleWindow( dpy, x_parent, 0, 0,
                                         desc->width, desc->height,
                                         0, 0, color );
    }

    if( !this->wnd )
    {
        xlib_window_destroy( super );
        return NULL;
    }

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
                                  PointerMotionMask |
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
        if( !create_context( fbc, desc->backend==SGUI_OPENGL_CORE, this ) )
        {
            xlib_window_destroy( super );
            return NULL;
        }

        super->canvas = NULL;
        super->swap_buffers = gl_swap_buffers;
#endif
    }
    else
    {
        super->canvas = canvas_xlib_create( this->wnd, attr.width,
                                            attr.height );

        if( !super->canvas )
        {
            xlib_window_destroy( super );
            return NULL;
        }
    }

    /*********** Create an input context ************/
    this->ic = XCreateIC( im, XNInputStyle,
                          XIMPreeditNothing|XIMStatusNothing, XNClientWindow,
                          this->wnd, XNFocusWindow, this->wnd, NULL );

    if( !this->ic )
    {
        xlib_window_destroy( super );
        return NULL;
    }

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
    super->destroy            = xlib_window_destroy;

    return (sgui_window*)this;
}

void sgui_window_make_current( sgui_window* this )
{
#ifdef SGUI_NO_OPENGL
    (void)this;
#else
    if( this && (this->backend==SGUI_OPENGL_CORE ||
                 this->backend==SGUI_OPENGL_COMPAT) )
    {
        glXMakeContextCurrent( dpy, TO_X11(this)->wnd, TO_X11(this)->wnd,
                               TO_X11(this)->gl );
    }
    else
    {
        glXMakeContextCurrent( dpy, 0, 0, 0 );
    }
#endif
}

void sgui_window_set_vsync( sgui_window* this, int vsync_on )
{
#ifdef SGUI_NO_OPENGL
    (void)this; (void)vsync_on;
#else
    if( this && (this->backend==SGUI_OPENGL_CORE ||
                 this->backend==SGUI_OPENGL_COMPAT) )
    {
        void(* SwapIntervalEXT )( Display*, GLXDrawable, int );

        SwapIntervalEXT = (void(*)(Display*,GLXDrawable,int))
                          LOAD_GLFUN( "glXSwapIntervalEXT" );

        if( SwapIntervalEXT )
            SwapIntervalEXT( dpy, TO_X11(this)->wnd, vsync_on ? 1 : 0 );
    }
#endif
}


void sgui_window_get_platform_data( sgui_window* this,
                                    void* window, void* context )
{
    Window* wnd_ptr;
#ifndef SGUI_NO_OPENGL
    GLXContext* glx_ptr;
#endif

    if( this )
    {
        if( window )
        {
            wnd_ptr = window;
            *wnd_ptr = TO_X11(this)->wnd;
        }

        if( context )
        {
        #ifndef SGUI_NO_OPENGL
            if( this->backend==SGUI_OPENGL_COMPAT ||
                this->backend==SGUI_OPENGL_CORE )
            {
                glx_ptr = context;
                *glx_ptr = TO_X11(this)->gl;
            }
        #endif
        }
    }
}

