/*
 * canvas_gl.c
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
#include "internal_gl.h"



#ifndef SGUI_NO_OPENGL
typedef struct
{
    sgui_canvas canvas;

    int state;
    GLint blend_src, blend_dst;
    GLint old_scissor[4];
    GLuint vbo;

    sgui_font_cache* font_cache;

    GLGENBUFFERSPROC glGenBuffers;
    GLDELETEBUFFERSPROC glDeleteBuffers;
    GLBINDBUFFERPROC glBindBuffer;
}
sgui_canvas_gl;



static void canvas_gl_destroy( sgui_canvas* canvas )
{
    sgui_canvas_gl* cv = (sgui_canvas_gl*)canvas;

    if( cv->glDeleteBuffers && cv->vbo )
        cv->glDeleteBuffers( 1, &(cv->vbo) );

    sgui_font_cache_destroy( cv->font_cache );
    free( canvas );
}

static void canvas_gl_begin( sgui_canvas* canvas, sgui_rect* r )
{
    sgui_canvas_gl* cv = (sgui_canvas_gl*)canvas;
    GLboolean v;

    if( !cv->vbo )
    {
        cv->glBindBuffer = (GLBINDBUFFERPROC)GL_LOAD_FUN( "glBindBuffer" );
        cv->glGenBuffers = (GLGENBUFFERSPROC)GL_LOAD_FUN( "glGenBuffers" );
        cv->glDeleteBuffers =
        (GLDELETEBUFFERSPROC)GL_LOAD_FUN( "glDeleteBuffers" );

        if( cv->glGenBuffers )
            cv->glGenBuffers( 1, &cv->vbo );
    }

    /* configure the viewport to canvas size */
    glViewport( 0, 0, canvas->width, canvas->height );

    /* save current state */
    glGetBooleanv( GL_DEPTH_WRITEMASK, &v );
    glGetIntegerv( GL_BLEND_SRC, &cv->blend_src );
    glGetIntegerv( GL_BLEND_DST, &cv->blend_dst );
    glGetIntegerv( GL_SCISSOR_BOX, cv->old_scissor );

    cv->state  = 0;
    cv->state |= glIsEnabled( GL_TEXTURE_2D ) ? TEX_ENABLE : 0;
    cv->state |= glIsEnabled( GL_BLEND ) ? BLEND_ENABLE : 0;
    cv->state |= v ? DEPTH_WRITE : 0;
    cv->state |= glIsEnabled( GL_DEPTH_TEST ) ? DEPTH_ENABLE : 0;
    cv->state |= glIsEnabled( GL_MULTISAMPLE ) ? MS_ENABLE : 0;
    cv->state |= glIsEnabled( GL_SCISSOR_TEST ) ? SCISSOR_ENABLE : 0;
    cv->state |= glIsEnabled( GL_CULL_FACE ) ? CULL_ENABLE : 0;

    /* configure state for widget rendering */
    glEnable( GL_TEXTURE_2D );
    glEnable( GL_BLEND );
    glEnable( GL_SCISSOR_TEST );
    glDisable( GL_DEPTH_TEST );
    glDisable( GL_MULTISAMPLE );
    glDisable( GL_CULL_FACE );
    glDepthMask( GL_FALSE );

    glBindTexture( GL_TEXTURE_2D, 0 );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    glScissor( 0, 0, canvas->width, canvas->height );

    (void)r;
}

static void canvas_gl_end( sgui_canvas* canvas )
{
    sgui_canvas_gl* cv = (sgui_canvas_gl*)canvas;

    /* restore state */
    glDepthMask( (cv->state & DEPTH_WRITE)!=0 );
    glBlendFunc( cv->blend_src, cv->blend_dst );
    glScissor( cv->old_scissor[0], cv->old_scissor[1],
               cv->old_scissor[2], cv->old_scissor[3] );

    if(   cv->state & CULL_ENABLE    ) glEnable( GL_CULL_FACE );
    if(   cv->state & SCISSOR_ENABLE ) glDisable( GL_SCISSOR_TEST );
    if(   cv->state & MS_ENABLE      ) glEnable( GL_MULTISAMPLE );
    if(   cv->state & DEPTH_ENABLE   ) glEnable( GL_DEPTH_TEST );
    if( !(cv->state & BLEND_ENABLE)  ) glDisable( GL_BLEND );
    if( !(cv->state & TEX_ENABLE)    ) glDisable( GL_TEXTURE_2D );
}

static void canvas_gl_clear( sgui_canvas* canvas, sgui_rect* r )
{
    (void)canvas; (void)r;
}

static void canvas_gl_blit( sgui_canvas* canvas, int x, int y,
                            sgui_pixmap* pixmap, sgui_rect* srcrect )
{
    (void)canvas; (void)x; (void)y; (void)pixmap; (void)srcrect;
}

static void canvas_gl_blend( sgui_canvas* canvas, int x, int y,
                             sgui_pixmap* pixmap, sgui_rect* srcrect )
{
    (void)canvas; (void)x; (void)y; (void)pixmap; (void)srcrect;
}

static void canvas_gl_draw_box( sgui_canvas* canvas, sgui_rect* r,
                                unsigned char* color, int format )
{
    (void)canvas; (void)r; (void)color; (void)format;
}

static void canvas_gl_blend_glyph( sgui_canvas* canvas, int x, int y,
                                   sgui_pixmap* pixmap, sgui_rect* r,
                                   unsigned char* color )
{
    (void)canvas; (void)pixmap; (void)r; (void)color; (void)x; (void)y;
}

static int canvas_gl_draw_string( sgui_canvas* canvas, int x, int y,
                                  sgui_font* font, unsigned char* color,
                                  const char* text, unsigned int length )
{
    (void)canvas; (void)font; (void)text; (void)color; (void)x; (void)y;
    (void)color; (void)length;
    return 60;
}

/****************************************************************************/

sgui_canvas* gl_canvas_create_core( unsigned int width, unsigned int height )
{
    sgui_canvas_gl* cv = malloc( sizeof(sgui_canvas_gl) );

    if( !cv )
        return NULL;

    sgui_internal_canvas_init( (sgui_canvas*)cv, width, height );

    cv->vbo = 0;
    cv->font_cache = NULL;
    cv->glGenBuffers = NULL;
    cv->glDeleteBuffers = NULL;
    cv->glBindBuffer = NULL;

    cv->canvas.destroy = canvas_gl_destroy;
    cv->canvas.begin = canvas_gl_begin;
    cv->canvas.end = canvas_gl_end;
    cv->canvas.clear = canvas_gl_clear;
    cv->canvas.blit = canvas_gl_blit;
    cv->canvas.blend = canvas_gl_blend;
    cv->canvas.blend_glyph = canvas_gl_blend_glyph;
    cv->canvas.draw_box = canvas_gl_draw_box;
    cv->canvas.draw_string = canvas_gl_draw_string;

    return (sgui_canvas*)cv;
}
#endif

