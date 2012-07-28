#include "sgui_skin.h"

#include "sgui_progress_bar.h"
#include "sgui_window.h"
#include "sgui_font_manager.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>



#define SGUI_YELLOW              0xFFFF00
#define SGUI_GRAY                0x646464
#define SGUI_GRAY_HALF           0x323232
#define SGUI_GRAY_QUARTER        0x161616
#define SGUI_WHITE               0xFFFFFF
#define SGUI_BLACK               0x000000

#define SGUI_DEFAULT_TEXT        SGUI_WHITE

#define SGUI_WINDOW_COLOR        SGUI_GRAY
#define SGUI_OUTSET_COLOR        SGUI_WHITE
#define SGUI_INSET_COLOR         SGUI_BLACK

#define SGUI_INSET_FILL_COLOR_L1 SGUI_GRAY_HALF
#define SGUI_INSET_FILL_COLOR_L2 SGUI_GRAY_QUARTER



static sgui_font* font_norm;
static sgui_font* font_bold;
static sgui_font* font_ital;
static sgui_font* font_boit;
static unsigned int font_height;



static unsigned char* scratch_buffer;
static unsigned int   scratch_pixels;



void assure_scratch_buffer_size( unsigned int width, unsigned int height )
{
    if( (width*height) > scratch_pixels )
    {
        scratch_pixels = width * height;
        scratch_buffer = realloc( scratch_buffer, scratch_pixels*4 );
    }
}



void sgui_skin_init( void )
{
    font_norm = NULL;
    font_bold = NULL;
    font_ital = NULL;
    font_boit = NULL;
    font_height = 0;

    scratch_buffer = malloc( 100*100*4 );
    scratch_pixels = 100*100;
}

void sgui_skin_deinit( void )
{
    free( scratch_buffer );
}



void sgui_skin_set_default_font( sgui_font* normal, sgui_font* bold,
                                 sgui_font* italic, sgui_font* bold_italic,
                                 unsigned int height )
{
    font_norm = normal;
    font_bold = bold;
    font_ital = italic;
    font_boit = bold_italic;
    font_height = height;
}

void sgui_skin_get_window_background_color( unsigned char* color )
{
    color[0] = (SGUI_WINDOW_COLOR>>16) & 0xFF;
    color[1] = (SGUI_WINDOW_COLOR>>8 ) & 0xFF;
    color[2] =  SGUI_WINDOW_COLOR      & 0xFF;
}

void sgui_skin_get_button_extents( const unsigned char* text,
                                   unsigned int* width,
                                   unsigned int* height,
                                   unsigned int* text_w )
{
    unsigned int len;

    len = strlen( (const char*)text );

    *text_w = sgui_font_extents( text, font_norm, font_height, len );
    *width  = *text_w + 2*font_height;
    *height = 2*font_height - font_height/4;
}

void sgui_skin_get_text_extents( const unsigned char* text,
                                 unsigned int* width, unsigned int* height )
{
    unsigned int lines = 1, linestart = 0, i = 0, line_width = 0;
    sgui_font* f = font_norm;
    sgui_font* font_stack[10];
    int font_stack_index = 0;

    *width = 0;
    *height = 0;

    /* determine size of the text buffer */
    for( ; text[ i ]; ++i )
    {
        if( text[ i ] == '<' )
        {
            if( !strncmp( (const char*)text+i+1, "color=", 6 ) )
            {
                i += 14;
            }
            else if( text[i+1] == 'b' )
            {
                font_stack[ font_stack_index++ ] = f;
                f = f==font_ital ? font_boit : font_bold;
            }
            else if( text[i+1] == 'i' )
            {
                font_stack[ font_stack_index++ ] = f;
                f = f==font_bold ? font_boit : font_ital;
            }
            else if( text[i+1] == '/' )
            {
                if( font_stack_index )
                    f = font_stack[ --font_stack_index ];

                ++i;
            }

            i += 2;
        }
        else if( text[ i ] == '\n' )
        {
            line_width = sgui_font_extents( text+linestart, f, font_height,
                                            i-linestart );

            if( line_width > *width )
                *width = line_width;

            ++lines;
            linestart = i+1;
        }
    }

    line_width = sgui_font_extents( text+linestart, f, font_height,
                                    i-linestart );

    if( line_width > *width )
        *width = line_width;

    *height = lines * font_height;
}



void sgui_skin_draw_progress_bar( sgui_window* wnd, int x, int y,
                                  unsigned int width, unsigned int height,
                                  int horizontal, int style, float value )
{
    int ox, oy;
    unsigned long color;
    unsigned int segments, i, wh;

    /* draw background box */
    sgui_window_draw_box( wnd, x, y, width, height,
                          SGUI_INSET_FILL_COLOR_L1, 1 );

    /* draw bar */
    if( style == SGUI_PROGRESS_BAR_STIPPLED )
    {
        ox = oy = 5;
        color = SGUI_WHITE;
    }
    else
    {
        ox = oy = 1;
        color = SGUI_YELLOW;
    }

    if( horizontal )
    {
        width  = (width - 2*ox) * value;
        height = height - 2*oy;

        if( width )
        {
            if( style == SGUI_PROGRESS_BAR_CONTINUOUS )
            {
                sgui_window_draw_box( wnd, x+ox, y+oy,
                                      width, height, color, 0 );
            }
            else
            {
                segments = width / 12;

                for( i=0; i<segments; ++i )
                {
                    sgui_window_draw_box( wnd, x+ox+(int)i*12, y+oy,
                                          7, height, color, 0 );
                }
            }
        }
    }
    else
    {
        wh = height;
        height = (height - 2*oy) * value;
        width  =  width  - 2*ox;

        if( height )
        {
            if( style == SGUI_PROGRESS_BAR_CONTINUOUS )
            {
                sgui_window_draw_box( wnd, x+ox, y+wh-oy-height,
                                      width, height-1, color, 0 );
            }
            else
            {
                segments = height / 12;

                for( i=0; i<segments; ++i )
                {
                    sgui_window_draw_box( wnd, x+ox, y+wh-oy - (int)i*12 - 7,
                                          width, 7, color, 0 );
                }
            }
        }
    }
}

void sgui_skin_draw_button( sgui_window* wnd, int x, int y, int state,
                            unsigned int width, unsigned int text_w,
                            unsigned int height, const unsigned char* text )
{
    unsigned char color[3] = { (SGUI_DEFAULT_TEXT>>16) & 0xFF,
                               (SGUI_DEFAULT_TEXT>>8 ) & 0xFF,
                               SGUI_DEFAULT_TEXT & 0xFF };

    unsigned int len = strlen( (const char*)text );

    sgui_window_draw_box( wnd, x, y, width, height, SGUI_WINDOW_COLOR,
                          state==0 ? -1 : 1 );

    assure_scratch_buffer_size( width, height );
    memset( scratch_buffer, 0, width*height*4 );

    sgui_font_print_alpha( text, font_norm, font_height, scratch_buffer,
                           width/2 - text_w/2,
                           height/2 - font_height/2 - font_height/8,
                           width, height, color, len );

    sgui_window_blend_image( wnd, x - state, y - state,
                             width, height, scratch_buffer );
}

void sgui_skin_draw_text( sgui_window* wnd, int x, int y,
                          unsigned int width, unsigned int height,
                          const unsigned char* text )
{
    unsigned int line = 0, linestart = 0, i = 0;
    unsigned int X = 0;
    sgui_font* f = font_norm;
    sgui_font* font_stack[10];
    int font_stack_index = 0;
    long c;
    unsigned char color[3] = { (SGUI_DEFAULT_TEXT>>16) & 0xFF,
                               (SGUI_DEFAULT_TEXT>>8 ) & 0xFF,
                               SGUI_DEFAULT_TEXT & 0xFF };

    /* render the text */
    assure_scratch_buffer_size( width, height );

    memset( scratch_buffer, 0, width*height*4 );

    for( ; text[ i ]; ++i )
    {
        if( text[ i ] == '<' )
        {
            sgui_font_print_alpha( text+linestart, f, font_height,
                                   scratch_buffer, X, line*font_height,
                                   width, height, color, i-linestart );

            X += sgui_font_extents( text+linestart, f, font_height,
                                    i-linestart );

            if( !strncmp( (const char*)text+i+1, "color=", 6 ) )
            {
                if( !strncmp( (const char*)text+i+9, "default", 7 ) )
                {
                    color[0] = (SGUI_DEFAULT_TEXT>>16) & 0xFF;
                    color[1] = (SGUI_DEFAULT_TEXT>>8 ) & 0xFF;
                    color[2] =  SGUI_DEFAULT_TEXT      & 0xFF;
                }
                else
                {
                    c = strtol( (const char*)text+i+9, NULL, 16 );

                    color[0] = (c>>16) & 0xFF;
                    color[1] = (c>>8 ) & 0xFF;
                    color[2] =  c      & 0xFF;
                }

                i += 14;
            }
            else if( text[ i+1 ] == 'b' )
            {
                font_stack[ font_stack_index++ ] = f;
                f = f==font_ital ? font_boit : font_bold;
            }
            else if( text[ i+1 ] == 'i' )
            {
                font_stack[ font_stack_index++ ] = f;
                f = f==font_bold ? font_boit : font_ital;
            }
            else if( text[ i+1 ] == '/' )
            {
                ++i;

                if( font_stack_index )
                    f = font_stack[ --font_stack_index ];
            }

            i += 2;
            linestart = i+1;
        }
        else if( text[ i ] == '\n' )
        {
            sgui_font_print_alpha( text+linestart, f, font_height,
                                   scratch_buffer, X, line*font_height,
                                   width, height, color, i-linestart );

            ++line;
            X = 0;
            linestart = i+1;
        }
    }

    sgui_font_print_alpha( text+linestart, f, font_height, scratch_buffer,
                           X, line*font_height, width, height,
                           color, i-linestart );

    sgui_window_blend_image( wnd, x, y, width, height, scratch_buffer );
}

