/*
    This file is part of the sgui samples collection. I, David Oberhollenzer,
    author of this file hereby place the contents of this file into
    the public domain.
 */
/*
    This small programm is supposed to demonstrate how to create a window
    an embedded OpenGL rendering widget through sgui and how to use a manual
    event/drawing loop in a thread for real-time rendering applications.
 */
#include "sgui.h"

#include <stdio.h>

#ifdef SGUI_WINDOWS
    #include <windows.h>
#endif
#include <GL/gl.h>

/* crude wrapper for system thread functions */
#ifdef SGUI_UNIX
    #include <pthread.h>
    #include <unistd.h>

    typedef pthread_t thread_type;
    typedef void* return_type;
    typedef void* arg_type;

    #define CREATE_THREAD( t, fun ) pthread_create( &(t), NULL, (fun), NULL )
    #define WAIT_THREAD( t ) pthread_join( (t), NULL )
    #define SLEEP_MS( ms ) usleep( (ms)*1000 )

    #define THREADCALL
#elif defined(SGUI_WINDOWS)
    typedef HANDLE thread_type;
    typedef DWORD return_type;
    typedef LPVOID arg_type;

    #define CREATE_THREAD( t, fun ) (t) = CreateThread( 0, 0, fun, 0, 0, 0 )
    #define WAIT_THREAD( t ) WaitForSingleObject( (t), INFINITE );\
                             CloseHandle( (t) )
    #define SLEEP_MS( ms ) Sleep( (ms) )

    #define THREADCALL __stdcall
#endif



static sgui_widget* gl_view;
static int running = 1;



/*
    A thread function that continously redraws the content of
    the OpenGL sub-view widget.
 */
return_type THREADCALL gl_drawing_thread( arg_type arg )
{
    unsigned int w, h;
    sgui_window* wnd;
    (void)arg;

    /* grab the window managed by the subview widget */
    sgui_widget_get_size( gl_view, &w, &h );
    wnd = sgui_subview_get_window( gl_view );

    /* make the context of the sub-window current to this thread */
    sgui_window_make_current( wnd );
    sgui_window_set_vsync( wnd, 1 );

    /* drawing loop */
    while( running )
    {
        /* draw a triangle and rotate it a bit every frame */
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

        /* swap buffers */
        SLEEP_MS( 20 );
        sgui_window_swap_buffers( wnd );
    }

    /* release the context and exit the thread */
    sgui_window_release_current( wnd );
    return 0;
}

int main( void )
{
    thread_type thread = 0;
    sgui_widget* text;
    sgui_window* wnd;

    sgui_init( );

    /* create a window */
    wnd = sgui_window_create( NULL, 200, 160, SGUI_FIXED_SIZE );

    sgui_window_set_title( wnd, "OpenGL widget" );
    sgui_window_move_center( wnd );
    sgui_window_set_visible( wnd, SGUI_VISIBLE );

    /* create widgets */
    text = sgui_label_create( 30, 130, "GL Drawing Thread" );

    /*
        create a subview widget with an OpenGL context.
        See gl2.c for further explanation.
     */
    gl_view = sgui_subview_create( wnd, 10, 10, 180, 120,
                                   SGUI_OPENGL_COMPAT, NULL );

    /* add widgets to the window */
    sgui_window_add_widget( wnd, text );
    sgui_window_add_widget( wnd, gl_view );

    /* create drawing thread */
    CREATE_THREAD( thread, gl_drawing_thread );

    /* main loop */
    sgui_main_loop( );

    /* wait for thread to terminate */
    running = 0;
    WAIT_THREAD( thread );

    /* clean up */
    sgui_window_destroy( wnd );
    sgui_widget_destroy( gl_view );
    sgui_widget_destroy( text );
    sgui_deinit( );

    return 0;
}

