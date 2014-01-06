#include "sgui.h"

#include <stdio.h>



void window_callback( sgui_window* wnd, int type, sgui_event* event )
{
    (void)event;

    if( type == SGUI_CHAR_EVENT )
    {
        printf( "%s\n", event->char_event.as_utf8_str );
    }
    else if( type == SGUI_BUTTON_CLICK_EVENT )
    {
        sgui_window_set_visible( wnd, SGUI_INVISIBLE );
    }
}



int main( )
{
    sgui_window* wnd;
    sgui_widget* text;
    sgui_widget* button;
    sgui_font* font;

    sgui_init( );

    /* Load a fon for text drawing */
    font = sgui_font_load( "../font/SourceSansPro-Regular.ttf", 16 );

    sgui_skin_set_default_font( font, NULL, NULL, NULL );

    /* create a window */
    wnd = sgui_window_create( NULL, 400, 300, SGUI_RESIZEABLE );

    sgui_window_set_title( wnd, "Simple Sample" );
    sgui_window_move_center( wnd );
    sgui_window_set_visible( wnd, SGUI_VISIBLE );

    /* create some widgets */
    text = sgui_static_text_create( 10, 10, "To close the window,\n"
                                            "press <color=\"#FF0000\">close");

    button = sgui_button_create( 30, 60, 75, 30, "Close" );

    /* add widgets to the window */
    sgui_window_add_widget( wnd, text );
    sgui_window_add_widget( wnd, button );

    /* hook event callbacks */
    sgui_window_on_event( wnd, window_callback );

    /* main loop */
    sgui_main_loop( );

    /* clean up */
    sgui_window_destroy( wnd );
    sgui_widget_destroy( text );
    sgui_widget_destroy( button );
    sgui_font_destroy( font );
    sgui_deinit( );

    return 0;
}

