#include "sgui.h"

#include <GL/gl.h>
#include <stddef.h>     /* for NULL */



int main( )
{
    sgui_window* wnd;
    sgui_window_description desc;

    sgui_init( );

    /* create a window */
    desc.parent         = NULL;
    desc.width          = 300;
    desc.height         = 300;
    desc.resizeable     = SGUI_FIXED_SIZE;
    desc.backend        = SGUI_OPENGL_COMPAT;
    desc.doublebuffer   = SGUI_DOUBLEBUFFERED;
    desc.bits_per_pixel = 32;
    desc.depth_bits     = 16;
    desc.stencil_bits   = 0;
    desc.samples        = 0;

    wnd = sgui_window_create_desc( &desc );

    sgui_window_set_title( wnd, "OpenGL Sample" );
    sgui_window_move_center( wnd );
    sgui_window_set_visible( wnd, SGUI_VISIBLE );

    /* main loop with continuos redrawing: */
    sgui_window_override_drawing( wnd, SGUI_OVERRIDE_ALL );
    sgui_window_make_current( wnd );
    sgui_window_set_vsync( wnd, 1 );

    while( sgui_main_loop_step( ) )
    {
        glClear( GL_COLOR_BUFFER_BIT );

        glBegin( GL_TRIANGLES );
        glColor3f( 1.0f, 0.0f, 0.0f );
        glVertex2f( -0.5f, -0.5f );
        glColor3f( 0.0f, 1.0f, 0.0f );
        glVertex2f(  0.5f, -0.5f );
        glColor3f( 0.0f, 0.0f, 1.0f );
        glVertex2f(  0.0f,  0.5f );
        glEnd( );

        sgui_window_swap_buffers( wnd );
    }

    /* clean up */
    sgui_window_make_current( NULL );
    sgui_window_destroy( wnd );
    sgui_deinit( );

    return 0;
}

