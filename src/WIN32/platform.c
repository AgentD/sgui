#include "sgui.h"
#include "internal.h"


static sgui_window_w32** windows = NULL;
static unsigned int num_windows = 0;
static unsigned int used_windows = 0;

HINSTANCE hInstance;
const char* wndclass = "sgui_wnd_class";



sgui_window_w32* add_window( void )
{
    sgui_window_w32** new_windows;

    if( used_windows == num_windows )
    {
        num_windows += 10;
        new_windows = realloc( windows,
                               sizeof(sgui_window_w32*) * num_windows );

        if( !new_windows )
            return NULL;

        windows = new_windows;
    }

    windows[ used_windows ] = malloc( sizeof(sgui_window_w32) );

    if( !windows[ used_windows ] )
        return NULL;

    memset( windows[ used_windows ], 0, sizeof(sgui_window_w32) );

    if( !sgui_internal_window_init( (sgui_window*)windows[ used_windows ] ) )
    {
        free( windows[ used_windows ] );
        return NULL;
    }

    return windows[ used_windows++ ];
}

void remove_window( sgui_window_w32* wnd )
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
    WNDCLASSEX wc;

    /* get hInstance */
    hInstance = GetModuleHandle( NULL );

    if( !hInstance )
        return 0;

    /* Register window class */
    memset( &wc, 0, sizeof(WNDCLASSEX) );

    wc.cbSize        = sizeof(WNDCLASSEX);
    wc.style         = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc   = WindowProcFun;
    wc.hInstance     = hInstance;
    wc.lpszClassName = wndclass;
    wc.hCursor       = LoadCursor( NULL, IDC_ARROW );

    if( RegisterClassEx( &wc ) == 0 )
    {
        hInstance = 0;
        return 0;
    }

    /* create window list */
    windows = malloc( 10 * sizeof(sgui_window_w32*) );

    if( !windows )
    {
        sgui_deinit( );
        return 0;
    }

    num_windows = 10;
    used_windows = 0;

    return 1;
}

void sgui_deinit( void )
{
    unsigned int i;

    /* free window list */
    if( windows )
    {
        for( i=0; i<used_windows; ++i )
            free( windows[ i ] );

        free( windows );
    }

    /* unregister window class */
    UnregisterClass( wndclass, hInstance );

    /* reset values */
    hInstance = 0;
    windows = NULL;
    num_windows = 0;
    used_windows = 0;
}

int sgui_main_loop_step( void )
{
    unsigned int i, active;
    MSG msg;

    /* handle a message if there is one */
    if( PeekMessage( &msg, 0, 0, 0, PM_REMOVE ) )
    {
        TranslateMessage( &msg );
        DispatchMessage( &msg );
    }

    /* update windows */
    for( i=0, active=0; i<used_windows; ++i )
    {
        if( windows[ i ]->base.visible )
        {
            update_window( windows[ i ] );
            active = 1;
        }
    }

    return active;
}

void sgui_main_loop( void )
{
    unsigned int i, active = 1;
    MSG msg;

    /* handle messages */
    while( active && GetMessage( &msg, 0, 0, 0 ) )
    {
        TranslateMessage( &msg );
        DispatchMessage( &msg );

        /* update windows */
        for( i=0, active=0; i<used_windows; ++i )
        {
            if( windows[ i ]->base.visible )
            {
                update_window( windows[ i ] );
                active = 1;
            }
        }
    }
}

