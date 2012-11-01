#include <GL/glfw.h>

#include "sgui.h"
#include "sgui_opengl.h"


int main( void )
{
    sgui_canvas* cv;
    sgui_rect r;
    GLuint tex;
    sgui_font* font;
    unsigned char color[3];

    /* create window */
    glfwInit( );
    glfwOpenWindow( 800, 600, 8, 8, 8, 8, 24, 8, GLFW_WINDOW );
    glfwSetWindowTitle( "sgui OpenGL test" );

    sgui_init( );

    /* create canvas */
    cv = sgui_opengl_canvas_create( 256, 256 );

    font = sgui_font_load( NULL, "font/SourceSansPro-Regular.ttf" );

    /* draw to the canvas */
    color[0] = color[1] = color[2] = 0x64;

    sgui_canvas_set_background_color( cv, color );
    sgui_canvas_clear( cv, NULL );

    sgui_rect_set_size( &r, 0, 0, 256, 256 );
    sgui_canvas_begin( cv, &r );

    color[0] = 0xFF; color[1] = color[2] = 0x00; color[3] = 0xFF;
    sgui_rect_set_size( &r, 10, 10, 100, 100 );
    sgui_canvas_draw_box( cv, &r, color, SCF_RGBA8 );

    color[0] = color[1] = color[2] = color[3] = 0x80;
    sgui_rect_set_size( &r, 20, 20, 40, 40 );
    sgui_canvas_draw_box( cv, &r, color, SCF_RGBA8 );

    color[0] = color[1] = color[2] = color[3] = 0xFF;
    sgui_canvas_draw_line( cv, 15, 62, 90, 1, color, SCF_RGB8 );
    sgui_font_draw_text_plain( cv, 14, 60, font, 16, color, "Hello OpenGL", -1 );

    sgui_canvas_end( cv );

    /* bind Canvas texture */
    tex = sgui_opengl_canvas_get_texture( cv );

    glBindTexture( GL_TEXTURE_2D, tex );
    glEnable( GL_TEXTURE_2D );

    glfwSwapInterval( 1 );

    /* main loop */
    while( glfwGetWindowParam( GLFW_OPENED ) )
    {
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
    sgui_opengl_canvas_destroy( cv );
    glfwCloseWindow( );
    glfwTerminate( );

    sgui_font_destroy( font );

    sgui_deinit( );

    return 0;
}

