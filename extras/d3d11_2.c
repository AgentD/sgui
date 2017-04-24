/*
    This file is part of the sgui samples collection. I, David Oberhollenzer,
    author of this file hereby place the contents of this file into
    the public domain.
 */
/*
    This small programm is supposed to demonstrate how to create a window
    an embedded Direct3D 11 rendering widget through sgui.
 */
#include "sgui.h"
#include "sgui_d3d11.h"

#include <d3dcompiler.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>




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




void d3dview_on_draw( sgui_widget* subview )
{
    sgui_window* window = sgui_subview_get_window( subview );
    sgui_d3d11_context* ctx =
    (sgui_d3d11_context*)sgui_window_get_context( window );
    FLOAT bg[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

    bg[0] = (float)(rand( ) % 256) / 255.0f;
    bg[1] = (float)(rand( ) % 256) / 255.0f;
    bg[2] = (float)(rand( ) % 256) / 255.0f;

    /* draw frame */
    ID3D11DeviceContext_ClearRenderTargetView(ctx->ctx, ctx->backbuffer, bg);
    ID3D11DeviceContext_ClearDepthStencilView( ctx->ctx, ctx->dsv,
                                               D3D11_CLEAR_DEPTH,
                                               1.0f, 0x00 );

    ID3D11DeviceContext_Draw( ctx->ctx, 3, 0 );
}

int main( void )
{
    D3D11_SUBRESOURCE_DATA InitData;
    ID3DBlob *vsblob, *psblob;
    sgui_d3d11_context* ctx;
    sgui_window* subwindow;
    ID3D11VertexShader* vs;
    ID3D11PixelShader* ps;
    ID3D11InputLayout* il;
    D3D11_BUFFER_DESC bd;
    sgui_widget* subview;
    UINT stride, offset;
    sgui_widget* button;
    ID3D11Buffer* vbo;
    sgui_widget* text;
    sgui_window* wnd;
    sgui_lib* lib;

    srand( time(NULL) );

    lib = sgui_init(NULL);

    /* create an ordinary window */
    wnd = sgui_window_create_simple(lib, NULL, 200, 200, SGUI_FIXED_SIZE);

    sgui_window_set_title( wnd, "D3D11 widget" );
    sgui_window_move_center( wnd );
    sgui_window_set_visible( wnd, SGUI_VISIBLE );

    /* create some widgets */
    text = sgui_label_create( 40, 130, "Direct3D\302\256 widget" );
    button = sgui_button_create( 10, 155, 75, 30, "Refresh", SGUI_BUTTON );

    /* create a subview widget. See gl2.c for further explanation */
    subview = sgui_subview_create( wnd, 10, 10, 180, 120,
                                   SGUI_DIRECT3D_11, NULL );

    subwindow = sgui_subview_get_window( subview );
    ctx = (sgui_d3d11_context*)sgui_window_get_context( subwindow );

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

    /* hook callbacks. See gl2.c for further explanation */
    sgui_subview_set_draw_callback( subview, d3dview_on_draw );
    sgui_event_connect( button, SGUI_BUTTON_OUT_EVENT,
                        sgui_subview_refresh, subview, SGUI_VOID );

    /* add widgets to the window */
    sgui_window_add_widget( wnd, text );
    sgui_window_add_widget( wnd, button );
    sgui_window_add_widget( wnd, subview );

    /* main loop */
    lib->main_loop(lib);

    /* clean up */
    ID3D11Buffer_Release( vbo );
    ID3D11InputLayout_Release( il );
    ID3D11VertexShader_Release( vs );
    ID3D11PixelShader_Release( ps );

    vsblob->lpVtbl->Release( vsblob );
    psblob->lpVtbl->Release( psblob );

    sgui_window_destroy( wnd );
    sgui_widget_destroy( subview );
    sgui_widget_destroy( text );
    sgui_widget_destroy( button );
    lib->destroy(lib);

    return 0;
}

