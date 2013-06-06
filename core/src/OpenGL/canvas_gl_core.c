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
    GLuint vbo, vsh, fsh, program;

    sgui_font_cache* font_cache;

    GLGENBUFFERSPROC glGenBuffers;
    GLDELETEBUFFERSPROC glDeleteBuffers;
    GLBINDBUFFERPROC glBindBuffer;
    GLBUFFERDATAPROC glBufferData;
    GLBUFFERSUBDATAPROC glBufferSubData;
    GLDRAWARRAYSPROC glDrawArrays;

    GLCREATESHADERPROC glCreateShader;
    GLDELETESHADERPROC glDeleteShader;
    GLSHADERSOURCEPROC glShaderSource;
    GLCOMPILESHADERPROC glCompileShader;

    GLUSEPROGRAMPROC glUseProgram;
    GLLINKPROGRAMPROC glLinkProgram;
    GLDELETEPROGRAMPROC glDeleteProgram;
    GLCREATEPROGRAMPROC glCreateProgram;
    GLATTACHSHADERPROC glAttachShader;
    GLDETACHSHADERPROC glDetachShader;

    GLGETPROGRAMINFOLOG glGetProgramInfoLog;
    GLGETSHADERINFOLOG glGetShaderInfoLog;
}
sgui_canvas_gl;

/****************************************************************************/
static const char* vsh =
"#version 130\n"
"in ivec2 v_position;\n"
"in vec4 v_color;\n"

"uniform mat4 projection;"

"out vec4 color;\n"

"void main( )\n"
"{\n"
"   color = v_color;\n"
"   gl_Position = projection * vec4( v_position, 0.0, 1.0 );\n"
"}";

static const char* fsh =
"#version 130\n"
"in vec4 color;\n"
"out vec4 COLOR0;\n"

"void main( )\n"
"{\n"
"   COLOR0 = color;\n"
"}";
/****************************************************************************/

static void canvas_gl_destroy( sgui_canvas* canvas )
{
    sgui_canvas_gl* cv = (sgui_canvas_gl*)canvas;

    if( cv->vbo )
        cv->glDeleteBuffers( 1, &(cv->vbo) );

    if( cv->program )
    {
        cv->glDetachShader( cv->program, cv->vsh );
        cv->glDetachShader( cv->program, cv->fsh );
        cv->glDeleteProgram( cv->program );
    }

    if( cv->vsh )
        cv->glDeleteShader( cv->vsh );

    if( cv->fsh )
        cv->glDeleteShader( cv->fsh );

    sgui_font_cache_destroy( cv->font_cache );
    free( canvas );
}

#define BAIL_IF( x ) if( x ) return

static void canvas_gl_begin( sgui_canvas* canvas, sgui_rect* r )
{
    sgui_canvas_gl* cv = (sgui_canvas_gl*)canvas;
    GLint buffer;
    GLboolean v;

    if( !cv->vbo )
    {
        /* load VBO functions */
        cv->glBindBuffer = (GLBINDBUFFERPROC)GL_LOAD_FUN( "glBindBuffer" );
        cv->glGenBuffers = (GLGENBUFFERSPROC)GL_LOAD_FUN( "glGenBuffers" );
        cv->glBufferData = (GLBUFFERDATAPROC)GL_LOAD_FUN( "glBufferData" );
        cv->glDrawArrays = (GLDRAWARRAYSPROC)GL_LOAD_FUN( "glDrawArrays" );
        cv->glBufferSubData =
        (GLBUFFERSUBDATAPROC)GL_LOAD_FUN( "glBufferSubData" );
        cv->glDeleteBuffers =
        (GLDELETEBUFFERSPROC)GL_LOAD_FUN( "glDeleteBuffers" );

        BAIL_IF( !cv->glGenBuffers || !cv->glBindBuffer ||
                 !cv->glGenBuffers || !cv->glBufferData ||
                 !cv->glDrawArrays || !cv->glBufferSubData ||
                 !cv->glDeleteBuffers );

        /* create and initialise VBO */
        cv->glGenBuffers( 1, &cv->vbo );

        BAIL_IF( !cv->vbo );

        glGetIntegerv( GL_ARRAY_BUFFER_BINDING, &buffer );
        cv->glBindBuffer( GL_ARRAY_BUFFER, cv->vbo );
        cv->glBufferData( GL_ARRAY_BUFFER, 1024,
                          NULL, GL_STREAM_DRAW );
        cv->glBindBuffer( GL_ARRAY_BUFFER, buffer );
    }

    if( !cv->program )
    {
        /* load shader functions */
        cv->glCreateShader=(GLCREATESHADERPROC)GL_LOAD_FUN("glCreateShader");
        cv->glDeleteShader=(GLDELETESHADERPROC)GL_LOAD_FUN("glDeleteShader");
        cv->glShaderSource=(GLSHADERSOURCEPROC)GL_LOAD_FUN("glShaderSource");
        cv->glCompileShader =
        (GLCOMPILESHADERPROC)GL_LOAD_FUN("glCompileShader");
        cv->glGetShaderInfoLog =
        (GLGETSHADERINFOLOG)GL_LOAD_FUN("glGetShaderInfoLog");

        BAIL_IF( !cv->glCreateShader || !cv->glDeleteShader ||
                 !cv->glShaderSource || !cv->glCompileShader );

        /* load program functions */
        cv->glUseProgram = (GLUSEPROGRAMPROC)GL_LOAD_FUN( "glUseProgram" );
        cv->glLinkProgram = (GLLINKPROGRAMPROC)GL_LOAD_FUN( "glLinkProgram" );
        cv->glDeleteProgram =
        (GLDELETEPROGRAMPROC)GL_LOAD_FUN( "glDeleteProgram" );
        cv->glCreateProgram =
        (GLCREATEPROGRAMPROC)GL_LOAD_FUN( "glCreateProgram" );
        cv->glAttachShader =
        (GLATTACHSHADERPROC)GL_LOAD_FUN( "glAttachShader" );
        cv->glDetachShader =
        (GLDETACHSHADERPROC)GL_LOAD_FUN( "glDetachShader" );
        cv->glGetProgramInfoLog =
        (GLGETPROGRAMINFOLOG)GL_LOAD_FUN("glGetProgramInfoLog");

        BAIL_IF( !cv->glUseProgram || !cv->glLinkProgram ||
                 !cv->glDeleteProgram || !cv->glCreateProgram ||
                 !cv->glAttachShader || !cv->glDetachShader );

        /* create shaders */
        if( !cv->vsh )
        {
            cv->vsh = cv->glCreateShader( GL_VERTEX_SHADER );
            BAIL_IF( !cv->vsh );

            cv->glShaderSource( cv->vsh, 1, &vsh, NULL );
            cv->glCompileShader( cv->vsh );
        }

        if( !cv->fsh )
        {
            cv->fsh = cv->glCreateShader( GL_FRAGMENT_SHADER );
            BAIL_IF( !cv->fsh );

            cv->glShaderSource( cv->fsh, 1, &fsh, NULL );
            cv->glCompileShader( cv->fsh );
        }

        /* create program */
        cv->program = cv->glCreateProgram( );
        BAIL_IF( !cv->program );

        cv->glAttachShader( cv->program, cv->vsh );
        cv->glAttachShader( cv->program, cv->fsh );
        cv->glLinkProgram( cv->program );
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

    memset( cv, 0, sizeof(sgui_canvas_gl) );

    sgui_internal_canvas_init( (sgui_canvas*)cv, width, height );

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

