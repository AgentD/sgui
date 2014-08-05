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




void draw_callback( sgui_window* window )
{
    sgui_d3d11_context* ctx;
    FLOAT bg[4] = { 0.0f, 0.2f, 0.4f, 1.0f };

    ctx = (sgui_d3d11_context*)sgui_window_get_context( window );

    /* draw frame */
    ID3D11DeviceContext_ClearRenderTargetView(ctx->ctx, ctx->backbuffer, bg);

    ID3D11DeviceContext_Draw( ctx->ctx, 3, 0 );

    sgui_window_swap_buffers( window );
}




int main( void )
{
    D3D11_SUBRESOURCE_DATA InitData;
    sgui_window_description desc;
    ID3DBlob *vsblob, *psblob;
    sgui_d3d11_context* ctx;
    ID3D11VertexShader* vs;
    ID3D11PixelShader* ps;
    ID3D11InputLayout* il;
    D3D11_BUFFER_DESC bd;
    UINT stride, offset;
    ID3D11Buffer* vbo;
    sgui_window* wnd;

    sgui_init( );

    /* create a window */
    desc.parent         = NULL;
    desc.share          = NULL;
    desc.width          = 300;
    desc.height         = 300;
    desc.resizeable     = SGUI_FIXED_SIZE;
    desc.backend        = SGUI_DIRECT3D_11;
    desc.doublebuffer   = SGUI_DOUBLEBUFFERED;
    desc.bits_per_pixel = 0;
    desc.depth_bits     = 0;
    desc.stencil_bits   = 0;
    desc.samples        = 0;

    wnd = sgui_window_create_desc( &desc );

    if( !wnd )
    {
        fprintf( stderr, "Could not create Direct3D 11 window!\n" );
        return -1;
    }

    sgui_window_set_title( wnd, "Direct3D 11 Sample" );
    sgui_window_move_center( wnd );
    sgui_window_set_visible( wnd, SGUI_VISIBLE );

    ctx = (sgui_d3d11_context*)sgui_window_get_context( wnd );

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

    /* hook event callbacks */
    sgui_event_connect( wnd, SGUI_EXPOSE_EVENT,
                        draw_callback, wnd, SGUI_VOID );

    sgui_event_connect( wnd, SGUI_EXPOSE_EVENT,
                        puts, "Redraw!", SGUI_VOID );

    /* main loop */
    sgui_main_loop( );

    /* clean up */
    ID3D11Buffer_Release( vbo );
    ID3D11InputLayout_Release( il );
    ID3D11VertexShader_Release( vs );
    ID3D11PixelShader_Release( ps );

    vsblob->lpVtbl->Release( vsblob );
    psblob->lpVtbl->Release( psblob );

    sgui_window_destroy( wnd );
    sgui_deinit( );

    return 0;
}

