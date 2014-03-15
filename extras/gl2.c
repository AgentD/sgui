#include "sgui.h"

#include <stdio.h>
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

    sgui_init( );

    /* create a window */
    wnd = sgui_window_create( NULL, 200, 200, SGUI_FIXED_SIZE );

    sgui_window_set_title( wnd, "OpenGL widget" );
    sgui_window_move_center( wnd );
    sgui_window_set_visible( wnd, SGUI_VISIBLE );

    /* create some widgets */
    text = sgui_static_text_create(10, 130, "OpenGL\302\256 subview widget");
    button = sgui_button_create( 10, 155, 75, 30, "Refresh" );
    gl_view = sgui_subview_create( wnd, 10, 10, 180, 120,
                                   SGUI_OPENGL_COMPAT, NULL );

    /* hook callbacks */
    sgui_subview_set_draw_callback( gl_view, glview_on_draw );
    sgui_event_connect( button, SGUI_BUTTON_CLICK_EVENT, 1,
                        sgui_subview_refresh, gl_view, SGUI_VOID );

    /* add widgets to the window */
    sgui_window_add_widget( wnd, text );
    sgui_window_add_widget( wnd, button );
    sgui_window_add_widget( wnd, gl_view );

    /* main loop */
    sgui_main_loop( );

    /* clean up */
    sgui_window_destroy( wnd );
    sgui_widget_destroy( gl_view );
    sgui_widget_destroy( text );
    sgui_widget_destroy( button );
    sgui_deinit( );

    return 0;
}

