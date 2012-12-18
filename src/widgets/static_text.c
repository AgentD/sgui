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
#include "sgui_canvas.h"
#include "sgui_skin.h"
#include "sgui_font.h"
#include "sgui_internal.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>



typedef struct
{
    sgui_widget widget;

    char* text;
}
sgui_static_text;



void sgui_static_text_draw( sgui_widget* w, sgui_canvas* cv )
{
    unsigned char color[3];
    sgui_static_text* t = (sgui_static_text*)w;
    sgui_font* font_norm = sgui_skin_get_default_font( 0, 0 );
    sgui_font* font_bold = sgui_skin_get_default_font( 1, 0 );
    sgui_font* font_ital = sgui_skin_get_default_font( 0, 1 );
    sgui_font* font_boit = sgui_skin_get_default_font( 1, 1 );
    unsigned int h = sgui_skin_get_default_font_height( );

    sgui_skin_get_default_font_color( color );

    sgui_canvas_clear( cv, &w->area );

    sgui_font_draw_text( cv, w->area.left, w->area.top, font_norm, font_bold,
                         font_ital, font_boit, h, color, t->text );
}



sgui_widget* sgui_static_text_create( int x, int y, const char* text )
{
    sgui_static_text* t;
    unsigned int w, h;
    sgui_font* font_norm = sgui_skin_get_default_font( 0, 0 );
    sgui_font* font_bold = sgui_skin_get_default_font( 1, 0 );
    sgui_font* font_ital = sgui_skin_get_default_font( 0, 1 );
    sgui_font* font_boit = sgui_skin_get_default_font( 1, 1 );
    unsigned int f_h = sgui_skin_get_default_font_height( );

    /* create widget */
    t = malloc( sizeof(sgui_static_text) );

    if( !t )
        return NULL;

    t->text = malloc( strlen(text)+1 );

    if( !t->text )
    {
        free( t );
        return NULL;
    }

    strcpy( t->text, text );

    sgui_font_get_text_extents( font_norm, font_bold, font_ital, font_boit,
                                f_h, text, &w, &h );

    sgui_internal_widget_init( (sgui_widget*)t, x, y, w, h );

    t->widget.draw_callback = sgui_static_text_draw;

    return (sgui_widget*)t;
}

void sgui_static_text_destroy( sgui_widget* widget )
{
    if( widget )
    {
        free( ((sgui_static_text*)widget)->text );
        free( widget );
    }
}

