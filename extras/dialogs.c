#include "sgui.h"

#include <stdio.h>


const char* text =
"Lorem ipsum dolor sit amet, consetetur sadipscing elitr,\n"
"sed diam nonumy eirmod tempor invidunt ut labore et dolore\n"
"magna aliquyam erat, sed diam voluptua.";

static void print_color( const char* space, unsigned char* c )
{
    printf( "%s: %d, %d, %d, %d\n", space, c[0], c[1], c[2], c[3] );
}

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

    sgui_event_connect( cp, SGUI_DIALOG_REJECTED,
                        puts, "Color dialog rejected", SGUI_VOID );

    sgui_event_connect( cp, SGUI_COLOR_SELECTED_RGBA_EVENT,
                        print_color, "RGBA", SGUI_FROM_EVENT, SGUI_COLOR );

    sgui_event_connect( cp, SGUI_COLOR_SELECTED_HSVA_EVENT,
                        print_color, "HSVA", SGUI_FROM_EVENT, SGUI_COLOR );

    sgui_message_box_display( mb );
    sgui_color_dialog_display( cp );
    sgui_main_loop( );

    sgui_color_dialog_destroy( cp );
    sgui_message_box_destroy( mb );
    sgui_deinit( );


    sgui_message_box_emergency( "Emergency Messagebox", text );
    return 0;
}

