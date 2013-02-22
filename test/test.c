#include "sgui.h"

#ifndef SGUI_NO_OPENGL
    #include <GL/gl.h>
#endif

#include <stdio.h>



sgui_window *a, *b;
sgui_widget *p0, *p1, *p2, *p3, *tex, *butt, *c0, *c1, *c2, *i0, *i1;
sgui_widget *r0, *r1, *r2, *eb, *f, *gb, *ra, *rb, *rc, *tab;
unsigned char image[128*128*4];

sgui_window* gl_sub;

const char* text =
    "Lorem <b>ipsum</b> dolor <i>sit</i> amet,\n"
    "<color=\"#FF0000\"><i>consectetuer</i> <b>adipiscing</b> elit.\n"
    "<color=\"#00FF00\"><b>Aenean <i>commodo</i> ligula <i>eget</i></b>\n"
    "<color=\"#0000FF\"><i>dolor. <b>Aenean</b> massa.</i>";



void gl_sub_cb( sgui_window* wnd, int type, sgui_event* event )
{
    unsigned int w, h;
    (void)event;
    (void)wnd;

    if( type == SGUI_EXPOSE_EVENT )
    {
        sgui_window_get_size( wnd, &w, &h );

#ifndef SGUI_NO_OPENGL
        glViewport( 0, 0, w, h );

        glClear( GL_COLOR_BUFFER_BIT );

        glBegin( GL_TRIANGLES );
        glColor3f( 1.0f, 0.0f, 0.0f );
        glVertex2f( -0.5f, -0.5f );
        glColor3f( 0.0f, 1.0f, 0.0f );
        glVertex2f(  0.5f, -0.5f );
        glColor3f( 0.0f, 0.0f, 1.0f );
        glVertex2f(  0.0f,  0.5f );
        glEnd( );
#endif

        sgui_window_swap_buffers( wnd );
    }
}



int main( void )
{
    int x, y;
    sgui_font* font;
    sgui_font* font_bold;
    sgui_font* font_ital;
    sgui_font* font_boit;

    sgui_init( );

    a = sgui_window_create( NULL, 400, 300, SGUI_RESIZEABLE, 0 );
    b = sgui_window_create( NULL, 100, 100, SGUI_FIXED_SIZE, 0 );

    sgui_window_set_visible( a, SGUI_VISIBLE );
    sgui_window_set_visible( b, SGUI_VISIBLE );

    sgui_window_set_title( a, "resizeable" );
    sgui_window_set_title( b, "fixed size" );

    sgui_window_move( a, 200, 200 );
    sgui_window_move_center( b );

    sgui_window_set_size( a, 520, 420 );
    sgui_window_set_size( b, 200, 100 );

    /* */
    for( y=0; y<128; ++y )
        for( x=0; x<128; ++x )
        {
            image[ (y*128 + x)*4     ] = 0xFF * (x/128.0f);
            image[ (y*128 + x)*4 + 1 ] = 0xFF * (y/128.0f);
            image[ (y*128 + x)*4 + 2 ] = 0x00;
            image[ (y*128 + x)*4 + 3 ] = 0xFF * (x/128.0f);
        }

    font     =sgui_font_load("../../font/SourceSansPro-Regular.ttf",16);
    font_bold=sgui_font_load("../../font/SourceSansPro-Semibold.ttf",16);
    font_ital=sgui_font_load("../../font/SourceSansPro-It.ttf",16);
    font_boit=sgui_font_load("../../font/SourceSansPro-SemiboldIt.ttf",16);

    sgui_skin_set_default_font( font, font_bold, font_ital, font_boit );

    tab = sgui_tab_group_create( 10, 10, 500, 400 );

    /* input widget tab */
    sgui_tab_group_add_tab( tab, "Input" );

    butt = sgui_button_create( 10, 270, 80, 30, "Button" );
    eb = sgui_edit_box_create( 10, 220, 100, 100 );

    sgui_edit_box_set_text(eb,"An edit box test string for an edit box test");

    f = sgui_frame_create( 10, 50, 150, 150 );
    r0 = sgui_radio_button_create( 10, 100, "Option 1" );
    r1 = sgui_radio_button_create( 10, 125, "Option 2" );
    r2 = sgui_radio_button_create( 10, 150, "Option 3" );
    c0 = sgui_checkbox_create( 10,  10, "Checkbox 1" );
    c1 = sgui_checkbox_create( 10,  35, "Checkbox 2" );
    c2 = sgui_checkbox_create( 10,  60, "Checkbox 3" );

    sgui_radio_button_connect( r0, NULL,   r1 );
    sgui_radio_button_connect( r1, r0,     r2 );
    sgui_radio_button_connect( r2, r1,   NULL );
    sgui_button_set_state( r0, 1 );

    sgui_frame_add_widget( f, c0 );
    sgui_frame_add_widget( f, c1 );
    sgui_frame_add_widget( f, c2 );
    sgui_frame_add_widget( f, r0 );
    sgui_frame_add_widget( f, r1 );
    sgui_frame_add_widget( f, r2 );

    gb = sgui_group_box_create( 200, 50, 130, 110, "Group Box" );
    ra = sgui_radio_button_create( 10, 25, "Option 1" );
    rb = sgui_radio_button_create( 10, 50, "Option 2" );
    rc = sgui_radio_button_create( 10, 75, "Option 3" );

    sgui_radio_button_connect( ra, NULL,   rb );
    sgui_radio_button_connect( rb, ra,     rc );
    sgui_radio_button_connect( rc, rb,   NULL );
    sgui_button_set_state( ra, 1 );

    sgui_group_box_add_widget( gb, ra );
    sgui_group_box_add_widget( gb, rb );
    sgui_group_box_add_widget( gb, rc );

    sgui_tab_group_add_widget( tab, 0, butt );
    sgui_tab_group_add_widget( tab, 0, gb );
    sgui_tab_group_add_widget( tab, 0, f );
    sgui_tab_group_add_widget( tab, 0, eb );

    /* static widget tab */
    sgui_tab_group_add_tab( tab, "Static" );

    tex = sgui_static_text_create( 10, 200, text );

    i0 = sgui_image_create(  10, 50, 128, 128, image, 1, 0, 0 );
    i1 = sgui_image_create( 150, 50, 128, 128, image, 1, 1, 0 );

    sgui_tab_group_add_widget( tab, 1, i0 );
    sgui_tab_group_add_widget( tab, 1, i1 );
    sgui_tab_group_add_widget( tab, 1, tex );

    /* output widget tab */
    sgui_tab_group_add_tab( tab, "Output" );

    p0 = sgui_progress_bar_create( 10,  40, 0, 0, 50, 300 );
    p1 = sgui_progress_bar_create( 10,  80, 1, 0, 50, 300 );
    p2 = sgui_progress_bar_create( 320, 40, 0, 1, 50, 300 );
    p3 = sgui_progress_bar_create( 355, 40, 1, 1, 50, 300 );

    sgui_tab_group_add_widget( tab, 2, p0 );
    sgui_tab_group_add_widget( tab, 2, p1 );
    sgui_tab_group_add_widget( tab, 2, p2 );
    sgui_tab_group_add_widget( tab, 2, p3 );

    /* OpenGL widget tab */
    sgui_tab_group_add_tab( tab, "OpenGL" );

    gl_sub = sgui_window_create( a, 200, 150, 0, SGUI_OPENGL_COMPAT );

    sgui_window_move( gl_sub, 200, 200 );
    sgui_window_set_visible( gl_sub, SGUI_VISIBLE );
    sgui_window_make_current( gl_sub );

    sgui_window_on_event( gl_sub, gl_sub_cb );

    /* */
    sgui_window_add_widget( a, tab );

    sgui_main_loop( );

    sgui_window_make_current( NULL );

    sgui_window_destroy( a );
    sgui_window_destroy( b );
    sgui_window_destroy( gl_sub );

    sgui_group_box_destroy( gb );
    sgui_frame_destroy( f );
    sgui_tab_group_destroy( tab );

    sgui_static_text_destroy( tex );

    sgui_edit_box_destroy( eb );

    sgui_button_destroy( butt );
    sgui_button_destroy( c0 );
    sgui_button_destroy( c1 );
    sgui_button_destroy( c2 );
    sgui_button_destroy( r0 );
    sgui_button_destroy( r1 );
    sgui_button_destroy( r2 );
    sgui_button_destroy( ra );
    sgui_button_destroy( rb );
    sgui_button_destroy( rc );

    sgui_progress_bar_destroy( p0 );
    sgui_progress_bar_destroy( p1 );
    sgui_progress_bar_destroy( p2 );
    sgui_progress_bar_destroy( p3 );

    sgui_image_destroy( i0 );
    sgui_image_destroy( i1 );

    sgui_font_destroy( font_bold );
    sgui_font_destroy( font_ital );
    sgui_font_destroy( font_boit );
    sgui_font_destroy( font );

    sgui_deinit( );

    return 0;
}

