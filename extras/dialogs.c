#include "sgui.h"

#include <stdio.h>


const char* text =
"Lorem ipsum dolor sit amet, consetetur sadipscing elitr,\n"
"sed diam nonumy eirmod tempor invidunt ut labore et dolore\n"
"magna aliquyam erat, sed diam voluptua.";

int main( void )
{
    sgui_color_dialog* cp;
    sgui_message_box* mb;

    sgui_init( );
    mb = sgui_message_box_create( SGUI_MB_WARNING, "Test", text,
                                  "Ok", "Foobar", 0 );

    cp = sgui_color_dialog_create( "color picker", "Ok", "Cancel" );

    sgui_event_connect( mb, SGUI_MESSAGE_BOX_BUTTON1_EVENT,
                        puts, "Okay", SGUI_VOID );

    sgui_event_connect( mb, SGUI_MESSAGE_BOX_BUTTON2_EVENT,
                        puts, "Foobar", SGUI_VOID );

    sgui_event_connect( mb, SGUI_MESSAGE_BOX_BUTTON1_EVENT,
                        sgui_color_dialog_display, cp, SGUI_VOID );

    sgui_event_connect( mb, SGUI_MESSAGE_BOX_BUTTON2_EVENT,
                        sgui_color_dialog_display, cp, SGUI_VOID );

    sgui_message_box_display( mb );
    sgui_main_loop( );

    sgui_color_dialog_destroy( cp );
    sgui_message_box_destroy( mb );
    sgui_deinit( );


    sgui_message_box_emergency( "Emergency Messagebox", text );
    return 0;
}

