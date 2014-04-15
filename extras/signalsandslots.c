#include "sgui.h"

#include <stddef.h>



int main( void )
{
    unsigned char image[ 64*64*3 ], *ptr;
    sgui_widget *b1, *b2, *b3, *b4, *img;
    sgui_window* wnd;
    int i, j;

    /* initialize and create window */
    sgui_init( );

    wnd = sgui_window_create( NULL, 200, 130, SGUI_FIXED_SIZE );
    sgui_window_set_visible( wnd, SGUI_VISIBLE );
    sgui_window_set_title( wnd, "Signals & Slots" );

    /* create widgets */
    b1 = sgui_button_create( 10, 10, 80, 30, "Show", 0 );
    b2 = sgui_button_create( 10, 45, 80, 30, "Hide", 0 );
    b3 = sgui_button_create( 10, 80, 80, 30, "Move", 0 );
    b4 = sgui_button_create( 95, 80, 80, 30, "Close", 0 );

    for( ptr=image, j=0; j<64; ++j )
    {
        for( i=0; i<64; ++i )
        {
            *(ptr++) = (j % (i+1))<<2;
            *(ptr++) = 0x00;
            *(ptr++) = ((j-i) % (i+1))<<2;
        }
    }

    img = sgui_image_create( 100, 10, 64, 64, image, SGUI_RGB8, 0, 1 );

    /* add widgets to window */
    sgui_window_add_widget( wnd, b1 );
    sgui_window_add_widget( wnd, b2 );
    sgui_window_add_widget( wnd, b3 );
    sgui_window_add_widget( wnd, b4 );
    sgui_window_add_widget( wnd, img );

    /* make some connections */
    sgui_event_connect( b1, SGUI_BUTTON_OUT_EVENT, 1,
                        sgui_widget_set_visible, img, SGUI_INT, 1 );

    sgui_event_connect( b2, SGUI_BUTTON_OUT_EVENT, 1,
                        sgui_widget_set_visible, img, SGUI_INT, 0 );

    sgui_event_connect( b3, SGUI_BUTTON_OUT_EVENT, 1,
                        sgui_window_move, wnd, SGUI_INT2, 50, 100 );

    sgui_event_connect( b4, SGUI_BUTTON_OUT_EVENT, 1,
                        sgui_window_set_visible, wnd, SGUI_INT, 0 );

    /* enter main loop */
    sgui_main_loop( );

    /* clean up */
    sgui_window_destroy( wnd );
    sgui_widget_destroy( b1 );
    sgui_widget_destroy( b2 );
    sgui_widget_destroy( b3 );
    sgui_widget_destroy( b4 );
    sgui_widget_destroy( img );
    sgui_deinit( );

    return 0;
}

