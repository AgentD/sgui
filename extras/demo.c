#include "sgui.h"

#ifndef SGUI_NO_OPENGL
    #include <GL/gl.h>
#endif

#include <stdio.h>
#include <string.h>

#ifdef MACHINE_OS_UNIX
    #include <pthread.h>
    #include <unistd.h>

    typedef pthread_t thread_type;

    #define CREATE_THREAD( t, fun ) pthread_create( &(t), NULL, (fun), NULL )
    #define WAIT_THREAD( t ) pthread_join( (t), NULL )
    #define SLEEP_MS( ms ) usleep( (ms)*1000 )
#else
    #include <windows.h>

    typedef HANDLE thread_type;

    #define CREATE_THREAD( t, fun ) (t) = CreateThread( 0, 0, fun, 0, 0, 0 )
    #define WAIT_THREAD( t ) WaitForSingleObject( (t), INFINITE );\
                             CloseHandle( (t) )
    #define SLEEP_MS( ms ) Sleep( (ms) )
#endif



sgui_window *a, *b;
sgui_widget *p0, *p1, *p2, *p3, *tex, *butt, *tb, *c0, *c1, *c2, *i0, *i1;
sgui_widget *r0, *r1, *r2, *eb, *ebn, *ebp, *f, *gb, *ra, *rb, *rc, *tab;
sgui_widget *gl_view, *gl_view2, *gl_sub0, *gl_sub1;
sgui_widget *t1, *t2, *t3, *iv;
sgui_icon_cache* ic;
unsigned char image[128*128*4];
int running = 1;

const char* text =
    "Lorem <b>ipsum</b> dolor <i>sit</i> amet,\n"
    "<color=\"#FF0000\"><i>consectetuer</i> <b>adipiscing</b> elit.\n"
    "<color=\"#00FF00\"><b>Aenean <i>commodo</i> ligula <i>eget</i></b>\n"
    "<color=\"#0000FF\"><i>dolor. <b>Aenean</b> massa.</i>";



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

#ifdef MACHINE_OS_UNIX
void* gl_drawing_thread( void* arg )
#else
DWORD __stdcall gl_drawing_thread( LPVOID arg )
#endif
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

void print_password( sgui_widget* e )
{
    puts( sgui_edit_box_get_text( e ) );
}



int main( int argc, char** argv )
{
    int x, y, nogl=0;
    sgui_window_description desc = { NULL, NULL, 100, 100, 1, 0, 1,
                                     32, 24, 8, 4 };
    thread_type thread = 0;

    for( x=1; x<argc; ++x )
    {
        if( !strcmp( argv[x], "--nogl" ) )
            nogl = 1;
    }

    sgui_init( );

    a = sgui_window_create_desc( &desc );

    desc.resizeable = SGUI_FIXED_SIZE;
    b = sgui_window_create_desc( &desc );

    sgui_window_set_visible( a, SGUI_VISIBLE );
    sgui_window_set_visible( b, SGUI_VISIBLE );

    sgui_window_set_title( a, "resizeable" );
    sgui_window_set_title( b, "fixed size" );

    sgui_window_move( a, 200, 200 );
    sgui_window_move_center( b );

    sgui_window_set_size( a, 520, 420 );
    sgui_window_set_size( b, 200, 100 );

    /* create icon cache */
    ic = sgui_icon_cache_create( sgui_window_get_canvas( a ), 32, 32, 0 );

    sgui_icon_cache_add_icon( ic, 0, 16, 16 );
    sgui_icon_cache_add_icon( ic, 1, 16, 16 );
    sgui_icon_cache_add_icon( ic, 2, 16, 16 );

    /* generte images */
    for( y=0; y<16; ++y )
        for( x=0; x<16; ++x )
        {
            image[ (y*16 + x)*3     ] = x << 4;
            image[ (y*16 + x)*3 + 1 ] = y << 4;
            image[ (y*16 + x)*3 + 2 ] = 0x00;
        }

    sgui_icon_cache_load_icon( ic, 0, image, 16, SGUI_RGB8 );

    for( y=0; y<16; ++y )
        for( x=0; x<16; ++x )
        {
            image[ (y*16 + x)*3     ] = 0x00;
            image[ (y*16 + x)*3 + 1 ] = x << 4;
            image[ (y*16 + x)*3 + 2 ] = y << 4;
        }

    sgui_icon_cache_load_icon( ic, 1, image, 16, SGUI_RGB8 );

    for( y=0; y<16; ++y )
        for( x=0; x<16; ++x )
        {
            image[ (y*16 + x)*3     ] = y << 4;
            image[ (y*16 + x)*3 + 1 ] = 0x00;
            image[ (y*16 + x)*3 + 2 ] = x << 4;
        }

    sgui_icon_cache_load_icon( ic, 2, image, 16, SGUI_RGB8 );

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
    sgui_tab_group_add_tab( tab, "Input" );

    butt = sgui_button_create( 10, 275, 80, 30, "Button", 0 );
    tb = sgui_button_create( 95, 275, 80, 30, "Toggle", 1 );
    t1 = sgui_icon_button_create( 360,  35, 30, 30, ic, 0, 1 );
    t2 = sgui_icon_button_create( 360,  70, 30, 30, ic, 1, 1 );
    t3 = sgui_icon_button_create( 360, 105, 30, 30, ic, 2, 1 );
    eb = sgui_edit_box_create( 10, 195, 100, 100, 0 );
    ebn = sgui_edit_box_create( 10, 235, 100, 100, SGUI_EDIT_NUMERIC );
    ebp = sgui_edit_box_create( 120, 195, 100, 100, SGUI_EDIT_PASSWORD );

    sgui_button_group_connect( t1, NULL, t2   );
    sgui_button_group_connect( t2, t1,   t3   );
    sgui_button_group_connect( t3, t2,   NULL );
    sgui_button_set_state( t2, 1 );

    sgui_edit_box_set_text(eb,"An edit box test string for an edit box test");
    sgui_edit_box_set_text( ebn, "1337" );
    sgui_edit_box_set_text( ebp, "secret" );

    f = sgui_frame_create( 10, 25, 150, 150 );
    r0 = sgui_radio_button_create( 10, 100, "Option 1" );
    r1 = sgui_radio_button_create( 10, 126, "Option 2" );
    r2 = sgui_radio_button_create( 10, 152, "Option 3" );
    c0 = sgui_checkbox_create( 10,  10, "Checkbox 1" );
    c1 = sgui_checkbox_create( 10,  35, "Checkbox 2" );
    c2 = sgui_checkbox_create( 10,  60, "Checkbox 3" );

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
    ra = sgui_radio_button_create( 10, 25, "Option 1" );
    rb = sgui_radio_button_create( 10, 51, "Option 2" );
    rc = sgui_radio_button_create( 10, 77, "Option 3" );

    sgui_button_group_connect( ra, NULL,   rb );
    sgui_button_group_connect( rb, ra,     rc );
    sgui_button_group_connect( rc, rb,   NULL );
    sgui_button_set_state( ra, 1 );

    sgui_widget_add_child( gb, ra );
    sgui_widget_add_child( gb, rb );
    sgui_widget_add_child( gb, rc );

    sgui_tab_group_add_widget( tab, 0, butt );
    sgui_tab_group_add_widget( tab, 0, tb );
    sgui_tab_group_add_widget( tab, 0, t1 );
    sgui_tab_group_add_widget( tab, 0, t2 );
    sgui_tab_group_add_widget( tab, 0, t3 );
    sgui_tab_group_add_widget( tab, 0, gb );
    sgui_tab_group_add_widget( tab, 0, f );
    sgui_tab_group_add_widget( tab, 0, eb );
    sgui_tab_group_add_widget( tab, 0, ebn );
    sgui_tab_group_add_widget( tab, 0, ebp );

    /* static widget tab */
    sgui_tab_group_add_tab( tab, "Static" );

    tex = sgui_label_create( 10, 175, text );

    i0 = sgui_image_create(  10, 25, 128, 128, image, SGUI_RGBA8, 0, 1 );
    i1 = sgui_image_create( 150, 25, 128, 128, image, SGUI_RGBA8, 1, 1 );

    sgui_tab_group_add_widget( tab, 1, i0 );
    sgui_tab_group_add_widget( tab, 1, i1 );
    sgui_tab_group_add_widget( tab, 1, tex );

    /* output widget tab */
    sgui_tab_group_add_tab( tab, "Output" );

    p0 = sgui_progress_bar_create( 10,  25, 0, 0, 50, 300 );
    p1 = sgui_progress_bar_create( 10,  65, 1, 0, 50, 300 );
    p2 = sgui_progress_bar_create( 320, 25, 0, 1, 50, 300 );
    p3 = sgui_progress_bar_create( 355, 25, 1, 1, 50, 300 );

    sgui_tab_group_add_widget( tab, 2, p0 );
    sgui_tab_group_add_widget( tab, 2, p1 );
    sgui_tab_group_add_widget( tab, 2, p2 );
    sgui_tab_group_add_widget( tab, 2, p3 );

    /* view widgets */
    sgui_tab_group_add_tab( tab, "Views" );

    iv = sgui_icon_view_create( 15, 15, 200, 150, ic, 1 );

    sgui_icon_view_add_icon( iv,  10, 10, "icon", 0, (void*)0x01 );
    sgui_icon_view_add_icon( iv,  80, 80, "view", 1, (void*)0x02 );
    sgui_icon_view_add_icon( iv, 150, 20, "test", 2, (void*)0x03 );

    sgui_tab_group_add_widget( tab, 3, iv );

    /* OpenGL widget tab */
    if( !nogl )
    {
        sgui_tab_group_add_tab( tab, "OpenGL" );

        gl_view=sgui_subview_create(a,10,25,200,150,SGUI_OPENGL_COMPAT,NULL);
        gl_view2 = sgui_subview_create(a,250,25,200,150,
                                       SGUI_OPENGL_COMPAT,NULL);
        gl_sub0 = sgui_label_create( 45, 175, "Redraw on demand" );
        gl_sub1 = sgui_label_create( 275, 175, "Redraw continuous" );

        sgui_subview_set_draw_callback( gl_view, glview_on_draw );

        sgui_tab_group_add_widget( tab, 4, gl_view );
        sgui_tab_group_add_widget( tab, 4, gl_view2 );
        sgui_tab_group_add_widget( tab, 4, gl_sub0 );
        sgui_tab_group_add_widget( tab, 4, gl_sub1 );

        CREATE_THREAD( thread, gl_drawing_thread );
    }

    /* */
    sgui_window_add_widget( a, tab );

    /* hook callbacks */
    sgui_event_connect( butt, SGUI_BUTTON_OUT_EVENT,
                        print_password, ebp, SGUI_VOID );
    sgui_event_connect( ebp, SGUI_EDIT_BOX_TEXT_ENTERED,
                        print_password, ebp, SGUI_VOID );
    sgui_event_connect( (void*)0x01, SGUI_ICON_SELECTED_EVENT,
                        puts, "Icon 1 selected -> snap to grid", SGUI_VOID );
    sgui_event_connect( (void*)0x02, SGUI_ICON_SELECTED_EVENT,
                        puts, "Icon 2 selected -> sort icons", SGUI_VOID );
    sgui_event_connect( (void*)0x03, SGUI_ICON_SELECTED_EVENT,
                        puts, "Icon 3 selected", SGUI_VOID );

    sgui_event_connect( (void*)0x01, SGUI_ICON_SELECTED_EVENT,
                        sgui_icon_view_snap_to_grid, iv, SGUI_VOID );
    sgui_event_connect( (void*)0x02, SGUI_ICON_SELECTED_EVENT,
                        sgui_icon_view_sort, iv, SGUI_POINTER, NULL );

    sgui_main_loop( );

    if( !nogl )
    {
        running = 0;
        WAIT_THREAD( thread );
    }

    sgui_window_release_current( a );

    sgui_window_destroy( a );
    sgui_window_destroy( b );

    sgui_widget_destroy( gl_view );

    sgui_widget_destroy( gb );
    sgui_widget_destroy( f );
    sgui_widget_destroy( tab );

    sgui_widget_destroy( gl_sub1 );
    sgui_widget_destroy( gl_sub0 );
    sgui_widget_destroy( tex );

    sgui_widget_destroy( eb );
    sgui_widget_destroy( ebn );
    sgui_widget_destroy( ebp );

    sgui_widget_destroy( butt );
    sgui_widget_destroy( tb );
    sgui_widget_destroy( t1 );
    sgui_widget_destroy( t2 );
    sgui_widget_destroy( t3 );
    sgui_widget_destroy( c0 );
    sgui_widget_destroy( c1 );
    sgui_widget_destroy( c2 );
    sgui_widget_destroy( r0 );
    sgui_widget_destroy( r1 );
    sgui_widget_destroy( r2 );
    sgui_widget_destroy( ra );
    sgui_widget_destroy( rb );
    sgui_widget_destroy( rc );

    sgui_widget_destroy( p0 );
    sgui_widget_destroy( p1 );
    sgui_widget_destroy( p2 );
    sgui_widget_destroy( p3 );

    sgui_widget_destroy( i0 );
    sgui_widget_destroy( i1 );

    sgui_widget_destroy( iv );

    sgui_icon_cache_destroy( ic );

    sgui_deinit( );

    return 0;
}

