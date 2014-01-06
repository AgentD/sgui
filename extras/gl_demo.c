#include "sgui.h"

#include <GL/gl.h>
#include <stdio.h>
#include <stdlib.h>



void window_callback( sgui_window* wnd, int type, sgui_event* event )
{
    (void)wnd; (void)type; (void)event;
}



const char* text =
    "Lorem <b>ipsum</b> dolor <i>sit</i> amet,\n"
    "<color=\"#FF0000\"><i>consectetuer</i> <b>adipiscing</b> elit.\n"
    "<color=\"#00FF00\"><b>Aenean <i>commodo</i> ligula <i>eget</i></b>\n"
    "<color=\"#0000FF\"><i>dolor. <b>Aenean</b> massa.</i>";



#define WIDTH 800
#define HEIGHT 600



typedef struct
{
    unsigned char* buffer;
    sgui_canvas* cv;
    sgui_pixmap* skinpixmap;
    GLuint texture;

    unsigned int width, height;
    int x, y;
}
window;

void window_init( window* wnd, unsigned int width, unsigned int height )
{
    unsigned char color[4];

    wnd->buffer = malloc( width*height*4 );
    wnd->cv = sgui_memory_canvas_create( wnd->buffer, width, height,
                                         SGUI_RGBA8, 0 );
    wnd->skinpixmap = sgui_canvas_create_pixmap( wnd->cv, 512, 512, SGUI_RGBA8 );

    sgui_skin_to_pixmap( wnd->skinpixmap );
    sgui_memory_canvas_set_skinpixmap( wnd->cv, wnd->skinpixmap );

    sgui_skin_get_window_background_color( color );
    sgui_canvas_set_background_color( wnd->cv, color );
    sgui_canvas_begin( wnd->cv, NULL );
    sgui_canvas_clear( wnd->cv, NULL );
    sgui_canvas_end( wnd->cv );

    glGenTextures( 1, &wnd->texture );
    glEnable( GL_TEXTURE_2D );
    glBindTexture( GL_TEXTURE_2D, wnd->texture );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA8, width, height,
                  0, GL_RGBA, GL_UNSIGNED_BYTE, wnd->buffer );

    wnd->width = width;
    wnd->height = height;
    wnd->x = 0;
    wnd->y = 0;
}

void window_update( window* wnd )
{
    unsigned int i, count;
    sgui_rect r, acc;

    count = sgui_canvas_num_dirty_rects( wnd->cv );

    if( !count )
        return;

    sgui_canvas_get_dirty_rect( wnd->cv, &acc, 0 );

    for( i=1; i<count; ++i )
    {
        sgui_canvas_get_dirty_rect( wnd->cv, &r, i );
        sgui_rect_join( &acc, &r, 0 );
    }

    sgui_canvas_redraw_widgets( wnd->cv, 1 );

    glPixelStorei( GL_UNPACK_ROW_LENGTH, wnd->width );

    glTexSubImage2D( GL_TEXTURE_2D, 0, acc.left, acc.top,
                     SGUI_RECT_WIDTH(acc), SGUI_RECT_HEIGHT(acc),
                     GL_RGBA, GL_UNSIGNED_BYTE,
                     wnd->buffer + acc.top*wnd->width*4 );

    glPixelStorei( GL_UNPACK_ROW_LENGTH, 0 );
}

void window_draw( window* wnd )
{
    glBegin( GL_QUADS );
    glColor4f( 1.0f, 1.0f, 1.0f, 0.8f );
    glTexCoord2f( 0.0f, 1.0f );
    glVertex2i( wnd->x, wnd->y );

    glColor4f( 1.0f, 1.0f, 1.0f, 0.8f );
    glTexCoord2f( 1.0f, 1.0f );
    glVertex2i( wnd->x + wnd->width-1, wnd->y );

    glColor4f( 1.0f, 1.0f, 1.0f, 0.8f );
    glTexCoord2f( 1.0f, 0.0f );
    glVertex2f( wnd->x + wnd->width-1, wnd->y + wnd->height-1 );

    glColor4f( 1.0f, 1.0f, 1.0f, 0.8f );
    glTexCoord2f( 0.0f, 0.0f );
    glVertex2f( wnd->x, wnd->y + wnd->height-1 );
    glEnd( );
}

void window_deinit( window* wnd )
{
    sgui_pixmap_destroy( wnd->skinpixmap );
    sgui_canvas_destroy( wnd->cv );
    free( wnd->buffer );
}



int main( void )
{
    sgui_widget *p0, *p1, *p2, *p3, *tex, *butt, *c0, *c1, *c2, *i0, *i1;
    sgui_widget *r0, *r1, *r2, *eb, *f, *gb, *ra, *rb, *rc, *tab;
    unsigned char image[128*128*4];
    int fh, x, y;
    sgui_font* font;
    sgui_font* font_bold;
    sgui_font* font_ital;
    sgui_font* font_boit;
    window w;

    sgui_window* wnd;
    sgui_window_description desc;

    sgui_init( );

    /* create a window */
    desc.parent         = NULL;
    desc.width          = WIDTH;
    desc.height         = HEIGHT;
    desc.resizeable     = SGUI_FIXED_SIZE;
    desc.backend        = SGUI_OPENGL_COMPAT;
    desc.doublebuffer   = SGUI_DOUBLEBUFFERED;
    desc.bits_per_pixel = 32;
    desc.depth_bits     = 16;
    desc.stencil_bits   = 0;
    desc.samples        = 0;

    wnd = sgui_window_create_desc( &desc );

    sgui_window_set_title( wnd, "OpenGL Demo" );
    sgui_window_move_center( wnd );
    sgui_window_set_visible( wnd, SGUI_VISIBLE );

    /* */
    for( y=0; y<128; ++y )
        for( x=0; x<128; ++x )
        {
            image[ (y*128 + x)*4     ] = 0xFF * (x/128.0f);
            image[ (y*128 + x)*4 + 1 ] = 0xFF * (y/128.0f);
            image[ (y*128 + x)*4 + 2 ] = 0x00;
            image[ (y*128 + x)*4 + 3 ] = 0xFF * (x/128.0f);
        }

    fh = sgui_skin_get_default_font_height( );

    font     =sgui_font_load( "../font/SourceSansPro-Regular.ttf", fh );
    font_bold=sgui_font_load( "../font/SourceSansPro-Semibold.ttf", fh );
    font_ital=sgui_font_load( "../font/SourceSansPro-It.ttf", fh );
    font_boit=sgui_font_load( "../font/SourceSansPro-SemiboldIt.ttf", fh );

    sgui_skin_set_default_font( font, font_bold, font_ital, font_boit );

    tab = sgui_tab_group_create( 10, 10, 500, 400 );

    /* input widget tab */
    sgui_tab_group_add_tab( tab, "Input" );

    butt = sgui_button_create( 10, 245, 80, 30, "Button" );
    eb = sgui_edit_box_create( 10, 195, 100, 100 );

    sgui_edit_box_set_text(eb,"An edit box test string for an edit box test");

    f = sgui_frame_create( 10, 25, 150, 150 );
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

    sgui_widget_add_child( f, c0 );
    sgui_widget_add_child( f, c1 );
    sgui_widget_add_child( f, c2 );
    sgui_widget_add_child( f, r0 );
    sgui_widget_add_child( f, r1 );
    sgui_widget_add_child( f, r2 );

    gb = sgui_group_box_create( 200, 25, 130, 110, "Group Box" );
    ra = sgui_radio_button_create( 10, 25, "Option 1" );
    rb = sgui_radio_button_create( 10, 50, "Option 2" );
    rc = sgui_radio_button_create( 10, 75, "Option 3" );

    sgui_radio_button_connect( ra, NULL,   rb );
    sgui_radio_button_connect( rb, ra,     rc );
    sgui_radio_button_connect( rc, rb,   NULL );
    sgui_button_set_state( ra, 1 );

    sgui_widget_add_child( gb, ra );
    sgui_widget_add_child( gb, rb );
    sgui_widget_add_child( gb, rc );

    sgui_tab_group_add_widget( tab, 0, butt );
    sgui_tab_group_add_widget( tab, 0, gb );
    sgui_tab_group_add_widget( tab, 0, f );
    sgui_tab_group_add_widget( tab, 0, eb );

    /* static widget tab */
    sgui_tab_group_add_tab( tab, "Static" );

    tex = sgui_static_text_create( 10, 175, text );

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

    /* hook event callbacks */
    sgui_window_on_event( wnd, window_callback );

    /* main loop with continuos redrawing */
    sgui_window_make_current( wnd );
    sgui_window_set_vsync( wnd, 1 );

    /* */
    window_init( &w, 520, 420 );
    sgui_widget_add_child( sgui_canvas_get_root( w.cv ), tab );
    window_update( &w );

    glViewport( 0, 0, WIDTH, HEIGHT );
    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity( );
    glScalef( (2.0f/((float)WIDTH)), (2.0f/((float)HEIGHT)), 1.0f );
    glTranslatef( -WIDTH/2, -HEIGHT/2, 0.0f );

    while( sgui_main_loop_step( ) )
    {
        glClear( GL_COLOR_BUFFER_BIT );

        window_draw( &w );

        sgui_window_swap_buffers( wnd );
    }

    sgui_window_make_current( NULL );

    /* clean up */
    window_deinit( &w );

    sgui_window_make_current( NULL );

    sgui_widget_destroy( gb );
    sgui_widget_destroy( f );
    sgui_widget_destroy( tab );

    sgui_widget_destroy( eb );

    sgui_widget_destroy( butt );
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

    sgui_font_destroy( font_bold );
    sgui_font_destroy( font_ital );
    sgui_font_destroy( font_boit );
    sgui_font_destroy( font );

    sgui_window_destroy( wnd );
    sgui_deinit( );

    return 0;
}

