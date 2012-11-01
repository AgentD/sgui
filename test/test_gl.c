#include <GL/glfw.h>

#include "sgui.h"
#include "sgui_opengl.h"
#include "sgui_screen.h"


sgui_screen* scr;


/* translates window to canvas coordinates */
void translate_pos( int* x, int* y )
{
    *x -= 800 * 0.2f;
    *y -= 600 * 0.1f;

    *x = (256*(*x)) / (800.0f * 0.6f);
    *y = (256*(*y)) / (600.0f * 0.8f);
}

void mouse_fun( int x, int y )
{
    sgui_event e;

    translate_pos( &x, &y );

    e.mouse_move.x = x;
    e.mouse_move.y = y;

    sgui_screen_inject_event( scr, SGUI_MOUSE_MOVE_EVENT, &e );
}

void mouse_pos_fun( int button, int state )
{
    sgui_event e;
    int type = SGUI_MOUSE_RELEASE_EVENT;

    if( state == GLFW_PRESS )
        type = SGUI_MOUSE_PRESS_EVENT;

    switch( button )
    {
    case GLFW_MOUSE_BUTTON_1:
        e.mouse_press.button = SGUI_MOUSE_BUTTON_LEFT;
        break;
    case GLFW_MOUSE_BUTTON_2:
        e.mouse_press.button = SGUI_MOUSE_BUTTON_MIDDLE;
        break;
    case GLFW_MOUSE_BUTTON_3:
        e.mouse_press.button = SGUI_MOUSE_BUTTON_RIGHT;
        break;
    default:
        return;
    }

    glfwGetMousePos( &e.mouse_press.x, &e.mouse_press.y );
    translate_pos( &e.mouse_press.x, &e.mouse_press.y );

    sgui_screen_inject_event( scr, type, &e );
}

int main( void )
{
    sgui_canvas* cv;
    GLuint tex;
    unsigned char color[4];
    sgui_widget *pbar, *c0, *c1, *c2, *butt;
    sgui_font *font, *font_bold, *font_ital, *font_boit;

    /* create window */
    glfwInit( );
    glfwOpenWindow( 800, 600, 8, 8, 8, 8, 24, 8, GLFW_WINDOW );
    glfwSetWindowTitle( "sgui OpenGL test" );

    glfwSetMousePosCallback( mouse_fun );
    glfwSetMouseButtonCallback( mouse_pos_fun );

    glViewport( 0, 0, 800, 600 );

    sgui_init( );

    font = sgui_font_load( NULL, "font/SourceSansPro-Regular.ttf" );
    font_bold = sgui_font_load( NULL, "font/SourceSansPro-Semibold.ttf" );
    font_ital = sgui_font_load( NULL, "font/SourceSansPro-It.ttf" );
    font_boit = sgui_font_load( NULL, "font/SourceSansPro-SemiboldIt.ttf" );

    sgui_skin_set_default_font( font, font_bold, font_ital,
                                font_boit, 16 );

    /* create canvas and screen */
    cv = sgui_opengl_canvas_create( 256, 256 );
    scr = sgui_screen_create( cv );

    sgui_skin_get_window_background_color( color );
    sgui_canvas_set_background_color( cv, color );
    sgui_canvas_clear( cv, NULL );

    pbar = sgui_progress_bar_create( 10, 10, SGUI_PROGRESS_BAR_STIPPLED, 0,
                                     50, 200 );

    c0 = sgui_checkbox_create( 10,  50, "Checkbox 1" );
    c1 = sgui_checkbox_create( 10,  75, "Checkbox 2" );
    c2 = sgui_checkbox_create( 10, 100, "Checkbox 3" );

    butt = sgui_button_create( 10, 150, 100, 30, "Button" );

    sgui_screen_add_widget( scr, pbar );
    sgui_screen_add_widget( scr, butt );
    sgui_screen_add_widget( scr, c0 );
    sgui_screen_add_widget( scr, c1 );
    sgui_screen_add_widget( scr, c2 );

    /* bind Canvas texture */
    tex = sgui_opengl_canvas_get_texture( cv );

    glBindTexture( GL_TEXTURE_2D, tex );
    glEnable( GL_TEXTURE_2D );

    glfwSwapInterval( 1 );

    /* main loop */
    while( glfwGetWindowParam( GLFW_OPENED ) )
    {
        sgui_screen_update( scr );

        glClear( GL_COLOR_BUFFER_BIT );

        glBegin( GL_QUADS );
        glVertex2f( -0.6f,  0.8f ); glTexCoord2f( 1.0f, 0.0f );
        glVertex2f(  0.6f,  0.8f ); glTexCoord2f( 1.0f, 1.0f );
        glVertex2f(  0.6f, -0.8f ); glTexCoord2f( 0.0f, 1.0f );
        glVertex2f( -0.6f, -0.8f ); glTexCoord2f( 0.0f, 0.0f );
        glEnd( );

        glfwSwapBuffers( );
    }

    /* cleanup */
    sgui_button_destroy( butt );
    sgui_button_destroy( c0 );
    sgui_button_destroy( c1 );
    sgui_button_destroy( c2 );
    sgui_progress_bar_destroy( pbar );
    sgui_screen_destroy( scr );
    sgui_opengl_canvas_destroy( cv );
    glfwCloseWindow( );
    glfwTerminate( );

    sgui_deinit( );

    return 0;
}

