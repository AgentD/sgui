/*
    This file is part of the sgui samples collection. I, David Oberhollenzer,
    author of this file hereby place the contents of this file into
    the public domain.
 */
/*
    This small programm is supposed to demonstrate how to use the sgui texture
    canvas with Direct3D 9.
 */
#include "sgui.h"
#include "sgui_d3d9.h"
#include "sgui_tex_canvas.h"

#include <math.h>



#define BGCOLOR D3DCOLOR_ARGB(0xFF,0x64,0x64,0x64)
#define CUSTOMFVF (D3DFVF_XYZ|D3DFVF_DIFFUSE|D3DFVF_TEX1)



/*
    a texture cube with top and bottom side missing
 */
typedef struct
{
    FLOAT x, y, z;
    DWORD color;
    FLOAT u, v;
}
CUSTOMVERTEX;

CUSTOMVERTEX vertices[] =
{
    /* front */
    {  -1.0f,  1.0f, 1.0f, BGCOLOR, 0.0f, 0.0f },
    {   1.0f, -1.0f, 1.0f, BGCOLOR, 1.0f, 1.0f },
    {  -1.0f, -1.0f, 1.0f, BGCOLOR, 0.0f, 1.0f },
    {  -1.0f,  1.0f, 1.0f, BGCOLOR, 0.0f, 0.0f },
    {   1.0f,  1.0f, 1.0f, BGCOLOR, 1.0f, 0.0f },
    {   1.0f, -1.0f, 1.0f, BGCOLOR, 1.0f, 1.0f },

    /* back */
    {  -1.0f,  1.0f, -1.0f, BGCOLOR, 1.0f, 0.0f },
    {   1.0f, -1.0f, -1.0f, BGCOLOR, 0.0f, 1.0f },
    {  -1.0f, -1.0f, -1.0f, BGCOLOR, 1.0f, 1.0f },
    {  -1.0f,  1.0f, -1.0f, BGCOLOR, 1.0f, 0.0f },
    {   1.0f,  1.0f, -1.0f, BGCOLOR, 0.0f, 0.0f },
    {   1.0f, -1.0f, -1.0f, BGCOLOR, 0.0f, 1.0f },

    /* left */
    { 1.0f, -1.0f,  1.0f, BGCOLOR, 0.0f, 1.0f },
    { 1.0f,  1.0f, -1.0f, BGCOLOR, 1.0f, 0.0f },
    { 1.0f, -1.0f, -1.0f, BGCOLOR, 1.0f, 1.0f },
    { 1.0f, -1.0f,  1.0f, BGCOLOR, 0.0f, 1.0f },
    { 1.0f,  1.0f,  1.0f, BGCOLOR, 0.0f, 0.0f },
    { 1.0f,  1.0f, -1.0f, BGCOLOR, 1.0f, 0.0f },

    /* right */
    { -1.0f, -1.0f,  1.0f, BGCOLOR, 1.0f, 1.0f },
    { -1.0f,  1.0f, -1.0f, BGCOLOR, 0.0f, 0.0f },
    { -1.0f, -1.0f, -1.0f, BGCOLOR, 0.0f, 1.0f },
    { -1.0f, -1.0f,  1.0f, BGCOLOR, 1.0f, 1.0f },
    { -1.0f,  1.0f,  1.0f, BGCOLOR, 1.0f, 0.0f },
    { -1.0f,  1.0f, -1.0f, BGCOLOR, 0.0f, 0.0f }
};



/*
    helper function to set up projection and transformation matrices
 */
static void perspective( float* m, float fov_deg, float aspect,
                         float N, float F )
{
    float f, iNF;

    f   = 1.0 / tan( fov_deg * (3.14159265358/180.0) * 0.5 );
    iNF = 1.0 / (N - F);

    m[0]=f/aspect; m[4]=0.0f; m[ 8]=0;       m[12]=0;
    m[1]=0.0f;     m[5]=f;    m[ 9]=0;       m[13]=0;
    m[2]=0.0f;     m[6]=0.0f; m[10]=F*iNF;   m[14]=F*N*iNF;
    m[3]=0.0f;     m[7]=0.0f; m[11]=-1;      m[15]=0;
}

static void transform( float* m, float a )
{
    float c = cos(a), s = sin(a);

    m[0]=c;    m[4]=0.0f; m[ 8]=s;    m[12]= 0.0f;
    m[1]=0.0f; m[5]=1.0f; m[ 9]=0.0f; m[13]= 0.0f;
    m[2]=-s;   m[6]=0.0f; m[10]=c;    m[14]=-5.0f;
    m[3]=0.0f; m[7]=0.0f; m[11]=0.0f; m[15]= 1.0f;
}

/****************************************************************************/

int main( void )
{
    LPDIRECT3DVERTEXBUFFER9 v_buffer;
    sgui_window_description desc;
    IDirect3DTexture9* texture;
    sgui_d3d9_context* ctx;
    sgui_canvas* texcanvas;
    IDirect3DDevice9* dev;
    sgui_widget* check2;
    sgui_widget* check;
    sgui_widget* butt;
    sgui_window* wnd;
    float a=0.0f;
    VOID* pVoid;
    float m[16];

    sgui_init( );

    /*************************** create a window **************************/
    desc.parent         = NULL;
    desc.share          = NULL;
    desc.width          = 640;
    desc.height         = 480;
    desc.flags          = SGUI_FIXED_SIZE|SGUI_DOUBLEBUFFERED;
    desc.backend        = SGUI_DIRECT3D_9;
    desc.bits_per_pixel = 32;
    desc.depth_bits     = 16;
    desc.stencil_bits   = 0;
    desc.samples        = 0;

    wnd = sgui_window_create_desc( &desc );

    sgui_window_set_title( wnd, "Direct3D 9 Texture Canvas" );
    sgui_window_move_center( wnd );
    sgui_window_set_visible( wnd, SGUI_VISIBLE );
    sgui_window_set_vsync( wnd, 1 );

    /************************ createtexture canvas ************************/
    texcanvas = sgui_tex_canvas_create( wnd, 128, 128 );

    butt = sgui_button_create( 10, 10, 60, 25, "Button", 0 );
    check = sgui_checkbox_create( 10, 40, "Direct3D" );
    check2 = sgui_checkbox_create( 10, 65, "Texture" );

    sgui_button_set_state( check, 1 );
    sgui_button_set_state( check2, 1 );

    sgui_widget_add_child( &texcanvas->root, butt );
    sgui_widget_add_child( &texcanvas->root, check );
    sgui_widget_add_child( &texcanvas->root, check2 );

    /************** connect keyboard input to texture canvas **************/
    sgui_event_connect( wnd, SGUI_KEY_PRESSED_EVENT,
                        sgui_canvas_send_window_event, texcanvas,
                        SGUI_FROM_EVENT, SGUI_EVENT );

    sgui_event_connect( wnd, SGUI_KEY_RELEASED_EVENT,
                        sgui_canvas_send_window_event, texcanvas,
                        SGUI_FROM_EVENT, SGUI_EVENT );

    sgui_event_connect( wnd, SGUI_CHAR_EVENT,
                        sgui_canvas_send_window_event, texcanvas,
                        SGUI_FROM_EVENT, SGUI_EVENT );

    /*************************** Direct3D setup ***************************/
    /* get the device context, set new present parameters */
    ctx = (sgui_d3d9_context*)sgui_window_get_context( wnd );
    dev = ctx->device;

    IDirect3DDevice9_Reset( dev, &ctx->present );

    /* create a vertex buffer */
    IDirect3DDevice9_CreateVertexBuffer( dev, sizeof(vertices), 0,
                                         CUSTOMFVF, D3DPOOL_MANAGED,
                                         &v_buffer, NULL );

    /* load vertex data */
    IDirect3DVertexBuffer9_Lock( v_buffer, 0, 0, (void**)&pVoid, 0 );
    memcpy( pVoid, vertices, sizeof(vertices) );
    IDirect3DVertexBuffer9_Unlock( v_buffer );

    /* setup renderer state */
    IDirect3DDevice9_SetRenderState( dev, D3DRS_CULLMODE, D3DCULL_NONE );
    IDirect3DDevice9_SetRenderState( dev, D3DRS_LIGHTING, FALSE );
    IDirect3DDevice9_SetRenderState( dev, D3DRS_ZENABLE, TRUE );

    /* set up perspective projection matrix */
    perspective( m, 45.0f, (float)desc.width/(float)desc.height,
                 0.1f, 100.0f );
    IDirect3DDevice9_SetTransform( dev, D3DTS_PROJECTION, (D3DMATRIX*)m );

    /* set up texturing, bind canvas texture to stage 0 */
    texture = sgui_tex_canvas_get_texture( texcanvas );

    IDirect3DDevice9_SetTexture( dev, 0, (IDirect3DBaseTexture9*)texture );
    IDirect3DDevice9_SetTextureStageState( dev, 0, D3DTSS_COLOROP,
                                           D3DTOP_BLENDTEXTUREALPHA );
    IDirect3DDevice9_SetTextureStageState( dev, 0, D3DTSS_COLORARG1,
                                           D3DTA_TEXTURE );
    IDirect3DDevice9_SetTextureStageState( dev, 0, D3DTSS_COLORARG2,
                                           D3DTA_DIFFUSE );
    IDirect3DDevice9_SetTextureStageState( dev, 0, D3DTSS_ALPHAOP,
                                           D3DTOP_DISABLE );

    IDirect3DDevice9_SetSamplerState(dev,0,D3DSAMP_MINFILTER,D3DTEXF_LINEAR);
    IDirect3DDevice9_SetSamplerState(dev,0,D3DSAMP_MAGFILTER,D3DTEXF_LINEAR);

    /****************************** main loop *****************************/
    while( sgui_main_loop_step( ) )
    {
        /* redraw widgets in dirty areas */
        sgui_canvas_redraw_widgets( texcanvas, 1 );

        /* draw scene */
        IDirect3DDevice9_Clear( dev, 0, NULL,
                                D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER,
                                0, 1.0f, 0 );

        IDirect3DDevice9_BeginScene( dev );

        transform( m, a );
        IDirect3DDevice9_SetTransform( dev, D3DTS_VIEW, (D3DMATRIX*)m );
        a += 0.01f;

        IDirect3DDevice9_SetFVF( dev, CUSTOMFVF );
        IDirect3DDevice9_SetStreamSource( dev, 0, v_buffer, 0,
                                          sizeof(CUSTOMVERTEX) );

        IDirect3DDevice9_DrawPrimitive( dev, D3DPT_TRIANGLELIST, 0, 8 );

        IDirect3DDevice9_EndScene( dev );

        /* swap front and back buffer */
        sgui_window_swap_buffers( wnd );
    }

    /****************************** clean up ******************************/
    sgui_canvas_destroy( texcanvas );
    sgui_widget_destroy( check2 );
    sgui_widget_destroy( check );
    sgui_widget_destroy( butt );

    IDirect3DVertexBuffer9_Release( v_buffer );

    sgui_window_destroy( wnd );
    sgui_deinit( );
    return 0;
}

