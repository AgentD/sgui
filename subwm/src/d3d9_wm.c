/*
 * d3d9_wm.c
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
#include "d3d9_wm.h"
#include "sgui_subwm_skin.h"



#if defined(SGUI_WINDOWS) && !defined(SGUI_NO_D3D9)
static void d3d9_wm_draw_gui( sgui_ctx_wm* super )
{
    IDirect3DBaseTexture9* wndtex;
    sgui_d3d9_context* ctx;
    sgui_ctx_window* wnd;
    WINDOWVERTEX vb[6];
    D3DVIEWPORT9 vp;

    vp.X      = 0;
    vp.Y      = 0;
    vp.Width  = super->wnd->w;
    vp.Height = super->wnd->h;
    vp.MinZ   = 0.0f;
    vp.MaxZ   = 1.0f;

    ctx = (sgui_d3d9_context*)sgui_window_get_context( super->wnd );
    IDirect3DDevice9_BeginScene( ctx->device );
    IDirect3DDevice9_SetViewport( ctx->device, &vp );
    IDirect3DDevice9_SetFVF( ctx->device, WINDOWFVF );

    IDirect3DDevice9_SetRenderState(ctx->device,D3DRS_FILLMODE,
                                    D3DFILL_SOLID);
    IDirect3DDevice9_SetRenderState(ctx->device,D3DRS_SRCBLEND,
                                    D3DBLEND_SRCALPHA);
    IDirect3DDevice9_SetRenderState(ctx->device,D3DRS_DESTBLEND,
                                    D3DBLEND_INVSRCALPHA);
    IDirect3DDevice9_SetRenderState(ctx->device,D3DRS_ALPHABLENDENABLE,TRUE);

    IDirect3DDevice9_SetTextureStageState( ctx->device, 0, D3DTSS_COLOROP,
                                           D3DTOP_MODULATE );
    IDirect3DDevice9_SetTextureStageState( ctx->device, 0, D3DTSS_COLORARG1,
                                           D3DTA_TEXTURE );
    IDirect3DDevice9_SetTextureStageState( ctx->device, 0, D3DTSS_COLORARG2,
                                           D3DTA_DIFFUSE );
    IDirect3DDevice9_SetTextureStageState( ctx->device, 0, D3DTSS_ALPHAOP,
                                           D3DTOP_SELECTARG1 );

    vb[0].z   = vb[1].z   = vb[2].z   = vb[5].z   = 0.0f;
    vb[0].rhw = vb[1].rhw = vb[2].rhw = vb[5].rhw = 1.0f;
    vb[0].u = 0.0f;
    vb[0].v = 0.0f;
    vb[1].u = 1.0f;
    vb[1].v = 0.0f;
    vb[2].u = 1.0f;
    vb[2].v = 1.0f;
    vb[5].u = 0.0f;
    vb[5].v = 1.0f;

    for( wnd=super->list; wnd!=NULL; wnd=wnd->next )
    {
        wndtex = sgui_ctx_window_get_texture( (sgui_window*)wnd );
        IDirect3DDevice9_SetTexture( ctx->device, 0, wndtex );

        vb[0].x = vb[5].x = wnd->super.x;
        vb[0].y = vb[1].y = wnd->super.y;
        vb[1].x = vb[2].x = wnd->super.x + wnd->super.w-1;
        vb[2].y = vb[5].y = wnd->super.y + wnd->super.h-1;
        vb[3] = vb[0];
        vb[4] = vb[2];

        IDirect3DDevice9_DrawPrimitiveUP( ctx->device, D3DPT_TRIANGLELIST, 2,
                                          vb, sizeof(WINDOWVERTEX) );
    }

    IDirect3DDevice9_SetTexture( ctx->device, 0, NULL );
    IDirect3DDevice9_EndScene( ctx->device );
}

static void d3d9_wm_destroy( sgui_ctx_wm* super )
{
    (void)super;
}



sgui_ctx_wm* d3d9_wm_create( sgui_window* wnd )
{
    sgui_d3d9_wm* this = malloc( sizeof(sgui_d3d9_wm) );
    sgui_ctx_wm* super = (sgui_ctx_wm*)this;
    unsigned int width, height;
    sgui_d3d9_context* ctx;
    sgui_subwm_skin* skin;
    D3DLOCKED_RECT locked;
    HRESULT status;
    RECT rect;

    if( !this )
        return NULL;

    ctx = (sgui_d3d9_context*)sgui_window_get_context( wnd );
    memset( this, 0, sizeof(sgui_d3d9_wm) );

    /* create skin texture */
    skin = sgui_subwm_skin_get( );
    skin->get_ctx_skin_texture_size( skin, &width, &height );
    rect.left = rect.top = 0;
    rect.right = width - 1;
    rect.bottom = height - 1;

    status = IDirect3DDevice9_CreateTexture( ctx->device, width, height,
                                             1, 0, D3DFMT_A8R8G8B8,
                                             D3DPOOL_MANAGED,
                                             &this->skintex, NULL );

    if( status!=D3D_OK || !this->skintex )
    {
        free( this );
        return NULL;
    }

    /* upload skin texture */
    status = IDirect3DTexture9_LockRect(this->skintex, 0, &locked, &rect, 0);

    if( status != D3D_OK )
    {
        IDirect3DTexture9_Release( this->skintex );
        free( this );
        return NULL;
    }

    memcpy( locked.pBits, skin->get_ctx_skin_texture(skin), width*height*4 );

    /* init */
    super->wnd      = wnd;
    super->draw_gui = d3d9_wm_draw_gui;
    super->destroy  = d3d9_wm_destroy;
    return super;
}
#endif /* SGUI_WINDOWS && !SGUI_NO_D3D9 */

