/*
    This file is part of the sgui samples collection. I, David Oberhollenzer,
    author of this file hereby place the contents of this file into
    the public domain.
 */
/*
    This small programm is supposed to demonstrate how to create a window
    with a OpenGL rendering context through sgui and how to use a manual
    event/drawing loop for real-time rendering applications.
 */
#include "sgui.h"

#ifdef SGUI_WINDOWS
    #include <windows.h>
#endif
#include <GL/gl.h>
#include <stdio.h>



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
    desc.flags          = SGUI_FIXED_SIZE|SGUI_DOUBLEBUFFERED;
    desc.backend        = SGUI_OPENGL_COMPAT;
    desc.bits_per_pixel = 32;
    desc.depth_bits     = 16;
    desc.stencil_bits   = 0;
    desc.samples        = 0;

    wnd = sgui_window_create_desc( &desc );

    /* set window title and make it visible */
    sgui_window_set_title( wnd, "OpenGL Sample" );
    sgui_window_move_center( wnd );
    sgui_window_set_visible( wnd, SGUI_VISIBLE );

    /*
        Make the rendering context of the window current and force the
        window to synchronize buffer swapping with vertical retrace.
     */
    sgui_window_make_current( wnd );
    sgui_window_set_vsync( wnd, 1 );

    /*
        Enter a manuall drawing loop.

        The function sgui_main_loop_step( ) fetches a message from the window
        systems, processes it and flushes the internal message queue.

        The function does not wait for system messages and returns
        immediately, even if there were no messages.

        If at least one window is visible, the function returns non-zero. It
        returns zero after the last window got set invisible.
     */
    while( sgui_main_loop_step( ) )
    {
        /* draw a triangle, rotate it a bit every frame */
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

        /* swap the front and back buffers */
        sgui_window_swap_buffers( wnd );
    }

    /* clean up */
    sgui_window_release_current( wnd );
    sgui_window_destroy( wnd );
    sgui_deinit( );

    return 0;
}

