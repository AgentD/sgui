#include "sgui.h"

#include <stdio.h>
#include <GL/gl.h>



void glview_on_draw( sgui_widget* glview )
{
    (void)glview;

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
    sgui_font* font;
    unsigned char bg_color[3] = { 0, 0, 0 };

    sgui_init( );

    /* Load a fon for text drawing */
    font = sgui_font_load( "../../font/SourceSansPro-Regular.ttf", 16 );

    sgui_skin_set_default_font( font, NULL, NULL, NULL );

    /* create a window */
    wnd = sgui_window_create( NULL, 200, 200, SGUI_FIXED_SIZE );

    sgui_window_set_title( wnd, "OpenGL widget" );
    sgui_window_move_center( wnd );
    sgui_window_set_visible( wnd, SGUI_VISIBLE );

    /* create some widgets */
    text = sgui_static_text_create(10, 130, "OpenGL\302\256 subview widget");
    button = sgui_button_create( 10, 155, 75, 30, "Refresh" );
    gl_view = sgui_subview_create(wnd, 10, 10, 180, 120, SGUI_OPENGL_COMPAT);

    sgui_subview_set_background_color( gl_view, bg_color );

    /* hook callbacks */
    sgui_subview_set_draw_callback( gl_view, glview_on_draw );

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
    sgui_font_destroy( font );
    sgui_deinit( );

    return 0;
}

