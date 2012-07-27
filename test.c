#include "sgui.h"

#include <stdio.h>



sgui_window *a, *b;
sgui_widget *p0, *p1, *p2, *p3, *tex, *butt;
unsigned char image[128*128*4];

const char* text =
    "Lorem <b>ipsum</b> dolor <i>sit</i> amet,\n"
    "<color=\"#FF0000\"><i>consectetuer</i> <b>adipiscing</b> elit.\n"
    "<color=\"#00FF00\"><b>Aenean <i>commodo</i> ligula <i>eget</i></b>\n"
    "<color=\"#0000FF\"><i>dolor. <b>Aenean</b> massa.</i>";



void window_callback( sgui_window* wnd, int type, sgui_event* e )
{
    const char* w = (wnd==a) ? "A" : "B";

    switch( type )
    {
    case SGUI_USER_CLOSED_EVENT:
        printf( "Window %s: got closed by the user\n", w );
        break;
    case SGUI_API_INVISIBLE_EVENT:
        printf( "Window %s: made invisible\n", w );
        break;
    case SGUI_API_DESTROY_EVENT:
        printf( "Window %s: got closed by the api\n", w );
        break;
    case SGUI_SIZE_CHANGE_EVENT:
        printf( "Window %s: size changed to %d x %d\n", w,
                                                        e->size.new_width,
                                                        e->size.new_height );
        break;
    case SGUI_MOUSE_MOVE_EVENT:
        printf( "Window %s: mouse moved to (%d,%d)\n", w, e->mouse_move.x,
                                                          e->mouse_move.y );

        if( wnd == a )
        {
            sgui_progress_bar_set_progress( p0, e->mouse_move.x/800.0f );
            sgui_progress_bar_set_progress( p1, e->mouse_move.x/800.0f );

            sgui_progress_bar_set_progress( p2, 1.0f-e->mouse_move.y/600.0f );
            sgui_progress_bar_set_progress( p3, 1.0f-e->mouse_move.y/600.0f );
        }
        break;
    case SGUI_MOUSE_PRESS_EVENT:
        printf( "Window %s: %s mouse button %s\n", w,
                e->mouse_press.button==SGUI_MOUSE_BUTTON_LEFT  ? "left" :
                e->mouse_press.button==SGUI_MOUSE_BUTTON_RIGHT ? "right":
                                                                 "middle",
                e->mouse_press.pressed ? "pressed" : "released" );
        break;
    case SGUI_MOUSE_WHEEL_EVENT:
        printf( "Window %s: mouse wheel moved %s\n", w,
                e->mouse_wheel.direction>0 ? "up" : "down" );
        break;
    case SGUI_DRAW_EVENT:
        if( wnd==a )
        {
            sgui_window_blit_image( a, 10, 100, 128, 128, image, 1 );
            sgui_window_blend_image( a, 10, 250, 128, 128, image );
        }
        break;
    };
}



int main( void )
{
    int a_active=1, b_active=1, x, y, len;
    unsigned char color[3];
    sgui_font* font;
    sgui_font* font_bold;
    sgui_font* font_italic;
    sgui_font* font_bold_italic;

    a = sgui_window_create( 400, 300, SGUI_RESIZEABLE );
    b = sgui_window_create( 100, 100, SGUI_FIXED_SIZE );

    sgui_window_set_visible( a, SGUI_VISIBLE );
    sgui_window_set_visible( b, SGUI_VISIBLE );

    sgui_window_set_title( a, "A" );
    sgui_window_set_title( b, "B" );

    sgui_window_move( a, 200, 200 );
    sgui_window_move_center( b );

    sgui_window_on_event( a, window_callback );
    sgui_window_on_event( b, window_callback );

    sgui_window_set_size( a, 800, 600 );
    sgui_window_set_size( b, 200, 100 );

    /* pixmap and font test */
    sgui_font_init( );

    for( y=0; y<128; ++y )
        for( x=0; x<128; ++x )
        {
            image[ (y*128 + x)*4     ] = 0xFF * (x/128.0f);
            image[ (y*128 + x)*4 + 1 ] = 0xFF * (y/128.0f);
            image[ (y*128 + x)*4 + 2 ] = 0x00;
            image[ (y*128 + x)*4 + 3 ] = 0xFF * (x/128.0f);
        }

    font = sgui_font_load_from_file( "font/FreeSans.ttf" );
    font_bold = sgui_font_load_from_file( "font/FreeSansBold.ttf" );
    font_italic = sgui_font_load_from_file( "font/FreeSansOblique.ttf" );
    font_bold_italic=sgui_font_load_from_file("font/FreeSansBoldOblique.ttf");

    color[0] = 0xFF;
    color[1] = 0xFF;
    color[2] = 0xFF;

    len = sgui_font_extents( (const unsigned char*)"Test AV", font, 16, 7 );

    sgui_font_print( (const unsigned char*)"Test AV", font, 16, image,
                      64 - len/2, 64 - 8, 128, 128, color, 7, 1 );

    sgui_window_blit_image( a, 10, 100, 128, 128, image, 1 );
    sgui_window_blend_image( a, 10, 250, 128, 128, image );

    /* widget test */
    p0 = sgui_progress_bar_create( 10, 10, 300, 30, 0.5f );
    p1 = sgui_progress_bar_create( 10, 45, 300, 30, 0.5f );

    p2 = sgui_progress_bar_create( 320, 10, 30, 300, 0.5f );
    p3 = sgui_progress_bar_create( 355, 10, 30, 300, 0.5f );

    sgui_progress_bar_set_style( p0, 0, 1 );
    sgui_progress_bar_set_style( p1, 1, 0 );
    sgui_progress_bar_set_style( p2, 0, 1 );
    sgui_progress_bar_set_style( p3, 1, 0 );

    sgui_progress_bar_set_color( p1, SGUI_YELLOW );
    sgui_progress_bar_set_color( p3, SGUI_YELLOW );

    sgui_progress_bar_set_direction( p2, SGUI_PROGRESS_BAR_VERTICAL );
    sgui_progress_bar_set_direction( p3, SGUI_PROGRESS_BAR_VERTICAL );



    tex = sgui_static_text_create( 10, 400, (const unsigned char*)text,
                                   font, font_bold, font_italic,
                                   font_bold_italic, 16 );


    butt = sgui_button_create( 180, 100, (const unsigned char*)"Button",
                               font, 16 );


    sgui_window_add_widget( a, tex );
    sgui_window_add_widget( a, butt );
    sgui_window_add_widget( a, p0 );
    sgui_window_add_widget( a, p1 );
    sgui_window_add_widget( a, p2 );
    sgui_window_add_widget( a, p3 );

    while( a_active || b_active )
    {
        if( a_active && !sgui_window_update( a ) )
        {
            a_active = 0;
            sgui_window_destroy( a );
        }

        if( b_active && !sgui_window_update( b ) )
        {
            b_active = 0;
            sgui_window_destroy( b );
        }
    }

    sgui_static_text_delete( tex );

    sgui_button_delete( butt );

    sgui_progress_bar_delete( p0 );
    sgui_progress_bar_delete( p1 );
    sgui_progress_bar_delete( p2 );
    sgui_progress_bar_delete( p3 );

    sgui_font_delete( font_bold );
    sgui_font_delete( font_italic );
    sgui_font_delete( font_bold_italic );
    sgui_font_delete( font );

    sgui_font_deinit( );

    return 0;
}

