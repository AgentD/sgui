/*
    This file is part of the sgui samples collection. I, David Oberhollenzer,
    author of this file hereby place the contents of this file into
    the public domain.
 */
/*
    This small programm is supposed to demonstrate how to use the sgui texture
    canvas with Direct3D 11.
 */
#include "sgui.h"
#include "sgui_d3d11.h"
#include "sgui_tex_canvas.h"

#include <d3dcompiler.h>
#include <string.h>
#include <stdio.h>
#include <math.h>



/* vertex and pixel shaders */
const char* vsh =
"cbuffer cbo : register(b0)\n"
"{\n"
"    float4x4 view;\n"
"    float4x4 projection;\n"
"};\n"
"\n"
"struct PS_INPUT\n"
"{\n"
"    float4 Pos : SV_POSITION;\n"
"    float2 Tex : TEXCOORD0;\n"
"};\n"
"\n"
"PS_INPUT main( float4 Pos : POSITION, float2 Tex : TEXCOORD )\n"
"{\n"
"    PS_INPUT output = (PS_INPUT)0;"
"    output.Pos = mul( view, Pos );\n"
"    output.Pos = mul( projection, output.Pos );\n"
"    output.Tex = Tex;"
"    return output;\n"
"}\n";

const char* psh =
"Texture2D txDiffuse : register( t0 );\n"
"SamplerState samLinear : register( s0 );\n"
"\n"
"struct PS_INPUT\n"
"{\n"
"    float4 Pos : SV_POSITION;\n"
"    float2 Tex : TEXCOORD0;\n"
"};\n"
"\n"
"float4 main( PS_INPUT input ) : SV_Target\n"
"{\n"
"    float4 color = txDiffuse.Sample( samLinear, input.Tex );"
"    return float4( lerp( float3(0.3,0.3,0.3), color.rgb, color.a ), 1.0 );\n"
"}\n";

/* input layout */
D3D11_INPUT_ELEMENT_DESC layout[] =
{
{"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0, 0,D3D11_INPUT_PER_VERTEX_DATA,0},
{"TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT,   0,12,D3D11_INPUT_PER_VERTEX_DATA,0}
};

#define LAYOUTCOUNT (sizeof(layout)/sizeof(layout[0]))

/* vertex data */
float vertices[] =
{
    /* front */
    -1.0f,  1.0f, 1.0f, 0.0f, 0.0f,
     1.0f, -1.0f, 1.0f, 1.0f, 1.0f,
    -1.0f, -1.0f, 1.0f, 0.0f, 1.0f,
    -1.0f,  1.0f, 1.0f, 0.0f, 0.0f,
     1.0f,  1.0f, 1.0f, 1.0f, 0.0f,
     1.0f, -1.0f, 1.0f, 1.0f, 1.0f,

    /* back */
    -1.0f,  1.0f, -1.0f, 1.0f, 0.0f,
     1.0f, -1.0f, -1.0f, 0.0f, 1.0f,
    -1.0f, -1.0f, -1.0f, 1.0f, 1.0f,
    -1.0f,  1.0f, -1.0f, 1.0f, 0.0f,
     1.0f,  1.0f, -1.0f, 0.0f, 0.0f,
     1.0f, -1.0f, -1.0f, 0.0f, 1.0f,

    /* left */
     1.0f, -1.0f,  1.0f, 0.0f, 1.0f,
     1.0f,  1.0f, -1.0f, 1.0f, 0.0f,
     1.0f, -1.0f, -1.0f, 1.0f, 1.0f,
     1.0f, -1.0f,  1.0f, 0.0f, 1.0f,
     1.0f,  1.0f,  1.0f, 0.0f, 0.0f,
     1.0f,  1.0f, -1.0f, 1.0f, 0.0f,

    /* right */
    -1.0f, -1.0f,  1.0f, 1.0f, 1.0f,
    -1.0f,  1.0f, -1.0f, 0.0f, 0.0f,
    -1.0f, -1.0f, -1.0f, 0.0f, 1.0f,
    -1.0f, -1.0f,  1.0f, 1.0f, 1.0f,
    -1.0f,  1.0f,  1.0f, 1.0f, 0.0f,
    -1.0f,  1.0f, -1.0f, 0.0f, 0.0f
};




static void perspective( float* m, float fov_deg, float aspect,
                         float N, float F )
{
    float f, iNF;

    f   = 1.0 / tan( fov_deg * (3.14159265358/180.0) * 0.5 );
    iNF = 1.0 / (N - F);

    m[0]=f/aspect; m[4]=0.0f; m[ 8]=0;       m[12]=0;
    m[1]=0.0f;     m[5]=f;    m[ 9]=0;       m[13]=0;
    m[2]=0.0f;     m[6]=0.0f; m[10]=F*iNF;   m[14]=F*N*iNF;
    m[3]=0.0f;     m[7]=0.0f; m[11]=-1;      m[15]=0;
}

static void transform( float* m, float a )
{
    float c = cos(a), s = sin(a);

    m[0]=c;    m[4]=0.0f; m[ 8]=s;    m[12]=  0.0f;
    m[1]=0.0f; m[5]=1.0f; m[ 9]=0.0f; m[13]=  0.0f;
    m[2]=-s;   m[6]=0.0f; m[10]=c;    m[14]= -5.0f;
    m[3]=0.0f; m[7]=0.0f; m[11]=0.0f; m[15]=  1.0f;
}




int main( void )
{
    float bg[4] = { 0.0f, 0.2f, 0.4f, 1.0f }, matrices[32], a = 0.0f;
    D3D11_SHADER_RESOURCE_VIEW_DESC rvdesc;
    sgui_widget *check2, *check, *butt;
    ID3D11ShaderResourceView* texview;
    ID3D11DepthStencilState* dsstate;
    D3D11_SUBRESOURCE_DATA InitData;
    D3D11_DEPTH_STENCIL_DESC dsdesc;
    ID3D11RasterizerState* rsstate;
    D3D11_RASTERIZER_DESC rsdesc;
    sgui_window_description desc;
    ID3D11SamplerState* sampler;
    D3D11_SAMPLER_DESC sampdesc;
    ID3DBlob *vsblob, *psblob;
    unsigned char bgcolor[4];
    sgui_d3d11_context* ctx;
    ID3D11Buffer *vbo, *ubo;
    sgui_canvas* texcanvas;
    ID3D11VertexShader* vs;
    ID3D11PixelShader* ps;
    ID3D11InputLayout* il;
    D3D11_BUFFER_DESC bd;
    ID3D11Texture2D* tex;
    UINT stride, offset;
    sgui_window* wnd;

    sgui_init( );

    /*************** create a window ***************/
    desc.parent         = NULL;
    desc.share          = NULL;
    desc.width          = 640;
    desc.height         = 480;
    desc.flags          = SGUI_DOUBLEBUFFERED;
    desc.backend        = SGUI_DIRECT3D_11;
    desc.bits_per_pixel = 32;
    desc.depth_bits     = 16;
    desc.stencil_bits   = 0;
    desc.samples        = 4;

    wnd = sgui_window_create_desc( &desc );

    if( !wnd )
    {
        fprintf( stderr, "Could not create Direct3D 11 window!\n" );
        return -1;
    }

    /* make the window visible, get the context */
    sgui_window_set_title( wnd, "Direct3D 11 Sample" );
    sgui_window_move_center( wnd );
    sgui_window_set_visible( wnd, SGUI_VISIBLE );

    ctx = (sgui_d3d11_context*)sgui_window_get_context( wnd );

    /*************** compile shaders ***************/
    if( D3DCompile( vsh, strlen(vsh), NULL, NULL, NULL, "main", "vs_4_0",
                    0, 0, &vsblob, NULL )!=S_OK )
    {
        fputs( "Failed to compile veretx shader!\n", stderr );
        return -1;
    }

    if( D3DCompile( psh, strlen(psh), NULL, NULL, NULL, "main", "ps_4_0",
                    0, 0, &psblob, NULL )!=S_OK )
    {
        fputs( "Failed to compile veretx shader!\n", stderr );
        return -1;
    }

    ID3D11Device_CreateVertexShader( ctx->dev,
                                     vsblob->lpVtbl->GetBufferPointer(vsblob),
                                     vsblob->lpVtbl->GetBufferSize(vsblob),
                                     NULL, &vs );

    ID3D11Device_CreatePixelShader( ctx->dev,
                                    psblob->lpVtbl->GetBufferPointer(psblob),
                                    psblob->lpVtbl->GetBufferSize(psblob),
                                    NULL, &ps );

    /*************** create input layout ***************/
    ID3D11Device_CreateInputLayout( ctx->dev, layout, LAYOUTCOUNT,
                                    vsblob->lpVtbl->GetBufferPointer(vsblob),
                                    vsblob->lpVtbl->GetBufferSize(vsblob),
                                    &il );

    /*************** create vertex buffer ***************/
    memset( &bd, 0, sizeof(bd) );
    bd.Usage     = D3D11_USAGE_DEFAULT;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.ByteWidth = sizeof(vertices);

    memset( &InitData, 0, sizeof(InitData) );
    InitData.pSysMem = vertices;

    ID3D11Device_CreateBuffer( ctx->dev, &bd, &InitData, &vbo );

    /*************** configure depth and stencil states ***************/
    memset( &dsdesc, 0, sizeof(dsdesc) );
    dsdesc.DepthEnable = TRUE;
    dsdesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    dsdesc.DepthFunc = D3D11_COMPARISON_LESS;

    memset( &rsdesc, 0, sizeof(rsdesc) );
    rsdesc.FillMode = D3D11_FILL_SOLID;
    rsdesc.CullMode = D3D11_CULL_NONE;

    ID3D11Device_CreateDepthStencilState( ctx->dev, &dsdesc, &dsstate );
    ID3D11Device_CreateRasterizerState( ctx->dev, &rsdesc, &rsstate );
    ID3D11DeviceContext_RSSetState( ctx->ctx, rsstate );
    ID3D11DeviceContext_OMSetDepthStencilState( ctx->ctx, dsstate, 0 );

    /*************** configure input assembler ***************/
    ID3D11DeviceContext_IASetInputLayout( ctx->ctx, il );

    stride = 5 * sizeof(float);
    offset = 0;
    ID3D11DeviceContext_IASetVertexBuffers( ctx->ctx, 0, 1, &vbo,
                                            &stride, &offset );

    ID3D11DeviceContext_IASetPrimitiveTopology(
                            ctx->ctx, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

    /*************** create constant buffer ***************/
    memset( &bd, 0, sizeof(bd) );
    bd.Usage     = D3D11_USAGE_DEFAULT;
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bd.ByteWidth = 32*sizeof(float);

    memset( &InitData, 0, sizeof(InitData) );
    InitData.pSysMem = matrices;
    transform( matrices, 0.0f );
    perspective( matrices+16, 60.0f, 4.0f/3.0f, 0.1f, 100.0f );

    if( ID3D11Device_CreateBuffer( ctx->dev, &bd, &InitData, &ubo )!=S_OK )
        return -1;

    /*************** create a texture canvas ***************/
    texcanvas = sgui_tex_canvas_create( wnd, sgui_window_get_context( wnd ),
                                        128, 128 );
    if( !texcanvas )
        return -1;

    butt = sgui_button_create( 10, 10, 60, 25, "Button", 0 );
    check = sgui_checkbox_create( 10, 40, "Direct3D 11" );
    check2 = sgui_checkbox_create( 10, 65, "Texture" );

    sgui_button_set_state( check, 1 );
    sgui_button_set_state( check2, 1 );

    sgui_widget_add_child( &texcanvas->root, butt );
    sgui_widget_add_child( &texcanvas->root, check );
    sgui_widget_add_child( &texcanvas->root, check2 );

    /* get window background color and clear canvas */
    memcpy( bgcolor, sgui_skin_get( )->window_color, 4 );

    sgui_canvas_begin( texcanvas, NULL );
    sgui_canvas_clear( texcanvas, NULL );
    sgui_canvas_end( texcanvas );

    /* redirect keyboard events to the texture canvas */
    sgui_event_connect( wnd, SGUI_KEY_PRESSED_EVENT,
                        sgui_canvas_send_window_event, texcanvas,
                        SGUI_FROM_EVENT, SGUI_EVENT );

    sgui_event_connect( wnd, SGUI_KEY_RELEASED_EVENT,
                        sgui_canvas_send_window_event, texcanvas,
                        SGUI_FROM_EVENT, SGUI_EVENT );

    sgui_event_connect( wnd, SGUI_CHAR_EVENT,
                        sgui_canvas_send_window_event, texcanvas,
                        SGUI_FROM_EVENT, SGUI_EVENT );

    /*************** create resource view and sampler ***************/
    tex = sgui_tex_canvas_get_texture( texcanvas );

    memset( &rvdesc, 0, sizeof(rvdesc) );
    memset( &sampdesc, 0, sizeof(sampdesc) );

    rvdesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    rvdesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    rvdesc.Texture2D.MostDetailedMip = 0;
    rvdesc.Texture2D.MipLevels = 1;

    sampdesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sampdesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    sampdesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    sampdesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    sampdesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sampdesc.MinLOD = 0;
    sampdesc.MaxLOD = D3D11_FLOAT32_MAX;

    ID3D11Device_CreateShaderResourceView( ctx->dev, (ID3D11Resource*)tex,
                                           &rvdesc, &texview );

    ID3D11Device_CreateSamplerState( ctx->dev, &sampdesc, &sampler );

    /*************** main loop ***************/
    while( sgui_main_loop_step( ) )
    {
        /* redraw dirty canvas widgets */
        sgui_canvas_redraw_widgets( texcanvas, 1 );

        /* clear framebuffer */
        ID3D11DeviceContext_ClearRenderTargetView( ctx->ctx,
                                                   ctx->backbuffer, bg);
        ID3D11DeviceContext_ClearDepthStencilView( ctx->ctx, ctx->dsv,
                                                   D3D11_CLEAR_DEPTH,
                                                   1.0f, 0x00 );

        /* update constant buffer */
        transform( matrices, a );
        a += 0.01f;

        ID3D11DeviceContext_UpdateSubresource( ctx->ctx, (ID3D11Resource*)ubo,
                                               0, 0, matrices, 0, 0 );

        /* draw frame */
        ID3D11DeviceContext_VSSetShader( ctx->ctx, vs, NULL, 0 );
        ID3D11DeviceContext_VSSetConstantBuffers( ctx->ctx, 0, 1, &ubo );
        ID3D11DeviceContext_PSSetShader( ctx->ctx, ps, NULL, 0 );
        ID3D11DeviceContext_PSSetShaderResources( ctx->ctx, 0, 1, &texview );
        ID3D11DeviceContext_PSSetSamplers( ctx->ctx, 0, 1, &sampler );

        ID3D11DeviceContext_Draw( ctx->ctx,
                                  sizeof(vertices)/(sizeof(float)*5), 0 );

        /* swap front and back buffer */
        sgui_window_swap_buffers( wnd );
    }

    /*************** clean up ***************/
    sgui_canvas_destroy( texcanvas );
    sgui_widget_destroy( check2 );
    sgui_widget_destroy( check );
    sgui_widget_destroy( butt );

    ID3D11SamplerState_Release( sampler );
    ID3D11ShaderResourceView_Release( texview );
    ID3D11Buffer_Release( vbo );
    ID3D11InputLayout_Release( il );
    ID3D11VertexShader_Release( vs );
    ID3D11PixelShader_Release( ps );

    vsblob->lpVtbl->Release( vsblob );
    psblob->lpVtbl->Release( psblob );
    dsstate->lpVtbl->Release( dsstate );
    rsstate->lpVtbl->Release( rsstate );

    sgui_window_destroy( wnd );
    sgui_deinit( );

    return 0;
}

