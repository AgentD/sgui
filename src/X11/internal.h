#ifndef INTERNAL_H
#define INTERNAL_H



#include "sgui_window.h"
#include "sgui_colors.h"

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>



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
        XDrawLine( wnd->dpy, wnd->wnd, wnd->gc, x0, y0, x1, y1 )

#define XLIB_DRAW_POINT( wnd, x, y ) \
        XDrawPoint( wnd->dpy, wnd->wnd, wnd->gc, x, y )

#define XLIB_FILL_RECT( wnd, x, y, w, h ) \
        XFillRectangle( wnd->dpy, wnd->wnd, wnd->gc, x, y, w, h )



struct sgui_window
{
    Display* dpy;
    Window wnd;
    Atom wmDelete;
    GC gc;

    int x, y;
    unsigned int w, h;
    int resizeable;
    int mapped;

    sgui_window_callback event_fun;
};



#endif /* INTERNAL_H */

