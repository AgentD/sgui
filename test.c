#include "sgui_window.h"
#include "sgui_colors.h"
#include "sgui_progress_bar.h"

#include <stdio.h>



sgui_window *a, *b;
sgui_widget* prog_bar;



void window_callback( sgui_window* wnd, int type, sgui_event* e )
{
    const char* w = (wnd==a) ? "A" : "B";

    switch( type )
    {
    case SGUI_USER_CLOSED_EVENT:
        printf( "Window %s: got closed by the user\n", w );
        break;
    case SGUI_API_INVISIBLE_EVENT:
        printf( "Window %s: made invisible\n", w );
        break;
    case SGUI_API_DESTROY_EVENT:
        printf( "Window %s: got closed by the api\n", w );
        break;
    case SGUI_SIZE_CHANGE_EVENT:
        printf( "Window %s: size changed to %d x %d\n", w,
                                                        e->size.new_width,
                                                        e->size.new_height );
        break;
    case SGUI_MOUSE_MOVE_EVENT:
        printf( "Window %s: mouse moved to (%d,%d)\n", w, e->mouse_move.x,
                                                          e->mouse_move.y );
        break;
    case SGUI_MOUSE_PRESS_EVENT:
        printf( "Window %s: %s mouse button %s\n", w,
                e->mouse_press.button==SGUI_MOUSE_BUTTON_LEFT  ? "left" :
                e->mouse_press.button==SGUI_MOUSE_BUTTON_RIGHT ? "right":
                                                                 "middle",
                e->mouse_press.pressed ? "pressed" : "released" );
        break;
    case SGUI_MOUSE_WHEEL_EVENT:
        printf( "Window %s: mouse wheel moved %s\n", w,
                e->mouse_wheel.direction>0 ? "up" : "down" );
        break;
    case SGUI_DRAW_EVENT:
        if( wnd==a )
        {
            sgui_widget_draw( prog_bar, wnd, e->draw.x, e->draw.y,
                                             e->draw.w, e->draw.h );
        }
    };
}



int main( void )
{
    int a_active=1, b_active=1;

    a = sgui_window_create( 400, 300, SGUI_RESIZEABLE );
    b = sgui_window_create( 100, 100, SGUI_FIXED_SIZE );

    sgui_window_set_visible( a, SGUI_VISIBLE );
    sgui_window_set_visible( b, SGUI_VISIBLE );

    sgui_window_set_title( a, "A" );
    sgui_window_set_title( b, "B" );

    sgui_window_move( a, 200, 200 );
    sgui_window_move_center( b );

    sgui_window_on_event( a, window_callback );
    sgui_window_on_event( b, window_callback );

    sgui_window_set_size( a, 800, 600 );
    sgui_window_set_size( b, 200, 100 );

    /* widget test */
    prog_bar = sgui_progress_bar_create( 10, 10, 300, 30, 0.5f );

    sgui_progress_bar_set_progress( prog_bar, 0.3f );

    while( a_active || b_active )
    {
        if( a_active && !sgui_window_update( a ) )
        {
            a_active = 0;
            sgui_window_destroy( a );
        }

        if( b_active && !sgui_window_update( b ) )
        {
            b_active = 0;
            sgui_window_destroy( b );
        }
    }

    sgui_progress_bar_delete( prog_bar );

    return 0;
}

