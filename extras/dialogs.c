/*
    This file is part of the sgui samples collection. I, David Oberhollenzer,
    author of this file hereby place the contents of this file into
    the public domain.
 */
/*
    This small programm is supposed to show off the different kinds of default
    dialog windows that sgui can create and how to use them.
 */
#include "sgui.h"

#include <stdio.h>


const char* text =
"Lorem ipsum dolor sit amet, consetetur sadipscing elitr,\n"
"sed diam nonumy eirmod tempor invidunt ut labore et dolore\n"
"magna aliquyam erat, sed diam voluptua.";

const char* emtext =
"This is an emergency message box fallback. It can be called\n"
"even if the sgui_init( ) function fails and uses low-level\n"
"system functions to draw a simple message box.";

static void print_color( const char* space, unsigned char* c )
{
    printf( "%s: %d, %d, %d, %d\n", space, c[0], c[1], c[2], c[3] );
}

int main( void )
{
    sgui_dialog* cp;
    sgui_dialog* mb;
    sgui_lib* lib;

    lib = sgui_init(NULL);

    /*
        Create a message box.

        The first argument is the icon to attach to the box. Possible values
        are:
            SGUI_MB_INFO
            SGUI_MB_WARNING
            SGUI_MB_CRITICAL
            SGUI_MB_QUESTION

        The next two arguemnts are the caption and the text inside the
        message box.

        What follows are the captions of up to three buttons underneath the
        text inside the message box. Since we only need two buttons, the
        third caption is set to NULL.
     */
    mb = sgui_message_box_create( lib, SGUI_ICON_MB_WARNING, "Test", text,
                                  "Ok", "Foobar", NULL );

    /*
        Create a color picker dialog.

        The first argument is the caption of the dialog window, the second
        argument is the text written on the accept button, the third argument
        the text written on the reject button.
     */
    cp = sgui_color_dialog_create( lib, "color picker", "Ok", "Cancel" );

    /*
        The message box triggers an SGUI_MESSAGE_BOX_BUTTON1_EVENT when the
        first button gets pressed, an SGUI_MESSAGE_BOX_BUTTON2_EVENT for the
        second button and an SGUI_MESSAGE_BOX_BUTTON3_EVENT for the third
        button.

        If the message box window gets closed without the user pressing
        a button, an SGUI_DIALOG_REJECTED is generated.
     */
    sgui_event_connect( mb, SGUI_MESSAGE_BOX_BUTTON1_EVENT,
                        puts, "Okay", SGUI_VOID );

    sgui_event_connect( mb, SGUI_MESSAGE_BOX_BUTTON2_EVENT,
                        puts, "Foobar", SGUI_VOID );

    /*
        When the reject button of a color selection dialog gets clicked,
        an SGUI_DIALOG_REJECTED event is generated.

        When the user presses the accept button, the dialog window generates
        an SGUI_COLOR_SELECTED_RGBA_EVENT with an RGBA color argument, and
        an SGUI_COLOR_SELECTED_HSVA_EVENT with an HSVA color argument.
     */
    sgui_event_connect( cp, SGUI_DIALOG_REJECTED,
                        puts, "Color dialog rejected", SGUI_VOID );

    sgui_event_connect( cp, SGUI_COLOR_SELECTED_RGBA_EVENT,
                        print_color, "RGBA", SGUI_FROM_EVENT, SGUI_COLOR );

    sgui_event_connect( cp, SGUI_COLOR_SELECTED_HSVA_EVENT,
                        print_color, "HSVA", SGUI_FROM_EVENT, SGUI_COLOR );

    /* display dialogs and enter main loop */
    sgui_dialog_display( mb );
    sgui_dialog_display( cp );
    lib->main_loop(lib);

    /* cleanup */
    sgui_dialog_destroy( cp );
    sgui_dialog_destroy( mb );
    lib->destroy(lib);

    /*
        Create emergency fallback message box. This function displays a
        message box using only low-level system functions and not relying on
        any other sgui functions, so it event works when sgui_init( ) failed.

        It is not possible to set any button texts, only caption and box text.
        The content of the box is not formated in any way.

        The function blocks until the user closes the message box window.
     */
    sgui_message_box_emergency( "Emergency Messagebox", emtext );
    return 0;
}

