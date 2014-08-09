#include "sgui.h"
#include "sgui_d3d11.h"

#include <d3dcompiler.h>
#include <string.h>
#include <stdio.h>



/* vertex and pixel shaders */
const char* vsh =
"float4 main( float4 Pos : POSITION ) : SV_POSITION\n"
"{\n"
"    return Pos;\n"
"}\n";

const char* psh =
"float4 main( float4 Pos : SV_POSITION ) : SV_Target\n"
"{\n"
"    return float4( 1.0f, 1.0f, 0.0f, 1.0f );\n"
"}\n";

/* input layout */
D3D11_INPUT_ELEMENT_DESC layout[] =
{
{"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0},
};

#define LAYOUTCOUNT (sizeof(layout)/sizeof(layout[0]))

/* vertex data */
float vertices[] =
{
     0.0f,  0.5f, 0.5f,
     0.5f, -0.5f, 0.5f,
    -0.5f, -0.5f, 0.5f
};



static sgui_widget* subview;
static int running = 1;



DWORD __stdcall d3d11_drawing_thread( LPVOID arg )
{
    FLOAT bg[4] = { 0.0f, 0.2f, 0.4f, 1.0f };
    int r=0, g=0, b=0, dr=1, dg=0, db=0;
    D3D11_SUBRESOURCE_DATA InitData;
    ID3DBlob *vsblob, *psblob;
    sgui_d3d11_context* ctx;
    ID3D11VertexShader* vs;
    ID3D11PixelShader* ps;
    ID3D11InputLayout* il;
    D3D11_BUFFER_DESC bd;
    UINT stride, offset;
    sgui_window* window;
    ID3D11Buffer* vbo;
    (void)arg;

    window = sgui_subview_get_window( subview );
    ctx = (sgui_d3d11_context*)sgui_window_get_context( window );

    /* compile shaders, create shader objects */
    D3DCompile( vsh, strlen(vsh), NULL, NULL, NULL, "main", "vs_4_0",
                0, 0, &vsblob, NULL );

    D3DCompile( psh, strlen(psh), NULL, NULL, NULL, "main", "ps_4_0",
                0, 0, &psblob, NULL );

    ID3D11Device_CreateVertexShader( ctx->dev,
                                     vsblob->lpVtbl->GetBufferPointer(vsblob),
                                     vsblob->lpVtbl->GetBufferSize(vsblob),
                                     NULL, &vs );

    ID3D11Device_CreatePixelShader( ctx->dev,
                                    psblob->lpVtbl->GetBufferPointer(psblob),
                                    psblob->lpVtbl->GetBufferSize(psblob),
                                    NULL, &ps );

    /* create an input layout */
    ID3D11Device_CreateInputLayout( ctx->dev, layout, LAYOUTCOUNT,
                                    vsblob->lpVtbl->GetBufferPointer(vsblob),
                                    vsblob->lpVtbl->GetBufferSize(vsblob),
                                    &il );

    /* create a vertex buffer */
    memset( &bd, 0, sizeof(bd) );
    bd.Usage     = D3D11_USAGE_DEFAULT;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.ByteWidth = sizeof(vertices);

    memset( &InitData, 0, sizeof(InitData) );
    InitData.pSysMem = vertices;

    ID3D11Device_CreateBuffer( ctx->dev, &bd, &InitData, &vbo );

    /* configure input assembler */
    ID3D11DeviceContext_IASetInputLayout( ctx->ctx, il );

    stride = 3 * sizeof(float);
    offset = 0;
    ID3D11DeviceContext_IASetVertexBuffers( ctx->ctx, 0, 1, &vbo,
                                            &stride, &offset );

    ID3D11DeviceContext_IASetPrimitiveTopology(
                            ctx->ctx, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

    /* set shaders */
    ID3D11DeviceContext_VSSetShader( ctx->ctx, vs, NULL, 0 );
    ID3D11DeviceContext_PSSetShader( ctx->ctx, ps, NULL, 0 );

    /* drawing loop */
    while( running )
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

        bg[0] = ((float)r) / ((float)255.0f);
        bg[1] = ((float)g) / ((float)255.0f);
        bg[2] = ((float)b) / ((float)255.0f);

        /* draw scene */
        ID3D11DeviceContext_ClearRenderTargetView( ctx->ctx, ctx->backbuffer,
                                                   bg );
        ID3D11DeviceContext_ClearDepthStencilView( ctx->ctx, ctx->dsv,
                                                   D3D11_CLEAR_DEPTH,
                                                   1.0f, 0x00 );

        ID3D11DeviceContext_Draw( ctx->ctx, 3, 0 );

        sgui_window_swap_buffers( window );

        Sleep( 20 );
    }

    ID3D11Buffer_Release( vbo );
    ID3D11InputLayout_Release( il );
    ID3D11VertexShader_Release( vs );
    ID3D11PixelShader_Release( ps );

    vsblob->lpVtbl->Release( vsblob );
    psblob->lpVtbl->Release( psblob );
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
    text = sgui_label_create( 20, 130, "D3D11 Drawing Thread" );
    subview = sgui_subview_create( wnd, 10, 10, 180, 120,
                                   SGUI_DIRECT3D_11, NULL );

    /* add widgets to the window */
    sgui_window_add_widget( wnd, text );
    sgui_window_add_widget( wnd, subview );

    /* create drawing thread */
    thread = CreateThread( 0, 0, d3d11_drawing_thread, 0, 0, 0 );

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

