/*
    This file is part of the sgui samples collection. I, David Oberhollenzer,
    author of this file hereby place the contents of this file into
    the public domain.
 */
/*
    This small programm is supposed to demonstrate how to use the sub-window
    manager module with various rendering contexts through sgui.
 */
#include "sgui.h"
#include "sgui_ctx_wm.h"
#include "sgui_ctx_window.h"

#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#ifdef SGUI_WINDOWS
    #include <windows.h>

    #include "sgui_d3d9.h"
#endif
#include <GL/gl.h>



#define WIDTH 800
#define HEIGHT 600



static void renderer_init( int backend, sgui_context* ctx );
static void renderer_draw( int backend, sgui_context* ctx );



int main( void )
{
    sgui_widget *butt, *check, *check2, *label;
    sgui_window_description desc;
    sgui_window* subwnd;
    sgui_window* subwnd2;
    sgui_context* ctx;
    sgui_window* wnd;
    sgui_ctx_wm* wm;
    int selection;

    puts( "Select rendering backend: " );
    puts( " 1) OpenGL(R) old" );
    puts( " 2) OpenGL(R) 3.0+ core" );
    puts( " 3) Direct3D(R) 9" );
    puts( " 4) Direct3D(R) 11" );
    puts( "\n 0) quit\n" );
    selection = 0;
    scanf( "%d", &selection );

    switch( selection )
    {
    case 1: selection = SGUI_OPENGL_COMPAT; break;
    case 2: selection = SGUI_OPENGL_CORE;   break;
    case 3: selection = SGUI_DIRECT3D_9;    break;
    case 4: selection = SGUI_DIRECT3D_11;   break;
    default:
        return 0;
    }

    sgui_init( );

    /* create a window */
    memset( &desc, 0, sizeof(desc) );

    desc.width          = WIDTH;
    desc.height         = HEIGHT;
    desc.flags          = SGUI_FIXED_SIZE|SGUI_DOUBLEBUFFERED;
    desc.backend        = selection;
    desc.bits_per_pixel = 32;
    desc.depth_bits     = 16;

    wnd = sgui_window_create_desc( &desc );

    sgui_window_set_title( wnd, "subwm" );
    sgui_window_move_center( wnd );
    sgui_window_set_visible( wnd, SGUI_VISIBLE );

    sgui_window_make_current( wnd );
    sgui_window_set_vsync( wnd, 1 );

    ctx = sgui_window_get_context( wnd );
    renderer_init( selection, ctx );

    /* */
    wm = sgui_ctx_wm_create( wnd );

    sgui_window_set_userptr( wnd, wm );
    sgui_window_on_event( wnd,
                          (sgui_window_callback)sgui_ctx_wm_inject_event );

    /* create some sub windows */
    subwnd = sgui_ctx_wm_create_window( wm, 256, 128, 0 );
    subwnd2 = sgui_ctx_wm_create_window( wm, 256, 128, 0 );

    sgui_window_set_visible( subwnd, 1 );
    sgui_window_set_visible( subwnd2, 1 );

    sgui_window_move( subwnd2, 10, 10 );

    sgui_window_set_title( subwnd, "Sub Window" );
    sgui_window_set_title( subwnd2, "Another Window" );

    /* create a few widgets */
    butt = sgui_button_create( 10, 35, 60, 25, "Button", 0 );
    check = sgui_checkbox_create( 10, 65, "OpenGL" );
    check2 = sgui_checkbox_create( 10, 90, "Texture" );

    label = sgui_label_create( 10, 35, "Hello, world!\n\n"
                                       "From a <b><i>sub</i></b> window." );

    sgui_button_set_state( check, 1 );
    sgui_button_set_state( check2, 1 );

    /* add the widgets to the sub windows */
    sgui_window_add_widget( subwnd, butt );
    sgui_window_add_widget( subwnd, check );
    sgui_window_add_widget( subwnd, check2 );

    sgui_window_add_widget( subwnd2, label );

    /* main loop */
    while( sgui_main_loop_step( ) )
    {
        renderer_draw( selection, ctx );
        sgui_ctx_wm_draw_gui( wm );
        sgui_window_swap_buffers( wnd );
    }

    /* clean up */
    sgui_window_destroy( subwnd );
    sgui_window_destroy( subwnd2 );
    sgui_widget_destroy( butt );
    sgui_widget_destroy( check );
    sgui_widget_destroy( check2 );
    sgui_widget_destroy( label );

    sgui_ctx_wm_destroy( wm );
    sgui_window_release_current( wnd );
    sgui_window_destroy( wnd );
    sgui_deinit( );

    return 0;
}

/****************************************************************************/

#define DEGTORAD (3.1415926359/180.0)

static float vertices[] =
{
    -1.0f, -1.0f,  1.0f,    /* 0: lower left front */
     1.0f,  0.0f,  0.0f,
     1.0f, -1.0f,  1.0f,    /* 1: lower right front */
     0.0f,  1.0f,  0.0f,
    -1.0f, -1.0f, -1.0f,    /* 2: lower left back */
     0.0f,  1.0f,  0.0f,
     1.0f, -1.0f, -1.0f,    /* 3: lower right back */
     1.0f,  0.0f,  0.0f,
    -1.0f,  1.0f,  1.0f,    /* 4: upper left front */
     0.0f,  1.0f,  0.0f,
     1.0f,  1.0f,  1.0f,    /* 5: upper right front */
     0.0f,  0.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,    /* 6: upper left back */
     0.0f,  0.0f,  1.0f,
     1.0f,  1.0f, -1.0f,    /* 7: upper right back */
     0.0f,  1.0f,  0.0f
};

static short indices[] =
{
    0, 1, 5, 0, 5, 4,
    1, 3, 7, 1, 7, 5,
    3, 2, 6, 3, 6, 7,
    2, 0, 4, 2, 4, 6,
    4, 7, 5, 7, 4, 6,
    0, 1, 3, 0, 3, 2
};

static float t = 0.0f;

/****************************************************************************/

static void gl_old_init( void )
{
    GLfloat m[16], f, iNF, fov_deg = 60.0f, Near = 0.1f, Far = 100.0f;
    GLfloat aspectratio = (float)WIDTH / (float)HEIGHT;

    glMatrixMode( GL_PROJECTION );
    f   = 1.0 / tan( fov_deg * DEGTORAD * 0.5 );
	iNF = 1.0 / ( Near - Far );
    m[0]=f/aspectratio; m[4]=0; m[ 8]=0;              m[12]=0;
    m[1]=0;             m[5]=f; m[ 9]=0;              m[13]=0;
    m[2]=0;             m[6]=0; m[10]=(Far+Near)*iNF; m[14]=2*Far*Near*iNF;
    m[3]=0;             m[7]=0; m[11]=-1;             m[15]=0;
    glLoadMatrixf( m );

    glEnable( GL_DEPTH_TEST );
}

static void gl_old_draw_cube( void )
{
    unsigned int i, idx;

    glBegin( GL_TRIANGLES );
    for( i=0; i<sizeof(indices)/sizeof(indices[0]); ++i )
    {
        idx = indices[i];
        glColor3f( vertices[idx*6+3], vertices[idx*6+4], vertices[idx*6+5] );
        glVertex3f( vertices[idx*6], vertices[idx*6+1], vertices[idx*6+2] );
    }
    glEnd( );
}

static void gl_old_draw( void )
{
    GLfloat m[16];

    m[3]=m[7]=m[11]=m[12]=m[13]=0.0f; m[14]=-5.0f; m[15]=1.0f;

    glViewport( 0, 0, WIDTH, HEIGHT );
    glClear( GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT );
    glMatrixMode( GL_MODELVIEW );

    /* */
    m[0]=m[10]=cos(t); m[2]=sin(t); m[8]=-m[2]; m[1]=m[4]=m[6]=m[9]=0; m[5]=1;
    glViewport( 0, HEIGHT/2, WIDTH/2, HEIGHT/2 );
    glLoadMatrixf( m );
    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
    gl_old_draw_cube( );
    glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );

    /* */
    m[1]=m[8]=-sin(t); m[0]=cos(t); m[9]=-m[0]; m[2]=m[4]=m[5]=m[10]=0;m[6]=1;
    glViewport( WIDTH/2, HEIGHT/2, WIDTH/2, HEIGHT/2 );
    glLoadMatrixf( m );
    gl_old_draw_cube( );

    /* */
    m[0]=m[10]=-sin(t); m[2]=cos(t); m[8]=-m[2]; m[1]=m[4]=m[6]=m[9]=0;m[5]=1;
    glViewport( 0, 0, WIDTH/2, HEIGHT/2 );
    glLoadMatrixf( m );
    gl_old_draw_cube( );

    t += 0.01f;
}

/****************************************************************************/

static const char* vsh_src =
"#version 130\n"

"uniform mat4 m_mv;\n"
"uniform mat4 m_proj;\n"

"in vec3 v_pos;\n"
"in vec3 v_color;\n"
"out vec3 color;\n"

"void main( )\n"
"{\n"
"    color = v_color;\n"
"    gl_Position = m_proj * m_mv * vec4( v_pos, 1.0 );\n"
"}";

static const char* fsh_src =
"#version 130\n"

"in vec3 color;\n"
"out vec3 f_color;\n"

"void main( )\n"
"{\n"
"    f_color = color;\n"
"}";

#ifndef GLAPIENTRY
    #define GLAPIENTRY APIENTRY
#endif

#ifndef GL_FRAGMENT_SHADER
    #define GL_FRAGMENT_SHADER 0x8B30
#endif

#ifndef GL_VERTEX_SHADER
    #define GL_VERTEX_SHADER 0x8B31
#endif

#ifndef GL_ARRAY_BUFFER
    #define GL_ARRAY_BUFFER 0x8892
#endif

#ifndef GL_ELEMENT_ARRAY_BUFFER
    #define GL_ELEMENT_ARRAY_BUFFER 0x8893
#endif

#ifndef GL_STATIC_DRAW
    #define GL_STATIC_DRAW 0x88E4
#endif

#ifndef GL_VERSION_2_0
    typedef char GLchar;
#endif

#ifndef GL_VERSION_1_5
    typedef ptrdiff_t GLsizeiptr;
    typedef ptrdiff_t GLintptr;
#endif

typedef GLuint (GLAPIENTRY * GLCREATESHADERPROC) (GLenum type);
typedef void (GLAPIENTRY * GLLINKPROGRAMPROC) (GLuint program);
typedef GLuint (GLAPIENTRY * GLCREATEPROGRAMPROC) (void);
typedef void (GLAPIENTRY * GLCOMPILESHADERPROC) (GLuint shader);
typedef void (GLAPIENTRY * GLGENBUFFERSPROC) (GLsizei n, GLuint* buffers);
typedef void (GLAPIENTRY * GLSHADERSOURCEPROC) (GLuint shader,
                                                GLsizei count,
                                                const char** string,
                                                const GLint* length);
typedef void (GLAPIENTRY * GLATTACHSHADERPROC) (GLuint program,
                                                GLuint shader);
typedef void (GLAPIENTRY * GLGENVERTEXARRAYSPROC) (GLsizei n,
                                                   GLuint* arrays);
typedef void (GLAPIENTRY * GLBINDVERTEXARRAYPROC) (GLuint array);
typedef void (GLAPIENTRY * GLBINDBUFFERPROC) (GLenum target, GLuint buffer);

typedef void (GLAPIENTRY * GLBINDFRAGDATALOCATIONPROC) (GLuint program,
                                                        GLuint colorNumber,
                                                        const char* name);
typedef void (GLAPIENTRY * GLBINDATTRIBLOCATIONPROC) (GLuint program,
                                                      GLuint index,
                                                      const char* name);
typedef void (GLAPIENTRY * GLENABLEVERTEXATTRIBARRAYPROC) (GLuint index);
typedef void (GLAPIENTRY * GLVERTEXATTRIBPOINTERPROC) (GLuint index,
                                                       GLint size,
                                                       GLenum type,
                                                       GLboolean normalized,
                                                       GLsizei stride,
                                                       const void* pointer);
typedef GLint (GLAPIENTRY * GLGETUNIFORMLOCATIONPROC) (GLuint program,
                                                       const char* name);
typedef void (GLAPIENTRY * GLBUFFERDATAPROC) (GLenum target, GLsizeiptr size,
                                              const void* data, GLenum usage);
typedef void (GLAPIENTRY * GLUSEPROGRAMPROC) (GLuint program);
typedef void (GLAPIENTRY * GLUNIFORMMATRIX4FVPROC) (GLint location,
                                                    GLsizei count,
                                                    GLboolean transpose,
                                                    const GLfloat* value);

GLCOMPILESHADERPROC CompileShader;
GLCREATESHADERPROC CreateShader;
GLCREATEPROGRAMPROC CreateProgram;
GLGENBUFFERSPROC GenBuffers;
GLLINKPROGRAMPROC LinkProgram;
GLSHADERSOURCEPROC ShaderSource;
GLBINDBUFFERPROC BindBuffer;
GLBUFFERDATAPROC BufferData;
GLATTACHSHADERPROC AttachShader;
GLUSEPROGRAMPROC UseProgram;
GLBINDVERTEXARRAYPROC BindVertexArray;
GLGENVERTEXARRAYSPROC GenVertexArrays;
GLGETUNIFORMLOCATIONPROC GetUniformLocation;
GLUNIFORMMATRIX4FVPROC UniformMatrix4fv;
GLBINDFRAGDATALOCATIONPROC BindFragDataLocation;
GLBINDATTRIBLOCATIONPROC BindAttribLocation;
GLVERTEXATTRIBPOINTERPROC VertexAttribPointer;
GLENABLEVERTEXATTRIBARRAYPROC EnableVertexAttribArray;
GLuint uniform_model_view, prog, vsh, fsh, vbo, ibo, vao;

static void gl_core_init( sgui_context* ctx )
{
    GLfloat m[16], f, iNF, fov_deg = 60.0f, Near = 0.1f, Far = 100.0f;
    GLfloat aspectratio = (float)WIDTH / (float)HEIGHT;

    f   = 1.0 / tan( fov_deg * DEGTORAD * 0.5 );
	iNF = 1.0 / ( Near - Far );
    m[0]=f/aspectratio; m[4]=0; m[ 8]=0;              m[12]=0;
    m[1]=0;             m[5]=f; m[ 9]=0;              m[13]=0;
    m[2]=0;             m[6]=0; m[10]=(Far+Near)*iNF; m[14]=2*Far*Near*iNF;
    m[3]=0;             m[7]=0; m[11]=-1;             m[15]=0;
    glLoadMatrixf( m );

    glEnable( GL_DEPTH_TEST );

    /* */
    CompileShader=(GLCOMPILESHADERPROC)ctx->load(ctx,"glCompileShader");
    CreateShader=(GLCREATESHADERPROC)ctx->load(ctx,"glCreateShader");
    CreateProgram=(GLCREATEPROGRAMPROC)ctx->load(ctx,"glCreateProgram");
    GenBuffers=(GLGENBUFFERSPROC)ctx->load(ctx,"glGenBuffers");
    LinkProgram=(GLLINKPROGRAMPROC)ctx->load(ctx,"glLinkProgram");
    ShaderSource=(GLSHADERSOURCEPROC)ctx->load(ctx,"glShaderSource");
    BindBuffer=(GLBINDBUFFERPROC)ctx->load(ctx,"glBindBuffer");
    BufferData=(GLBUFFERDATAPROC)ctx->load(ctx,"glBufferData");
    AttachShader=(GLATTACHSHADERPROC)ctx->load(ctx,"glAttachShader");
    UseProgram=(GLUSEPROGRAMPROC)ctx->load(ctx,"glUseProgram");
    BindVertexArray=(GLBINDVERTEXARRAYPROC)ctx->load(ctx,"glBindVertexArray");
    GenVertexArrays=(GLGENVERTEXARRAYSPROC)ctx->load(ctx,"glGenVertexArrays");
    GetUniformLocation = (GLGETUNIFORMLOCATIONPROC)
    ctx->load( ctx, "glGetUniformLocation" );
    UniformMatrix4fv = (GLUNIFORMMATRIX4FVPROC)
    ctx->load( ctx, "glUniformMatrix4fv" );
    BindFragDataLocation = (GLBINDFRAGDATALOCATIONPROC)
    ctx->load( ctx, "glBindFragDataLocation" );
    BindAttribLocation = (GLBINDATTRIBLOCATIONPROC)
    ctx->load( ctx, "glBindAttribLocation" );
    VertexAttribPointer = (GLVERTEXATTRIBPOINTERPROC)
    ctx->load( ctx, "glVertexAttribPointer" );
    EnableVertexAttribArray = (GLENABLEVERTEXATTRIBARRAYPROC)
    ctx->load( ctx, "glEnableVertexAttribArray" );

    /* */
    GenVertexArrays( 1, &vao );
    BindVertexArray( vao );

    GenBuffers( 1, &vbo );
    GenBuffers( 1, &ibo );

    BindBuffer( GL_ARRAY_BUFFER,         vbo );
    BindBuffer( GL_ELEMENT_ARRAY_BUFFER, ibo );

    BufferData( GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW );
    BufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof(indices),
                indices, GL_STATIC_DRAW );

    EnableVertexAttribArray( 0 );
    EnableVertexAttribArray( 1 );

    VertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*6, 0 );
    VertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*6,
                         (GLvoid*)(3*sizeof(GLfloat)) );

    vsh  = CreateShader( GL_VERTEX_SHADER );
    fsh  = CreateShader( GL_FRAGMENT_SHADER );
    prog = CreateProgram( );

    ShaderSource( vsh, 1, &vsh_src, NULL );
    ShaderSource( fsh, 1, &fsh_src, NULL );
    CompileShader( vsh );
    CompileShader( fsh );
    AttachShader( prog, vsh );
    AttachShader( prog, fsh );
    LinkProgram( prog );
    UseProgram( prog );

    uniform_model_view = GetUniformLocation( prog, "m_mv" );
    UniformMatrix4fv( GetUniformLocation( prog, "m_proj" ), 1, GL_FALSE, m );
}

static void gl_core_draw( void )
{
    unsigned int count = sizeof(indices)/sizeof(indices[0]);
    GLfloat m[16];

    glClear( GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT );

    m[3]=m[7]=m[11]=m[12]=m[13]=0.0f; m[14]=-5.0f; m[15]=1.0f;

    glViewport( 0, 0, WIDTH, HEIGHT );
    glClear( GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT );
    BindVertexArray( vao );

    /* */
    m[0]=m[10]=cos(t); m[2]=sin(t); m[8]=-m[2]; m[1]=m[4]=m[6]=m[9]=0; m[5]=1;
    glViewport( 0, HEIGHT/2, WIDTH/2, HEIGHT/2 );
    UniformMatrix4fv( uniform_model_view, 1, GL_FALSE, m );
    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
    glDrawElements( GL_TRIANGLES, count, GL_UNSIGNED_SHORT, NULL );
    glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );

    /* */
    m[1]=m[8]=-sin(t); m[0]=cos(t); m[9]=-m[0]; m[2]=m[4]=m[5]=m[10]=0;m[6]=1;
    glViewport( WIDTH/2, HEIGHT/2, WIDTH/2, HEIGHT/2 );
    UniformMatrix4fv( uniform_model_view, 1, GL_FALSE, m );
    glDrawElements( GL_TRIANGLES, count, GL_UNSIGNED_SHORT, NULL );

    /* */
    m[0]=m[10]=-sin(t); m[2]=cos(t); m[8]=-m[2]; m[1]=m[4]=m[6]=m[9]=0;m[5]=1;
    glViewport( 0, 0, WIDTH/2, HEIGHT/2 );
    UniformMatrix4fv( uniform_model_view, 1, GL_FALSE, m );
    glDrawElements( GL_TRIANGLES, count, GL_UNSIGNED_SHORT, NULL );

    t += 0.01f;
}

/****************************************************************************/

typedef struct
{
    FLOAT x, y, z;
    DWORD color;
}
CUSTOMVERTEX;

#define CUSTOMFVF (D3DFVF_XYZ|D3DFVF_DIFFUSE)

LPDIRECT3DVERTEXBUFFER9 v_buffer;
VOID* pVoid;

static void d3d9_init( sgui_context* context )
{
    float m[16], f, iNF, fov_deg = 60.0f, Near = 0.1f, Far = 100.0f;
    CUSTOMVERTEX vdat[ sizeof(indices)/sizeof(indices[0]) ];
    sgui_d3d9_context* ctx = (sgui_d3d9_context*)context;
    float aspectratio = (float)WIDTH / (float)HEIGHT;
    unsigned int i, idx, r, g, b;

    for( i=0; i<sizeof(indices)/sizeof(indices[0]); ++i )
    {
        idx = indices[ i ];
        r = vertices[ idx*6+3 ] * 255.0f;
        g = vertices[ idx*6+4 ] * 255.0f;
        b = vertices[ idx*6+5 ] * 255.0f;

        vdat[ i ].x = vertices[ idx*6   ];
        vdat[ i ].y = vertices[ idx*6+1 ];
        vdat[ i ].z = vertices[ idx*6+2 ];
        vdat[ i ].color = D3DCOLOR_ARGB(0xFF,r,g,b);
    }

    IDirect3DDevice9_Reset( ctx->device, &ctx->present );

    IDirect3DDevice9_CreateVertexBuffer( ctx->device, sizeof(vdat), 0,
                                         CUSTOMFVF, D3DPOOL_MANAGED,
                                         &v_buffer, NULL );

    /* load vertex data */
    IDirect3DVertexBuffer9_Lock( v_buffer, 0, 0, (void**)&pVoid, 0 );
    memcpy( pVoid, vdat, sizeof(vdat) );
    IDirect3DVertexBuffer9_Unlock( v_buffer );

    /* setup renderer state */
    IDirect3DDevice9_SetRenderState(ctx->device,D3DRS_CULLMODE,D3DCULL_NONE);
    IDirect3DDevice9_SetRenderState(ctx->device,D3DRS_LIGHTING,FALSE       );
    IDirect3DDevice9_SetRenderState(ctx->device,D3DRS_ZENABLE, TRUE        );

    /* set up perspective projection matrix */
    f   = 1.0 / tan( fov_deg * DEGTORAD * 0.5 );
	iNF = 1.0 / ( Near - Far );
    m[0]=f/aspectratio; m[4]=0; m[ 8]=0;              m[12]=0;
    m[1]=0;             m[5]=f; m[ 9]=0;              m[13]=0;
    m[2]=0;             m[6]=0; m[10]=(Far+Near)*iNF; m[14]=2*Far*Near*iNF;
    m[3]=0;             m[7]=0; m[11]=-1;             m[15]=0;
    IDirect3DDevice9_SetTransform(ctx->device,D3DTS_PROJECTION,(D3DMATRIX*)m);
}

static void d3d9_draw( sgui_context* context )
{
    sgui_d3d9_context* ctx = (sgui_d3d9_context*)context;
    D3DVIEWPORT9 vp;
    float m[16];

    m[3]=m[7]=m[11]=m[12]=m[13]=0.0f; m[14]=-5.0f; m[15]=1.0f;
    vp.X=0; vp.Y=0; vp.Width=WIDTH; vp.Height=HEIGHT;
    vp.MinZ = 0.0f; vp.MaxZ = 1.0f;

    IDirect3DDevice9_SetViewport( ctx->device, &vp );
    IDirect3DDevice9_Clear( ctx->device, 0, NULL,
                            D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER, 0, 1.0f, 0 );
    vp.Width=WIDTH/2; vp.Height=HEIGHT/2;

    IDirect3DDevice9_BeginScene( ctx->device );

    IDirect3DDevice9_SetFVF( ctx->device, CUSTOMFVF );
    IDirect3DDevice9_SetStreamSource( ctx->device, 0, v_buffer, 0,
                                      sizeof(CUSTOMVERTEX) );

    m[0]=m[10]=cos(t); m[2]=sin(t); m[8]=-m[2]; m[1]=m[4]=m[6]=m[9]=0; m[5]=1;
    vp.X = 0; vp.Y = HEIGHT/2;
    IDirect3DDevice9_SetTransform( ctx->device, D3DTS_VIEW, (D3DMATRIX*)m );
    IDirect3DDevice9_SetViewport( ctx->device, &vp );
    IDirect3DDevice9_DrawPrimitive( ctx->device, D3DPT_TRIANGLELIST, 0, 12 );

    m[1]=m[8]=-sin(t); m[0]=cos(t); m[9]=-m[0]; m[2]=m[4]=m[5]=m[10]=0;m[6]=1;
    vp.X = WIDTH/2; vp.Y = 0;
    IDirect3DDevice9_SetTransform( ctx->device, D3DTS_VIEW, (D3DMATRIX*)m );
    IDirect3DDevice9_SetViewport( ctx->device, &vp );
    IDirect3DDevice9_DrawPrimitive( ctx->device, D3DPT_TRIANGLELIST, 0, 12 );

    m[0]=m[10]=-sin(t); m[2]=cos(t); m[8]=-m[2]; m[1]=m[4]=m[6]=m[9]=0;m[5]=1;
    vp.X = 0; vp.Y = 0;
    IDirect3DDevice9_SetTransform( ctx->device, D3DTS_VIEW, (D3DMATRIX*)m );
    IDirect3DDevice9_SetViewport( ctx->device, &vp );
    IDirect3DDevice9_SetRenderState(ctx->device,D3DRS_FILLMODE,D3DFILL_SOLID);
    IDirect3DDevice9_DrawPrimitive( ctx->device, D3DPT_TRIANGLELIST, 0, 12 );
    IDirect3DDevice9_SetRenderState(ctx->device,D3DRS_FILLMODE,
                                    D3DFILL_WIREFRAME);

    IDirect3DDevice9_EndScene( ctx->device );
    t += 0.01f;
}

/****************************************************************************/

static void renderer_init( int backend, sgui_context* ctx )
{
    switch( backend )
    {
    case SGUI_OPENGL_CORE:   gl_core_init( ctx ); break;
    case SGUI_OPENGL_COMPAT: gl_old_init( );      break;
    case SGUI_DIRECT3D_9:    d3d9_init( ctx );    break;
    case SGUI_DIRECT3D_11:   break;
    }
}

static void renderer_draw( int backend, sgui_context* ctx )
{
    switch( backend )
    {
    case SGUI_OPENGL_CORE:   gl_core_draw( );  break;
    case SGUI_OPENGL_COMPAT: gl_old_draw( );   break;
    case SGUI_DIRECT3D_9:    d3d9_draw( ctx ); break;
    case SGUI_DIRECT3D_11:   break;
    }
}

