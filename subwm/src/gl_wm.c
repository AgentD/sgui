/*
 * gl_wm.c
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
#include "gl_wm.h"

#include <stdlib.h>
#include <string.h>



static unsigned short ibo[] =
{
     0,  1,  4,  1,  5,  4, /* top left */
     1,  2,  5,  2,  6,  5, /* top center */
     2,  3,  6,  3,  7,  6, /* top right */
     4,  5,  8,  5,  9,  8, /* left */
     5,  6,  9,  9,  6, 10, /* center */
     6,  7, 10,  7, 11, 10, /* right */
     8,  9, 12, 12,  9, 13, /* bottom left */
     9, 10, 13, 13, 10, 14, /* bottom center */
    10, 11, 14, 14, 11, 15  /* bottom right */
};

static void window_vertices( unsigned int width, unsigned int height,
                             float* vbo )
{
    /* top row vertices */
    vbo[ 0] = 0.0f; vbo[ 1] = 0.0f; vbo[ 2] = 0.0f;    vbo[ 3] = 0.0f;
    vbo[ 4] = 0.5f; vbo[ 5] = 0.0f; vbo[ 6] = 8.0f;    vbo[ 7] = 0.0f;
    vbo[ 8] = 0.5f; vbo[ 9] = 0.0f; vbo[10] = width-8; vbo[11] = 0.0f;
    vbo[12] = 1.0f; vbo[13] = 0.0f; vbo[14] = width;   vbo[15] = 0.0f;

    /* center upper vertices */
    vbo[16] = 0.0f; vbo[17] = 0.5f; vbo[18] = 0.0f;    vbo[19] = 8.0f;
    vbo[20] = 0.5f; vbo[21] = 0.5f; vbo[22] = 8.0f;    vbo[23] = 8.0f;
    vbo[24] = 0.5f; vbo[25] = 0.5f; vbo[26] = width-8; vbo[27] = 8.0f;
    vbo[28] = 1.0f; vbo[29] = 0.5f; vbo[30] = width;   vbo[31] = 8.0f;

    /* center lower vertices */
    vbo[32] = 0.0f; vbo[33] = 0.5f; vbo[34] = 0.0f;    vbo[35] = height-8;
    vbo[36] = 0.5f; vbo[37] = 0.5f; vbo[38] = 8.0f;    vbo[39] = height-8;
    vbo[40] = 0.5f; vbo[41] = 0.5f; vbo[42] = width-8; vbo[43] = height-8;
    vbo[44] = 1.0f; vbo[45] = 0.5f; vbo[46] = width;   vbo[47] = height-8;

    /* bottom row vertices */
    vbo[48] = 0.0f; vbo[49] = 1.0f; vbo[50] = 0.0f;    vbo[51] = height;
    vbo[52] = 0.5f; vbo[53] = 1.0f; vbo[54] = 8.0f;    vbo[55] = height;
    vbo[56] = 0.5f; vbo[57] = 1.0f; vbo[58] = width-8; vbo[59] = height;
    vbo[60] = 1.0f; vbo[61] = 1.0f; vbo[62] = width;   vbo[63] = height;
}




static void gl_wm_destroy( sgui_ctx_wm* this )
{
    glDeleteTextures( 1, &(((sgui_gl_wm*)this)->wndtex) );
    free( this );
}

static void gl_wm_draw_gui( sgui_ctx_wm* super )
{
    sgui_gl_wm* this = (sgui_gl_wm*)super;
    GLfloat m[16], vbo[16*4], w, h;
    int alpha, view[4], wx, wy;
    unsigned int ww, wh, i;
    unsigned char color[4];
    sgui_ctx_window* wnd;
    GLuint* tex;

    w = (float)super->wnd->w;
    h = (float)super->wnd->h;

    m[0] = 2.0f/w; m[4] = 0.0f;   m[ 8] = 0.0f; m[12] =-1.0f;
    m[1] = 0.0f;   m[5] =-2.0f/h; m[ 9] = 0.0f; m[13] = 1.0f;
    m[2] = 0.0f;   m[6] = 0.0f;   m[10] = 1.0f; m[14] = 0.0f;
    m[3] = 0.0f;   m[7] = 0.0f;   m[11] = 0.0f; m[15] = 1.0f;

    memcpy( color, sgui_skin_get( )->window_color, 4 );

    glMatrixMode( GL_PROJECTION );
    glPushMatrix( );
    glLoadIdentity( );
    glMatrixMode( GL_MODELVIEW );
    glPushMatrix( );
    glLoadMatrixf( m );

    glGetIntegerv( GL_VIEWPORT, view );
    glViewport( 0, 0, super->wnd->w, super->wnd->h );

    for( wnd=super->list; wnd!=NULL; wnd=wnd->next )
    {
        tex = sgui_ctx_window_get_texture( (sgui_window*)wnd );
        alpha = wnd->next ? 0x80 : (super->draging ? 0xC0 : 0xF0);

        sgui_window_get_position( (sgui_window*)wnd, &wx, &wy );
        sgui_window_get_size( (sgui_window*)wnd, &ww, &wh );

        window_vertices( ww, wh, vbo );

        glTranslatef( (float)wx, (float)wy, 0.0f );

        glBindTexture( GL_TEXTURE_2D, this->wndtex );

        /* draw window background */
        glBegin( GL_TRIANGLES );
        glColor4ub( 0xFF, 0xFF, 0xFF, alpha );

        for( i=0; i<(sizeof(ibo)/sizeof(ibo[0])); ++i )
        {
            glTexCoord2f( vbo[ibo[i]*4], vbo[ibo[i]*4+1] );
            glVertex2i( vbo[ibo[i]*4+2], vbo[ibo[i]*4+3] );
        }

        glEnd( );

        /* draw window contents */
        glBindTexture( GL_TEXTURE_2D, *tex );

        glBegin( GL_QUADS );
        glColor4ub( 0xFF, 0xFF, 0xFF, alpha );
        glTexCoord2f(0.0f,0.0f); glVertex2i(0, 0 );
        glTexCoord2f(1.0f,0.0f); glVertex2i(ww,0 );
        glTexCoord2f(1.0f,1.0f); glVertex2i(ww,wh);
        glTexCoord2f(0.0f,1.0f); glVertex2i(0, wh);
        glEnd( );

        glTranslatef( -(float)wx, -(float)wy, 0.0f );
    }

    glViewport( view[0], view[1], view[2], view[3] );

    glPopMatrix( );
    glMatrixMode( GL_PROJECTION );
    glPopMatrix( );
}

/****************************************************************************/

sgui_ctx_wm* gl_wm_create( sgui_window* wnd )
{
    sgui_gl_wm* this = malloc( sizeof(sgui_gl_wm) );
    sgui_ctx_wm* super = (sgui_ctx_wm*)this;
    unsigned char data[ 16*16*4 ];
    GLuint old;
    int x, y;

    if( this )
    {
        memset( this, 0, sizeof(sgui_gl_wm) );
        super->wnd = wnd;
        super->destroy = gl_wm_destroy;
        super->draw_gui = gl_wm_draw_gui;

        /* generate skin texture */
        memset( data, 0, sizeof(data) );

        for( y=0; y<16; ++y )
        {
            for( x=0; x<16; ++x )
            {
                if( ((x-8)*(x-8) + (y-8)*(y-8))<64 )
                {
                    data[ (y*16 + x)*4   ] = 0x64;
                    data[ (y*16 + x)*4+1 ] = 0x64;
                    data[ (y*16 + x)*4+2 ] = 0x64;
                    data[ (y*16 + x)*4+3 ] = 0xFF;
                }
            }
        }

        /* create and upload skin texture */
        glGenTextures( 1, &this->wndtex );

        glGetIntegerv( GL_TEXTURE_BINDING_2D, (GLint*)&old );
        glBindTexture( GL_TEXTURE_2D, this->wndtex );
        glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, 16, 16, 0, GL_RGBA,
                      GL_UNSIGNED_BYTE, data );

        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );

        glBindTexture( GL_TEXTURE_2D, old );
    }

    return (sgui_ctx_wm*)this;
}

