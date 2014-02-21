#include "sgui.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#define VAR_NONE  0
#define VAR_INT   2
#define VAR_INT2  4

typedef struct
{
    union
    {
        int i;

        struct { int x; int y; } i2;
    }
    data;

    int type;
}
varying;

/***************************************************************************/
typedef void(* callback_function )( void* object, ... );

typedef struct listener_s
{
    callback_function function; /* callback function ("methode") */
    varying argument;           /* argument to pass to the callback */

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

    switch( l->argument.type )
    {
    case VAR_NONE:
        l->function( l->receiver );
        break;
    case VAR_INT:
        l->function( l->receiver, l->argument.data.i );
        break;
    case VAR_INT2:
        l->function( l->receiver,
                     l->argument.data.i2.x, l->argument.data.i2.y );
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
    l->argument.type = param_type;
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
    case VAR_INT:
        l->argument.data.i = va_arg( ap, int );
        break;
    case VAR_INT2:
        l->argument.data.i2.x = va_arg( ap, int );
        l->argument.data.i2.y = va_arg( ap, int );
        break;
    }

    va_end( ap );
}

void event_manager_event_callback( event_manager* mgr, sgui_event* event )
{
    listener* l;

    if( mgr && event && event->type>SGUI_MAX_CANVAS_EVENT )
    {
        /* call the process function on every listener */
        for( l=mgr->listeners; l!=NULL; l=l->next )
            listener_process( l, event->type, event->widget );
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

    wnd = sgui_window_create( NULL, 200, 130, SGUI_FIXED_SIZE );
    sgui_window_set_visible( wnd, SGUI_VISIBLE );
    sgui_window_set_title( wnd, "Signals & Slots" );

    /* load font */
    font = sgui_font_load( "../font/SourceSansPro-Regular.ttf", 16 );

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

    sgui_window_set_userptr( wnd, mgr );
    sgui_window_on_event( wnd,
                          (sgui_window_callback)event_manager_event_callback);

    event_manager_connect( mgr, SGUI_BUTTON_CLICK_EVENT, b1, img,
                           (callback_function)sgui_widget_set_visible,
                           VAR_INT, 1 );

    event_manager_connect( mgr, SGUI_BUTTON_CLICK_EVENT, b2, img,
                           (callback_function)sgui_widget_set_visible,
                           VAR_INT, 0 );

    event_manager_connect( mgr, SGUI_BUTTON_CLICK_EVENT, b3, wnd,
                           (callback_function)sgui_window_move,
                           VAR_INT2, 50, 100 );

    event_manager_connect( mgr, SGUI_BUTTON_CLICK_EVENT, b4, wnd,
                           (callback_function)sgui_window_set_visible,
                           VAR_INT, 0 );

    /* enter main loop */
    sgui_main_loop( );

    /* clean up */
    event_manager_destroy( mgr );
    sgui_window_on_event( wnd, NULL );

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

