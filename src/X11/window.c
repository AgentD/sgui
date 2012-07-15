#include "internal.h"

#include <stdlib.h>
#include <string.h>



const char* wmDeleteWindow = "WM_DELETE_WINDOW";



sgui_window* sgui_window_create( unsigned int width, unsigned int height,
                                 int resizeable )
{
    sgui_window* wnd;
    XSizeHints* hints;
    XWindowAttributes attr;

    if( !width || !height )
        return NULL;

    /* try to allocate space for the window structure */
    wnd = malloc( sizeof(sgui_window) );

    if( !wnd )
        return NULL;

    memset( wnd, 0, sizeof(sgui_window) );

    /* try to connect to the X server */
    wnd->dpy = XOpenDisplay( 0 );

    if( !wnd->dpy )
    {
        sgui_window_destroy( wnd );
        return NULL;
    }

    /* try to create the window */
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
        hints = XAllocSizeHints( );

        if( !hints )
        {
            sgui_window_destroy( wnd );
            return NULL;
        }

        hints->flags       = PSize | PMinSize | PMaxSize;
        hints->base_width  = (int)width;
        hints->base_height = (int)height;
        hints->min_width   = hints->max_width  = hints->base_width;
        hints->min_height  = hints->max_height = hints->base_height;

        XSetWMNormalHints( wnd->dpy, wnd->wnd, hints );
        XFree( hints );
    }

    /* tell X11 what events we will handle */
    XSelectInput( wnd->dpy, wnd->wnd, ExposureMask |
                                      StructureNotifyMask |
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

    /* get a GC */
    wnd->gc = XCreateGC( wnd->dpy, wnd->wnd, 0, 0 );

    /* store the remaining information */
    wnd->resizeable = resizeable;
    wnd->mapped = 0;
    wnd->event_fun = NULL;

    return wnd;
}

void sgui_window_destroy( sgui_window* wnd )
{
    if( wnd )
    {
        if( wnd->event_fun )
            wnd->event_fun( wnd, SGUI_API_DESTROY_EVENT, NULL );

        if( wnd->gc  ) XFreeGC( wnd->dpy, wnd->gc );
        if( wnd->wnd ) XDestroyWindow( wnd->dpy, wnd->wnd );
        if( wnd->dpy ) XCloseDisplay( wnd->dpy );
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

            if( wnd->event_fun )
                wnd->event_fun( wnd, SGUI_API_INVISIBLE_EVENT, NULL );
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
    XSizeHints* hints;
    XWindowAttributes attr;

    if( !wnd || !width || !height )
        return;

    /* Make the window resizeable */
    if( !wnd->resizeable )
    {
        hints = XAllocSizeHints( );

        if( !hints )
            return;

        hints->flags       = PSize | PMinSize | PMaxSize;
        hints->min_width   = (int)MIN( wnd->w, width );
        hints->min_height  = (int)MIN( wnd->h, height );
        hints->base_width  = (int)width;
        hints->base_height = (int)height;
        hints->max_width   = (int)MAX( wnd->w, width );
        hints->max_height  = (int)MAX( wnd->h, height );

        XSetWMNormalHints( wnd->dpy, wnd->wnd, hints );
    }

    /* resize the window */
    XResizeWindow( wnd->dpy, wnd->wnd, width, height );
    wnd->w = width;
    wnd->h = height;

    /* get the real geometry as the window manager is free to change it */
    XGetWindowAttributes( wnd->dpy, wnd->wnd, &attr );
    width  = wnd->w = (unsigned int)attr.width;
    height = wnd->h = (unsigned int)attr.height;

    /* make the window non resizeable */
    if( !wnd->resizeable )
    {
        hints->max_width  = (int)width;
        hints->max_height = (int)height;
        hints->min_width  = hints->base_width  = hints->max_width;
        hints->min_height = hints->base_height = hints->max_height;

        XSetWMNormalHints( wnd->dpy, wnd->wnd, hints );

        XFree( hints );
    }

    XFlush( wnd->dpy );
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
    int st;

    if( !wnd || !wnd->mapped )
        return 0;

    while( XPending( wnd->dpy )>0 )
    {
        XNextEvent( wnd->dpy, &e );
        st = -1;

        switch( e.type )
        {
        case ButtonPress:
        case ButtonRelease:
            if( (e.xbutton.button==Button4||e.xbutton.button==Button5) &&
                e.type==ButtonPress )
            {
                se.mouse_wheel.direction = (e.xbutton.button==Button4)?1:-1;
                st = SGUI_MOUSE_WHEEL_EVENT;
            }
            else
            {
                se.mouse_press.pressed = (e.type==ButtonPress);
                st = SGUI_MOUSE_PRESS_EVENT;

                if( e.xbutton.button == Button1 )
                    se.mouse_press.button = SGUI_MOUSE_BUTTON_LEFT;
                else if( e.xbutton.button == Button2 )
                    se.mouse_press.button = SGUI_MOUSE_BUTTON_MIDDLE;
                else if( e.xbutton.button == Button3 )
                    se.mouse_press.button = SGUI_MOUSE_BUTTON_RIGHT;
                else
                    st = -1;
            }
            break;
        case MotionNotify:
            se.mouse_move.x = e.xmotion.x<0 ? 0 : e.xmotion.x;
            se.mouse_move.y = e.xmotion.y<0 ? 0 : e.xmotion.y;
            st = SGUI_MOUSE_MOVE_EVENT;
            break;
        case ConfigureNotify:
            if( ((int)wnd->w)!=e.xconfigure.width ||
                ((int)wnd->h)!=e.xconfigure.height )
            {
                se.size.new_width  = wnd->w;
                se.size.new_height = wnd->h;
                st = SGUI_SIZE_CHANGE_EVENT;
            }

            wnd->x = e.xconfigure.x;
            wnd->y = e.xconfigure.y;
            wnd->w = (unsigned int)e.xconfigure.width;
            wnd->h = (unsigned int)e.xconfigure.height;
            break;
        case ClientMessage:
            atom = XGetAtomName( wnd->dpy, e.xclient.message_type );

            if( *atom == *wmDeleteWindow )
            {
                wnd->mapped = 0;
                st = SGUI_USER_CLOSED_EVENT;
            }

            XFree( atom );
            break;
        case Expose:
            st = SGUI_DRAW_EVENT;
            se.draw.x = e.xexpose.x     <0 ? 0 : e.xexpose.x;
            se.draw.y = e.xexpose.y     <0 ? 0 : e.xexpose.y;
            se.draw.w = e.xexpose.width <0 ? 0 : (unsigned)e.xexpose.width;
            se.draw.h = e.xexpose.height<0 ? 0 : (unsigned)e.xexpose.height;
            break;
        };

        if( wnd->event_fun && (st >= 0) )
            wnd->event_fun( wnd, st, &se );
    }

    return wnd->mapped;
}

void sgui_window_on_event( sgui_window* wnd, sgui_window_callback fun )
{
    if( wnd )
        wnd->event_fun = fun;
}

void sgui_window_force_redraw( sgui_window* wnd, int x, int y,
                               unsigned int width, unsigned int height )
{
    XExposeEvent ev;

    if( wnd )
    {
        ev.type       = Expose;
        ev.serial     = 0;
        ev.send_event = 1;
        ev.display    = wnd->dpy;
        ev.window     = wnd->wnd;
        ev.x          = x;
        ev.y          = y;
        ev.width      = (int)width;
        ev.height     = (int)height;
        ev.count      = 0;

        XSendEvent( wnd->dpy, wnd->wnd, False, ExposureMask, (XEvent*)&ev );
    }
}






void sgui_window_draw_box( sgui_window* wnd, int x, int y,
                           unsigned int width, unsigned int height,
                           unsigned long bgcolor, int inset )
{
    int lr_x = x + (int)width -1;     /* lower right x and y */
    int lr_y = y + (int)height-1;

    XLIB_DRAW_COLOR( wnd, bgcolor );
    XLIB_FILL_RECT( wnd, x, y, width, height );

    if( inset>0 )
    {
        XLIB_DRAW_COLOR( wnd, SGUI_INSET_COLOR );
        XLIB_DRAW_LINE( wnd, x, y, lr_x, y    );
        XLIB_DRAW_LINE( wnd, x, y, x,    lr_y );

        XLIB_DRAW_COLOR( wnd, SGUI_OUTSET_COLOR );
        XLIB_DRAW_LINE( wnd, lr_x, y,    lr_x, lr_y );
        XLIB_DRAW_LINE( wnd, x,    lr_y, lr_x, lr_y );
    }
    else if( inset<0 )
    {
        XLIB_DRAW_COLOR( wnd, SGUI_OUTSET_COLOR );
        XLIB_DRAW_LINE( wnd, x, y, lr_x, y    );
        XLIB_DRAW_LINE( wnd, x, y, x,    lr_y );

        XLIB_DRAW_COLOR( wnd, SGUI_INSET_COLOR );
        XLIB_DRAW_LINE( wnd, lr_x, y,    lr_x, lr_y );
        XLIB_DRAW_LINE( wnd, x,    lr_y, lr_x, lr_y );
    }
}

void sgui_window_draw_fancy_lines( sgui_window* wnd, int x, int y,
                                   int* length, unsigned int num_lines,
                                   int start_horizontal )
{
    unsigned int i;
    int h, oldx = x, oldy = y;

    XLIB_DRAW_COLOR( wnd, SGUI_OUTSET_COLOR );

    for( h=start_horizontal, i=0; i<num_lines; ++i, h=!h )
    {
        if( h )
        {
            XLIB_DRAW_LINE( wnd, x, y+1, x+length[i]+1, y+1 );
            x += length[i];
        }
        else
        {
            XLIB_DRAW_LINE( wnd, x+1, y, x+1, y+length[i]+1 );
            y += length[i];
        }
    }

    XLIB_DRAW_COLOR( wnd, SGUI_INSET_COLOR );

    for( x=oldx, y=oldy, h=start_horizontal, i=0; i<num_lines; ++i, h=!h )
    {
        if( h )
        {
            XLIB_DRAW_LINE( wnd, x, y, x+length[i], y );
            x += length[i];
        }
        else
        {
            XLIB_DRAW_LINE( wnd, x, y, x, y+length[i] );
            y += length[i];
        }
    }
}

void sgui_window_draw_radio_button( sgui_window* wnd, int x, int y,
                                    int selected )
{
    XLIB_DRAW_COLOR( wnd, SGUI_INSET_COLOR );
    XLIB_DRAW_LINE( wnd, x+4, y,   x+7, y   );
    XLIB_DRAW_LINE( wnd, x+2, y+1, x+3, y+1 );
    XLIB_DRAW_LINE( wnd, x+8, y+1, x+9, y+1 );

    XLIB_DRAW_LINE( wnd, x+1, y+2, x+1, y+3 );
    XLIB_DRAW_LINE( wnd, x,   y+4, x,   y+7 );
    XLIB_DRAW_LINE( wnd, x+1, y+8, x+1, y+9 );

    XLIB_DRAW_COLOR( wnd, SGUI_OUTSET_COLOR );
    XLIB_DRAW_LINE( wnd, x+2, y+10, x+3, y+10 );
    XLIB_DRAW_LINE( wnd, x+4, y+11, x+7, y+11 );
    XLIB_DRAW_LINE( wnd, x+8, y+10, x+9, y+10 );

    XLIB_DRAW_LINE( wnd, x+10, y+9, x+10, y+8 );
    XLIB_DRAW_LINE( wnd, x+11, y+7, x+11, y+4 );
    XLIB_DRAW_LINE( wnd, x+10, y+3, x+10, y+2 );

    XLIB_DRAW_COLOR( wnd, SGUI_INSET_FILL_COLOR_L1 );
    XLIB_FILL_RECT( wnd, x+2, y+2,  8,  8 );
    XLIB_FILL_RECT( wnd, x+1, y+4, 10,  4 );
    XLIB_FILL_RECT( wnd, x+4, y+1,  4, 10 );

    if( selected )
    {
        XLIB_DRAW_COLOR( wnd, SGUI_RADIO_BUTTON_COLOR );
        XLIB_FILL_RECT( wnd, x+3, y+4, 6, 4 );
        XLIB_FILL_RECT( wnd, x+4, y+3, 4, 6 );
    }
}

void sgui_window_draw_checkbox( sgui_window* wnd, int x, int y,
                                int selected )
{
    sgui_window_draw_box( wnd, x, y, 12, 12, SGUI_INSET_FILL_COLOR_L1, 1 );

    if( selected )
    {
        XLIB_DRAW_COLOR( wnd, SGUI_CHECKBOX_TICK_COLOR );

        XLIB_DRAW_LINE( wnd, x+2, y+4, x+2, y+6 );
        XLIB_DRAW_LINE( wnd, x+3, y+5, x+3, y+7 );
        XLIB_DRAW_LINE( wnd, x+4, y+6, x+4, y+8 );
        XLIB_DRAW_LINE( wnd, x+5, y+5, x+5, y+7 );
        XLIB_DRAW_LINE( wnd, x+6, y+4, x+6, y+6 );
        XLIB_DRAW_LINE( wnd, x+7, y+3, x+7, y+5 );
        XLIB_DRAW_LINE( wnd, x+8, y+2, x+8, y+4 );
    }
}

