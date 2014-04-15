#include "sgui.h"

#include <stdio.h>



void read_clipboard( sgui_window* wnd )
{
    const char* text = sgui_window_read_clipboard( wnd );

    if( text )
        puts( text );
}

void write_clipboard( sgui_window* wnd, sgui_widget* editbox )
{
    sgui_window_write_clipboard( wnd, sgui_edit_box_get_text( editbox ), -1 );
}

int main( void )
{
    sgui_window* wnd;
    sgui_widget* rb;
    sgui_widget* wb;
    sgui_widget* eb;

    sgui_init( );

    /* create a window */
    wnd = sgui_window_create( NULL, 320, 150, SGUI_RESIZEABLE );

    sgui_window_set_title( wnd, "Clipboard" );
    sgui_window_move_center( wnd );
    sgui_window_set_visible( wnd, SGUI_VISIBLE );

    /* create some widgets */
    rb = sgui_button_create( 30, 30, 120, 30, "Read Clipboard", 0 );
    wb = sgui_button_create( 30, 70, 120, 30, "Write Clipboard", 0 );
    eb = sgui_edit_box_create( 160, 70, 120, 128, 0 );

    /* add widgets to the window */
    sgui_window_add_widget( wnd, rb );
    sgui_window_add_widget( wnd, wb );
    sgui_window_add_widget( wnd, eb );

    /* hook event callbacks */
    sgui_event_connect( rb, SGUI_BUTTON_OUT_EVENT, 1,
                        read_clipboard, wnd, SGUI_VOID );

    sgui_event_connect( wb, SGUI_BUTTON_OUT_EVENT, 1,
                        write_clipboard, wnd, SGUI_POINTER, eb );

    /* main loop */
    sgui_main_loop( );

    /* clean up */
    sgui_window_destroy( wnd );
    sgui_widget_destroy( rb );
    sgui_widget_destroy( wb );
    sgui_widget_destroy( eb );
    sgui_deinit( );

    return 0;
}

