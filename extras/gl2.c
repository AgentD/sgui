/*
    This file is part of the sgui samples collection. I, David Oberhollenzer,
    author of this file hereby place the contents of this file into
    the public domain.
 */
/*
    This small programm is supposed to demonstrate how to create a window
    an embedded OpenGL rendering widget through sgui.
 */
#include "sgui.h"

#include <stdio.h>

#ifdef SGUI_WINDOWS
    #include <windows.h>
#endif
#include <GL/gl.h>



void glview_on_draw( sgui_widget* glview )
{
    unsigned int w, h;

    sgui_widget_get_size( glview, &w, &h );

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
}

int main( void )
{
    sgui_window* wnd;
    sgui_widget* button;
    sgui_widget* text;
    sgui_widget* gl_view;
    sgui_lib* lib;

    lib = sgui_init(NULL);

    /* create a normal window and make it visible */
    wnd = sgui_window_create( NULL, 200, 200, SGUI_FIXED_SIZE );

    sgui_window_set_title( wnd, "OpenGL widget" );
    sgui_window_move_center( wnd );
    sgui_window_set_visible( wnd, SGUI_VISIBLE );

    /* create some widgets */
    text = sgui_label_create(10, 130, "OpenGL\302\256 subview widget");
    button = sgui_button_create( 10, 155, 75, 30, "Refresh", SGUI_BUTTON );

    /*
        Create a sub view widget. The sub-view widget manages a sub-window.

        The first argument is the parent window. The second and third
        arguments are the position of the sub view widget (10,10), followed
        by the size of the sub-window (180x120).

        The sixth argument is the window backend to use (OpenGL compatibillity
        profile).

        The last argument would allow us to supply a supply a window
        description structure for more fine grained controll. We don't
        use this, so we set it to NULL.
     */
    gl_view = sgui_subview_create( wnd, 10, 10, 180, 120,
                                   SGUI_OPENGL_COMPAT, NULL );

    /*
        The sub-view widget has a callback that it calls when the
        window it manages wants to be redrawn.
     */
    sgui_subview_set_draw_callback( gl_view, glview_on_draw );

    /*
        The functon sgui_subview_refresh forces the window of the
        sub-view to redraw itself.
     */
    sgui_event_connect( button, SGUI_BUTTON_OUT_EVENT,
                        sgui_subview_refresh, gl_view, SGUI_VOID );

    /* add widgets to the window */
    sgui_window_add_widget( wnd, text );
    sgui_window_add_widget( wnd, button );
    sgui_window_add_widget( wnd, gl_view );

    /* main loop */
    lib->main_loop(lib);

    /* clean up */
    sgui_window_destroy( wnd );
    sgui_widget_destroy( gl_view );
    sgui_widget_destroy( text );
    sgui_widget_destroy( button );
    lib->destroy(lib);

    return 0;
}

