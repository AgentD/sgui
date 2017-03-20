/*
    This file is part of the sgui samples collection. I, David Oberhollenzer,
    author of this file hereby place the contents of this file into
    the public domain.
 */
/*
    Sorry for this mess. This is a demo program to display all sgui
    capabillities at once. The program grew over time and has no real
    purpose, so it's quite messy.

    If you want to checkout some simple examples on how to use sgui, take a
    look at the other source files in this directory.
 */
#include "sgui.h"

#ifdef SGUI_WINDOWS
    #include <windows.h>
#endif

#ifndef SGUI_NO_OPENGL
    #include <GL/gl.h>
#endif

#include <stdio.h>
#include <string.h>

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



/* all widgets are global, because, uh..... it just sort of happened */
sgui_window *a, *b;
sgui_widget *p0, *p1, *p2, *p3, *tex, *butt, *tb, *c0, *c1, *c2, *i0, *i1;
sgui_widget *r0, *r1, *r2, *eb, *ebn, *ebp, *f, *gb, *ra, *rb, *rc, *tab;
sgui_widget *gl_view, *gl_view2, *gl_sub0, *gl_sub1;
sgui_widget *t1, *t2, *t3, *iv, *s1, *s2, *s3, *s4;
sgui_item *item1, *item2, *item3;
sgui_icon_cache* ic;
sgui_model* model;
unsigned char image[128*128*4];
int running = 1;

const char* text =
    "Lorem <b>ipsum</b> dolor <i>sit</i> amet,\n"
    "<color=\"#FF0000\"><i>consectetuer</i> <b>adipiscing</b> elit.\n"
    "<color=\"#00FF00\"><b>Aenean <i>commodo</i> ligula <i>eget</i></b>\n"
    "<color=\"#0000FF\"><i>dolor. <b>Aenean</b> massa.</i>";



/* redraw callback for GL widgets */
void glview_on_draw( sgui_widget* glview )
{
    unsigned int w, h;

    sgui_widget_get_size( glview, &w, &h );

#ifndef SGUI_NO_OPENGL
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
#endif
}

/* redrawing thread for continously redrawing GL widget */
return_type THREADCALL gl_drawing_thread( arg_type arg )
{
    sgui_window* wnd = sgui_subview_get_window( gl_view2 );
    (void)arg;

    sgui_window_make_current( wnd );
    sgui_window_set_vsync( wnd, 1 );

    while( running )
    {
        glview_on_draw( gl_view2 );
        SLEEP_MS( 20 );
        sgui_window_swap_buffers( wnd );
    }

    sgui_window_release_current( wnd );
    return 0;
}

/* callback for password box, so we can see the text */
void print_password( sgui_widget* e )
{
    puts( sgui_pass_box_get_text( e ) );
}

/* callback for sliders to print out the new value */
void print_slider( const char* name, int value )
{
    printf( "%s: %d%%\n", name, value );
}


/*
    big-ass main function that does everything.

    The manuall creation of widgets is a mess. In an actual programm you would
    move this to a dedicated (possibly autogenerated) source file, or load it
    from a configuration file.
 */
int main( int argc, char** argv )
{
    sgui_widget* t;
    int x, y, nogl=0;
    sgui_window_description desc = {NULL, NULL, 100, 100, 0, 0, 32, 24, 8, 4};
    thread_type thread = 0;

    for( x=1; x<argc; ++x )
    {
        if( !strcmp( argv[x], "--nogl" ) )
            nogl = 1;
    }

    sgui_init( );

    /* create windows */
    b = sgui_window_create_desc( &desc );

    desc.flags |= SGUI_FIXED_SIZE;
    a = sgui_window_create_desc( &desc );

    sgui_window_set_visible( a, SGUI_VISIBLE );
    sgui_window_set_visible( b, SGUI_VISIBLE );

    sgui_window_set_title( b, "resizeable" );
    sgui_window_set_title( a, "fixed size" );

    sgui_window_move( a, 200, 200 );
    sgui_window_move_center( b );

    sgui_window_set_size( a, 520, 420 );
    sgui_window_set_size( b, 200, 100 );

    /* create icon cache */
    ic = sgui_icon_map_create( sgui_window_get_canvas( a ), 32, 32, 0 );

    sgui_icon_map_add_icon( ic, 0, 16, 16 );
    sgui_icon_map_add_icon( ic, 1, 16, 16 );
    sgui_icon_map_add_icon( ic, 2, 16, 16 );

    /* intialize model for MVC widget demo */
    model = sgui_simple_model_create( 1, ic );

    item1 = sgui_simple_model_add_item( model, NULL );
    sgui_simple_item_set_text( model, item1, 0, "foo" );
    sgui_simple_item_set_icon( model, item1, 0, sgui_icon_map_find(ic,0) );

    item2 = sgui_simple_model_add_item( model, NULL );
    sgui_simple_item_set_text( model, item2, 0, "bar" );
    sgui_simple_item_set_icon( model, item2, 0, sgui_icon_map_find(ic,1) );
    sgui_event_connect( item2, SGUI_ICON_SELECTED_EVENT,
                        puts, "Icon 2 selected -> sort icons", SGUI_VOID );

    item3 = sgui_simple_model_add_item( model, NULL );
    sgui_simple_item_set_text( model, item3, 0, "baz" );
    sgui_simple_item_set_icon( model, item3, 0, sgui_icon_map_find(ic,2) );

    /* generte and load images */
    for( y=0; y<16; ++y )
        for( x=0; x<16; ++x )
        {
            image[ (y*16 + x)*3     ] = x << 4;
            image[ (y*16 + x)*3 + 1 ] = y << 4;
            image[ (y*16 + x)*3 + 2 ] = 0x00;
        }

    sgui_icon_cache_load_icon( ic, sgui_icon_map_find(ic,0),
                               image, 16, SGUI_RGB8 );

    for( y=0; y<16; ++y )
        for( x=0; x<16; ++x )
        {
            image[ (y*16 + x)*3     ] = 0x00;
            image[ (y*16 + x)*3 + 1 ] = x << 4;
            image[ (y*16 + x)*3 + 2 ] = y << 4;
        }

    sgui_icon_cache_load_icon( ic, sgui_icon_map_find(ic,1),
                               image, 16, SGUI_RGB8 );

    for( y=0; y<16; ++y )
        for( x=0; x<16; ++x )
        {
            image[ (y*16 + x)*3     ] = y << 4;
            image[ (y*16 + x)*3 + 1 ] = 0x00;
            image[ (y*16 + x)*3 + 2 ] = x << 4;
        }

    sgui_icon_cache_load_icon( ic, sgui_icon_map_find(ic,2),
                               image, 16, SGUI_RGB8 );

    for( y=0; y<128; ++y )
        for( x=0; x<128; ++x )
        {
            image[ (y*128 + x)*4     ] = 0xFF * (x/128.0f);
            image[ (y*128 + x)*4 + 1 ] = 0xFF * (y/128.0f);
            image[ (y*128 + x)*4 + 2 ] = 0x00;
            image[ (y*128 + x)*4 + 3 ] = 0xFF * (x/128.0f);
        }

    tab = sgui_tab_group_create( 10, 10, 500, 400 );

    /* input widget tab */
    s1 = sgui_slider_create( 450, 35, 200, 1, 0, 100, 11 );
    s2 = sgui_slider_create( 420, 35, 200, 1, 0, 100, 0  );
    s3 = sgui_slider_create( 280, 310, 200, 0, 0, 100, 0  );
    s4 = sgui_slider_create( 280, 335, 200, 0, 0, 100, 11 );
    butt = sgui_button_create( 10, 275, 80, 30, "Button", SGUI_BUTTON );
    tb = sgui_button_create( 95, 275, 80, 30, "Toggle", SGUI_TOGGLE_BUTTON );
    t1 = sgui_icon_button_create(360,  35, 30, 30, ic,
                                 sgui_icon_map_find(ic,0), SGUI_TOGGLE_BUTTON);
    t2 = sgui_icon_button_create(360,  70, 30, 30, ic,
                                 sgui_icon_map_find(ic,1), SGUI_TOGGLE_BUTTON);
    t3 = sgui_icon_button_create(360, 105, 30, 30, ic,
                                 sgui_icon_map_find(ic,2), SGUI_TOGGLE_BUTTON);
    eb = sgui_edit_box_create( 10, 195, 100, 100 );
    ebn = sgui_spin_box_create( 10, 235, 100, -2000, 2000, 1337, 42, 1 );
    ebp = sgui_pass_box_create( 120, 195, 100, 10 );

    sgui_button_group_connect( t1, NULL, t2   );
    sgui_button_group_connect( t2, t1,   t3   );
    sgui_button_group_connect( t3, t2,   NULL );
    sgui_button_set_state( t2, 1 );

    sgui_edit_box_set_text(eb,"An edit box test string for an edit box test");
    sgui_edit_box_set_text( ebp, "secret" );

    f = sgui_frame_create( 10, 25, 150, 150 );
    r0 = sgui_button_create( 10, 100, 0, 0, "Option 1", SGUI_RADIO_BUTTON );
    r1 = sgui_button_create( 10, 126, 0, 0, "Option 2", SGUI_RADIO_BUTTON );
    r2 = sgui_button_create( 10, 152, 0, 0, "Option 3", SGUI_RADIO_BUTTON );
    c0 = sgui_button_create( 10,  10, 0, 0, "Checkbox 1", SGUI_CHECKBOX );
    c1 = sgui_button_create( 10,  35, 0, 0, "Checkbox 2", SGUI_CHECKBOX );
    c2 = sgui_button_create( 10,  60, 0, 0, "Checkbox 3", SGUI_CHECKBOX );

    sgui_button_group_connect( r0, NULL,   r1 );
    sgui_button_group_connect( r1, r0,     r2 );
    sgui_button_group_connect( r2, r1,   NULL );
    sgui_button_set_state( r0, 1 );

    sgui_widget_add_child( f, c0 );
    sgui_widget_add_child( f, c1 );
    sgui_widget_add_child( f, c2 );
    sgui_widget_add_child( f, r0 );
    sgui_widget_add_child( f, r1 );
    sgui_widget_add_child( f, r2 );

    gb = sgui_group_box_create( 200, 25, 130, 110, "Group Box" );
    ra = sgui_button_create( 10, 25, 0, 0, "Option 1", SGUI_RADIO_BUTTON );
    rb = sgui_button_create( 10, 51, 0, 0, "Option 2", SGUI_RADIO_BUTTON );
    rc = sgui_button_create( 10, 77, 0, 0, "Option 3", SGUI_RADIO_BUTTON );

    sgui_button_group_connect( ra, NULL,   rb );
    sgui_button_group_connect( rb, ra,     rc );
    sgui_button_group_connect( rc, rb,   NULL );
    sgui_button_set_state( ra, 1 );

    sgui_widget_add_child( gb, ra );
    sgui_widget_add_child( gb, rb );
    sgui_widget_add_child( gb, rc );

    t = sgui_tab_create( tab, "Input" );
    sgui_widget_add_child( tab, t );

    sgui_widget_add_child( t, f );
    sgui_widget_add_child( t, gb );
    sgui_widget_add_child( t, t1 );
    sgui_widget_add_child( t, t2 );
    sgui_widget_add_child( t, t3 );
    sgui_widget_add_child( t, s2 );
    sgui_widget_add_child( t, s1 );
    sgui_widget_add_child( t, eb );
    sgui_widget_add_child( t, ebp );
    sgui_widget_add_child( t, ebn );
    sgui_widget_add_child( t, butt );
    sgui_widget_add_child( t, tb );
    sgui_widget_add_child( t, s3 );
    sgui_widget_add_child( t, s4 );

    /* static widget tab */
    tex = sgui_label_create( 10, 175, text );

    i0 = sgui_image_create(  10, 25, 128, 128, image, SGUI_RGBA8,
                            SGUI_IMAGE_KEEP_PTR );
    i1 = sgui_image_create( 150, 25, 128, 128, image, SGUI_RGBA8,
                            SGUI_IMAGE_KEEP_PTR | SGUI_IMAGE_BLEND );

    t = sgui_tab_create( tab, "Static" );
    sgui_widget_add_child( tab, t );
    sgui_widget_add_child( t, i0 );
    sgui_widget_add_child( t, i1 );
    sgui_widget_add_child( t, tex );

    /* output widget tab */
    p0 = sgui_progress_bar_create( 10, 25, 0, 50, 300 );
    p1 = sgui_progress_bar_create( 10, 65, SGUI_PROGRESS_BAR_CONTINUOUS,
                                   50, 300 );
    p2 = sgui_progress_bar_create( 320, 25, SGUI_PROGRESS_BAR_VERTICAL,
                                   50, 300 );
    p3 = sgui_progress_bar_create( 355, 25, SGUI_PROGRESS_BAR_CONTINUOUS |
                                   SGUI_PROGRESS_BAR_VERTICAL, 50, 300 );

    t = sgui_tab_create( tab, "Output" );
    sgui_widget_add_child( tab, t );
    sgui_widget_add_child( t, p0 );
    sgui_widget_add_child( t, p1 );
    sgui_widget_add_child( t, p2 );
    sgui_widget_add_child( t, p3 );

    /* view widgets */
    iv = sgui_icon_view_create( 15, 15, 200, 150, model, 1, 0, 0 );
    sgui_icon_view_populate( iv, NULL );

    t = sgui_tab_create( tab, "Views" );
    sgui_widget_add_child( tab, t );
    sgui_widget_add_child( t, iv );

    /* OpenGL widget tab */
    if( !nogl )
    {
        gl_view=sgui_subview_create(a,10,25,200,150,SGUI_OPENGL_COMPAT,NULL);
        gl_view2 = sgui_subview_create(a,250,25,200,150,
                                       SGUI_OPENGL_COMPAT,NULL);
        gl_sub0 = sgui_label_create( 45, 175, "Redraw on demand" );
        gl_sub1 = sgui_label_create( 275, 175, "Redraw continuous" );

        sgui_subview_set_draw_callback( gl_view, glview_on_draw );

        t = sgui_tab_create( tab, "OpenGL" );
        sgui_widget_add_child( tab, t );
        sgui_widget_add_child( t, gl_view );
        sgui_widget_add_child( t, gl_view2 );
        sgui_widget_add_child( t, gl_sub0 );
        sgui_widget_add_child( t, gl_sub1 );

        CREATE_THREAD( thread, gl_drawing_thread );
    }

    /* add the tab group itself to the window */
    sgui_window_add_widget( a, tab );

    /* hook callbacks */
    sgui_event_connect( butt, SGUI_BUTTON_OUT_EVENT,
                        print_password, ebp, SGUI_VOID );
    sgui_event_connect( ebp, SGUI_EDIT_BOX_TEXT_ENTERED,
                        print_password, ebp, SGUI_VOID );

    sgui_event_connect( s1, SGUI_SLIDER_CHANGED_EVENT,
                        print_slider, "slider 1", SGUI_FROM_EVENT, SGUI_I );
    sgui_event_connect( s2, SGUI_SLIDER_CHANGED_EVENT,
                        print_slider, "slider 2", SGUI_FROM_EVENT, SGUI_I );
    sgui_event_connect( s3, SGUI_SLIDER_CHANGED_EVENT,
                        print_slider, "slider 3", SGUI_FROM_EVENT, SGUI_I );
    sgui_event_connect( s4, SGUI_SLIDER_CHANGED_EVENT,
                        print_slider, "slider 4", SGUI_FROM_EVENT, SGUI_I );

    sgui_event_connect( item1, SGUI_ICON_SELECTED_EVENT,
                        puts, "Icon 1 selected -> snap to grid", SGUI_VOID );
    sgui_event_connect( item1, SGUI_ICON_SELECTED_EVENT,
                        sgui_icon_view_snap_to_grid, iv, SGUI_VOID );
    sgui_event_connect( item2, SGUI_ICON_SELECTED_EVENT,
                        sgui_icon_view_sort, iv, SGUI_POINTER, NULL );
    sgui_event_connect( item3, SGUI_ICON_SELECTED_EVENT,
                        puts, "Icon 3 selected", SGUI_VOID );

    sgui_event_connect( b, SGUI_MOUSE_MOVE_EVENT,
                        sgui_window_make_topmost, b, SGUI_VOID );

    /* enter main loop */
    sgui_main_loop( );

    /* cleanup */
    if( !nogl )
    {
        running = 0;
        WAIT_THREAD( thread );
    }

    sgui_window_release_current( a );

    sgui_window_destroy( a );
    sgui_window_destroy( b );

    sgui_widget_destroy_all_children( tab );
    sgui_widget_destroy( tab );

    sgui_model_destroy( model );
    sgui_icon_cache_destroy( ic );

    sgui_deinit( );

    return 0;
}

