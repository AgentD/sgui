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
#include "sgui_subwm_skin.h"
#include "sgui_context.h"
#include "gl_wm.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>



#ifndef SGUI_NO_OPENGL
static const char* window_vsh =
"#version 130\n"

"in vec2 v_pos;\n"      /* 2D vertex position in screen space */
"in vec2 v_tc0;\n"      /* skin texture coordinate */
"in vec2 v_tc1;\n"      /* window content texture coordinate */

"out vec2 tc0;\n"       /* output skin texture coordinate */
"out vec2 tc1;\n"       /* output window content texture coordinate */

"uniform mat4 mvp;\n"   /* screen space scaling matrix */

"void main( )\n"
"{\n"
"    tc0 = v_tc0;\n"
"    tc1 = v_tc1;\n"
"    gl_Position = mvp * vec4( v_pos, 0.0, 1.0 );\n"
"}";

static const char* window_fsh =
"#version 130\n"

"in vec2 tc0;\n"        /* interpolated skin texture coordinate */
"in vec2 tc1;\n"        /* interpolated window content texture coordinate */

"out vec4 color;\n"     /* output fragment color */

"uniform sampler2D tex0;\n"         /* skin texture */
"uniform sampler2D tex1;\n"         /* window content texture */
"uniform float transparency;\n"     /* window alpha value */

"void main( )\n"
"{\n"
"    vec4 skin = texture( tex0, tc0 );\n"
"    vec4 content = texture( tex1, tc1 );\n"

"    vec3 mixed = mix( skin.rgb, content.rgb, content.a );\n"
"    color = vec4( mixed, skin.a*transparency );\n"
"}";

/*
    0_1__________2_3
    |_|__________|_|
   4| |5        6| |7
    | |          | |
    | |          | |
   8|_|9_______10|_|11
    |_|__________|_|
   12 13       14  15
 */
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
                             float* vbo, sgui_subwm_skin* skin )
{
    int a, b, c, d, e, f, g, h;
    sgui_rect tl, tr, bl, br;
    unsigned int tw, th;
    float sx, sy;

    skin->get_ctx_window_corner( skin, &tl, SGUI_WINDOW_TOP_LEFT );
    skin->get_ctx_window_corner( skin, &tr, SGUI_WINDOW_TOP_RIGHT );
    skin->get_ctx_window_corner( skin, &bl, SGUI_WINDOW_BOTTOM_LEFT );
    skin->get_ctx_window_corner( skin, &br, SGUI_WINDOW_BOTTOM_RIGHT );
    skin->get_ctx_skin_texture_size( skin, &tw, &th );
    sx = 1.0f / (float)(tw-1);
    sy = 1.0f / (float)(th-1);

    a = SGUI_RECT_WIDTH(tl);
    b = SGUI_RECT_WIDTH(tr);
    c = SGUI_RECT_HEIGHT(tl);
    d = SGUI_RECT_HEIGHT(tr);
    e = SGUI_RECT_WIDTH(bl);
    f = SGUI_RECT_WIDTH(br);
    g = SGUI_RECT_HEIGHT(bl);
    h = SGUI_RECT_HEIGHT(br);

    /* top row texture coordinates */
    vbo[ 0]=vbo[16]=(float)tl.left*sx;
    vbo[ 1]=vbo[ 5]=(float)tl.top*sy;
    vbo[ 4]=vbo[20]=(float)tl.right*sx;
    vbo[17]=vbo[21]=(float)tl.bottom*sy;

    vbo[ 8]=vbo[24]=(float)tr.left*sx;
    vbo[ 9]=vbo[13]=(float)tr.top*sy;
    vbo[12]=vbo[28]=(float)tr.right*sx;
    vbo[25]=vbo[29]=(float)tr.bottom*sy;

    /* bottom row texture coordinates */
    vbo[32]=vbo[48]=(float)bl.left*sx;
    vbo[36]=vbo[52]=(float)bl.right*sx;
    vbo[33]=vbo[37]=(float)bl.top*sy;
    vbo[49]=vbo[53]=(float)bl.bottom*sy;

    vbo[40]=vbo[56]=(float)br.left*sx;
    vbo[44]=vbo[60]=(float)br.right*sx;
    vbo[41]=vbo[45]=(float)br.top*sy;
    vbo[57]=vbo[61]=(float)br.bottom*sy;

    /* vertex positions */
    vbo[2]=vbo[3]=vbo[7]=vbo[11]=vbo[15]=vbo[18]=vbo[34]=vbo[50]=0.0f;

    vbo[ 6] = vbo[22] = a;
    vbo[10] = vbo[26] = width-b;
    vbo[19] = vbo[23] = c;
    vbo[27] = vbo[31] = d;
    vbo[38] = vbo[54] = e;
    vbo[42] = vbo[58] = width-f;
    vbo[35] = vbo[39] = height-g;
    vbo[43] = vbo[47] = height-h;
    vbo[14] = vbo[30] = vbo[46] = vbo[62] = width;
    vbo[51] = vbo[55] = vbo[59] = vbo[63] = height;
}

static void set_gl_state( sgui_ctx_wm* super, gl_state* state, int core )
{
    GLfloat w=super->wnd->w, h=super->wnd->h, m[16];

    m[0] = 2.0f/w; m[4] = 0.0f;   m[ 8] = 0.0f; m[12] =-1.0f;
    m[1] = 0.0f;   m[5] =-2.0f/h; m[ 9] = 0.0f; m[13] = 1.0f;
    m[2] = 0.0f;   m[6] = 0.0f;   m[10] = 1.0f; m[14] = 0.0f;
    m[3] = 0.0f;   m[7] = 0.0f;   m[11] = 0.0f; m[15] = 1.0f;

    glGetIntegerv( GL_CURRENT_PROGRAM, &state->program );
    glGetIntegerv( GL_ACTIVE_TEXTURE, &state->activetex );
    glGetIntegerv( GL_VIEWPORT, state->view );
    glGetIntegerv( GL_BLEND, &state->blending );
    glGetIntegerv( GL_BLEND_SRC, &state->blend_src );
    glGetIntegerv( GL_BLEND_DST, &state->blend_dst );
    glGetIntegerv( GL_DEPTH_TEST, &state->depth_test );
    glGetIntegerv( GL_DEPTH_WRITEMASK, &state->depth_write );
    glGetIntegerv( GL_CULL_FACE, &state->cull );
    glGetIntegerv( GL_POLYGON_MODE, state->modes );

    glEnable( GL_BLEND );
    glEnable( GL_TEXTURE_2D );
    glDisable( GL_LIGHTING );
    glDisable( GL_DEPTH_TEST );
    glDisable( GL_CULL_FACE );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    glViewport( 0, 0, super->wnd->w, super->wnd->h );
    glDepthMask( GL_FALSE );
    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );

    if( core )
    {
        sgui_gl_core_wm* this = (sgui_gl_core_wm*)super;
        sgui_gl_functions* gl = &this->gl;

        gl->UseProgram( this->prog );
        gl->UniformMatrix4fv( this->u_mvp, 1, GL_FALSE, m );

        gl->ActiveTexture( GL_TEXTURE0 );
        glGetIntegerv( GL_TEXTURE_BINDING_2D, &state->tex[0] );
        glBindTexture( GL_TEXTURE_2D, this->super.wndtex );
        gl->ActiveTexture( GL_TEXTURE0+1 );
        glGetIntegerv( GL_TEXTURE_BINDING_2D, &state->tex[1] );

        gl->BindVertexArray( this->vao );
    }
    else
    {
        glGetIntegerv( GL_LIGHTING, &state->lighting );
        glGetIntegerv( GL_MATRIX_MODE, &state->matrixmode );
        glMatrixMode( GL_PROJECTION );
        glPushMatrix( );
        glLoadIdentity( );
        glMatrixMode( GL_MODELVIEW );
        glPushMatrix( );
        glLoadMatrixf( m );

        glGetIntegerv( GL_TEXTURE_BINDING_2D, &state->tex[0] );
    }
}

static void restore_gl_state(sgui_gl_functions* gl, gl_state* state, int core)
{
    glViewport(state->view[0],state->view[1],state->view[2],state->view[3]);
    glBlendFunc( state->blend_src, state->blend_dst );

    if( !state->blending )
        glDisable( GL_BLEND );

    if( state->depth_test )
        glEnable( GL_DEPTH_TEST );

    if( state->cull )
        glEnable( GL_CULL_FACE );

    glDepthMask( state->depth_write );
    glPolygonMode( GL_FRONT, state->modes[0] );
    glPolygonMode( GL_BACK, state->modes[1] );

    if( core )
    {
        gl->ActiveTexture( GL_TEXTURE0 );
        glBindTexture( GL_TEXTURE_2D, state->tex[0] );
        gl->ActiveTexture( GL_TEXTURE0+1 );
        glBindTexture( GL_TEXTURE_2D, state->tex[1] );
        gl->ActiveTexture( state->activetex );

        gl->UseProgram( state->program );
    }
    else
    {
        glMatrixMode( GL_MODELVIEW );
        glPopMatrix( );
        glMatrixMode( GL_PROJECTION );
        glPopMatrix( );

        glMatrixMode( state->matrixmode );
        glBindTexture( GL_TEXTURE_2D, state->tex[0] );

        if( state->lighting )
            glEnable( GL_LIGHTING );
    }
}

static GLuint create_skin_texture( void )
{
    sgui_subwm_skin* skin;
    unsigned int w, h;
    GLuint old, tex;

    skin = sgui_subwm_skin_get( );
    skin->get_ctx_skin_texture_size( skin, &w, &h );

    glGenTextures( 1, &tex );

    if( tex )
    {
        glGetIntegerv( GL_TEXTURE_BINDING_2D, (GLint*)&old );
        glBindTexture( GL_TEXTURE_2D, tex );
        glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA,
                      GL_UNSIGNED_BYTE, skin->get_ctx_skin_texture(skin) );

        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );

        glBindTexture( GL_TEXTURE_2D, old );
    }
    return tex;
}

static void sgui_gl_functions_load( sgui_gl_functions* this,
                                    sgui_context* ctx )
{
    this->Uniform1f=(GLUNIFORM1FPROC)ctx->load(ctx,"glUniform1f");
    this->BufferSubData=(GLBUFFERSUBDATAPROC)ctx->load(ctx,"glBufferSubData");
    this->UseProgram=(GLUSEPROGRAMPROC)ctx->load(ctx,"glUseProgram");
    this->ActiveTexture=(GLACTIVETEXTUREPROC)ctx->load(ctx,"glActiveTexture");

    this->UniformMatrix4fv = (GLUNIFORMMATRIX4FVPROC)
    ctx->load( ctx, "glUniformMatrix4fv" );

    this->BindVertexArray = (GLBINDVERTEXARRAYPROC)
    ctx->load( ctx, "glBindVertexArray" );

    this->DrawElementsBaseVertex = (GLDRAWELEMENTSBASEVERTEXPROC)
    ctx->load( ctx, "glDrawElementsBaseVertex" );
}

/****************************************************************************/

static void gl_wm_destroy( sgui_ctx_wm* this )
{
    glDeleteTextures( 1, &(((sgui_gl_wm*)this)->wndtex) );
    free( this );
}

static void gl_wm_draw_gui( sgui_ctx_wm* super )
{
    sgui_gl_wm* this = (sgui_gl_wm*)super;
    int alpha, wx, wy;
    unsigned int ww, wh, i;
    GLfloat vbo[16*4];
    sgui_ctx_window* wnd;
    sgui_subwm_skin* skin;
    gl_state state;
    GLuint* tex;

    set_gl_state( super, &state, 0 );

    skin = sgui_subwm_skin_get( );

    for( wnd=super->list; wnd!=NULL; wnd=wnd->next )
    {
        if( !wnd->super.visible )
            continue;

        /* detemine window transparency */
        alpha = wnd->next ? SGUI_WINDOW_BACKGROUND :
                super->draging ? SGUI_WINDOW_DRAGING : SGUI_WINDOW_TOPMOST;
        alpha = skin->get_window_transparency( skin, alpha );

        /* get window texture and dimensions */
        if( !(tex = sgui_ctx_window_get_texture( (sgui_window*)wnd )) )
            continue;

        sgui_window_get_position( (sgui_window*)wnd, &wx, &wy );
        sgui_window_get_size( (sgui_window*)wnd, &ww, &wh );
        window_vertices( ww, wh, vbo, skin );

        /* draw window background */
        glTranslatef( (float)wx, (float)wy, 0.0f );

        glBindTexture( GL_TEXTURE_2D, this->wndtex );

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

    restore_gl_state( NULL, &state, 0 );
}

sgui_ctx_wm* gl_wm_create( sgui_window* wnd )
{
    sgui_gl_wm* this = malloc( sizeof(sgui_gl_wm) );
    sgui_ctx_wm* super = (sgui_ctx_wm*)this;

    if( this )
    {
        memset( this, 0, sizeof(sgui_gl_wm) );
        super->wnd = wnd;
        super->destroy = gl_wm_destroy;
        super->draw_gui = gl_wm_draw_gui;

        this->wndtex = create_skin_texture( );
    }

    return (sgui_ctx_wm*)this;
}

/****************************************************************************/

static void gl_wm_core_destroy( sgui_ctx_wm* super )
{
    sgui_gl_core_wm* this = (sgui_gl_core_wm*)super;
    sgui_context* ctx = sgui_window_get_context( super->wnd );
    GLDELETEVERTEXARRAYSPROC DeleteVertexArrays;
    GLDELETEBUFFERSPROC DeleteBuffers;
    GLDELETEPROGRAMPROC DeleteProgram;
    GLDELETESHADERPROC DeleteShader;

    /* load extensions */
    DeleteBuffers = (GLDELETEBUFFERSPROC)ctx->load( ctx, "glDeleteBuffers" );
    DeleteShader = (GLDELETESHADERPROC)ctx->load( ctx, "glDeleteShader" );
    DeleteProgram = (GLDELETEPROGRAMPROC)ctx->load( ctx, "glDeleteProgram" );
    DeleteVertexArrays = (GLDELETEVERTEXARRAYSPROC)
                         ctx->load(ctx,"glDeleteVertexArrays");

    /* delete objects */
    DeleteProgram( this->prog );
    DeleteShader( this->vsh );
    DeleteShader( this->fsh );
    DeleteVertexArrays( 1, &this->vao );
    DeleteBuffers( 2, this->buffers );
    glDeleteTextures( 1, &(((sgui_gl_wm*)super)->wndtex) );
    free( this );
}

static void gl_wm_core_draw_gui( sgui_ctx_wm* super )
{
    sgui_gl_core_wm* this = (sgui_gl_core_wm*)super;
    float buffer[ 6*16 ], vb[ 4*16 ];
    unsigned int i, j, alpha;
    sgui_subwm_skin* skin;
    sgui_gl_functions* gl;
    sgui_ctx_window* wnd;
    gl_state state;
    GLint *tex;

    skin = sgui_subwm_skin_get( );
    gl = &this->gl;

    for( j=0, wnd=super->list; wnd && j<GLWM_CORE_MAX_WINDOWS; wnd=wnd->next )
    {
        if( !wnd->super.visible )
            continue;

        window_vertices( wnd->super.w, wnd->super.h, vb, skin );

        for( i=0; i<16; ++i )
        {
            buffer[i*6    ] = vb[i*4+2] + wnd->super.x;
            buffer[i*6 + 1] = vb[i*4+3] + wnd->super.y;
            buffer[i*6 + 2] = vb[i*4  ];
            buffer[i*6 + 3] = vb[i*4+1];
            buffer[i*6 + 4] = vb[i*4+2] / (float)wnd->super.w;
            buffer[i*6 + 5] = vb[i*4+3] / (float)wnd->super.h;
        }

        gl->BufferSubData( GL_ARRAY_BUFFER, (j++)*sizeof(buffer),
                           sizeof(buffer), buffer );
    }

    set_gl_state( super, &state, 1 );

    for( j=0, wnd=super->list; wnd && j<GLWM_CORE_MAX_WINDOWS; wnd=wnd->next )
    {
        if( !wnd->super.visible )
            continue;
        if( !(tex = sgui_ctx_window_get_texture( (sgui_window*)wnd )) )
            continue;

        alpha = wnd->next ? SGUI_WINDOW_BACKGROUND :
                super->draging ? SGUI_WINDOW_DRAGING : SGUI_WINDOW_TOPMOST;
        alpha = skin->get_window_transparency( skin, alpha );

        glBindTexture( GL_TEXTURE_2D, *tex );
        gl->Uniform1f( this->u_alpha, (float)alpha/255.0f );
        gl->DrawElementsBaseVertex( GL_TRIANGLES, sizeof(ibo)/sizeof(ibo[0]),
                                    GL_UNSIGNED_SHORT, 0, (j++)*16 );
    }

    restore_gl_state( gl, &state, 1 );
}

sgui_ctx_wm* gl_wm_create_core( sgui_window* wnd )
{
    sgui_context* ctx = sgui_window_get_context( wnd );
    GLENABLEVERTEXATTRIBARRAYPROC EnableVertexAttribArray;
    GLBINDFRAGDATALOCATIONPROC BindFragDataLocation;
    GLVERTEXATTRIBPOINTERPROC VertexAttribPointer;
    GLGETUNIFORMLOCATIONPROC GetUniformLocation;
    GLBINDATTRIBLOCATIONPROC BindAttribLocation;
    GLGENVERTEXARRAYSPROC GenVertexArrays;
    GLCREATEPROGRAMPROC CreateProgram;
    GLCOMPILESHADERPROC CompileShader;
    GLCREATESHADERPROC CreateShader;
    GLSHADERSOURCEPROC ShaderSource;
    GLATTACHSHADERPROC AttachShader;
    GLLINKPROGRAMPROC LinkProgram;
    GLGENBUFFERSPROC GenBuffers;
    GLUNIFORM1IPROC Uniform1i;
    GLBINDBUFFERPROC BindBuffer;
    GLBUFFERDATAPROC BufferData;
    sgui_gl_functions* gl;
    sgui_gl_core_wm* this;
    sgui_gl_wm* super;
    const char* str;
    GLint program;
#ifndef NDEBUG
    GLGETPROGRAMINFOLOGPROC GetProgramInfoLog;
    GLGETSHADERINFOLOGPROC GetShaderInfoLog;
    char buffer[ 128 ];
    GLsizei length;
#endif

    /* use normal gl wm if version < 3.0 */
    str = (const char*)glGetString( GL_VERSION );

    if( str && strtol( str, NULL, 0 )<3 )
        return gl_wm_create( wnd );

    /* create object */
    this = malloc( sizeof(sgui_gl_core_wm) );
    super = (sgui_gl_wm*)this;

    if( !this )
        return NULL;

    memset( this, 0, sizeof(sgui_gl_core_wm) );
    super->super.wnd = wnd;

    /* load extensions */
    gl = &(this->gl);
    sgui_gl_functions_load( gl, wnd->ctx.ctx );

    Uniform1i = (GLUNIFORM1IPROC)ctx->load( ctx, "glUniform1i" );
    GetUniformLocation = (GLGETUNIFORMLOCATIONPROC)
                         ctx->load( ctx, "glGetUniformLocation" );

    CompileShader=(GLCOMPILESHADERPROC)ctx->load(ctx,"glCompileShader");
    CreateShader=(GLCREATESHADERPROC)ctx->load(ctx,"glCreateShader");
    CreateProgram=(GLCREATEPROGRAMPROC)ctx->load(ctx,"glCreateProgram");
    GenBuffers=(GLGENBUFFERSPROC)ctx->load(ctx,"glGenBuffers");
    LinkProgram=(GLLINKPROGRAMPROC)ctx->load(ctx,"glLinkProgram");
    ShaderSource=(GLSHADERSOURCEPROC)ctx->load(ctx,"glShaderSource");
    AttachShader=(GLATTACHSHADERPROC)ctx->load(ctx,"glAttachShader");
    GenVertexArrays=(GLGENVERTEXARRAYSPROC)ctx->load(ctx,"glGenVertexArrays");
    BindBuffer=(GLBINDBUFFERPROC)ctx->load(ctx,"glBindBuffer");
    BufferData=(GLBUFFERDATAPROC)ctx->load(ctx,"glBufferData");
    BindFragDataLocation = (GLBINDFRAGDATALOCATIONPROC)
    ctx->load( ctx, "glBindFragDataLocation" );

    BindAttribLocation=(GLBINDATTRIBLOCATIONPROC)
    ctx->load( ctx, "glBindAttribLocation" );

    VertexAttribPointer = (GLVERTEXATTRIBPOINTERPROC)
    ctx->load( ctx, "glVertexAttribPointer" );

    EnableVertexAttribArray = (GLENABLEVERTEXATTRIBARRAYPROC)
    ctx->load( ctx, "glEnableVertexAttribArray" );

    /* create buffer objects */
    GenVertexArrays( 1, &(this->vao) );
    GenBuffers( 2, this->buffers );

    /* configure vertex array layout */
    gl->BindVertexArray( this->vao );
    BindBuffer( GL_ARRAY_BUFFER, this->buffers[0] );
    BindBuffer( GL_ELEMENT_ARRAY_BUFFER, this->buffers[1] );

    EnableVertexAttribArray( 0 );
    EnableVertexAttribArray( 1 );
    EnableVertexAttribArray( 2 );

    VertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,sizeof(GLfloat)*6,0);
    VertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,sizeof(GLfloat)*6,
                        (GLvoid*)(2*sizeof(GLfloat)) );
    VertexAttribPointer(2,2,GL_FLOAT,GL_FALSE,sizeof(GLfloat)*6,
                        (GLvoid*)(4*sizeof(GLfloat)) );

    /* upload initial buffer data */
    BufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof(ibo), ibo, GL_STATIC_DRAW );

    BufferData( GL_ARRAY_BUFFER,
                16 * (6*sizeof(GLfloat)) * GLWM_CORE_MAX_WINDOWS,
                NULL, GL_STREAM_DRAW );

    /* create shader program objects */
    this->fsh = CreateShader( GL_FRAGMENT_SHADER );
    this->vsh = CreateShader( GL_VERTEX_SHADER );
    this->prog = CreateProgram( );

    /* compile shaders */
    ShaderSource( this->vsh, 1, &window_vsh, NULL );
    ShaderSource( this->fsh, 1, &window_fsh, NULL );

    CompileShader( this->vsh );
    CompileShader( this->fsh );

    /* link shader program */
    AttachShader( this->prog, this->vsh );
    AttachShader( this->prog, this->fsh );

    BindFragDataLocation( this->prog, 0, "color" );
    BindAttribLocation( this->prog, 0, "v_pos" );
    BindAttribLocation( this->prog, 1, "v_tc0" );
    BindAttribLocation( this->prog, 2, "v_tc1" );
    LinkProgram( this->prog );

#ifndef NDEBUG
    /* print compile and link logs */
    GetProgramInfoLog = (GLGETPROGRAMINFOLOGPROC)
                        ctx->load( ctx, "glGetProgramInfoLog" );
    GetShaderInfoLog = (GLGETSHADERINFOLOGPROC)
                       ctx->load( ctx, "glGetShaderInfoLog" );

    GetShaderInfoLog( this->vsh, sizeof(buffer), &length, buffer );
    if( length > 3 )
        fprintf( stderr, "SGUI VERTEX SHADER COMPILE FAILED!!\n\n%s\n\n",
                 buffer );

    GetShaderInfoLog( this->fsh, sizeof(buffer), &length, buffer );
    if( length > 3 )
        fprintf(stderr,"SGUI FRAGMENT SHADER COMPILE FAILED!!\n\n%s\n\n",
                buffer);

    GetProgramInfoLog( this->prog, sizeof(buffer), &length, buffer );
    if( length > 3 )
        fprintf( stderr, "SGUI SHADER LINK FAILED!!\n\n%s\n\n", buffer );
#endif
    /* get shader unfiroms */
    this->u_mvp   = GetUniformLocation( this->prog, "mvp" );
    this->u_alpha = GetUniformLocation( this->prog, "transparency" );

    /* initialize shader variables */
    glGetIntegerv( GL_CURRENT_PROGRAM, &program );

    gl->UseProgram( this->prog );
    Uniform1i( GetUniformLocation( this->prog, "tex0" ), 0 );
    Uniform1i( GetUniformLocation( this->prog, "tex1" ), 1 );
    gl->UseProgram( program );

    /* hook callbacks */
    super->super.destroy = gl_wm_core_destroy;
    super->super.draw_gui = gl_wm_core_draw_gui;

    super->wndtex = create_skin_texture( );
    return (sgui_ctx_wm*)this;
}
#endif /* SGUI_NO_OPENGL */

