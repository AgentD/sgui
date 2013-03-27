/*
 * pixmap.c
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
#include "internal.h"



struct sgui_pixmap
{
    unsigned int width, height;
    int format, backend;

    union
    {
        void* native;

#ifndef SGUI_NO_OPENGL
        GLuint opengl;
#endif
    }
    pm;
};



sgui_pixmap* sgui_pixmap_create( unsigned int width, unsigned int height,
                                 int format, int backend )
{
    sgui_pixmap* pix;

    if( !width || !height || !format )
        return NULL;

    pix = malloc( sizeof(sgui_pixmap) );

    if( !pix )
        return NULL;

    pix->width   = width;
    pix->height  = height;
    pix->format  = format;
    pix->backend = backend;

    if( backend==SGUI_OPENGL_CORE || backend==SGUI_OPENGL_COMPAT )
    {
#ifndef SGUI_NO_OPENGL
        GLint current;

        glGenTextures( 1, &pix->pm.opengl );

        if( !pix->pm.opengl )
        {
            free( pix );
            pix = NULL;
        }
        else
        {
            /* get current texture and bind ours */
            glGetIntegerv( GL_TEXTURE_BINDING_2D, &current );
            glBindTexture( GL_TEXTURE_2D, pix->pm.opengl );

            /* reserve texture memory */
            if( format==SGUI_RGBA8 )
            {
                glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA8, width, height,
                              0, GL_RGBA, GL_UNSIGNED_BYTE, NULL );
            }
            else
            {
                glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0,
                              GL_RGB, GL_UNSIGNED_BYTE, NULL );
            }

            /* disable mipmapping */
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            /* rebind old texture */
            glBindTexture( GL_TEXTURE_2D, current );
        }
#endif
    }
    else
    {
        pix->pm.native = malloc( width*height*(format==SGUI_RGBA8 ? 4 : 3) );

        if( !pix->pm.native )
        {
            free( pix );
            pix = NULL;
        }
    }

    return pix;
}

void sgui_pixmap_load( sgui_pixmap* pixmap, sgui_rect* dstrect,
                       const unsigned char* data, int srcx, int srcy,
                       unsigned int width, unsigned int height,
                       int format )
{
    int dstx, dsty, subw, subh;

    if( !pixmap || !data || !width || !height )
        return;

    dstx = dstrect ? dstrect->left : 0;
    dsty = dstrect ? dstrect->top  : 0;
    subw = dstrect ? SGUI_RECT_WIDTH_V( dstrect ) : (int)width;
    subh = dstrect ? SGUI_RECT_HEIGHT_V( dstrect ) : (int)height;

    data += (srcy*width + srcx)*(format==SGUI_RGB8 ? 3 : 4);

    if( pixmap->backend==SGUI_OPENGL_CORE ||
        pixmap->backend==SGUI_OPENGL_COMPAT )
    {
#ifndef SGUI_NO_OPENGL
        GLint current;

        glGetIntegerv( GL_TEXTURE_BINDING_2D, &current );
        glBindTexture( GL_TEXTURE_2D, pixmap->pm.opengl );

        glPixelStorei( GL_UNPACK_ROW_LENGTH, width );

        glTexSubImage2D( GL_TEXTURE_2D, 0, dstx, dsty, subw, subh,
                         format==SGUI_RGB8 ? GL_RGB : GL_RGBA,
                         GL_UNSIGNED_BYTE, data );

        glPixelStorei( GL_UNPACK_ROW_LENGTH, 0 );

        glBindTexture( GL_TEXTURE_2D, current );
#endif
    }
    else
    {
        unsigned char* dst;
        const unsigned char *src, *row;
        int i, j, bpp = format==SGUI_RGB8 ? 3 : 4, alpha;

        dst = pixmap->pm.native;

        for( src=data, j=0; j<subh; ++j, src+=width*bpp )
        {
            for( row=src, i=0; i<subw; ++i, row+=bpp )
            {
                alpha = bpp==4 ? row[3] : 0xFF;

                *(dst++) = row[2]*alpha >> 8;
                *(dst++) = row[1]*alpha >> 8;
                *(dst++) = row[0]*alpha >> 8;

                if( pixmap->format==SGUI_RGBA8 )
                    *(dst++) = 0xFF - alpha;
            }
        }
    }
}

void sgui_pixmap_get_size( sgui_pixmap* pixmap, unsigned int* width,
                           unsigned int* height )
{
    if( pixmap )
    {
        if( width  ) *width  = pixmap->width;
        if( height ) *height = pixmap->height;
    }
    else
    {
        if( width  ) *width  = 0;
        if( height ) *height = 0;
    }
}

void sgui_pixmap_destroy( sgui_pixmap* pixmap )
{
    if( !pixmap )
        return;

    if( pixmap->backend==SGUI_OPENGL_CORE ||
        pixmap->backend==SGUI_OPENGL_COMPAT )
    {
#ifndef SGUI_NO_OPENGL
        glDeleteTextures( 1, &pixmap->pm.opengl );
#endif
    }
    else
    {
        free( pixmap->pm.native );
    }

    free( pixmap );
}

/****************************************************************************/

int pixmap_get_bpp( sgui_pixmap* pixmap )
{
    if( !pixmap )
        return 0;

    return pixmap->format==SGUI_RGBA8 ? 4 : 3;
}

unsigned char* pixmap_get_data( sgui_pixmap* pixmap )
{
    if( pixmap && pixmap->backend==SGUI_NATIVE )
        return pixmap->pm.native;

    return NULL;
}

