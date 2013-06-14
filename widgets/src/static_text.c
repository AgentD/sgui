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
#define SGUI_BUILDING_DLL
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



static void static_text_draw( sgui_widget* w )
{
    sgui_static_text* t = (sgui_static_text*)w;

    sgui_canvas_draw_text( w->canvas, w->area.left, w->area.top, t->text );
}

static void static_text_destroy( sgui_widget* widget )
{
    free( ((sgui_static_text*)widget)->text );
    free( widget );
}



sgui_widget* sgui_static_text_create( int x, int y, const char* text )
{
    sgui_static_text* t;
    unsigned int w, h;
    sgui_rect r;

    /* create widget */
    t = malloc( sizeof(sgui_static_text) );

    if( !t )
        return NULL;

    /* allocate space for the text */
    t->text = malloc( strlen(text)+1 );

    if( !t->text )
    {
        free( t );
        return NULL;
    }

    /* copy the text */
    strcpy( t->text, text );

    /* compute width and height of the text */
    sgui_skin_get_text_extents( text, &r );
    w = SGUI_RECT_WIDTH( r );
    h = SGUI_RECT_HEIGHT( r );

    /* store results */
    sgui_internal_widget_init( (sgui_widget*)t, x, y, w, h );

    t->widget.draw_callback = static_text_draw;
    t->widget.destroy       = static_text_destroy;

    return (sgui_widget*)t;
}

