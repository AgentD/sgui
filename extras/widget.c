#include "sgui.h"
#include "sgui_internal.h"

#include <stdlib.h>



typedef struct
{
    sgui_widget super;

    int hour;
    int minute;
    int second;
}
clock_widget;



static void draw_segment_digit( sgui_canvas* cv, int digit, int x, int y,
                                int seglength )
{
    unsigned char color[4] = { 0xFF, 0x00, 0x00, 0xFF };
    int o = seglength / 4;
    sgui_rect r;

    /* top */
    if( digit!=1 && digit!=4 )
    {
        sgui_rect_set_size( &r, x+o, y, seglength, o );
        sgui_canvas_draw_box( cv, &r, color, SGUI_RGB8 );
    }

    /* top left */
    if( digit!=1 && digit!=2 && digit!=3 && digit!=7 )
    {
        sgui_rect_set_size( &r, x, y+o, o, seglength );
        sgui_canvas_draw_box( cv, &r, color, SGUI_RGB8 );
    }

    /* top right */
    if( digit!=5 && digit!=6 )
    {
        sgui_rect_set_size( &r, x+seglength+o, y+o, o, seglength );
        sgui_canvas_draw_box( cv, &r, color, SGUI_RGB8 );
    }

    /* center */
    if( digit!=0 && digit!=1 && digit!=7 )
    {
        sgui_rect_set_size( &r, x+o, y+seglength+o, seglength, o );
        sgui_canvas_draw_box( cv, &r, color, SGUI_RGB8 );
    }

    /* bottom left */
    if( digit==0 || digit==2 || digit==6 || digit==8 )
    {
        sgui_rect_set_size( &r, x, y+seglength+2*o, o, seglength );
        sgui_canvas_draw_box( cv, &r, color, SGUI_RGB8 );
    }

    /* bottom right */
    if( digit!=2 )
    {
        sgui_rect_set_size( &r, x+seglength+o, y+seglength+2*o, o, seglength );
        sgui_canvas_draw_box( cv, &r, color, SGUI_RGB8 );
    }

    /* bottom */
    if( digit!=1 && digit!=4 && digit!=7 )
    {
        sgui_rect_set_size( &r, x+o, y+2*seglength+2*o, seglength, o );
        sgui_canvas_draw_box( cv, &r, color, SGUI_RGB8 );
    }
}

static void draw_colon( sgui_canvas* cv, int x, int y, int seglength )
{
    unsigned char color[4] = { 0xFF, 0x00, 0x00, 0xFF };
    int o = seglength / 4;
    sgui_rect r;

    sgui_rect_set_size( &r, x, y+o/2+seglength/2, o, o );
    sgui_canvas_draw_box( cv, &r, color, SGUI_RGB8 );

    sgui_rect_set_size( &r, x, y+2*o+seglength+seglength/2-o/2, o, o );
    sgui_canvas_draw_box( cv, &r, color, SGUI_RGB8 );
}

static void clock_widget_draw( sgui_widget* super )
{
    clock_widget* this = (clock_widget*)super;
    int x, y;
    
    x = super->area.left;
    y = super->area.top;

    draw_segment_digit( super->canvas, this->hour / 10, x,    y, 10 );
    draw_segment_digit( super->canvas, this->hour % 10, x+16, y, 10 );
    draw_colon( super->canvas, x+33, y, 10 );
    draw_segment_digit( super->canvas, this->minute / 10, x+37, y, 10 );
    draw_segment_digit( super->canvas, this->minute % 10, x+56, y, 10 );
    draw_colon( super->canvas, x+73, y, 10 );
    draw_segment_digit( super->canvas, this->second / 10, x+77, y, 10 );
    draw_segment_digit( super->canvas, this->second % 10, x+93, y, 10 );
}

static void clock_widget_destroy( sgui_widget* this )
{
    free( this );
}



sgui_widget* clock_widget_create( int x, int y, int hour, int minute,
                                  int second )
{
    clock_widget* this;
    sgui_widget* super;

    /* create widget */
    this = malloc( sizeof(clock_widget) );
    super = (sgui_widget*)this;

    if( !this )
        return NULL;

    /* initialize super structure */
    sgui_widget_init( super, x, y, 110, 30 );

    super->draw_callback         = clock_widget_draw;
    super->destroy               = clock_widget_destroy;
    super->focus_policy          = 0;
    this->hour                   = hour<0 ? 0 : (hour % 12);
    this->minute                 = minute<0 ? 0 : (minute % 60);
    this->second                 = second<0 ? 0 : (second % 60);

    return (sgui_widget*)this;
}

/****************************************************************************/

int main( void )
{
    sgui_window* wnd;
    sgui_widget* clk;

    sgui_init( );

    /* create a window */
    wnd = sgui_window_create( NULL, 130, 50, SGUI_FIXED_SIZE );

    sgui_window_set_title( wnd, "Custom Widget" );
    sgui_window_move_center( wnd );
    sgui_window_set_visible( wnd, SGUI_VISIBLE );

    /* */
    clk = clock_widget_create( 10, 10, 1, 2, 3 );

    sgui_window_add_widget( wnd, clk );

    /* main loop */
    sgui_main_loop( );

    /* clean up */
    sgui_window_destroy( wnd );
    sgui_widget_destroy( clk );
    sgui_deinit( );

    return 0;
}

