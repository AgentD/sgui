/*
    This file is part of the sgui samples collection. I, David Oberhollenzer,
    author of this file hereby place the contents of this file into
    the public domain.
 */
/*
    This small programm is supposed to demonstrate how to create a window
    with a OpenGL rendering context through sgui.
 */
#include "sgui.h"

#ifdef SGUI_WINDOWS
    #include <windows.h>
#endif
#include <GL/gl.h>
#include <stdio.h>



void draw_callback( sgui_window* window )
{
    unsigned int w, h;

    /*
        Make the OpenGL context of the window current.
     */
    sgui_window_get_size( window, &w, &h );
    sgui_window_make_current( window );

    /*
        Draw a triangle. Rotate it a little, so we can
        notice window redraws.
     */
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

    /*
        Swap the front and back buffers of the window and
        release the OpenGL context again.
     */
    sgui_window_swap_buffers( window );
    sgui_window_release_current( window );
}



int main( void )
{
    sgui_lib* lib;
    sgui_window* wnd;
    sgui_window_description desc;

    lib = sgui_init(NULL);

    /*
        Create a window.

        Since we need to set a few "advanced" parameters, we use the
        sgui_window_create_desc function that takes a pointer to a window
        description structure.
     */
    desc.parent         = NULL;     /* parent window pointer */
    desc.share          = NULL;     /* window to share context data with */
    desc.width          = 300;
    desc.height         = 300;
    desc.flags          = SGUI_DOUBLEBUFFERED;
    desc.backend        = SGUI_OPENGL_COMPAT;
    desc.bits_per_pixel = 32;
    desc.depth_bits     = 16;
    desc.stencil_bits   = 0;
    desc.samples        = 0;

    wnd = lib->create_window(lib, &desc);

    /* set window title, move to center and make visible */
    sgui_window_set_title( wnd, "OpenGL Sample" );
    sgui_window_move_center( wnd );
    sgui_window_set_visible( wnd, SGUI_VISIBLE );

    /*
        A window with a rendering context object generates
        an SGUI_EXPOSE_EVENT every time the window systems
        aks the window to redraw itself.

        Redirect the redraw request to our redraw callback
        and print a string to show that we are redrawing.
     */
    sgui_event_connect( wnd, SGUI_EXPOSE_EVENT,
                        draw_callback, wnd, SGUI_VOID );

    sgui_event_connect( wnd, SGUI_EXPOSE_EVENT,
                        puts, "Readraw!", SGUI_VOID );

    /* main loop */
    lib->main_loop(lib);

    /* clean up */
    sgui_window_release_current( wnd );
    sgui_window_destroy( wnd );
    lib->destroy(lib);

    return 0;
}

