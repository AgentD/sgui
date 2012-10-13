#include "sgui.h"
#include "internal.h"



Display* dpy = NULL;
XIM im = 0;
Atom atom_wm_delete = 0;

static sgui_window_xlib** windows = NULL;
static unsigned int num_windows = 0;
static unsigned int used_windows = 0;



static const char* wm_delete_window = "WM_DELETE_WINDOW";

/****************************************************************************/

sgui_window_xlib* add_window( void )
{
    sgui_window_xlib** new_windows;

    if( used_windows == num_windows )
    {
        num_windows += 10;
        new_windows = realloc( windows,
                               sizeof(sgui_window_xlib*) * num_windows );

        if( !new_windows )
            return NULL;

        windows = new_windows;
    }

    windows[ used_windows ] = malloc( sizeof(sgui_window_xlib) );

    if( !windows[ used_windows ] )
        return NULL;

    memset( windows[ used_windows ], 0, sizeof(sgui_window_xlib) );

    if( !sgui_internal_window_init( (sgui_window*)windows[ used_windows ] ) )
    {
        free( windows[ used_windows ] );
        return NULL;
    }

    return windows[ used_windows++ ];
}

void remove_window( sgui_window_xlib* wnd )
{
    unsigned int i;

    for( i=0; i<used_windows; ++i )
    {
        if( windows[ i ] == wnd )
        {
            windows[ i ] = windows[ used_windows - 1 ];
            --used_windows;

            sgui_internal_window_deinit( (sgui_window*)wnd );
            free( wnd );
            break;
        }
    }
}

/****************************************************************************/

int sgui_init( void )
{
    windows = malloc( 10 * sizeof(sgui_window_xlib*) );

    if( !windows )
    {
        sgui_deinit( );
        return 0;
    }

    num_windows = 10;
    used_windows = 0;

    /* open display connection */
    dpy = XOpenDisplay( 0 );

    if( !dpy )
    {
        sgui_deinit( );
        return 0;
    }

    /* create input method */
    im = XOpenIM( dpy, NULL, NULL, NULL );

    if( !im )
    {
        sgui_deinit( );
        return 0;
    }

    /* get wm delete atom */
    atom_wm_delete = XInternAtom( dpy, wm_delete_window, True );

    /* initialise keycode translation LUT */
    init_keycodes( );

    return 1;
}

void sgui_deinit( void )
{
    unsigned int i;

    if( windows )
    {
        for( i=0; i<used_windows; ++i )
            free( windows[ i ] );

        free( windows );
    }

    if( dpy )
        XCloseDisplay( dpy );

    if( im )
        XCloseIM( im );

    dpy = NULL;
    im = 0;

    windows = NULL;
    num_windows = 0;
    used_windows = 0;
}

void sgui_main_loop( void )
{
    unsigned int i, active;
    XEvent e;

    do
    {
        /* update all visible windows */
        for( i=0, active=0; i<used_windows; ++i )
        {
            if( windows[ i ]->base.visible )
            {
                active = 1; /* there is at least 1 window still active */

                update_window( windows[ i ] );
            }
        }

        /* Handle all pending events */
        while( XPending( dpy ) > 0 )
        {
            XNextEvent( dpy, &e );

            if( XFilterEvent( &e, None ) )
                continue;

            /* route the event to it's window */
            for( i=0; i<used_windows; ++i )
            {
                if( windows[ i ]->wnd == e.xany.window )
                {
                    handle_window_events( windows[ i ], &e );
                    break;
                }
            }
        }
    }
    while( active );
}

