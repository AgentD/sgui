
/*
    This file is part of the sgui samples collection. I, David Oberhollenzer,
    author of this file hereby place the contents of this file into
    the public domain.
 */
/*
    This small programm is supposed to demonstrate how to use the sgui texture
    canvas with OpenGL.
 */
#include "sgui.h"
#include "sgui_tex_canvas.h"



#ifdef SGUI_WINDOWS
    #include <windows.h>
#endif
#include <GL/gl.h>
#include <string.h>
#include <math.h>



/*
    vertex data for a cube with bottom and top side missing
 */
float cube[] =
{
    /* back */
    1.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,

    0.0f, 1.0f,
    0.5f, -0.5f, -0.5f,

    0.0f, 0.0f,
    0.5f, 0.5f, -0.5f,

    1.0f, 0.0f,
    -0.5f, 0.5f, -0.5f,

    /* front */
    0.0f, 1.0f,
    -0.5f, -0.5f, 0.5f,

    1.0f, 1.0f,
    0.5f, -0.5f, 0.5f,

    1.0f, 0.0f,
    0.5f, 0.5f, 0.5f,

    0.0f, 0.0f,
    -0.5f, 0.5f, 0.5f,

    /* left */
    0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,

    1.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,

    1.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,

    0.0f, 0.0f,
    -0.5f, 0.5f, -0.5f,

    /* right */
    1.0f, 1.0f,
    0.5f, -0.5f, -0.5f,

    0.0f, 1.0f,
    0.5f, -0.5f,  0.5f,

    0.0f, 0.0f,
    0.5f,  0.5f,  0.5f,

    1.0f, 0.0f,
    0.5f, 0.5f, -0.5f,
};

/*
    helper function to set up perspective projection
 */
static void gl_perspective( float fov_deg, float aspectratio,
                            float N, float F )
{
    float f, iNF, m[16];

    f   = 1.0 / tan( fov_deg * (3.14159265358/180.0) * 0.5 );
	iNF = 1.0 / ( N - F );

    m[0]=f/aspectratio; m[4]=0; m[ 8]=0;         m[12]=0;
    m[1]=0;             m[5]=f; m[ 9]=0;         m[13]=0;
    m[2]=0;             m[6]=0; m[10]=(F+N)*iNF; m[14]=2*F*N*iNF;
    m[3]=0;             m[7]=0; m[11]=-1;        m[15]=0;

    glLoadMatrixf( m );
}

/****************************************************************************/

int main( void )
{
    sgui_window_description desc;
    sgui_canvas* texcanvas;
    sgui_widget* check;
    sgui_widget* check2;
    sgui_color bgcolor;
    sgui_widget* butt;
    sgui_window* wnd;
    GLuint* texture;
    float a = 0.0f;
    sgui_lib* lib;
    int i;

    lib = sgui_init(NULL);

    /* create a window with an OpenGL context, compatibillity profile */
    desc.parent         = NULL;
    desc.share          = NULL;
    desc.width          = 640;
    desc.height         = 480;
    desc.flags          = SGUI_DOUBLEBUFFERED|SGUI_FIXED_SIZE;
    desc.backend        = SGUI_OPENGL_COMPAT;
    desc.bits_per_pixel = 32;
    desc.depth_bits     = 16;
    desc.stencil_bits   = 0;
    desc.samples        = 0;

    wnd = sgui_window_create_desc( &desc );

    sgui_window_set_title( wnd, "Texture Canvas" );
    sgui_window_move_center( wnd );
    sgui_window_set_visible( wnd, SGUI_VISIBLE );

    /********** make the window current and enable vsync **********/
    sgui_window_make_current( wnd );
    sgui_window_set_vsync( wnd, 1 );

    /********* setup viewport and transformation matrices *********/
    glViewport( 0, 0, desc.width, desc.height );
    glMatrixMode( GL_PROJECTION );
    glOrtho( 0, desc.width, desc.height, 0, 0.0, 100.0 );
    gl_perspective( 45, (float)desc.width/(float)desc.height, 0.1, 100 );

    /******* enable texturing, blend texture onto geometry *******/
    glEnable( GL_TEXTURE_2D );
    glEnable( GL_DEPTH_TEST );
    glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL );

    /****************** create a texture canvas ******************/
    texcanvas = sgui_tex_canvas_create( wnd, 128, 128 );

    butt = sgui_button_create( 10, 10, 60, 25, "Button", SGUI_BUTTON );
    check = sgui_button_create( 10, 40, 0, 0, "OpenGL", SGUI_CHECKBOX );
    check2 = sgui_button_create( 10, 65, 0, 0, "Texture", SGUI_CHECKBOX );

    sgui_button_set_state( check, 1 );
    sgui_button_set_state( check2, 1 );

    sgui_widget_add_child( &texcanvas->root, butt );
    sgui_widget_add_child( &texcanvas->root, check );
    sgui_widget_add_child( &texcanvas->root, check2 );

    /******* get window background color and clear canvas *******/
    bgcolor = sgui_skin_get( )->window_color;

    sgui_canvas_begin( texcanvas, NULL );
    sgui_canvas_clear( texcanvas, NULL );
    sgui_canvas_end( texcanvas );

    texture = sgui_tex_canvas_get_texture( texcanvas );

    /****** redirect keyboard events to the texture canvas ******/
    sgui_event_connect( wnd, SGUI_KEY_PRESSED_EVENT,
                        sgui_canvas_send_window_event, texcanvas,
                        SGUI_FROM_EVENT, SGUI_EVENT );

    sgui_event_connect( wnd, SGUI_KEY_RELEASED_EVENT,
                        sgui_canvas_send_window_event, texcanvas,
                        SGUI_FROM_EVENT, SGUI_EVENT );

    sgui_event_connect( wnd, SGUI_CHAR_EVENT,
                        sgui_canvas_send_window_event, texcanvas,
                        SGUI_FROM_EVENT, SGUI_EVENT );

    /************************ main loop ************************/
    while( lib->main_loop_step(lib) )
    {
        /* redraw dirty canvas widgets */
        sgui_canvas_redraw_widgets( texcanvas, 1 );

        /* transform cube */
        glMatrixMode( GL_MODELVIEW );
        glLoadIdentity( );
        glTranslatef( 0.0, 0.0, -3.0 );
        glRotatef( a, 0.0, 1.0, 0.0 );
        a += 0.25f;

        /* draw textured cube */
        glClear( GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT );
        glBindTexture( GL_TEXTURE_2D, *texture );

        glBegin( GL_QUADS );
        glColor4ub( bgcolor.c.r, bgcolor.c.g, bgcolor.c.b, 0xFF );

        for( i=0; i<16; ++i )
        {
            glTexCoord2f( cube[i*5], cube[i*5+1] );
            glVertex3f( cube[i*5+2], cube[i*5+3], cube[i*5+4] );
        }

        glEnd( );

        /* swap back and front buffer */
        sgui_window_swap_buffers( wnd );
    }

    /************************ clean up ************************/
    sgui_canvas_destroy( texcanvas );
    sgui_widget_destroy( check2 );
    sgui_widget_destroy( check );
    sgui_widget_destroy( butt );

    sgui_window_release_current( wnd );
    sgui_window_destroy( wnd );
    lib->destroy(lib);

    return 0;
}

