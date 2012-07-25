#ifndef INTERNAL_H
#define INTERNAL_H



#include "sgui_window.h"
#include "sgui_colors.h"
#include "sgui_widget_manager.h"

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xft/Xft.h>

#define DPY_WIDTH DisplayWidth( wnd->dpy, DefaultScreen(wnd->dpy) )
#define DPY_HEIGHT DisplayHeight( wnd->dpy, DefaultScreen(wnd->dpy) )

#ifndef MIN
    #define MIN( a, b ) (((a)<(b)) ? (a) : (b))
#endif

#ifndef MAX
    #define MAX( a, b ) (((a)>(b)) ? (a) : (b))
#endif

#define XLIB_DRAW_COLOR( wnd, c ) XSetForeground( wnd->dpy, wnd->gc, c )

#define XLIB_DRAW_LINE( wnd, x0, y0, x1, y1 ) \
        XDrawLine( wnd->dpy, wnd->pixmap, wnd->gc, x0, y0, x1, y1 )

#define XLIB_DRAW_POINT( wnd, x, y ) \
        XDrawPoint( wnd->dpy, wnd->pixmap, wnd->gc, x, y )

#define XLIB_FILL_RECT( wnd, x, y, w, h ) \
        XFillRectangle( wnd->dpy, wnd->pixmap, wnd->gc, x, y, w, h )

#define SEND_EVENT( wnd, event, e )\
        if( wnd->event_fun )\
            wnd->event_fun( wnd, event, e );\
        sgui_widget_manager_send_event( wnd->mgr, wnd, event, e );



struct sgui_window
{
    Display* dpy;
    Window wnd;
    Atom wmDelete;
    GC gc;

    Pixmap pixmap;

    sgui_widget_manager* mgr;

    int x, y;
    unsigned int w, h;
    int resizeable;
    int mapped;

    sgui_window_callback event_fun;
};

struct sgui_pixmap
{
    XImage image;
};



#endif /* INTERNAL_H */

