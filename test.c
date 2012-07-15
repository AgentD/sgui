#include "sgui_window.h"
#include "sgui_colors.h"

#include <stdio.h>



sgui_window *a, *b;



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
            static int length[] = { 150, 150, -150, -150 };

            sgui_window_draw_box( wnd, 10, 10, 400, 300, SGUI_INSET_FILL_COLOR_L1, 1 );
            sgui_window_draw_box( wnd, 50, 50, 150, 150, SGUI_INSET_FILL_COLOR_L2, 1 );

            sgui_window_draw_box( wnd, 10, 320, 80, 25, SGUI_WINDOW_COLOR, -1 );


            sgui_window_draw_fancy_lines( wnd, 420, 30, length, 4, 1 );

            sgui_window_draw_radio_button( wnd, 440, 50, 0 );
            sgui_window_draw_radio_button( wnd, 440, 65, 0 );
            sgui_window_draw_radio_button( wnd, 440, 80, 1 );

            sgui_window_draw_checkbox( wnd, 440, 100, 0 );
            sgui_window_draw_checkbox( wnd, 440, 115, 0 );
            sgui_window_draw_checkbox( wnd, 440, 130, 1 );

            sgui_window_draw_box( wnd, 10, 380, 300, 30, SGUI_INSET_FILL_COLOR_L1, 1 );

            sgui_window_draw_box( wnd, 15, 385, 100, 20, SGUI_OUTSET_COLOR, 0 );
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

    return 0;
}

