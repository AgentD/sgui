#include "sgui.h"
#include <d3d9.h>



int main( void )
{
    int r=0, g=0, b=0, dr=1, dg=0, db=0;
    sgui_window_description desc;
    IDirect3DDevice9* dev;
    sgui_context* ctx;
    sgui_window* wnd;

    sgui_init( );

    /* create a window */
    desc.parent         = NULL;
    desc.share          = NULL;
    desc.width          = 300;
    desc.height         = 300;
    desc.resizeable     = SGUI_FIXED_SIZE;
    desc.backend        = SGUI_DIRECT3D_9;
    desc.doublebuffer   = SGUI_DOUBLEBUFFERED;
    desc.bits_per_pixel = 32;
    desc.depth_bits     = 16;
    desc.stencil_bits   = 0;
    desc.samples        = 0;

    wnd = sgui_window_create_desc( &desc );

    sgui_window_set_title( wnd, "Direct3D 9 Sample" );
    sgui_window_move_center( wnd );
    sgui_window_set_visible( wnd, SGUI_VISIBLE );
    sgui_window_set_vsync( wnd, 1 );

    ctx = sgui_window_get_context( wnd );
    dev = sgui_context_get_internal( ctx );

    /* main loop */
    while( sgui_main_loop_step( ) )
    {
        /* background color animation */
        r += dr;
        g += dg;
        b += db;

        if( r==0xFF && dr>0 ) { dr= 0; dg=1; db=0; }
        if( g==0xFF && dg>0 ) { dr= 0; dg=0; db=1; }
        if( b==0xFF && db>0 ) { dr=-1; dg=0; db=0; }

        if( r==0 && dr<0 ) { dr=0; dg=-1; db= 0; }
        if( g==0 && dg<0 ) { dr=0; dg= 0; db=-1; }
        if( b==0 && db<0 ) { dr=1; dg= 0; db= 0; }

        /* draw scene */
        IDirect3DDevice9_Clear( dev, 0, NULL, D3DCLEAR_TARGET,
                                D3DCOLOR_XRGB(r, g, b), 1.0f, 0 );

        /* XXX: there should be a rotating triangle here,
                but my Direct3D-9-foo sucks */

        sgui_window_swap_buffers( wnd );
    }

    /* clean up */
    sgui_window_destroy( wnd );
    sgui_deinit( );

    return 0;
}

