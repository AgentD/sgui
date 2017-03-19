/*
    This file is part of the sgui samples collection. I, David Oberhollenzer,
    author of this file hereby place the contents of this file into
    the public domain.
 */
/*
    This small programm is supposed to demonstrate how to use the sgui
    event system.
 */
#include "sgui.h"

#include <stddef.h>
#include <stdio.h>



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
    b1 = sgui_button_create( 10, 10, 80, 30, "Show", SGUI_BUTTON );
    b2 = sgui_button_create( 10, 45, 80, 30, "Hide", SGUI_BUTTON );
    b3 = sgui_button_create( 10, 80, 80, 30, "Move", SGUI_BUTTON );
    b4 = sgui_button_create( 95, 80, 80, 30, "Close", SGUI_BUTTON );

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

    /*
        when the object "b1" triggers an event of type SGUI_BUTTON_OUT_EVENT,
        the function "sgui_widget_set_visible" is called on the object "img",
        i.e. the pointer "img" is passed as first argument to the callback.
        The second argument to the callback is of type int (SGUI_INT) and set
        to value 1

        Simply put, when b1 sends an SGUI_BUTTON_OUT_EVENT event,
        the event system calls "sgui_widget_set_visible( img, 1 )"
     */
    sgui_event_connect( b1, SGUI_BUTTON_OUT_EVENT,
                        sgui_widget_set_visible, img, SGUI_INT, 1 );

    sgui_event_connect( b2, SGUI_BUTTON_OUT_EVENT,
                        sgui_widget_set_visible, img, SGUI_INT, 0 );

    /*
        Similar to the two above, but with argument type SGUI_INT2, which is
        decomposed to two integer arguments.

        Simply put, when b3 sends an SGUI_BUTTON_OUT_EVENT event,
        the event system calls "sgui_widget_move( wnd, 50, 100 )"
     */
    sgui_event_connect( b3, SGUI_BUTTON_OUT_EVENT,
                        sgui_window_move, wnd, SGUI_INT2, 50, 100 );

    sgui_event_connect( b4, SGUI_BUTTON_OUT_EVENT,
                        sgui_window_set_visible, wnd, SGUI_INT, 0 );

    /*
        When b4 triggers an SGUI_BUTTON_OUT_EVENT, call the function "puts".

        HACK: The object pointer passed as first argument to puts is a pointer
        to a string.

        The value SGUI_VOID indicates that there is no further argument to the
        callback function.
     */
    sgui_event_connect( b4, SGUI_BUTTON_OUT_EVENT,
                        puts, "Hello World!", SGUI_VOID );

    /*
        Whenever the object "wnd", which is our window, triggers an event
        of type SGUI_CHAR_EVENT, call the printf function.

        HACK: The object pointer, our first argument to printf is a pointer
        to a format string.

        The value SGUI_FROM_EVENT tells the event system that the second
        argument for printf is from the event structure received from "wnd".

        More precisely, SGUI_UTF8 tells the system to use the utf8 argument
        from the event structure.

        Simply put, when "wnd" sends an SGUI_CHAR_EVENT, the event system
        calls printf( "%s\n", event->arg.utf8 );
     */
    sgui_event_connect( wnd, SGUI_CHAR_EVENT,
                        printf, "%s\n", SGUI_FROM_EVENT, SGUI_UTF8 );

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

