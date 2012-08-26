/*
 * skin.c
 * This file is part of sgui
 *
 * Copyright (C) 2012 - David Oberhollenzer
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */
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

unsigned int sgui_skin_default_font_extents( const unsigned char* text,
                                             unsigned int length,
                                             int bold, int italic )
{
    if( bold && italic )
        return sgui_font_extents( text, font_boit, font_height, length );

    if( bold )
        return sgui_font_extents( text, font_bold, font_height, length );

    if( italic )
        return sgui_font_extents( text, font_ital, font_height, length );

    return sgui_font_extents( text, font_norm, font_height, length );
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

void sgui_skin_get_checkbox_extents( const unsigned char* text,
                                     unsigned int* width,
                                     unsigned int* height,
                                     unsigned int* text_w )
{
    unsigned int len = strlen( (const char*)text );

    *text_w = sgui_font_extents( text, font_norm, font_height, len );
    *width  = *text_w + 20;
    *height = font_height > 12 ? font_height : 12;
}

void sgui_skin_get_radio_menu_extents( const unsigned char** text,
                                       unsigned int num_lines,
                                       unsigned int* width,
                                       unsigned int* height )
{
    unsigned int i, w;

    *width  = 0;
    *height = (font_height + 10) * num_lines;

    for( i=0; i<num_lines; ++i )
    {
        w = sgui_font_extents( text[i], font_norm, font_height,
                               strlen((const char*)text[i]) );

        if( w > *width )
            *width = w;
    }

    *width += 20;
}

void sgui_skin_get_progress_bar_extents( unsigned int length, int style,
                                         int vertical, unsigned int* width,
                                         unsigned int* height )
{
    (void)style;

    if( vertical )
    {
        *width  = 30;
        *height = length;
    }
    else
    {
        *width  = length;
        *height = 30;
    }
}

unsigned int sgui_skin_get_edit_box_height( void )
{
    return (font_height + (font_height / 2) + 4);
}

/***************************************************************************/

unsigned int sgui_skin_get_radio_menu_option_from_point( int y )
{
    return (y / (font_height + 10));
}

/***************************************************************************/

void sgui_skin_draw_progress_bar( sgui_window* wnd, int x, int y,
                                  unsigned int length, int vertical,
                                  int style, float value )
{
    int ox, oy;
    unsigned char color[4];
    unsigned int segments, i, ww = vertical ? 30 : length;
    unsigned int wh = vertical ? length : 30;
    unsigned int width, height;

    assure_scratch_buffer_size( ww, wh );

    /* draw background box */
    color[0] = color[1] = color[2] = 0x00; color[3] = 0x80;

    draw_box( 0, 0, ww, wh, ww, color );

    color[0] = color[1] = color[2] = 0x00; color[3] = 0xFF;

    draw_line( 0, 0, ww, 1, ww, color );
    draw_line( 0, 0, wh, 0, ww, color );

    color[0] = color[1] = color[2] = 0xFF;

    draw_line( 0,       wh-1, ww, 1, ww, color );
    draw_line( ww-1, 0,       wh, 0, ww, color );

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

    if( vertical )
    {
        height = (wh - 2*oy) * value;
        width  =  ww - 2*ox;

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
    else
    {
        width  = (ww - 2*ox) * value;
        height =  wh - 2*oy;

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

    sgui_window_blend_image( wnd, x, y, ww, wh, scratch_buffer );
}

void sgui_skin_draw_button( sgui_window* wnd, int x, int y, int state,
                            unsigned int width, unsigned int text_w,
                            unsigned int height, const unsigned char* text )
{
    unsigned char color[4] = { 0x00, 0x00, 0x00, 0xFF };
    unsigned int len = strlen( (const char*)text );
    unsigned int text_h = font_height+10;

    assure_scratch_buffer_size( width, 1 );

    color[0] = color[1] = color[2] = state ? 0x00 : 0xFF;
    draw_line( 0, 0, width, 1, width, color );
    sgui_window_blend_image( wnd, x, y, width, 1, scratch_buffer );

    color[0] = color[1] = color[2] = state ? 0xFF : 0x00;
    draw_line( 0, 0, width,  1, width, color );
    sgui_window_blend_image( wnd, x, y+height-1, width, 1, scratch_buffer );

    assure_scratch_buffer_size( 1, height );

    color[0] = color[1] = color[2] = state ? 0x00 : 0xFF;
    draw_line( 0, 0, height, 0, 1, color );
    sgui_window_blend_image( wnd, x, y, 1, height, scratch_buffer );

    color[0] = color[1] = color[2] = state ? 0xFF : 0x00;
    draw_line( 0, 0, height, 0, 1, color );
    sgui_window_blend_image( wnd, x+width-1, y, 1, height, scratch_buffer );

    assure_scratch_buffer_size( text_w, text_h );
    memset( scratch_buffer, 0, text_w*text_h*4 );

    color[0] = color[1] = color[2] = color[3] = 0xFF;

    sgui_font_print_alpha( text, font_norm, font_height, scratch_buffer,
                           0, 0, text_w, text_h, color, len );

    x += width /2 - text_w/2 - state;
    y += height/2 - font_height/2 - font_height/8 - state;

    sgui_window_blend_image( wnd, x, y, text_w, text_h, scratch_buffer );
}

void sgui_skin_draw_text( sgui_window* wnd, int x, int y,
                          unsigned int width, const unsigned char* text )
{
    unsigned int line = 0, linestart = 0, i = 0;
    unsigned int X = 0, h = font_height + 10;
    sgui_font* f = font_norm;
    sgui_font* font_stack[10];
    int font_stack_index = 0;
    long c;
    unsigned char color[3] = { 0xFF, 0xFF, 0xFF };

    assure_scratch_buffer_size( width, h );
    memset( scratch_buffer, 0, width*h*4 );

    for( ; text[ i ]; ++i )
    {
        if( text[ i ] == '<' )
        {
            sgui_font_print_alpha( text+linestart, f, font_height,
                                   scratch_buffer, X, 0, width, h,
                                   color, i-linestart );

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
                                   scratch_buffer, X, 0, width, h,
                                   color, i-linestart );

            sgui_window_blend_image( wnd, x, y+font_height*line, width, h,
                                     scratch_buffer );

            memset( scratch_buffer, 0, width*h*4 );

            ++line;
            X = 0;
            linestart = i+1;
        }
    }

    sgui_font_print_alpha( text+linestart, f, font_height, scratch_buffer,
                           X, 0, width, h, color, i-linestart );

    sgui_window_blend_image( wnd, x, y+font_height*line, width, h,
                             scratch_buffer );
}

void sgui_skin_draw_checkbox( sgui_window* wnd, int x, int y,
                              unsigned int text_w, const unsigned char* text,
                              int state )
{
    unsigned char color[4] = { 0x00, 0x00, 0x00, 0x80 };
    unsigned int len = strlen( (const char*)text );
    unsigned int w = text_w + 20;
    unsigned int h = font_height < 12 ? 18 : (font_height + font_height/2);
    int oy = font_height > 12 ? font_height/2-6 : 0;

    assure_scratch_buffer_size( w, h );
    memset( scratch_buffer, 0, w*h*4 );

    draw_box( 0, oy, 12, 12, w, color );

    color[0] = color[1] = color[2] = 0x00; color[3] = 0xFF;

    draw_line( 0, oy, 12, 1, w, color );
    draw_line( 0, oy, 12, 0, w, color );

    color[0] = color[1] = color[2] = 0xFF;

    draw_line( 0, 11+oy, 12, 1, w, color );
    draw_line( 11,   oy, 12, 0, w, color );

    color[0] = color[1] = color[2] = 0xFF;

    sgui_font_print_alpha( text, font_norm, font_height, scratch_buffer,
                           20, -oy, w, h, color, len );

    if( state )
    {
        draw_line( 2, 4+oy, 3, 0, w, color );
        draw_line( 3, 5+oy, 3, 0, w, color );
        draw_line( 4, 6+oy, 3, 0, w, color );
        draw_line( 5, 5+oy, 3, 0, w, color );
        draw_line( 6, 4+oy, 3, 0, w, color );
        draw_line( 7, 3+oy, 3, 0, w, color );
        draw_line( 8, 2+oy, 3, 0, w, color );
    }

    sgui_window_blend_image( wnd, x, y, w, h, scratch_buffer );
}

void sgui_skin_draw_radio_menu( sgui_window* wnd, int x, int y,
                                const unsigned char** text,
                                unsigned int num_lines, unsigned int selected,
                                unsigned int width, unsigned int height )
{
    unsigned char color[4] = { 0xFF, 0xFF, 0xFF, 0xFF };
    unsigned int i, dy = font_height + 10;
    int oy = dy / 2 - 10;

    assure_scratch_buffer_size( width, dy );

    for( i=0; i<num_lines; ++i, y+=dy )
    {
        memset( scratch_buffer, 0, width*dy*4 );

        color[0] = color[1] = color[2] = 0x00; color[3] = 0x80;

        draw_box( 2, 2+oy,  8,  8, width, color );
        draw_box( 4, 1+oy,  4, 10, width, color );
        draw_box( 1, 4+oy, 10,  4, width, color );

        color[3] = 0xFF;

        draw_line( 2, 1+oy, 2, 1, width, color );
        draw_line( 4,   oy, 4, 1, width, color );
        draw_line( 8, 1+oy, 2, 1, width, color );

        draw_line( 1, 2+oy, 2, 0, width, color );
        draw_line( 0, 4+oy, 4, 0, width, color );
        draw_line( 1, 8+oy, 2, 0, width, color );

        color[0] = color[1] = color[2] = 0xFF;

        draw_line( 10, 2+oy, 2, 0, width, color );
        draw_line( 11, 4+oy, 4, 0, width, color );
        draw_line( 10, 8+oy, 2, 0, width, color );

        draw_line(  2, 10+oy, 2, 1, width, color );
        draw_line(  4, 11+oy, 4, 1, width, color );
        draw_line(  8, 10+oy, 2, 1, width, color );

        if( i==selected )
        {
            draw_box( 4, 3+oy, 4, 6, width, color );
            draw_box( 3, 4+oy, 6, 4, width, color );
        }

        sgui_font_print_alpha( text[ i ], font_norm, font_height,
                               scratch_buffer, 20, 0,
                               width, height, color,
                               strlen( (const char*)text[i] ) );

        sgui_window_blend_image( wnd, x, y, width, dy, scratch_buffer );
    }
}

void sgui_skin_draw_edit_box( sgui_window* wnd, int x, int y,
                              const unsigned char* text,
                              unsigned int width, int cursor )
{
    unsigned char color[4];
    unsigned int height, cx;

    height = font_height + (font_height / 2) + 4;

    assure_scratch_buffer_size( width, height );

    /* draw background box */
    color[0] = color[1] = color[2] = 0x00; color[3] = 0x80;

    draw_box( 0, 0, width, height, width, color );

    /* draw text */
    color[0] = color[1] = color[2] = color[3] = 0xFF;
    sgui_font_print_alpha( text, font_norm, font_height, scratch_buffer, 2, 2,
                           width, height, color,
                           strlen( (const char*)text ) );

    /* draw borders */
    color[0] = color[1] = color[2] = 0x00;

    draw_line( 0, 0, width,  1, width, color );
    draw_line( 0, 0, height, 0, width, color );

    color[0] = color[1] = color[2] = 0xFF;

    draw_line( 0,       height-1, width,  1, width, color );
    draw_line( width-1,        0, height, 0, width, color );

    /* draw cursor */
    if( cursor >= 0 )
    {
        cx = sgui_font_extents( text, font_norm, font_height, cursor );

        if( cx == 0 )
            cx = 3;

        if( cx < (width-2) )
        {
            color[0] = color[1] = color[2] = 0x7F;

            draw_line( cx, 5, height-10, 0, width, color );
        }
    }

    sgui_window_blend_image( wnd, x, y, width, height, scratch_buffer );
}

