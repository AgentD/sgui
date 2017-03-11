/*
 * d3d11_canvas.c
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
#include "d3d11_canvas.h"



#if !defined(SGUI_NO_D3D11) && defined(SGUI_WINDOWS)
void* d3d11_canvas_get_texture( sgui_canvas* this )
{
    return ((sgui_d3d11_canvas*)this)->tex;
}

void d3d11_canvas_destroy( sgui_canvas* super )
{
    sgui_d3d11_canvas* this = (sgui_d3d11_canvas*)super;

    IUnknown_Release( (IUnknown*)this->tex );
    IUnknown_Release( (IUnknown*)this->ctx );
    free( this->buffer );
    free( this );
}

int d3d11_canvas_begin( sgui_canvas* this, const sgui_rect* r )
{
    ((sgui_d3d11_canvas*)this)->locked = *r;
    return 1;
}

void d3d11_canvas_end( sgui_canvas* super )
{
    sgui_d3d11_canvas* this = (sgui_d3d11_canvas*)super;
    D3D11_MAPPED_SUBRESOURCE mapped;
    HRESULT status;

    status = ID3D11DeviceContext_Map( this->ctx, (ID3D11Resource*)this->tex,
                                      0, D3D11_MAP_WRITE_DISCARD, 0,
                                      &mapped );

    if( status!=S_OK )
        return;

    memcpy( mapped.pData, this->buffer, super->width*super->height*4 );

    ID3D11DeviceContext_Unmap( this->ctx, (ID3D11Resource*)this->tex, 0 );
}



sgui_canvas* sgui_d3d11_canvas_create( sgui_context* ctx,
                                       unsigned width, unsigned int height )
{
    sgui_d3d11_canvas* this = calloc( 1, sizeof(sgui_d3d11_canvas) );
    sgui_d3d11_context* ctx11 = (sgui_d3d11_context*)ctx;
    D3D11_TEXTURE2D_DESC desc;
    HRESULT status;

    if( !this )
        return NULL;

    /* create in-memory drawing buffer */
    this->buffer = malloc( width*height*4 );

    if( !this->buffer )
    {
        free( this );
        return NULL;
    }

    /* initialize base structure */
    if( !sgui_memory_canvas_init( (sgui_canvas*)this, this->buffer,
                                  width, height, SGUI_RGBA8, 0 ) )
    {
        free( this->buffer );
        free( this );
        return NULL;
    }

    /* create a texture */
    desc.Width              = width;
    desc.Height             = height;
    desc.MipLevels          = 1;
    desc.ArraySize          = 1;
    desc.Format             = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count   = 1;
    desc.SampleDesc.Quality = 0;
    desc.Usage              = D3D11_USAGE_DYNAMIC;
    desc.BindFlags          = D3D11_BIND_SHADER_RESOURCE;
    desc.CPUAccessFlags     = D3D11_CPU_ACCESS_WRITE;
    desc.MiscFlags          = 0;

    status = ID3D11Device_CreateTexture2D(ctx11->dev,&desc,NULL,&this->tex);

    if( status != S_OK )
    {
        free( this->buffer );
        free( this );
        return NULL;
    }

    /* hook callbacks */
    ((sgui_tex_canvas*)this)->get_texture = d3d11_canvas_get_texture;
    ((sgui_canvas*)this)->destroy = d3d11_canvas_destroy;
    ((sgui_canvas*)this)->begin = d3d11_canvas_begin;
    ((sgui_canvas*)this)->end = d3d11_canvas_end;
    this->ctx = ctx11->ctx;

    IUnknown_AddRef( (IUnknown*)this->ctx );

    return (sgui_canvas*)this;
}
#endif

