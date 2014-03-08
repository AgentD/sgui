#include "sgui.h"

#include <GL/gl.h>
#include <stdio.h>



void draw_callback( sgui_window* window )
{
    unsigned int w, h;

    sgui_window_get_size( window, &w, &h );
    sgui_window_make_current( window );

    glViewport( 0, 0, w, h );
    glClear( GL_COLOR_BUFFER_BIT );

    glMatrixMode( GL_MODELVIEW );
    glRotatef( 5.0f, 0.0f, 1.0f, 0.0f );

    glBegin( GL_TRIANGLES );
    glColor3f( 1.0f, 0.0f, 0.0f );
    glVertex2f( -0.5f, -0.5f );
    glColor3f( 0.0f, 1.0f, 0.0f );
    glVertex2f(  0.5f, -0.5f );
    glColor3f( 0.0f, 0.0f, 1.0f );
    glVertex2f(  0.0f,  0.5f );
    glEnd( );

    sgui_window_swap_buffers( window );
    sgui_window_make_current( NULL );
}



int main( void )
{
    sgui_window* wnd;
    sgui_window_description desc;

    sgui_init( );

    /* create a window */
    desc.parent         = NULL;
    desc.share          = NULL;
    desc.width          = 300;
    desc.height         = 300;
    desc.resizeable     = SGUI_RESIZEABLE;
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

    /* hook event callbacks */
    sgui_event_connect( wnd, SGUI_EXPOSE_EVENT, 0,
                        draw_callback, wnd, SGUI_VOID );

    sgui_event_connect( wnd, SGUI_EXPOSE_EVENT, 0,
                        puts, "Readraw!", SGUI_VOID );

    /* main loop */
    sgui_main_loop( );

    /* clean up */
    sgui_window_make_current( NULL );
    sgui_window_destroy( wnd );
    sgui_deinit( );

    return 0;
}

