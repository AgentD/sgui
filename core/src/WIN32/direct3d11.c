/*
 * direct3d11.c
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
#define SGUI_BUILDING_DLL
#include "direct3d11.h"
#include "window.h"

#include <stdio.h>


#ifndef SGUI_NO_D3D11
typedef HRESULT (__stdcall * CREATEDEVSWAPCHAIN)( IDXGIAdapter*,
                                                  D3D_DRIVER_TYPE,
                                                  HMODULE, UINT,
                                                  const D3D_FEATURE_LEVEL*,
                                                  UINT, UINT,
                                                  const DXGI_SWAP_CHAIN_DESC*,
                                                  IDXGISwapChain**,
                                                  ID3D11Device**,
                                                  D3D_FEATURE_LEVEL*,
                                                  ID3D11DeviceContext** );

static HINSTANCE libd3d11 = 0;
static size_t refcount = 0;
static CREATEDEVSWAPCHAIN CreateDeviceAndSwapChain = NULL;

static const D3D_FEATURE_LEVEL levels[] =
{
    D3D_FEATURE_LEVEL_11_0,
    D3D_FEATURE_LEVEL_10_1,
    D3D_FEATURE_LEVEL_10_0,
    D3D_FEATURE_LEVEL_9_3,
    D3D_FEATURE_LEVEL_9_2,
    D3D_FEATURE_LEVEL_9_1
};

static const D3D_DRIVER_TYPE drivers[] =
{
    D3D_DRIVER_TYPE_HARDWARE,
    D3D_DRIVER_TYPE_WARP,
    D3D_DRIVER_TYPE_REFERENCE
};

#define NUMDRIVERS (sizeof(drivers)/sizeof(drivers[0]))
#define NUMLEVELS (sizeof(levels)/sizeof(levels[0]))



static int load_d3d11( void )
{
    sgui_internal_lock_mutex( );

    if( libd3d11 )
    {
        ++refcount;
    }
    else
    {
        libd3d11 = LoadLibrary( "d3d11.dll" );

        if( !libd3d11 )
            goto fail;

        CreateDeviceAndSwapChain = (CREATEDEVSWAPCHAIN)
        GetProcAddress( libd3d11, "D3D11CreateDeviceAndSwapChain" );

        if( !CreateDeviceAndSwapChain )
        {
            FreeLibrary( libd3d11 );
            libd3d11 = 0;
            goto fail;
        }

        refcount = 1;
    }

    sgui_internal_unlock_mutex( );
    return 1;
fail:
    sgui_internal_unlock_mutex( );
    return 0;
}

static void release_d3d11( void )
{
    sgui_internal_lock_mutex( );

    if( libd3d11 && (--refcount)==0 )
    {
        FreeLibrary( libd3d11 );
        libd3d11 = 0;
        CreateDeviceAndSwapChain = NULL;
    }

    sgui_internal_unlock_mutex( );
}

/****************************************************************************/

static void context_d3d11_destroy( sgui_context* super )
{
    sgui_d3d11_context* this = (sgui_d3d11_context*)super;

    if( this )
    {
        if( this->dsv )
            ID3D11DepthStencilView_Release( this->dsv );

        if( this->ds_texture )
            ID3D11Texture2D_Release( this->ds_texture );

        if( this->backbuffer )
            ID3D11RenderTargetView_Release( this->backbuffer );

        if( this->ctx )
            ID3D11DeviceContext_Release( this->ctx );

        if( this->dev )
            ID3D11Device_Release( this->dev );

        if( this->swapchain )
            IDXGISwapChain_Release( this->swapchain );

        free( this );
        release_d3d11( );
    }
}

static void* context_d3d11_get_internal( sgui_context* this )
{
    return this ? ((sgui_d3d11_context*)this)->dev : NULL;
}

void d3d11_swap_buffers( sgui_window* wnd )
{
    sgui_d3d11_context* this = (sgui_d3d11_context*)wnd->ctx.ctx;

    IDXGISwapChain_Present( this->swapchain, this->syncrate ? 1 : 0, 0 );
}

void d3d11_set_vsync( sgui_window* wnd, int interval )
{
    sgui_d3d11_context* this = (sgui_d3d11_context*)wnd->ctx.ctx;

    this->syncrate = interval ? 1 : 0;
}

void d3d11_resize( sgui_context* super )
{
    sgui_d3d11_context* this = (sgui_d3d11_context*)super;
    D3D11_DEPTH_STENCIL_VIEW_DESC ds_view_desc;
    D3D11_TEXTURE2D_DESC ds_tex_desc;
    ID3D11Texture2D* pBackBuffer;
    ID3D11RenderTargetView* tv;
    DXGI_SWAP_CHAIN_DESC scd;
    D3D11_VIEWPORT viewport;
    int is_bound, has_ds;

    /* check if the backbuffer is currently bound, unbind if so */
    ID3D11DeviceContext_OMGetRenderTargets( this->ctx, 1, &tv, NULL );

    is_bound = (tv == this->backbuffer);
    has_ds = (this->dsv != NULL);

    if( tv )
        ID3D11RenderTargetView_Release( tv );

    if( is_bound )
        ID3D11DeviceContext_OMSetRenderTargets( this->ctx, 0, NULL, NULL );

    /* destroy old buffers, get description first */
    IDXGISwapChain_GetDesc( this->swapchain, &scd );
    ID3D11RenderTargetView_Release( this->backbuffer );

    if( has_ds )
    {
        ID3D11Texture2D_GetDesc( this->ds_texture, &ds_tex_desc );

        ID3D11Texture2D_Release( this->ds_texture );
        ID3D11DepthStencilView_Release( this->dsv );
    }

    /* resize */
    IDXGISwapChain_ResizeBuffers( this->swapchain, scd.BufferCount,
                                  this->wnd->w, this->wnd->h,
                                  scd.BufferDesc.Format,
                                  scd.Flags );

    /* get back buffer */
    IDXGISwapChain_GetBuffer( this->swapchain, 0, &IID_ID3D11Texture2D,
                              (LPVOID*)&pBackBuffer );

    ID3D11Device_CreateRenderTargetView( this->dev,
                                         (ID3D11Resource*)pBackBuffer,
                                         NULL, &this->backbuffer );

    ID3D11Texture2D_Release( pBackBuffer );

    /* recreate detph/stencil buffer */
    if( has_ds )
    {
        ds_tex_desc.Width = this->wnd->w;
        ds_tex_desc.Height = this->wnd->h;

        memset( &ds_view_desc, 0, sizeof(ds_view_desc) );
        ds_view_desc.Format = ds_tex_desc.Format;
        ds_view_desc.ViewDimension = scd.SampleDesc.Count>0 ?
                                     D3D11_DSV_DIMENSION_TEXTURE2DMS :
                                     D3D11_DSV_DIMENSION_TEXTURE2D;

        ID3D11Device_CreateTexture2D( this->dev, &ds_tex_desc, NULL,
                                      &this->ds_texture );

        ID3D11Device_CreateDepthStencilView( this->dev,
                                             (ID3D11Resource*)
                                             this->ds_texture,
                                             &ds_view_desc, &this->dsv );
    }

    /* rebind if it was bound */
    if( is_bound )
    {
        ID3D11DeviceContext_OMSetRenderTargets( this->ctx, 1,
                                                &this->backbuffer,
                                                this->dsv );

        memset( &viewport, 0, sizeof(D3D11_VIEWPORT) );
        viewport.Width    = this->wnd->w;
        viewport.Height   = this->wnd->h;
        viewport.MinDepth = 0.0f;
        viewport.MaxDepth = 1.0f;

        ID3D11DeviceContext_RSSetViewports( this->ctx, 1, &viewport );
    }
}

sgui_context* d3d11_context_create( sgui_window* wnd,
                                    const sgui_window_description* desc )
{
    D3D11_DEPTH_STENCIL_VIEW_DESC ds_view_desc;
    D3D11_TEXTURE2D_DESC ds_tex_desc;
    ID3D11Texture2D* pBackBuffer;
    DXGI_SWAP_CHAIN_DESC scd;
    sgui_d3d11_context* this;
    D3D11_VIEWPORT viewport;
    sgui_context* super;
    HRESULT hr;
    size_t i;

    if( !load_d3d11( ) )
        return NULL;

    /* allocate memory for the context object */
    this = calloc( 1, sizeof(sgui_d3d11_context) );
    super = (sgui_context*)this;

    if( !this )
    {
        release_d3d11( );
        return NULL;
    }

    /* setup swap chain parameters */
    memset( &scd, 0, sizeof(scd) );

    scd.BufferDesc.RefreshRate.Numerator = 60;
    scd.BufferDesc.RefreshRate.Denominator = 1;
    scd.BufferDesc.Format = desc->bits_per_pixel==16 ?
                            DXGI_FORMAT_B5G6R5_UNORM :
                            DXGI_FORMAT_R8G8B8A8_UNORM;
    scd.SampleDesc.Count = desc->samples>0 ? desc->samples : 1;
    scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    scd.BufferCount = (desc->flags & SGUI_DOUBLEBUFFERED) ? 2 : 1;
    scd.OutputWindow = TO_W32(wnd)->hWnd;
    scd.Windowed = TRUE;
    scd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    scd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

    /* try to create a swapchain and a device */
    for( i=0; i<NUMDRIVERS; ++i )
    {
        hr = CreateDeviceAndSwapChain( NULL, drivers[i], NULL, 0,
                                       levels, NUMLEVELS, D3D11_SDK_VERSION,
                                       &scd, &this->swapchain, &this->dev,
                                       NULL, &this->ctx );

        if( hr >= 0 )
            break;
    }

    if( hr < 0 )
        goto fail;

    /* create render target view from the backbuffer */
    hr = IDXGISwapChain_GetBuffer( this->swapchain, 0, &IID_ID3D11Texture2D,
                                   (LPVOID*)&pBackBuffer );

    if( hr < 0 )
        goto fail;

    hr = ID3D11Device_CreateRenderTargetView( this->dev,
                                              (ID3D11Resource*)pBackBuffer,
                                              NULL, &this->backbuffer );

    ID3D11Texture2D_Release( pBackBuffer );

    if( hr < 0 )
        goto fail;

    /* create a depth/stencil buffer if requested */
    if( desc->depth_bits>0 || desc->stencil_bits>0 )
    {
        /* Create depth stencil texture */
        memset( &ds_tex_desc, 0, sizeof(ds_tex_desc) );

        ds_tex_desc.Width              = desc->width;
        ds_tex_desc.Height             = desc->height;
        ds_tex_desc.MipLevels          = 1;
        ds_tex_desc.ArraySize          = 1;
        ds_tex_desc.SampleDesc.Count   = scd.SampleDesc.Count;
        ds_tex_desc.Usage              = D3D11_USAGE_DEFAULT;
        ds_tex_desc.BindFlags          = D3D11_BIND_DEPTH_STENCIL;

        if( desc->stencil_bits )
        {
            ds_tex_desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        }
        else
        {
            ds_tex_desc.Format = desc->depth_bits<=16 ?
                                 DXGI_FORMAT_D16_UNORM :
                                 DXGI_FORMAT_D32_FLOAT;
        }

        hr = ID3D11Device_CreateTexture2D( this->dev, &ds_tex_desc, NULL,
                                           &this->ds_texture );

        if( hr < 0 )
            goto fail;

        /* create depth stencil view for the texture */
        memset( &ds_view_desc, 0, sizeof(ds_view_desc) );

        ds_view_desc.Format        = ds_tex_desc.Format;
        ds_view_desc.ViewDimension = desc->samples>0 ?
                                     D3D11_DSV_DIMENSION_TEXTURE2DMS :
                                     D3D11_DSV_DIMENSION_TEXTURE2D;

        hr = ID3D11Device_CreateDepthStencilView( this->dev,
                                                  (ID3D11Resource*)
                                                  this->ds_texture,
                                                  &ds_view_desc, &this->dsv );

        if( hr < 0 )
            goto fail;
    }

    /* "activate" the render target view & depth/stencil view */
    ID3D11DeviceContext_OMSetRenderTargets( this->ctx, 1,
                                            &this->backbuffer, this->dsv );

    /* set viewport */
    memset( &viewport, 0, sizeof(D3D11_VIEWPORT) );

    viewport.Width    = desc->width;
    viewport.Height   = desc->height;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;

    ID3D11DeviceContext_RSSetViewports( this->ctx, 1, &viewport );

    /* finish initialisation */
    this->wnd = wnd;

    super->destroy      = context_d3d11_destroy;
    super->get_internal = context_d3d11_get_internal;

    return super;
fail:
    context_d3d11_destroy( super );
    return NULL;
}
#endif /* SGUI_NO_D3D11 */

