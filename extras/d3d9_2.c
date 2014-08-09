#include "sgui.h"
#include "sgui_d3d9.h"

#include <stdlib.h>
#include <time.h>



typedef struct
{
    FLOAT x, y, z, rhw;
    DWORD color;
}
CUSTOMVERTEX;

CUSTOMVERTEX vertices[] =
{
    {  90.0f,  20.0f, 1.0f, 1.0f, D3DCOLOR_XRGB(0, 0, 255) },
    { 140.0f, 100.0f, 1.0f, 1.0f, D3DCOLOR_XRGB(0, 255, 0) },
    {  40.0f, 100.0f, 1.0f, 1.0f, D3DCOLOR_XRGB(255, 0, 0) }
};

LPDIRECT3DVERTEXBUFFER9 v_buffer;

#define CUSTOMFVF (D3DFVF_XYZRHW | D3DFVF_DIFFUSE)



void d3dview_on_draw( sgui_widget* subview )
{
    sgui_window* window = sgui_subview_get_window( subview );
    sgui_context* ctx = sgui_window_get_context( window );
    IDirect3DDevice9* dev = sgui_context_get_internal( ctx );
    int r = rand( ) % 256, g = rand( ) % 256, b = rand( ) % 256;

    IDirect3DDevice9_Clear( dev, 0, NULL, D3DCLEAR_TARGET,
                            D3DCOLOR_XRGB(r, g, b), 1.0f, 0 );

    IDirect3DDevice9_BeginScene( dev );

    IDirect3DDevice9_SetFVF( dev, CUSTOMFVF );
    IDirect3DDevice9_SetStreamSource( dev, 0, v_buffer, 0,
                                      sizeof(CUSTOMVERTEX) );

    IDirect3DDevice9_DrawPrimitive( dev, D3DPT_TRIANGLELIST, 0, 1 );

    IDirect3DDevice9_EndScene( dev );
}

int main( void )
{
    sgui_window* subwindow;
    IDirect3DDevice9* dev;
    sgui_widget* subview;
    sgui_widget* button;
    sgui_widget* text;
    sgui_context* ctx;
    sgui_window* wnd;
    VOID* pVoid;

    srand( time(NULL) );

    sgui_init( );

    /* create a window */
    wnd = sgui_window_create( NULL, 200, 200, SGUI_FIXED_SIZE );

    sgui_window_set_title( wnd, "D3D9 widget" );
    sgui_window_move_center( wnd );
    sgui_window_set_visible( wnd, SGUI_VISIBLE );

    /* create some widgets */
    text = sgui_label_create( 40, 130, "Direct3D\302\256 widget" );
    button = sgui_button_create( 10, 155, 75, 30, "Refresh", 0 );
    subview = sgui_subview_create( wnd, 10, 10, 180, 120,
                                   SGUI_DIRECT3D_9, NULL );

    /* create a vertex buffer */
    subwindow = sgui_subview_get_window( subview );
    ctx = sgui_window_get_context( subwindow );
    dev = sgui_context_get_internal( ctx );

    IDirect3DDevice9_CreateVertexBuffer( dev, 3*sizeof(CUSTOMVERTEX), 0,
                                         CUSTOMFVF, D3DPOOL_MANAGED,
                                         &v_buffer, NULL );

    /* load vertex data */
    IDirect3DVertexBuffer9_Lock( v_buffer, 0, 0, (void**)&pVoid, 0 );
    memcpy( pVoid, vertices, sizeof(vertices) );
    IDirect3DVertexBuffer9_Unlock( v_buffer );

    /* hook callbacks */
    sgui_subview_set_draw_callback( subview, d3dview_on_draw );
    sgui_event_connect( button, SGUI_BUTTON_OUT_EVENT,
                        sgui_subview_refresh, subview, SGUI_VOID );

    /* add widgets to the window */
    sgui_window_add_widget( wnd, text );
    sgui_window_add_widget( wnd, button );
    sgui_window_add_widget( wnd, subview );

    /* main loop */
    sgui_main_loop( );

    /* clean up */
    IDirect3DVertexBuffer9_Release( v_buffer );

    sgui_window_destroy( wnd );
    sgui_widget_destroy( subview );
    sgui_widget_destroy( text );
    sgui_widget_destroy( button );
    sgui_deinit( );

    return 0;
}

