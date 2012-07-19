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

void sgui_widget_get_size( sgui_widget* w,
                           unsigned int* width, unsigned int* height )
{
    if( w )
    {
        if( width  ) *width  = w->width;
        if( height ) *height = w->height;
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

int sgui_widget_is_point_inside( sgui_widget* w, int x, int y )
{
    if( !w )
        return 0;

    if( (x < w->x) || (y < w->y) )
        return 0;

    if( (x >= (w->x + (int)w->width)) || (y >= (w->y + (int)w->height)) )
        return 0;

    return 1;
}

void sgui_widget_update( sgui_widget* widget )
{
    if( widget && widget->update_callback )
        widget->update_callback( widget );
}

void sgui_widget_send_window_event( sgui_widget* widget, sgui_window* wnd,
                                    int type, sgui_event* event )
{
    if( widget && widget->window_event_callback )
        widget->window_event_callback( widget, wnd, type, event );
}

int sgui_widget_need_redraw( sgui_widget* widget )
{
    int result = 0;

    if( widget )
    {
        result = widget->need_redraw;

        widget->need_redraw = 0;
    }

    return result;
}

