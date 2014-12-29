/*
    This file is part of the sgui samples collection. I, David Oberhollenzer,
    author of this file hereby place the contents of this file into
    the public domain.
 */
/*
    This small programm is supposed to demonstrate how to use the sub-window
    manager module with various rendering contexts through sgui.
 */
#include "sgui.h"
#include "sgui_ctx_wm.h"
#include "sgui_ctx_window.h"

#include <stddef.h>
#include <string.h>
#include <stdio.h>

#ifdef SGUI_WINDOWS
    #include <windows.h>
#endif
#include <GL/gl.h>



int main( void )
{
    sgui_widget *butt, *check, *check2, *label;
    sgui_window_description desc;
    sgui_window* subwnd;
    sgui_window* subwnd2;
    sgui_window* wnd;
    sgui_ctx_wm* wm;
    int selection;

    puts( "Select rendering backend: " );
    puts( " 1) OpenGL(R) old" );
    puts( " 2) OpenGL(R) 3.0+ core" );
    puts( " 3) Direct3D(R) 9" );
    puts( " 4) Direct3D(R) 11" );
    puts( "\n 0) quit\n" );
    selection = 0;
    scanf( "%d", &selection );

    switch( selection )
    {
    case 1: selection = SGUI_OPENGL_COMPAT; break;
    case 2: selection = SGUI_OPENGL_CORE;   break;
    case 3: selection = SGUI_DIRECT3D_9;    break;
    case 4: selection = SGUI_DIRECT3D_11;   break;
    default:
        return 0;
    }

    sgui_init( );

    /* create a window */
    memset( &desc, 0, sizeof(desc) );

    desc.width          = 800;
    desc.height         = 600;
    desc.flags          = SGUI_FIXED_SIZE|SGUI_DOUBLEBUFFERED;
    desc.backend        = selection;
    desc.bits_per_pixel = 32;

    wnd = sgui_window_create_desc( &desc );

    sgui_window_set_title( wnd, "subwm" );
    sgui_window_move_center( wnd );
    sgui_window_set_visible( wnd, SGUI_VISIBLE );

    sgui_window_make_current( wnd );
    sgui_window_set_vsync( wnd, 1 );

    /* */
    wm = sgui_ctx_wm_create( wnd );

    sgui_window_set_userptr( wnd, wm );
    sgui_window_on_event( wnd,
                          (sgui_window_callback)sgui_ctx_wm_inject_event );

    /* create some sub windows */
    subwnd = sgui_ctx_wm_create_window( wm, 256, 128, 0 );
    subwnd2 = sgui_ctx_wm_create_window( wm, 256, 128, 0 );

    sgui_window_set_visible( subwnd, 1 );
    sgui_window_set_visible( subwnd2, 1 );

    sgui_window_move( subwnd2, 10, 10 );

    sgui_window_set_title( subwnd, "Sub Window" );
    sgui_window_set_title( subwnd2, "Another Window" );

    /* create a few widgets */
    butt = sgui_button_create( 10, 35, 60, 25, "Button", 0 );
    check = sgui_checkbox_create( 10, 65, "OpenGL" );
    check2 = sgui_checkbox_create( 10, 90, "Texture" );

    label = sgui_label_create( 10, 35, "Hello, world!\n\n"
                                       "From a <b><i>sub</i></b> window." );

    sgui_button_set_state( check, 1 );
    sgui_button_set_state( check2, 1 );

    /* add the widgets to the sub windows */
    sgui_window_add_widget( subwnd, butt );
    sgui_window_add_widget( subwnd, check );
    sgui_window_add_widget( subwnd, check2 );

    sgui_window_add_widget( subwnd2, label );

    /* main loop */
    while( sgui_main_loop_step( ) )
    {
        glClear( GL_COLOR_BUFFER_BIT );
        sgui_ctx_wm_draw_gui( wm );
        sgui_window_swap_buffers( wnd );
    }

    /* clean up */
    sgui_window_destroy( subwnd );
    sgui_window_destroy( subwnd2 );
    sgui_widget_destroy( butt );
    sgui_widget_destroy( check );
    sgui_widget_destroy( check2 );
    sgui_widget_destroy( label );

    sgui_window_release_current( wnd );
    sgui_window_destroy( wnd );
    sgui_deinit( );

    return 0;
}

