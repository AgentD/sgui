#include "sgui.h"

#include <stdio.h>



sgui_window *a, *b;
sgui_widget *p0, *p1, *p2, *p3;



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

        if( wnd == a )
        {
            sgui_progress_bar_set_progress( p0, e->mouse_move.x/800.0f );
            sgui_progress_bar_set_progress( p1, e->mouse_move.x/800.0f );

            sgui_progress_bar_set_progress( p2, 1.0f-e->mouse_move.y/600.0f );
            sgui_progress_bar_set_progress( p3, 1.0f-e->mouse_move.y/600.0f );
        }
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
        break;
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
    p0 = sgui_progress_bar_create( 10, 10, 300, 30, 0.5f );
    p1 = sgui_progress_bar_create( 10, 45, 300, 30, 0.5f );

    p2 = sgui_progress_bar_create( 320, 10, 30, 300, 0.5f );
    p3 = sgui_progress_bar_create( 355, 10, 30, 300, 0.5f );

    sgui_progress_bar_set_style( p0, 0, 1 );
    sgui_progress_bar_set_style( p1, 1, 0 );
    sgui_progress_bar_set_style( p2, 0, 1 );
    sgui_progress_bar_set_style( p3, 1, 0 );

    sgui_progress_bar_set_color( p1, SGUI_YELLOW );
    sgui_progress_bar_set_color( p3, SGUI_YELLOW );

    sgui_progress_bar_set_direction( p2, SGUI_PROGRESS_BAR_VERTICAL );
    sgui_progress_bar_set_direction( p3, SGUI_PROGRESS_BAR_VERTICAL );


    sgui_window_add_widget( a, p0 );
    sgui_window_add_widget( a, p1 );
    sgui_window_add_widget( a, p2 );
    sgui_window_add_widget( a, p3 );

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

    sgui_progress_bar_delete( p0 );
    sgui_progress_bar_delete( p1 );
    sgui_progress_bar_delete( p2 );
    sgui_progress_bar_delete( p3 );

    return 0;
}

