/*
 * canvas_gl_tex.c
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
#include "sgui_opengl.h"
#include "sgui_internal.h"
#include "sgui_font.h"
#include "sgui_utf8.h"
#include "sgui_pixmap.h"

#include "internal_gl.h"

#ifndef SGUI_NO_OPENGL

#define TEX_ENABLE   0x01
#define DEPTH_ENABLE 0x02
#define DEPTH_WRITE  0x04
#define BLEND_ENABLE 0x08
#define MS_ENABLE    0x10



typedef struct
{
    sgui_canvas canvas;

    int state;
    GLint blend_src, blend_dst;
}
sgui_canvas_gl;




void canvas_gl_destroy( sgui_canvas* canvas )
{
    free( canvas );
}

sgui_pixmap* canvas_gl_create_pixmap( sgui_canvas* canvas, unsigned int width,
                                      unsigned int height, int format )
{
    (void)canvas;

    return gl_pixmap_create( width, height, format );
}

void canvas_gl_begin( sgui_canvas* canvas, sgui_rect* r )
{
    GLboolean v;
    sgui_canvas_gl* cv = (sgui_canvas_gl*)canvas;

    cv->state = 0;

    /* configure the viewport to canvas size */
    glViewport( 0, 0, canvas->width, canvas->height );

    /* push matrices we use and setup 2D rendering */
    glMatrixMode( GL_MODELVIEW );
    glPushMatrix( );
    glLoadIdentity( );
    glMatrixMode( GL_PROJECTION );
    glPushMatrix( );
    glOrtho( 0, canvas->width-1, canvas->height-1, 0, -1.0, 1.0 );
    glMatrixMode( GL_TEXTURE );
    glPushMatrix( );

    /* enable 2D texturing */
    cv->state |= glIsEnabled( GL_TEXTURE_2D ) ? TEX_ENABLE : 0;
    glEnable( GL_TEXTURE_2D );
    glBindTexture( GL_TEXTURE_2D, 0 );

    /* enable alpha blending */
    cv->state |= glIsEnabled( GL_BLEND ) ? BLEND_ENABLE : 0;

    glGetIntegerv( GL_BLEND_SRC, &cv->blend_src );
    glGetIntegerv( GL_BLEND_DST, &cv->blend_dst );

    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

    /* disable depth write and depth test */
    glGetBooleanv( GL_DEPTH_WRITEMASK, &v );
    cv->state |= v ? DEPTH_WRITE : 0;
    cv->state |= glIsEnabled( GL_DEPTH_TEST ) ? DEPTH_ENABLE : 0;

    glDisable( GL_DEPTH_TEST );
    glDepthMask( GL_FALSE );

    /* disable multisampling */
    cv->state |= glIsEnabled( GL_MULTISAMPLE ) ? MS_ENABLE : 0;
    glDisable( GL_MULTISAMPLE );

    /* start rendering rectangles */
    glBegin( GL_QUADS );

    (void)r;
}

void canvas_gl_end( sgui_canvas* canvas )
{
    sgui_canvas_gl* cv = (sgui_canvas_gl*)canvas;

    glEnd( );

    /* restore multisampling */
    if( cv->state & MS_ENABLE )
        glEnable( GL_MULTISAMPLE );

    /* restore depth test and depth write mask */
    glDepthMask( (cv->state & DEPTH_WRITE)!=0 );

    if( cv->state & DEPTH_ENABLE )
        glEnable( GL_DEPTH_TEST );

    /* restore blending */
    glBlendFunc( cv->blend_src, cv->blend_dst );

    if( !(cv->state & BLEND_ENABLE) )
        glDisable( GL_BLEND );

    /* restore texture state */
    if( !(cv->state & TEX_ENABLE) )
        glDisable( GL_TEXTURE_2D );

    /* restore matrices */
    glMatrixMode( GL_TEXTURE );
    glPopMatrix( );
    glMatrixMode( GL_PROJECTION );
    glPopMatrix( );
    glMatrixMode( GL_MODELVIEW );
    glPopMatrix( );
}

void canvas_gl_clear( sgui_canvas* canvas, sgui_rect* r )
{
    glColor3ub(canvas->bg_color[0], canvas->bg_color[1], canvas->bg_color[2]);
    glVertex2i( r->left,  r->top    );
    glVertex2i( r->right, r->top    );
    glVertex2i( r->right, r->bottom );
    glVertex2i( r->left,  r->bottom );
}

void canvas_gl_blit( sgui_canvas* canvas, int x, int y,
                     sgui_pixmap* pixmap, sgui_rect* srcrect )
{
    unsigned int w, h, tex_w, tex_h;
    GLuint new_tex;
    (void)canvas;

    new_tex = ((pixmap_gl*)pixmap)->texture;

    if( !new_tex )
        return;

    sgui_pixmap_get_size( pixmap, &tex_w, &tex_h );

    w = SGUI_RECT_WIDTH_V( srcrect );
    h = SGUI_RECT_HEIGHT_V( srcrect );

    glEnd( );
    glBindTexture( GL_TEXTURE_2D, new_tex );
    glMatrixMode( GL_TEXTURE );
    glLoadIdentity( );
    glScalef( 1.0f/((float)tex_w), 1.0f/((float)tex_h), 1.0f );
    glBlendFunc( GL_SRC_ALPHA, GL_ZERO );
    glBegin( GL_QUADS );

    glColor4ub( 0xFF, 0xFF, 0xFF, 0xFF );
    glTexCoord2i( srcrect->left, srcrect->top );
    glVertex2i( x, y );
    glTexCoord2i( srcrect->right, srcrect->top );
    glVertex2i( x+w-1, y );
    glTexCoord2i( srcrect->right, srcrect->bottom );
    glVertex2i( x+w-1, y+h-1 );
    glTexCoord2i( srcrect->left, srcrect->bottom );
    glVertex2i( x, y+h-1 );

    glEnd( );
    glBindTexture( GL_TEXTURE_2D, 0 );
    glLoadIdentity( );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    glBegin( GL_QUADS );
}

void canvas_gl_blend( sgui_canvas* canvas, int x, int y,
                      sgui_pixmap* pixmap, sgui_rect* srcrect )
{
    unsigned int w, h, tex_w, tex_h;
    GLuint new_tex;
    (void)canvas;

    new_tex = ((pixmap_gl*)pixmap)->texture;

    if( !new_tex )
        return;

    sgui_pixmap_get_size( pixmap, &tex_w, &tex_h );

    w = SGUI_RECT_WIDTH_V( srcrect );
    h = SGUI_RECT_HEIGHT_V( srcrect );

    glEnd( );
    glBindTexture( GL_TEXTURE_2D, new_tex );
    glMatrixMode( GL_TEXTURE );
    glLoadIdentity( );
    glScalef( 1.0f/((float)tex_w), 1.0f/((float)tex_h), 1.0f );
    glBegin( GL_QUADS );

    glColor4ub( 0xFF, 0xFF, 0xFF, 0xFF );
    glTexCoord2i( srcrect->left, srcrect->top );
    glVertex2i( x, y );
    glTexCoord2i( srcrect->right, srcrect->top );
    glVertex2i( x+w-1, y );
    glTexCoord2i( srcrect->right, srcrect->bottom );
    glVertex2i( x+w-1, y+h-1 );
    glTexCoord2i( srcrect->left, srcrect->bottom );
    glVertex2i( x, y+h-1 );

    glEnd( );
    glBindTexture( GL_TEXTURE_2D, 0 );
    glLoadIdentity( );
    glBegin( GL_QUADS );
}

void canvas_gl_draw_box( sgui_canvas* canvas, sgui_rect* r,
                         unsigned char* color, int format )
{
    GLubyte alpha = 0xFF;
    (void)canvas;

    if( format==SGUI_RGBA8 )
        alpha = color[3];

    glColor4ub( color[0], color[1], color[2], alpha );
    glVertex2i( r->left,    r->top      );
    glVertex2i( r->right+1, r->top      );
    glVertex2i( r->right+1, r->bottom+1 );
    glVertex2i( r->left,    r->bottom+1 );
}

int canvas_gl_draw_string( sgui_canvas* canvas, int x, int y,
                           sgui_font* font, unsigned char* color,
                           const char* text, unsigned int length )
{
    (void)canvas; (void)x; (void)y; (void)font; (void)color, (void)text;
    (void)length;

    return 60;
}

/****************************************************************************/
sgui_canvas* sgui_opengl_canvas_create( unsigned int width,
                                        unsigned int height )
{
    sgui_canvas_gl* cv = malloc( sizeof(sgui_canvas_gl) );

    if( !cv )
        return NULL;

    sgui_internal_canvas_init( (sgui_canvas*)cv, width, height );

    cv->canvas.destroy = canvas_gl_destroy;
    cv->canvas.begin = canvas_gl_begin;
    cv->canvas.end = canvas_gl_end;
    cv->canvas.clear = canvas_gl_clear;
    cv->canvas.blit = canvas_gl_blit;
    cv->canvas.blend = canvas_gl_blend;
    cv->canvas.draw_box = canvas_gl_draw_box;
    cv->canvas.draw_string = canvas_gl_draw_string;
    cv->canvas.create_pixmap = canvas_gl_create_pixmap;

    return (sgui_canvas*)cv;
}
#else
sgui_canvas* sgui_opengl_canvas_create( unsigned int width,
                                        unsigned int height )
{
    (void)width;
    (void)height;
    return NULL;
}
#endif

