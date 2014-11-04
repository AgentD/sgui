#include "sgui.h"
#include "sgui_d3d9.h"



typedef struct
{
    FLOAT x, y, z, rhw;
    DWORD color;
}
CUSTOMVERTEX;

CUSTOMVERTEX vertices[] =
{
    { 150.0f,  50.0f, 1.0f, 1.0f, D3DCOLOR_XRGB(0, 0, 255) },
    { 250.0f, 250.0f, 1.0f, 1.0f, D3DCOLOR_XRGB(0, 255, 0) },
    {  50.0f, 250.0f, 1.0f, 1.0f, D3DCOLOR_XRGB(255, 0, 0) }
};

#define CUSTOMFVF (D3DFVF_XYZRHW | D3DFVF_DIFFUSE)



int main( void )
{
    int r=0, g=0, b=0, dr=1, dg=0, db=0;
    LPDIRECT3DVERTEXBUFFER9 v_buffer;
    sgui_window_description desc;
    sgui_d3d9_context* ctx;
    IDirect3DDevice9* dev;
    sgui_window* wnd;
    VOID* pVoid;

    sgui_init( );

    /* create a window */
    desc.parent         = NULL;
    desc.share          = NULL;
    desc.width          = 300;
    desc.height         = 300;
    desc.flags          = SGUI_FIXED_SIZE|SGUI_DOUBLEBUFFERED;
    desc.backend        = SGUI_DIRECT3D_9;
    desc.bits_per_pixel = 32;
    desc.depth_bits     = 16;
    desc.stencil_bits   = 0;
    desc.samples        = 0;

    wnd = sgui_window_create_desc( &desc );

    sgui_window_set_title( wnd, "Direct3D 9 Sample" );
    sgui_window_move_center( wnd );
    sgui_window_set_visible( wnd, SGUI_VISIBLE );
    sgui_window_set_vsync( wnd, 1 );

    /* get the device context, set new present parameters */
    ctx = (sgui_d3d9_context*)sgui_window_get_context( wnd );
    dev = ctx->device;

    IDirect3DDevice9_Reset( dev, &ctx->present );

    /* create a vertex buffer */
    IDirect3DDevice9_CreateVertexBuffer( dev, 3*sizeof(CUSTOMVERTEX), 0,
                                         CUSTOMFVF, D3DPOOL_MANAGED,
                                         &v_buffer, NULL );

    /* load vertex data */
    IDirect3DVertexBuffer9_Lock( v_buffer, 0, 0, (void**)&pVoid, 0 );
    memcpy( pVoid, vertices, sizeof(vertices) );
    IDirect3DVertexBuffer9_Unlock( v_buffer );

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
        IDirect3DDevice9_Clear( dev, 0, NULL,
                                D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER,
                                D3DCOLOR_XRGB(r, g, b), 1.0f, 0 );

        IDirect3DDevice9_BeginScene( dev );

        IDirect3DDevice9_SetFVF( dev, CUSTOMFVF );
        IDirect3DDevice9_SetStreamSource( dev, 0, v_buffer, 0,
                                          sizeof(CUSTOMVERTEX) );

        IDirect3DDevice9_DrawPrimitive( dev, D3DPT_TRIANGLELIST, 0, 1 );

        IDirect3DDevice9_EndScene( dev );

        sgui_window_swap_buffers( wnd );
    }

    /* clean up */
    IDirect3DVertexBuffer9_Release( v_buffer );

    sgui_window_destroy( wnd );
    sgui_deinit( );
    return 0;
}

