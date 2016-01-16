/*
 * d3d11_wm.c
 * This file is part of sgui
 *
 * Copyright (C) 2012 - David Oberhollenzer
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */
#include "d3d11_wm.h"
#include "../ctx_mesh.h"
#include "sgui_subwm_skin.h"



#if defined(SGUI_WINDOWS) && !defined(SGUI_NO_D3D11)
static void d3d11_wm_draw_gui( sgui_ctx_wm* super )
{
    (void)super;
    /* TODO */
}

static void d3d11_wm_destroy( sgui_ctx_wm* super )
{
    sgui_d3d11_wm* this = (sgui_d3d11_wm*)super;

    ID3D11SamplerState_Release( this->sampler );
    ID3D11ShaderResourceView_Release( this->view );
    ID3D11Texture2D_Release( this->skintex );
    free( this );
}



sgui_ctx_wm* d3d11_wm_create( sgui_window* wnd )
{
    D3D11_SHADER_RESOURCE_VIEW_DESC rvdesc;
    D3D11_SAMPLER_DESC sampdesc;
    D3D11_SUBRESOURCE_DATA data;
    unsigned int width, height;
    D3D11_TEXTURE2D_DESC desc;
    sgui_d3d11_context* ctx;
    sgui_subwm_skin* skin;
    sgui_d3d11_wm* this;
    sgui_ctx_wm* super;
    HRESULT status;

    ctx = (sgui_d3d11_context*)sgui_window_get_context( wnd );

    if( !ctx )
        return NULL;

    this = calloc( 1, sizeof(sgui_d3d11_wm) );
    super = (sgui_ctx_wm*)this;

    if( !this )
        return NULL;

    memset( &desc,     0, sizeof(desc)     );
    memset( &data,     0, sizeof(data)     );
    memset( &rvdesc,   0, sizeof(rvdesc)   );
    memset( &sampdesc, 0, sizeof(sampdesc) );

    skin = sgui_subwm_skin_get( );
    skin->get_ctx_skin_texture_size( skin, &width, &height );

    desc.Width                 = width;
    desc.Height                = height;
    desc.MipLevels             = 1;
    desc.ArraySize             = 1;
    desc.SampleDesc.Count      = 1;
    desc.Format                = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.Usage                 = D3D11_USAGE_IMMUTABLE;
    desc.BindFlags             = D3D11_BIND_SHADER_RESOURCE;

    data.pSysMem               = skin->get_ctx_skin_texture( skin );
    data.SysMemPitch           = width*4;

    rvdesc.Format              = DXGI_FORMAT_R8G8B8A8_UNORM;
    rvdesc.ViewDimension       = D3D11_SRV_DIMENSION_TEXTURE2D;
    rvdesc.Texture2D.MipLevels = 1;

    sampdesc.Filter            = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
    sampdesc.ComparisonFunc    = D3D11_COMPARISON_NEVER;
    sampdesc.AddressU          = D3D11_TEXTURE_ADDRESS_WRAP;
    sampdesc.AddressV          = D3D11_TEXTURE_ADDRESS_WRAP;
    sampdesc.AddressW          = D3D11_TEXTURE_ADDRESS_WRAP;

    status = ID3D11Device_CreateTexture2D( ctx->dev, &desc, &data,
                                           &this->skintex );

    if( status!=S_OK || !this->skintex )
        goto fail;

    status = ID3D11Device_CreateShaderResourceView( ctx->dev,
                                                    (void*)this->skintex,
                                                    &rvdesc, &this->view );

    if( status!=S_OK || !this->view )
        goto failtex;

    status = ID3D11Device_CreateSamplerState( ctx->dev, &sampdesc,
                                              &this->sampler );

    if( status!=S_OK || !this->sampler )
        goto failview;

    /* init */
    super->wnd      = wnd;
    super->draw_gui = d3d11_wm_draw_gui;
    super->destroy  = d3d11_wm_destroy;
    return super;
failview:
    ID3D11ShaderResourceView_Release( this->view );
failtex:
    ID3D11Texture2D_Release( this->skintex );
fail:
    free( this );
    return NULL;
}
#endif /* SGUI_WINDOWS && !SGUI_NO_D3D11 */

