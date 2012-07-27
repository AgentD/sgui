/*
 * static_text.c
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
#include "sgui_static_text.h"
#include "sgui_font_manager.h"
#include "sgui_colors.h"

#include "widget_internal.h"

#include <stdlib.h>
#include <string.h>



typedef struct
{
    sgui_widget widget;

    unsigned char* buffer;
}
sgui_static_text;



void sgui_static_text_on_event( sgui_widget* widget, sgui_window* wnd,
                                int type, sgui_event* event )
{
    sgui_static_text* t = (sgui_static_text*)widget;
    (void)event;

    if( type == SGUI_DRAW_EVENT )
    {
        sgui_window_blend_image( wnd, widget->x, widget->y,
                                 widget->width, widget->height,
                                 t->buffer );
    }
}



sgui_widget* sgui_static_text_create( int x, int y, const unsigned char* text,
                                      sgui_font* norm, sgui_font* bold,
                                      sgui_font* ital, sgui_font* boit,
                                      unsigned int size )
{
    unsigned int lines = 1, linestart = 0, i = 0;
    unsigned int line_width, width = 0, height;
    unsigned int X;
    sgui_static_text* t;
    sgui_font* f = norm;
    sgui_font* font_stack[10];
    int font_stack_index = 0;
    long c;
    unsigned char color[3] = { (SGUI_DEFAULT_TEXT>>16) & 0xFF,
                               (SGUI_DEFAULT_TEXT>>8 ) & 0xFF,
                               SGUI_DEFAULT_TEXT & 0xFF };

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
                f = f==ital ? boit : bold;
            }
            else if( text[i+1] == 'i' )
            {
                font_stack[ font_stack_index++ ] = f;
                f = f==bold ? boit : ital;
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
            line_width = sgui_font_extents( text+linestart, f, size,
                                            i-linestart );

            if( line_width > width )
                width = line_width;

            ++lines;
            linestart = i+1;
        }
    }

    line_width = sgui_font_extents( text+linestart, norm, size, i-linestart );

    if( line_width > width )
        width = line_width;

    height = lines * size;

    /* create widget */
    t = malloc( sizeof(sgui_static_text) );

    memset( t, 0, sizeof(sgui_static_text) );

    t->widget.x                     = x;
    t->widget.y                     = y;
    t->widget.width                 = width;
    t->widget.height                = height;
    t->widget.window_event_callback = sgui_static_text_on_event;
    t->widget.need_redraw           = 1;

    /* render the text */
    t->buffer = malloc( width*height*4 );

    memset( t->buffer, 0, width*height*4 );

    for( linestart=0, lines=0, i=0, X=0; text[ i ]; ++i )
    {
        if( text[ i ] == '<' )
        {
            sgui_font_print_alpha( text+linestart, f, size, t->buffer,
                                   X, lines*size, width, height,
                                   color, i-linestart );

            X += sgui_font_extents( text+linestart, f, size, i-linestart );

            if( !strncmp( (const char*)text+i+1, "color=", 6 ) )
            {
                c = strtol( (const char*)text+i+9, NULL, 16 );

                color[0] = (c>>16) & 0xFF;
                color[1] = (c>>8 ) & 0xFF;
                color[2] =  c      & 0xFF;

                i += 14;
            }
            else if( text[ i+1 ] == 'b' )
            {
                font_stack[ font_stack_index++ ] = f;
                f = f==ital ? boit : bold;
            }
            else if( text[ i+1 ] == 'i' )
            {
                font_stack[ font_stack_index++ ] = f;
                f = f==bold ? boit : ital;
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
            sgui_font_print_alpha( text+linestart, f, size, t->buffer,
                                   X, lines*size, width, height,
                                   color, i-linestart );

            ++lines;
            X = 0;
            linestart = i+1;
        }
    }

    sgui_font_print_alpha( text+linestart, f, size, t->buffer,
                           X, lines*size, width, height,
                           color, i-linestart );

    return (sgui_widget*)t;
}

void sgui_static_text_delete( sgui_widget* widget )
{
    free( ((sgui_static_text*)widget)->buffer );
    free( widget );
}

