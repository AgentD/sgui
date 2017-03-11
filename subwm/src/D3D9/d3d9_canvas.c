/*
 * d3d9_canvas.c
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
#include "d3d9_canvas.h"



#if !defined(SGUI_NO_D3D9) && defined(SGUI_WINDOWS)
void* d3d9_canvas_get_texture( sgui_canvas* this )
{
    return ((sgui_d3d9_canvas*)this)->tex;
}

void d3d9_canvas_destroy( sgui_canvas* this )
{
    IDirect3DTexture9_Release( ((sgui_d3d9_canvas*)this)->tex );
    free( this );
}

int d3d9_canvas_begin( sgui_canvas* super, const sgui_rect* r )
{
    sgui_d3d9_canvas* this = (sgui_d3d9_canvas*)super;
    D3DLOCKED_RECT lr;
    HRESULT status;
    RECT wr;

    wr.left   = r->left;
    wr.top    = r->top;
    wr.right  = r->right;
    wr.bottom = r->bottom;

    status = IDirect3DTexture9_LockRect( this->tex, 0, &lr, &wr, 0 );

    if( status!=D3D_OK )
        return 0;

    ((sgui_mem_canvas*)this)->data = lr.pBits;
    ((sgui_mem_canvas*)this)->startx = r->left;
    ((sgui_mem_canvas*)this)->starty = r->top;
    ((sgui_mem_canvas*)this)->pitch = lr.Pitch;
    return 1;
}

void d3d9_canvas_end( sgui_canvas* super )
{
    sgui_d3d9_canvas* this = (sgui_d3d9_canvas*)super;

    IDirect3DTexture9_UnlockRect( this->tex, 0 );
}



sgui_canvas* sgui_d3d9_canvas_create( sgui_context* ctx,
                                      unsigned width, unsigned int height )
{
    sgui_d3d9_canvas* this = calloc( 1, sizeof(sgui_d3d9_canvas) );
    sgui_d3d9_context* ctx9 = (sgui_d3d9_context*)ctx;
    HRESULT status;

    if( !this )
        return NULL;

    /* initialize base structure */
    if( !sgui_memory_canvas_init( (sgui_canvas*)this, NULL,
                                  width, height, SGUI_RGBA8, 1 ) )
    {
        free( this );
        return NULL;
    }

    /* create a texture */
    status = IDirect3DDevice9_CreateTexture( ctx9->device, width, height, 1,
                                             D3DUSAGE_DYNAMIC,
                                             D3DFMT_A8R8G8B8,
                                             D3DPOOL_SYSTEMMEM,
                                             &this->tex, NULL );

    if( status != D3D_OK )
    {
        free( this );
        return NULL;
    }

    /* hook callbacks */
    ((sgui_tex_canvas*)this)->get_texture = d3d9_canvas_get_texture;
    ((sgui_canvas*)this)->destroy = d3d9_canvas_destroy;
    ((sgui_canvas*)this)->begin = d3d9_canvas_begin;
    ((sgui_canvas*)this)->end = d3d9_canvas_end;

    return (sgui_canvas*)this;
}
#endif

