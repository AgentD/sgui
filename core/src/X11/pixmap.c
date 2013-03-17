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
#include "sgui_pixmap.h"
#include "internal.h"



struct sgui_pixmap
{
    unsigned int width;
    unsigned int height;

    int backend;
    int format;

    union
    {
        struct
        {
            Pixmap pix;
            Picture pic;
        }
        native;

#ifndef SGUI_NO_OPENGL
        GLuint opengl;
#endif
    }
    pm;
};



sgui_pixmap* sgui_pixmap_create( unsigned int width, unsigned int height,
                                 int format, int backend )
{
    sgui_pixmap* pixmap = NULL;
    XRenderPictFormat* fmt;

    if( backend==SGUI_NATIVE )
    {
        pixmap = malloc( sizeof(sgui_pixmap) ); 

        if( !pixmap )
            return NULL;

        /* try to create an X11 Pixmap */
        pixmap->pm.native.pix = XCreatePixmap( dpy, DefaultRootWindow(dpy),
                                               width, height,
                                               format==SGUI_RGB8 ? 24 : 32 );

        if( !pixmap->pm.native.pix )
        {
            free( pixmap );
            return NULL;
        }

        if( format==SGUI_RGB8 )
            fmt = XRenderFindStandardFormat( dpy, PictStandardRGB24 );
        else
            fmt = XRenderFindStandardFormat( dpy, PictStandardARGB32 );

        pixmap->pm.native.pic = XRenderCreatePicture( dpy,
                                                      pixmap->pm.native.pix,
                                                      fmt, 0, NULL );

        if( !pixmap->pm.native.pic )
        {
            XFreePixmap( dpy, pixmap->pm.native.pix );
            free( pixmap );
            return NULL;
        }
    }
    else if( backend==SGUI_OPENGL_CORE || backend==SGUI_OPENGL_COMPAT )
    {
#ifndef SGUI_NO_OPENGL
        GLint current;

        pixmap = malloc( sizeof(sgui_pixmap) ); 

        /* try to create an OpenGL texture */
        if( pixmap )
        {
            glGenTextures( 1, &pixmap->pm.opengl );

            if( !pixmap->pm.opengl )
            {
                free( pixmap );
                pixmap = NULL;
            }
            else
            {
                /* get current texture and bind ours */
                glGetIntegerv( GL_TEXTURE_BINDING_2D, &current );
                glBindTexture( GL_TEXTURE_2D, pixmap->pm.opengl );

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
                glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                                 GL_LINEAR );
                glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
                                 GL_LINEAR );

                /* rebind old texture */
                glBindTexture( GL_TEXTURE_2D, current );
            }
        }
#endif
    }

    if( pixmap )
    {
        pixmap->width   = width;
        pixmap->height  = height;
        pixmap->backend = backend;
        pixmap->format  = format;
    }

    return pixmap;
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

    if( pixmap->backend==SGUI_NATIVE )
    {
        XRenderColor c;
        const unsigned char *src, *row;
        int i, j, bpp = format==SGUI_RGB8 ? 3 : 4, alpha;

        for( src=data, j=0; j<subh; ++j, src+=width*bpp )
        {
            for( row=src, i=0; i<subw; ++i, row+=bpp )
            {
                alpha = bpp==4 ? row[3] : 0xFF;

                c.red   = row[0]*alpha;
                c.green = row[1]*alpha;
                c.blue  = row[2]*alpha;
                c.alpha = alpha<<8;

                XRenderFillRectangle( dpy, PictOpSrc, pixmap->pm.native.pic,
                                      &c, dstx+i, dsty+j, 1, 1 );
            }
        }
    }
    else if( pixmap->backend==SGUI_OPENGL_CORE ||
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

    if( pixmap->backend==SGUI_NATIVE )
    {
        XRenderFreePicture( dpy, pixmap->pm.native.pic );
        XFreePixmap( dpy, pixmap->pm.native.pix );
    }
    else if( pixmap->backend==SGUI_OPENGL_CORE ||
             pixmap->backend==SGUI_OPENGL_COMPAT )
    {
#ifndef SGUI_NO_OPENGL
        glDeleteTextures( 1, &pixmap->pm.opengl );
#endif
    }

    free( pixmap );
}

/****************************************************************************/

Picture pixmap_get_picture( sgui_pixmap* pixmap )
{
    if( pixmap->backend==SGUI_NATIVE )
        return pixmap->pm.native.pic;

    return 0;
}

Pixmap pixmap_get_pixmap( sgui_pixmap* pixmap )
{
    if( pixmap->backend==SGUI_NATIVE )
        return pixmap->pm.native.pix;

    return 0;
}

