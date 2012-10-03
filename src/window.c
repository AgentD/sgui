#include "sgui_internal.h"
#include "sgui_window.h"

#include <stddef.h>



int sgui_internal_window_init( sgui_window* wnd )
{
    wnd->mgr = sgui_widget_manager_create( );

    if( !wnd->mgr )
        return 0;

    wnd->event_fun = NULL;
    wnd->back_buffer = NULL;
    wnd->visible = 0;

    return 1;
}

void sgui_internal_window_deinit( sgui_window* wnd )
{
    sgui_widget_manager_destroy( wnd->mgr );
}

void sgui_internal_window_fire_event( sgui_window* wnd, int event,
                                      sgui_event* e )
{
    if( wnd->event_fun )
        wnd->event_fun( wnd, event, e );

    sgui_widget_manager_send_window_event( wnd->mgr, event, e );
}

/****************************************************************************/

int sgui_window_is_visible( sgui_window* wnd )
{
    return wnd ? wnd->visible : 0;
}

void sgui_window_get_position( sgui_window* wnd, int* x, int* y )
{
    if( wnd )
    {
        if( x ) *x = wnd->x;
        if( y ) *y = wnd->y;
    }
    else
    {
        if( x ) *x = 0;
        if( y ) *y = 0;
    }
}

void sgui_window_get_size( sgui_window* wnd, unsigned int* width,
                           unsigned int* height )
{
    if( wnd )
    {
        if( width  ) *width  = wnd->w;
        if( height ) *height = wnd->h;
    }
    else
    {
        if( width  ) *width  = 0;
        if( height ) *height = 0;
    }
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

void sgui_window_on_widget_event( sgui_window* wnd,
                                  sgui_widget_callback fun, void* user )
{
    if( wnd )
        sgui_widget_manager_on_event( wnd->mgr, fun, user );
}

void sgui_window_on_event( sgui_window* wnd, sgui_window_callback fun )
{
    if( wnd )
        wnd->event_fun = fun;
}

sgui_canvas* sgui_window_get_canvas( sgui_window* wnd )
{
    return wnd ? wnd->back_buffer : NULL;
}

/****************************************************************************/

void sgui_window_get_mouse_position( sgui_window* wnd, int* x, int* y )
{
    int X = 0, Y = 0;

    if( wnd )
        wnd->get_mouse_position( wnd, &X, &Y );

    if( x ) *x = X<0 ? 0 : (X>=(int)wnd->w ? ((int)wnd->w-1) : X);
    if( y ) *y = Y<0 ? 0 : (Y>=(int)wnd->h ? ((int)wnd->h-1) : Y);
}

void sgui_window_set_mouse_position( sgui_window* wnd, int x, int y,
                                     int send_event )
{
    sgui_event e;

    if( wnd && wnd->visible )
    {
        x = x<0 ? 0 : (x>=(int)wnd->w ? ((int)wnd->w-1) : x);
        y = y<0 ? 0 : (y>=(int)wnd->h ? ((int)wnd->h-1) : y);

        wnd->set_mouse_position( wnd, x, y );

        if( send_event )
        {
            e.mouse_move.x = x;
            e.mouse_move.y = y;
            sgui_internal_window_fire_event( wnd, SGUI_MOUSE_MOVE_EVENT, &e );
        }
    }
}

void sgui_window_set_visible( sgui_window* wnd, int visible )
{
    if( wnd && (wnd->visible!=visible) )
    {
        wnd->set_visible( wnd, visible );

        wnd->visible = visible;

        if( !visible )
            sgui_internal_window_fire_event( wnd, SGUI_API_INVISIBLE_EVENT,
                                             NULL );
    }
}

void sgui_window_set_title( sgui_window* wnd, const char* title )
{
    if( wnd )
        wnd->set_title( wnd, title );
}

void sgui_window_set_size( sgui_window* wnd,
                           unsigned int width, unsigned int height )
{
    if( !wnd || !width || !height )
        return;

    wnd->set_size( wnd, width, height );

    sgui_canvas_clear( wnd->back_buffer, NULL );

    sgui_widget_manager_draw_all( wnd->mgr, wnd->back_buffer );
}

void sgui_window_move_center( sgui_window* wnd )
{
    if( wnd )
        wnd->move_center( wnd );
}

void sgui_window_move( sgui_window* wnd, int x, int y )
{
    if( wnd )
    {
        wnd->move( wnd, x, y );
        wnd->x = x;
        wnd->y = y;
    }
}

