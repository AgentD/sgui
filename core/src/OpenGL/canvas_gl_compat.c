/*
 * canvas_gl_compat.c
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

    sgui_font_cache* font_cache;
}
sgui_canvas_gl;



static void canvas_gl_destroy( sgui_canvas* canvas )
{
    sgui_canvas_gl* cv = (sgui_canvas_gl*)canvas;

    sgui_font_cache_destroy( cv->font_cache );
    free( canvas );
}

static void canvas_gl_begin( sgui_canvas* canvas, sgui_rect* r )
{
    sgui_canvas_gl* cv = (sgui_canvas_gl*)canvas;
    GLboolean v;

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

    glBegin( GL_TRIANGLES );

    (void)r;
}

static void canvas_gl_end( sgui_canvas* canvas )
{
    sgui_canvas_gl* cv = (sgui_canvas_gl*)canvas;

    glEnd( );

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

    /* restore matrices */
    glMatrixMode( GL_TEXTURE );
    glPopMatrix( );
    glMatrixMode( GL_PROJECTION );
    glPopMatrix( );
    glMatrixMode( GL_MODELVIEW );
    glPopMatrix( );
}

static void canvas_gl_clear( sgui_canvas* canvas, sgui_rect* r )
{
    glColor3ub(canvas->bg_color[0], canvas->bg_color[1], canvas->bg_color[2]);
    glVertex2i( r->left,  r->top    );
    glVertex2i( r->right, r->top    );
    glVertex2i( r->left,  r->bottom );

    glVertex2i( r->right, r->top    );
    glVertex2i( r->right, r->bottom );
    glVertex2i( r->left,  r->bottom );
}

static void canvas_gl_blit( sgui_canvas* canvas, int x, int y,
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
    glBegin( GL_TRIANGLES );

    glColor4ub( 0xFF, 0xFF, 0xFF, 0xFF );

    glTexCoord2i( srcrect->left, srcrect->top );
    glVertex2i( x, y );
    glTexCoord2i( srcrect->right, srcrect->top );
    glVertex2i( x+w-1, y );
    glTexCoord2i( srcrect->left, srcrect->bottom );
    glVertex2i( x, y+h-1 );

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
    glBegin( GL_TRIANGLES );
}

static void canvas_gl_blend( sgui_canvas* canvas, int x, int y,
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
    glBegin( GL_TRIANGLES );

    glColor4ub( 0xFF, 0xFF, 0xFF, 0xFF );

    glTexCoord2i( srcrect->left, srcrect->top );
    glVertex2i( x, y );
    glTexCoord2i( srcrect->right, srcrect->top );
    glVertex2i( x+w-1, y );
    glTexCoord2i( srcrect->left, srcrect->bottom );
    glVertex2i( x, y+h-1 );

    glTexCoord2i( srcrect->right, srcrect->top );
    glVertex2i( x+w-1, y );
    glTexCoord2i( srcrect->right, srcrect->bottom );
    glVertex2i( x+w-1, y+h-1 );
    glTexCoord2i( srcrect->left, srcrect->bottom );
    glVertex2i( x, y+h-1 );

    glEnd( );
    glBindTexture( GL_TEXTURE_2D, 0 );
    glLoadIdentity( );
    glBegin( GL_TRIANGLES );
}

static void canvas_gl_draw_box( sgui_canvas* canvas, sgui_rect* r,
                                unsigned char* color, int format )
{
    GLubyte A=0x00, R=0x00, G=0x00, B=0x00;
    (void)canvas;

    if( format==SGUI_RGB8 || format==SGUI_RGBA8 )
    {
        R = color[0];
        G = color[1];
        B = color[2];
        A = format==SGUI_RGBA8 ? color[3] : 0xFF;
    }
    else if( format==SGUI_A8 )
        R = G = B = A = color[0];

    glColor4ub( R, G, B, A );

    glVertex2i( r->left,    r->top      );
    glVertex2i( r->right+1, r->top      );
    glVertex2i( r->left,    r->bottom+1 );

    glVertex2i( r->right+1, r->top      );
    glVertex2i( r->right+1, r->bottom+1 );
    glVertex2i( r->left,    r->bottom+1 );
}

static void canvas_gl_blend_glyph( sgui_canvas* canvas, int x, int y,
                                   sgui_pixmap* pixmap, sgui_rect* r,
                                   unsigned char* color )
{
    unsigned int w, h;
    sgui_rect test;
    (void)canvas; (void)pixmap;

    w = SGUI_RECT_WIDTH_V( r );
    h = SGUI_RECT_HEIGHT_V( r );

    sgui_rect_set_size( &test, x, y, w, h );

    if( !sgui_rect_get_intersection( NULL, &canvas->sc, &test ) )
        return;

    glColor4ub( color[0], color[1], color[2], 0xFF );

    glTexCoord2i( r->left, r->top );
    glVertex2i( x, y );
    glTexCoord2i( r->right+1, r->top );
    glVertex2i( x+w, y );
    glTexCoord2i( r->left, r->bottom+1 );
    glVertex2i( x, y+h );

    glTexCoord2i( r->right+1, r->top );
    glVertex2i( x+w, y );
    glTexCoord2i( r->right+1, r->bottom+1 );
    glVertex2i( x+w, y+h );
    glTexCoord2i( r->left, r->bottom+1 );
    glVertex2i( x, y+h );
}

static int canvas_gl_draw_string( sgui_canvas* canvas, int x, int y,
                                  sgui_font* font, unsigned char* color,
                                  const char* text, unsigned int length )
{
    sgui_canvas_gl* cv = (sgui_canvas_gl*)canvas;
    unsigned long character, previous=0;
    unsigned int i, len = 0;
    sgui_pixmap* pixmap;
    const char* temp;
    int oldx = x;

    /* texture cannot be altered during begin/end cycle */
    glEnd( );

    /* make sure we actually have a font cache and the caching texture */
    if( cv->font_cache )
    {
        pixmap = sgui_font_cache_get_pixmap( cv->font_cache );
    }
    else
    {
        pixmap = gl_pixmap_create( FONT_MAP_WIDTH, FONT_MAP_HEIGHT, SGUI_A8 );

        if( !pixmap )
        {
            glBegin( GL_TRIANGLES );
            return 0;
        }

        cv->font_cache = sgui_font_cache_create( pixmap );

        if( !cv->font_cache )
        {
            sgui_pixmap_destroy( pixmap );
            glBegin( GL_TRIANGLES );
            return 0;
        }
    }

    /* make sure all characters are loaded to the texture */
    for( i=0, temp=text; i<length && (*temp) && (*temp!='\n');
         temp+=len, i+=len )
    {
        character = sgui_utf8_decode( temp, &len );

        sgui_font_cache_load_glyph( cv->font_cache, font, character );
    }

    /* bind the texture and begin rendering */
    glBindTexture( GL_TEXTURE_2D, ((pixmap_gl*)pixmap)->texture );
    glMatrixMode( GL_TEXTURE );
    glLoadIdentity( );
    glScalef( 1.0f/((float)FONT_MAP_WIDTH), 1.0f/((float)FONT_MAP_HEIGHT),
              1.0f );
    glScissor( canvas->sc.left, canvas->height - canvas->sc.bottom,
               SGUI_RECT_WIDTH(canvas->sc), SGUI_RECT_HEIGHT(canvas->sc) );
    glBegin( GL_TRIANGLES );

    /* for each character */
    for( i=0; i<length && (*text) && (*text!='\n'); text+=len, i+=len )
    {
        /* load the next glyph */
        character = sgui_utf8_decode( text, &len );

        /* apply kerning */
        x += sgui_font_get_kerning_distance( font, previous, character );

        /* blend onto destination buffer */
        x += sgui_font_cache_draw_glyph( cv->font_cache, font, character,
                                         x, y, canvas, color ) + 1;

        /* store previous glyph index for kerning */
        previous = character;
    }

    /* restore texture state */
    glEnd( );
    glBindTexture( GL_TEXTURE_2D, 0 );
    glLoadIdentity( );
    glScissor( 0, 0, canvas->width, canvas->height );
    glBegin( GL_TRIANGLES );

    return x - oldx;
}

/****************************************************************************/

sgui_canvas* gl_canvas_create_compat( unsigned int width,
                                      unsigned int height )
{
    sgui_canvas_gl* cv = malloc( sizeof(sgui_canvas_gl) );

    if( !cv )
        return NULL;

    sgui_internal_canvas_init( (sgui_canvas*)cv, width, height );

    cv->font_cache = NULL;

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

