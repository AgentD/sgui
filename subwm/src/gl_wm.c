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
#include "ctx_mesh.h"
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

static void set_gl_state( sgui_ctx_wm* super, gl_state* state, int core )
{
    GLfloat w=super->wnd->w, h=super->wnd->h, m[16];

    m[0] = 2.0f/w; m[4] = 0.0f;   m[ 8] = 0.0f; m[12] =-1.0f;
    m[1] = 0.0f;   m[5] =-2.0f/h; m[ 9] = 0.0f; m[13] = 1.0f;
    m[2] = 0.0f;   m[6] = 0.0f;   m[10] = 1.0f; m[14] = 0.0f;
    m[3] = 0.0f;   m[7] = 0.0f;   m[11] = 0.0f; m[15] = 1.0f;

    if( core )
    {
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
    }
    else
    {
        glPushAttrib( GL_ACCUM_BUFFER_BIT|GL_COLOR_BUFFER_BIT|GL_CURRENT_BIT|
                      GL_DEPTH_BUFFER_BIT|GL_ENABLE_BIT|GL_LIGHTING_BIT|
                      GL_SCISSOR_BIT|GL_STENCIL_BUFFER_BIT|GL_TEXTURE_BIT|
                      GL_TRANSFORM_BIT|GL_VIEWPORT_BIT );

        glMatrixMode( GL_PROJECTION );
        glPushMatrix( );
        glLoadIdentity( );
        glMatrixMode( GL_MODELVIEW );
        glPushMatrix( );
        glLoadMatrixf( m );
    }

    glEnable( GL_BLEND );
    glEnable( GL_TEXTURE_2D );
    glDisable( GL_LIGHTING );
    glDisable( GL_DEPTH_TEST );
    glDisable( GL_ALPHA_TEST );
    glDisable( GL_STENCIL_TEST );
    glDisable( GL_CULL_FACE );
    glDisable( GL_SCISSOR_TEST );
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
}

static void restore_gl_state(sgui_gl_functions* gl, gl_state* state, int core)
{
    if( core )
    {
        glViewport( state->view[0], state->view[1],
                    state->view[2], state->view[3] );
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

        glPopAttrib( );
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

static int sgui_gl_functions_load( sgui_gl_functions* this,
                                   sgui_context* ctx )
{
    this->Uniform1f=(GLUNIFORM1F)ctx->load(ctx,"glUniform1f");
    this->BufferSubData=(GLBUFFERSUBDATA)ctx->load(ctx,"glBufferSubData");
    this->UseProgram=(GLUSEPROGRAM)ctx->load(ctx,"glUseProgram");
    this->ActiveTexture=(GLACTIVETEXTURE)ctx->load(ctx,"glActiveTexture");

    this->UniformMatrix4fv = (GLUNIFORMMATRIX4FV)
    ctx->load( ctx, "glUniformMatrix4fv" );

    this->BindVertexArray = (GLBINDVERTEXARRAY)
    ctx->load( ctx, "glBindVertexArray" );

    this->DrawElementsBaseVertex = (GLDRAWELEMENTSBASEVERTEX)
    ctx->load( ctx, "glDrawElementsBaseVertex" );

    if(!this->Uniform1f||!this->BufferSubData||!this->UseProgram||
       !this->ActiveTexture||!this->UniformMatrix4fv||!this->BindVertexArray||
       !this->DrawElementsBaseVertex)
    {
        return 0;
    }

    return 1;
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
    unsigned int ww, wh, i, num_indices;
    unsigned short* indices;
    GLfloat vbo[16*4];
    sgui_ctx_window* wnd;
    sgui_subwm_skin* skin;
    gl_state state;
    GLuint* tex;

    set_gl_state( super, &state, 0 );

    skin = sgui_subwm_skin_get( );

    num_indices = ctx_get_window_indices( &indices );

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
        ctx_get_window_vertices( ww, wh, vbo, skin, 0 );

        /* draw window background */
        glTranslatef( (float)wx, (float)wy, 0.0f );

        glBindTexture( GL_TEXTURE_2D, this->wndtex );

        glBegin( GL_TRIANGLES );
        glColor4ub( 0xFF, 0xFF, 0xFF, alpha );

        for( i=0; i<num_indices; ++i )
        {
            glTexCoord2f( vbo[indices[i]*4], vbo[indices[i]*4+1] );
            glVertex2i( vbo[indices[i]*4+2], vbo[indices[i]*4+3] );
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
    GLDELETEVERTEXARRAYS DeleteVertexArrays;
    GLDELETEBUFFERS DeleteBuffers;
    GLDELETEPROGRAM DeleteProgram;
    GLDELETESHADER DeleteShader;

    /* load extensions */
    DeleteBuffers = (GLDELETEBUFFERS)ctx->load( ctx, "glDeleteBuffers" );
    DeleteShader = (GLDELETESHADER)ctx->load( ctx, "glDeleteShader" );
    DeleteProgram = (GLDELETEPROGRAM)ctx->load( ctx, "glDeleteProgram" );
    DeleteVertexArrays = (GLDELETEVERTEXARRAYS)
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
    unsigned int i, j, alpha, num_indices;
    float buffer[ 6*16 ], vb[ 4*16 ];
    sgui_subwm_skin* skin;
    sgui_gl_functions* gl;
    sgui_ctx_window* wnd;
    gl_state state;
    GLint *tex;

    skin = sgui_subwm_skin_get( );
    gl = &this->gl;
    num_indices = ctx_get_window_indices( NULL );

    for( j=0, wnd=super->list; wnd && j<GLWM_CORE_MAX_WINDOWS; wnd=wnd->next )
    {
        if( !wnd->super.visible )
            continue;

        ctx_get_window_vertices( wnd->super.w, wnd->super.h, vb, skin, 0 );

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
        gl->DrawElementsBaseVertex( GL_TRIANGLES, num_indices,
                                    GL_UNSIGNED_SHORT, 0, (j++)*16 );
    }

    restore_gl_state( gl, &state, 1 );
}

sgui_ctx_wm* gl_wm_create_core( sgui_window* wnd )
{
    sgui_context* ctx = sgui_window_get_context( wnd );
    GLENABLEVERTEXATTRIBARRAY EnableVertexAttribArray;
    GLBINDFRAGDATALOCATION BindFragDataLocation;
    GLVERTEXATTRIBPOINTER VertexAttribPointer;
    GLGETUNIFORMLOCATION GetUniformLocation;
    GLBINDATTRIBLOCATION BindAttribLocation;
    GLGENVERTEXARRAYS GenVertexArrays;
    GLCREATEPROGRAM CreateProgram;
    GLCOMPILESHADER CompileShader;
    GLCREATESHADER CreateShader;
    GLSHADERSOURCE ShaderSource;
    GLATTACHSHADER AttachShader;
    GLLINKPROGRAM LinkProgram;
    GLGENBUFFERS GenBuffers;
    GLUNIFORM1I Uniform1i;
    GLBINDBUFFER BindBuffer;
    GLBUFFERDATA BufferData;
    unsigned int num_indices;
    unsigned short* indices;
    sgui_gl_functions* gl;
    sgui_gl_core_wm* this;
    sgui_gl_wm* super;
    const char* str;
    GLint program;
#ifndef NDEBUG
    GLGETPROGRAMINFOLOG GetProgramInfoLog;
    GLGETSHADERINFOLOG GetShaderInfoLog;
    char buffer[ 128 ];
    GLsizei length;
#endif

    /* use normal gl wm if version < 3.0 */
    str = (const char*)glGetString( GL_VERSION );

    if( !str )
        return NULL;

    if( strtol( str, NULL, 0 )<3 )
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
    if( !sgui_gl_functions_load( gl, wnd->ctx.ctx ) )
    {
        free( this );
        return NULL;
    }

    Uniform1i = (GLUNIFORM1I)ctx->load( ctx, "glUniform1i" );
    GetUniformLocation = (GLGETUNIFORMLOCATION)
                         ctx->load( ctx, "glGetUniformLocation" );

    CompileShader=(GLCOMPILESHADER)ctx->load(ctx,"glCompileShader");
    CreateShader=(GLCREATESHADER)ctx->load(ctx,"glCreateShader");
    CreateProgram=(GLCREATEPROGRAM)ctx->load(ctx,"glCreateProgram");
    GenBuffers=(GLGENBUFFERS)ctx->load(ctx,"glGenBuffers");
    LinkProgram=(GLLINKPROGRAM)ctx->load(ctx,"glLinkProgram");
    ShaderSource=(GLSHADERSOURCE)ctx->load(ctx,"glShaderSource");
    AttachShader=(GLATTACHSHADER)ctx->load(ctx,"glAttachShader");
    GenVertexArrays=(GLGENVERTEXARRAYS)ctx->load(ctx,"glGenVertexArrays");
    BindBuffer=(GLBINDBUFFER)ctx->load(ctx,"glBindBuffer");
    BufferData=(GLBUFFERDATA)ctx->load(ctx,"glBufferData");
    BindFragDataLocation = (GLBINDFRAGDATALOCATION)
    ctx->load( ctx, "glBindFragDataLocation" );

    BindAttribLocation=(GLBINDATTRIBLOCATION)
    ctx->load( ctx, "glBindAttribLocation" );

    VertexAttribPointer = (GLVERTEXATTRIBPOINTER)
    ctx->load( ctx, "glVertexAttribPointer" );

    EnableVertexAttribArray = (GLENABLEVERTEXATTRIBARRAY)
    ctx->load( ctx, "glEnableVertexAttribArray" );

    if(!Uniform1i||!GetUniformLocation||!CompileShader||!CreateShader||
       !CreateProgram||!GenBuffers||!LinkProgram||!ShaderSource||
       !AttachShader||!GenVertexArrays||!BindBuffer||!BufferData||
       !BindFragDataLocation||!BindAttribLocation||!VertexAttribPointer||
       !EnableVertexAttribArray)
    {
        free( this );
        return NULL;
    }

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
    num_indices = ctx_get_window_indices( &indices );

    BufferData( GL_ELEMENT_ARRAY_BUFFER, num_indices*sizeof(short),
                indices, GL_STATIC_DRAW );

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
    GetProgramInfoLog = (GLGETPROGRAMINFOLOG)
                        ctx->load( ctx, "glGetProgramInfoLog" );
    GetShaderInfoLog = (GLGETSHADERINFOLOG)
                       ctx->load( ctx, "glGetShaderInfoLog" );

    if( GetProgramInfoLog!=NULL && GetShaderInfoLog!=NULL )
    {
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
    }
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

