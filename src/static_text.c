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
#include "sgui_skin.h"

#include "widget_internal.h"

#include <stdlib.h>
#include <string.h>



typedef struct
{
    sgui_widget widget;

    unsigned char* text;
}
sgui_static_text;



void sgui_static_text_draw( sgui_widget* w, sgui_canvas* cv )
{
    sgui_static_text* t = (sgui_static_text*)w;

    sgui_skin_draw_text( cv, w->x, w->y, t->text );
}



sgui_widget* sgui_static_text_create( int x, int y,
                                      const unsigned char* text )
{
    sgui_static_text* t;
    unsigned int w, h;

    /* create widget */
    t = malloc( sizeof(sgui_static_text) );

    sgui_skin_get_text_extents( text, &w, &h );
    sgui_internal_widget_init( (sgui_widget*)t, x, y, w, h, 0 );

    t->widget.draw_callback = sgui_static_text_draw;
    t->text                 = malloc( strlen((const char*)text)+1 );

    strcpy( (char*)t->text, (const char*)text );

    return (sgui_widget*)t;
}

void sgui_static_text_destroy( sgui_widget* widget )
{
    if( widget )
    {
        sgui_internal_widget_deinit( widget );
        free( ((sgui_static_text*)widget)->text );
        free( widget );
    }
}

