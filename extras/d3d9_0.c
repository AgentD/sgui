/*
    This file is part of the sgui samples collection. I, David Oberhollenzer,
    author of this file hereby place the contents of this file into
    the public domain.
 */
/*
    This small programm is supposed to demonstrate how to create a window
    with a Direct3D 9 rendering context through sgui.
 */
#include "sgui.h"
#include "sgui_d3d9.h"

#include <string.h>
#include <stdio.h>
#include <d3d9.h>



typedef struct
{
    FLOAT x, y, z, rhw;
    DWORD color;
}
CUSTOMVERTEX;

CUSTOMVERTEX vertices[] =
{
    { 150.0f,  50.0f, 1.0f, 1.0f, 0x000000FF },
    { 250.0f, 250.0f, 1.0f, 1.0f, 0x0000FF00 },
    {  50.0f, 250.0f, 1.0f, 1.0f, 0x00FF0000 }
};

LPDIRECT3DVERTEXBUFFER9 v_buffer;

#define CUSTOMFVF (D3DFVF_XYZRHW | D3DFVF_DIFFUSE)



void draw_callback( sgui_window* window )
{
    /* get the context from the window */
    sgui_d3d9_context* ctx =
    (sgui_d3d9_context*)sgui_window_get_context( window );

    /* clear the screen, begin drawing */
    IDirect3DDevice9_Clear( ctx->device, 0, NULL, D3DCLEAR_TARGET,
                            D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0 );

    IDirect3DDevice9_BeginScene( ctx->device );

    /* draw the scene */
    IDirect3DDevice9_SetFVF( ctx->device, CUSTOMFVF );
    IDirect3DDevice9_SetStreamSource( ctx->device, 0, v_buffer, 0,
                                      sizeof(CUSTOMVERTEX) );

    IDirect3DDevice9_DrawPrimitive( ctx->device, D3DPT_TRIANGLELIST, 0, 1 );

    IDirect3DDevice9_EndScene( ctx->device );

    /* swap front and back buffer */
    sgui_window_swap_buffers( window );
}



int main( void )
{
    sgui_window_description desc;
    IDirect3DDevice9* dev;
    sgui_context* ctx;
    sgui_window* wnd;
    sgui_lib* lib;
    VOID* pVoid;

    lib = sgui_init(NULL);

    /* create a window. See gl0.c for furhter explanation */
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

    /* make the window visible */
    sgui_window_set_title( wnd, "Direct3D 9 Sample" );
    sgui_window_move_center( wnd );
    sgui_window_set_visible( wnd, SGUI_VISIBLE );

    /* create a vertex buffer */
    ctx = sgui_window_get_context( wnd );
    dev = ctx->get_internal( ctx );

    IDirect3DDevice9_CreateVertexBuffer( dev, 3*sizeof(CUSTOMVERTEX), 0,
                                         CUSTOMFVF, D3DPOOL_MANAGED,
                                         &v_buffer, NULL );

    /* load vertex data */
    IDirect3DVertexBuffer9_Lock( v_buffer, 0, 0, (void**)&pVoid, 0 );
    memcpy( pVoid, vertices, sizeof(vertices) );
    IDirect3DVertexBuffer9_Unlock( v_buffer );

    /* hook event callbacks. See gl0.c for furhter explanation */
    sgui_event_connect( wnd, SGUI_EXPOSE_EVENT,
                        draw_callback, wnd, SGUI_VOID );

    sgui_event_connect( wnd, SGUI_EXPOSE_EVENT,
                        puts, "Readraw!", SGUI_VOID );

    /* main loop */
    lib->main_loop(lib);

    /* clean up */
    IDirect3DVertexBuffer9_Release( v_buffer );

    sgui_window_destroy( wnd );
    lib->destroy(lib);

    return 0;
}

