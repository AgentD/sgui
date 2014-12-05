/*
    This file is part of the sgui samples collection. I, David Oberhollenzer,
    author of this file hereby place the contents of this file into
    the public domain.
 */
/*
    This small programm is supposed to demonstrate the very basics of the
    sgui API.
 */
#include "sgui.h"

#include <stdio.h>



/*
    A window event callback. Gets called by the window before posting an
    event to the event system.

    A user pointer can be set to the window that gets passed to the callback,
    together with a structure describing the event.

    All this function does is print characters entered in the window.
 */
void window_callback( void* user, const sgui_event* event )
{
    (void)user;
    if( event->type == SGUI_CHAR_EVENT )
    {
        printf( "%s\n", event->arg.utf8 );
    }
}



int main( )
{
    sgui_window* wnd;
    sgui_widget* text;
    sgui_widget* button;

    /*
        Initialize the sgui library backend. Always call this function before
        any other sgui function.
     */
    sgui_init( );

    /*
        Create a window.

        The first argument is a pointer to a parent window.
        We don't have a parent window, so we set this to NULL.

        The next two argument are the window width and height.
        In this case 400x300.

        The last argument is a field of various flags. We don't
        need any fancy stuff, so we set this to 0.
     */
    wnd = sgui_window_create( NULL, 400, 300, 0 );

    /*
        Set the window title bar text and move it to the center
        of the screen.

        The window is initially invisible, so we need to set it
        to visible.
     */
    sgui_window_set_title( wnd, "Simple Sample" );
    sgui_window_move_center( wnd );
    sgui_window_set_visible( wnd, SGUI_VISIBLE );

    /*
        Create a label widget at position (10,10) relative to the top left of
        the parent and set the initial text. 
     */
    text = sgui_label_create( 10, 10, "To close the window,\n"
                                      "press <color=\"#FF0000\">close");

    /*
        Create a button at position (50,60) with 60 pixel width and 30 pixel
        height. Set the text on the button and flags to 0. (There are various
        ways to make fancy buttons with the flag field)
     */
    button = sgui_button_create( 50, 60, 80, 30, "Close", 0 );

    /* add widgets to the window */
    sgui_window_add_widget( wnd, text );
    sgui_window_add_widget( wnd, button );

    /*
        Connect an event to a callback.

        When the object "button" triggers an SGUI_BUTTON_OUT_EVENT, call the
        function "sgui_window_set_visible" on the object "wnd" (first
        argument).

        The second argument to the function is an integer with the value of
        SGUI_INVISIBLE.

        In short, when "button" sends an SGUI_BUTTON_OUT_EVENT, the event
        system calls "sgui_window_set_visible( wnd, SGUI_INVISIBLE )"
     */
    sgui_event_connect( button, SGUI_BUTTON_OUT_EVENT,
                        sgui_window_set_visible, wnd,
                        SGUI_INT, SGUI_INVISIBLE );

    /*
        Register a window event callback.

        Whenever the given window generates an event, it calls the function
        "window_callback" before posting the event to the event system.
     */
    sgui_window_on_event( wnd, window_callback );

    /*
        Resize the window to the idal size that fits all widgets
        in the window.
     */
    sgui_window_pack( wnd );

    /*
        Enter ther sgui main loop. This function waits for window events,
        passes them to the windows for processing and processes the internal
        event queue.

        This function returnes once all windows are closed.
     */
    sgui_main_loop( );

    /* Cleanup all windows and widgets we created before */
    sgui_window_destroy( wnd );
    sgui_widget_destroy( text );
    sgui_widget_destroy( button );

    /*
        Cleanup all internal state and memory allocated by the sgui backend.

        Call this functon once you are done using sgui, after all other sgui
        functions.
     */
    sgui_deinit( );

    return 0;
}

