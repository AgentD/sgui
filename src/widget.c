#include "sgui_widget.h"
#include "widget_internal.h"



void sgui_widget_set_position( sgui_widget* w, int x, int y )
{
    if( w )
    {
        w->x = x;
        w->y = y;
    }
}

void sgui_widget_get_position( sgui_widget* w, int* x, int* y )
{
    if( w )
    {
        if( x ) *x = w->x;
        if( y ) *y = w->y;
    }
}

int sgui_widget_intersects_area( sgui_widget* w, int x, int y,
                                 unsigned int width, unsigned int height )
{
    if( !w )
        return 0;

    if( (w->x >= (x+(int)width)) || (w->y >= (y+(int)height)) )
        return 0;

    if( ((w->x + (int)w->width) < x) || ((w->y + (int)w->height) < y) )
        return 0;

    return 1;
}

void sgui_widget_draw( sgui_widget* widget, sgui_window* wnd, int x, int y,
                       unsigned int w, unsigned int h )
{
    if( widget && wnd && widget->draw_callback )
    {
        widget->draw_callback( widget, wnd, x, y, w, h );
    }
}

