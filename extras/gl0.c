#include "sgui.h"

#include <GL/gl.h>
#include <stdio.h>



void window_callback( sgui_window* wnd, int type, sgui_event* event )
{
    (void)wnd; (void)event;

    if( type == SGUI_EXPOSE_EVENT )
    {
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

        printf( "Readraw!\n" );
    }
}

void scrollbar_callback( void* userptr, int new_offset, int delta )
{
    sgui_widget* pbar = userptr;
    (void)delta;

    sgui_progress_bar_set_progress( pbar, new_offset );
}



int main( void )
{
    sgui_window* wnd;
    sgui_window_description desc;
    sgui_widget* pbar;
    sgui_widget* sbar;

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

    /* create a few widgets */
    pbar = sgui_progress_bar_create( 10, 10, SGUI_PROGRESS_BAR_STIPPLED,
                                     SGUI_PROGRESS_BAR_HORIZONTAL,
                                     40, 280 );

    sbar = sgui_scroll_bar_create( 10, 45, SGUI_SCROLL_BAR_HORIZONTAL,
                                   280, 100, 10 );

    sgui_scroll_bar_set_offset( sbar, 40 );

    sgui_window_add_widget( wnd, pbar );
    sgui_window_add_widget( wnd, sbar );

    /* hook event callbacks */
    sgui_window_on_event( wnd, window_callback );
    sgui_scroll_bar_on_scroll( sbar, scrollbar_callback, pbar );

    /* main loop */
    sgui_main_loop( );

    /* clean up */
    sgui_window_make_current( NULL );
    sgui_window_destroy( wnd );
    sgui_widget_destroy( pbar );
    sgui_widget_destroy( sbar );
    sgui_deinit( );

    return 0;
}

