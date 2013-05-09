#include "sgui.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

/***************************************************************************/
#define PARAM_NONE  0
#define PARAM_INT   2
#define PARAM_INT2  4

typedef void(* callback_function )( void* object, ... );

typedef struct listener_s
{
    union
    {
        int i;

        struct { int x; int y; } i2;
    }
    param;                      /* the parameters to pass to the function */

    int param_type;             /* PARAM_* constant */
    callback_function function; /* callback function ("methode") */

    void* sender;               /* the sender object */
    void* receiver;             /* the receiver object */
    int event;                  /* the event to wait for */

    struct listener_s* next;    /* linked list pointer */
}
listener;

void listener_process( listener* l, int event, void* sender )
{
    if( !l || l->event!=event || l->sender!=sender )
        return;

    switch( l->param_type )
    {
    case PARAM_NONE:
        l->function( l->receiver );
        break;
    case PARAM_INT:
        l->function( l->receiver, l->param.i );
        break;
    case PARAM_INT2:
        l->function( l->receiver, l->param.i2.x, l->param.i2.y );
        break;
    }
}

/***************************************************************************/

typedef struct
{
    listener* listeners;
}
event_manager;

event_manager* event_manager_create( void )
{
    event_manager* mgr = malloc( sizeof(event_manager) );

    if( mgr )
        mgr->listeners = NULL;

    return mgr;
}

void event_manager_destroy( event_manager* mgr )
{
    listener* l;
    listener* old;

    if( mgr )
    {
        l = mgr->listeners;

        while( l!=NULL )
        {
            old = l;
            l = l->next;
            free( old );
        }

        free( mgr );
    }
}

void event_manager_connect( event_manager* mgr, int event,
                            sgui_widget* sender, void* receiver,
                            callback_function function, int param_type, ... )
{
    listener* l;
    va_list ap;

    if( !mgr || !function )
        return;

    /* allocate listener */
    l = malloc( sizeof(listener) );

    if( !l )
        return;

    /* fill in listener fields */
    l->param_type = param_type;
    l->function = function;
    l->sender = sender;
    l->receiver = receiver;
    l->event = event;
    l->next = mgr->listeners;

    mgr->listeners = l;

    /* get callback parameters */
    va_start( ap, param_type );

    switch( param_type )
    {
    case PARAM_INT:
        l->param.i = va_arg( ap, int );
        break;
    case PARAM_INT2:
        l->param.i2.x = va_arg( ap, int );
        l->param.i2.y = va_arg( ap, int );
        break;
    }

    va_end( ap );
}

void event_manager_event_callback( sgui_widget* widget, int type, void* user )
{
    event_manager* mgr = user;
    listener* l;

    if( mgr )
    {
        /* call the process function on every listener */
        for( l=mgr->listeners; l!=NULL; l=l->next )
            listener_process( l, type, widget );
    }
}

/***************************************************************************/

int main( void )
{
    unsigned char image[ 64*64*3 ], *ptr;
    sgui_font* font;
    sgui_widget *b1, *b2, *b3, *b4, *img;
    event_manager* mgr;
    sgui_window* wnd;
    int i, j;

    /* initialize and create window */
    sgui_init( );

    wnd = sgui_window_create( NULL, 200, 130, SGUI_FIXED_SIZE, SGUI_NATIVE );
    sgui_window_set_visible( wnd, SGUI_VISIBLE );
    sgui_window_set_title( wnd, "Signals & Slots" );

    /* load font */
    font = sgui_font_load( "../../font/SourceSansPro-Regular.ttf", 16 );

    sgui_skin_set_default_font( font, NULL, NULL, NULL );

    /* create widgets */
    b1 = sgui_button_create( 10, 10, 80, 30, "Show" );
    b2 = sgui_button_create( 10, 45, 80, 30, "Hide" );
    b3 = sgui_button_create( 10, 80, 80, 30, "Move" );
    b4 = sgui_button_create( 95, 80, 80, 30, "Close" );

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

    /* create the "event manager" and make some connections */
    mgr = event_manager_create( );

    sgui_window_on_widget_event( wnd, event_manager_event_callback, mgr );

    event_manager_connect( mgr, SGUI_BUTTON_CLICK_EVENT, b1, img,
                           (callback_function)sgui_widget_set_visible,
                           PARAM_INT, 1 );

    event_manager_connect( mgr, SGUI_BUTTON_CLICK_EVENT, b2, img,
                           (callback_function)sgui_widget_set_visible,
                           PARAM_INT, 0 );

    event_manager_connect( mgr, SGUI_BUTTON_CLICK_EVENT, b3, wnd,
                           (callback_function)sgui_window_move,
                           PARAM_INT2, 50, 100 );

    event_manager_connect( mgr, SGUI_BUTTON_CLICK_EVENT, b4, wnd,
                           (callback_function)sgui_window_set_visible,
                           PARAM_INT, 0 );

    /* enter main loop */
    sgui_main_loop( );

    /* clean up */
    event_manager_destroy( mgr );
    sgui_window_destroy( wnd );
    sgui_widget_destroy( b1 );
    sgui_widget_destroy( b2 );
    sgui_widget_destroy( b3 );
    sgui_widget_destroy( b4 );
    sgui_widget_destroy( img );
    sgui_font_destroy( font );
    sgui_deinit( );

    return 0;
}

