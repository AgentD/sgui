#include "sgui_skin.h"

#include "sgui_progress_bar.h"
#include "sgui_window.h"
#include "sgui_font_manager.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>



static sgui_font* font_norm;
static sgui_font* font_bold;
static sgui_font* font_ital;
static sgui_font* font_boit;
static unsigned int font_height;


/************************ scratch buffer management ************************/
static unsigned char* scratch_buffer;
static unsigned int   scratch_pixels;



static void assure_scratch_buffer_size( unsigned int width,
                                        unsigned int height )
{
    if( (width*height) > scratch_pixels )
    {
        scratch_pixels = width * height;
        scratch_buffer = realloc( scratch_buffer, scratch_pixels*4 );
    }
}

static void draw_box( int x, int y, unsigned int w, unsigned int h,
                      unsigned int scanpixels, unsigned char* color )
{
    unsigned char* base;
    unsigned char* row;
    unsigned int i, j;

    base = scratch_buffer + (y*scanpixels + x)*4;

    for( i=0; i<h; ++i, base+=scanpixels*4 )
    {
        for( row=base, j=0; j<w; ++j )
        {
            *(row++) = color[0];
            *(row++) = color[1];
            *(row++) = color[2];
            *(row++) = color[3];
        }
    }
}

static void draw_line( int x, int y, unsigned int length, int horizontal,
                       unsigned int scanpixels, unsigned char* color )
{
    unsigned int i = 0;
    unsigned char* ptr = scratch_buffer + (y*scanpixels + x)*4;

    if( horizontal )
    {
        for( ; i<length; ++i )
        {
            *(ptr++) = color[0];
            *(ptr++) = color[1];
            *(ptr++) = color[2];
            *(ptr++) = color[3];
        }
    }
    else
    {
        for( ; i<length; ++i, ptr+=scanpixels*4 )
        {
            ptr[0] = color[0];
            ptr[1] = color[1];
            ptr[2] = color[2];
            ptr[3] = color[3];
        }
    }
}


/***************************************************************************/

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

/***************************************************************************/

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
    color[0] = 0x64;
    color[1] = 0x64;
    color[2] = 0x64;
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

/***************************************************************************/

void sgui_skin_draw_progress_bar( sgui_window* wnd, int x, int y,
                                  unsigned int width, unsigned int height,
                                  int horizontal, int style, float value )
{
    int ox, oy;
    unsigned char color[4];
    unsigned int segments, i, ww = width, wh = height;

    assure_scratch_buffer_size( ww, wh );

    /* draw background box */
    color[0] = color[1] = color[2] = 0x32; color[3] = 0xFF;

    draw_box( 0, 0, width, height, ww, color );

    color[0] = color[1] = color[2] = 0x00;

    draw_line( 0, 0, width,  1, width, color );
    draw_line( 0, 0, height, 0, width, color );

    color[0] = color[1] = color[2] = 0xFF;

    draw_line( 0,       height-1, width,  1, width, color );
    draw_line( width-1, 0,        height, 0, width, color );

    /* draw bar */
    if( style == SGUI_PROGRESS_BAR_STIPPLED )
    {
        ox = oy = 5;
        color[0] = color[1] = color[2] = 0xFF;
    }
    else
    {
        ox = oy = 1;
        color[0] = color[1] = 0xFF; color[2] = 0x00;
    }

    if( horizontal )
    {
        width  = (width - 2*ox) * value;
        height = height - 2*oy;

        if( width )
        {
            if( style == SGUI_PROGRESS_BAR_CONTINUOUS )
            {
                draw_box( ox, oy, width, height, ww, color );
            }
            else
            {
                segments = width / 12;

                for( i=0; i<segments; ++i )
                    draw_box( ox+(int)i*12, oy, 7, height, ww, color );
            }
        }
    }
    else
    {
        height = (height - 2*oy) * value;
        width  =  width  - 2*ox;

        if( height )
        {
            if( style == SGUI_PROGRESS_BAR_CONTINUOUS )
            {
                draw_box( ox, wh-oy-height, width, height, ww, color );
            }
            else
            {
                segments = height / 12;

                for( i=0; i<segments; ++i )
                    draw_box( ox, wh-oy-(int)i*12 - 7, width, 7, ww, color );
            }
        }
    }

    sgui_window_blit_image( wnd, x, y, ww, wh, scratch_buffer, 1 );
}

void sgui_skin_draw_button( sgui_window* wnd, int x, int y, int state,
                            unsigned int width, unsigned int text_w,
                            unsigned int height, const unsigned char* text )
{
    unsigned char color[4] = { 0x64, 0x64, 0x64, 0xFF };
    unsigned int len = strlen( (const char*)text );

    assure_scratch_buffer_size( width, height );

    draw_box( 0, 0, width, height, width, color );

    color[0] = color[1] = color[2] = state ? 0x00 : 0xFF;

    draw_line( 0, 0, width,  1, width, color );
    draw_line( 0, 0, height, 0, width, color );

    color[0] = color[1] = color[2] = state ? 0xFF : 0x00;

    draw_line( 0,       height-1, width,  1, width, color );
    draw_line( width-1, 0,        height, 0, width, color );

    color[0] = color[1] = color[2] = color[3] = 0xFF;

    sgui_font_print( text, font_norm, font_height, scratch_buffer,
                     width/2 - text_w/2 - state,
                     height/2 - font_height/2 - font_height/8 - state,
                     width, height, color, len, 1 );

    sgui_window_blit_image( wnd, x, y, width, height, scratch_buffer, 1 );
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
    unsigned char color[3] = { 0xFF, 0xFF, 0xFF };

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
                    color[0] = 0xFF;
                    color[1] = 0xFF;
                    color[2] = 0xFF;
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

