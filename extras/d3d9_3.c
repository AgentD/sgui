#include "sgui.h"
#include "sgui_d3d9.h"

#include <windows.h>
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

#define CUSTOMFVF (D3DFVF_XYZRHW | D3DFVF_DIFFUSE)



static sgui_widget* subview;
static int running = 1;



DWORD __stdcall d3d9_drawing_thread( LPVOID arg )
{
    sgui_window* window = sgui_subview_get_window( subview );
    sgui_context* ctx = sgui_window_get_context( window );
    IDirect3DDevice9* dev = sgui_context_get_internal( ctx );
    LPDIRECT3DVERTEXBUFFER9 v_buffer;
    VOID* pVoid;
    (void)arg;

    /* create a vertex buffer */
    IDirect3DDevice9_CreateVertexBuffer( dev, 3*sizeof(CUSTOMVERTEX), 0,
                                         CUSTOMFVF, D3DPOOL_MANAGED,
                                         &v_buffer, NULL );

    /* load vertex data */
    IDirect3DVertexBuffer9_Lock( v_buffer, 0, 0, (void**)&pVoid, 0 );
    memcpy( pVoid, vertices, sizeof(vertices) );
    IDirect3DVertexBuffer9_Unlock( v_buffer );

    /* drawing loop */
    while( running )
    {
        int r = rand( ) % 256, g = rand( ) % 256, b = rand( ) % 256;

        IDirect3DDevice9_Clear( dev, 0, NULL, D3DCLEAR_TARGET,
                                D3DCOLOR_XRGB(r, g, b), 1.0f, 0 );

        IDirect3DDevice9_BeginScene( dev );

        IDirect3DDevice9_SetFVF( dev, CUSTOMFVF );
        IDirect3DDevice9_SetStreamSource( dev, 0, v_buffer, 0,
                                          sizeof(CUSTOMVERTEX) );

        IDirect3DDevice9_DrawPrimitive( dev, D3DPT_TRIANGLELIST, 0, 1 );

        IDirect3DDevice9_EndScene( dev );

        sgui_window_swap_buffers( window );

        Sleep( 500 );
    }

    IDirect3DVertexBuffer9_Release( v_buffer );
    return 0;
}

int main( void )
{
    HANDLE thread = 0;
    sgui_widget* text;
    sgui_window* wnd;

    sgui_init( );

    /* create a window */
    wnd = sgui_window_create( NULL, 200, 160, SGUI_FIXED_SIZE );

    sgui_window_set_title( wnd, "Direct3D widget" );
    sgui_window_move_center( wnd );
    sgui_window_set_visible( wnd, SGUI_VISIBLE );

    /* create widgets */
    text = sgui_label_create( 20, 130, "D3D9 Drawing Thread" );
    subview = sgui_subview_create( wnd, 10, 10, 180, 120,
                                   SGUI_DIRECT3D_9, NULL );

    /* add widgets to the window */
    sgui_window_add_widget( wnd, text );
    sgui_window_add_widget( wnd, subview );

    /* create drawing thread */
    thread = CreateThread( 0, 0, d3d9_drawing_thread, 0, 0, 0 );

    /* main loop */
    sgui_main_loop( );

    /* wait for the thread to terminate */
    running = 0;
    WaitForSingleObject( thread, INFINITE );
    CloseHandle( thread );

    /* clean up */
    sgui_window_destroy( wnd );
    sgui_widget_destroy( subview );
    sgui_widget_destroy( text );
    sgui_deinit( );

    return 0;
}

