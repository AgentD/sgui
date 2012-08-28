/*
 * image.c
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
#include "sgui_image.h"
#include "sgui_canvas.h"

#include "widget_internal.h"

#include <stdlib.h>
#include <string.h>



typedef struct
{
    sgui_widget widget;

    void* data;
    int alpha, blend, is_mine;
}
sgui_image;



void sgui_image_draw( sgui_widget* widget, sgui_canvas* cv )
{
    sgui_image* img = (sgui_image*)widget;

    if( img->blend )
    {
        sgui_canvas_blend( cv, widget->x, widget->y,
                           widget->width, widget->height, SCF_RGBA8,
                           img->data );
    }
    else
    {
        sgui_canvas_blit( cv, widget->x, widget->y,
                          widget->width, widget->height,
                          img->alpha ? SCF_RGBA8 : SCF_RGB8, img->data );
    }
}

sgui_widget* sgui_image_create( int x, int y,
                                unsigned int width, unsigned int height,
                                const void* data, int alpha,
                                int blend, int copy )
{
    sgui_image* img;

    img = malloc( sizeof(sgui_image) );

    sgui_internal_widget_init( (sgui_widget*)img, x, y, width, height, 0 );

    if( copy )
    {
        img->data = malloc( width*height*(alpha ? 4 : 3) );

        memcpy( img->data, data, width*height*(alpha ? 4 : 3) );
    }
    else
    {
        img->data = (void*)data;
    }

    img->widget.draw_callback = sgui_image_draw;
    img->alpha = alpha;
    img->blend = blend && alpha;
    img->is_mine = copy;

    return (sgui_widget*)img;
}

void sgui_image_destroy( sgui_widget* widget )
{
    sgui_image* img = (sgui_image*)widget;

    if( img )
    {
        if( img->is_mine )
            free( img->data );

        free( img );
    }
}

