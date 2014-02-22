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
#define SGUI_BUILDING_DLL
#include "sgui_image.h"
#include "sgui_canvas.h"
#include "sgui_widget.h"
#include "sgui_internal.h"
#include "sgui_pixmap.h"

#include <stdlib.h>
#include <string.h>



typedef struct
{
    sgui_widget widget;

    void* data;
    int format, blend, backend, useptr;

    sgui_pixmap* pixmap;
}
sgui_image;



static void image_draw( sgui_widget* widget )
{
    sgui_image* img = (sgui_image*)widget;

    if( img->blend )
    {
        sgui_canvas_blend(widget->canvas, widget->area.left, widget->area.top,
                          img->pixmap, NULL );
    }
    else
    {
        sgui_canvas_blit( widget->canvas, widget->area.left, widget->area.top,
                          img->pixmap, NULL );
    }
}

static void image_destroy( sgui_widget* widget )
{
    sgui_image* img = (sgui_image*)widget;

    if( !img->useptr )
        free( img->data );

    sgui_pixmap_destroy( img->pixmap );
    free( img );
}

static void image_on_state_change( sgui_widget* widget, int change )
{
    sgui_image* img = (sgui_image*)widget;
    unsigned int w, h;

    if( change & WIDGET_CANVAS_CHANGED )
    {
        sgui_internal_lock_mutex( );

        sgui_widget_get_size( widget, &w, &h );

        sgui_pixmap_destroy( img->pixmap );

        img->pixmap = sgui_canvas_create_pixmap( widget->canvas, w, h,
                                                 img->format );

        sgui_pixmap_load( img->pixmap, 0, 0, img->data, 0, 0, w, h,
                          w, img->format );

        sgui_internal_unlock_mutex( );
    }
}

/****************************************************************************/

sgui_widget* sgui_image_create( int x, int y,
                                unsigned int width, unsigned int height,
                                const void* data, int format,
                                int blend, int useptr )
{
    sgui_image* img = malloc( sizeof(sgui_image) );
    unsigned int num_bytes;

    if( !img )
        return NULL;

    sgui_internal_widget_init( (sgui_widget*)img, x, y, width, height );

    if( !useptr )
    {
        num_bytes = width*height*(format==SGUI_RGBA8 ? 4 :
                                  (format==SGUI_RGB8 ? 3 : 1));
        img->data = malloc( num_bytes );

        if( !img->data )
        {
            free( img );
            return NULL;
        }

        memcpy( img->data, data, num_bytes );
    }
    else
    {
        img->data = (unsigned char*)data;
    }

    img->widget.draw_callback         = image_draw;
    img->widget.state_change_callback = image_on_state_change;
    img->widget.destroy               = image_destroy;
    img->pixmap  = NULL;
    img->format  = format;
    img->blend   = blend;
    img->useptr  = useptr;

    return (sgui_widget*)img;
}

