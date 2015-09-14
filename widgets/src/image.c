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
    sgui_widget super;

    void* data;
    int format, blend, backend, useptr;

    sgui_pixmap* pixmap;
}
sgui_image;



static void image_draw( sgui_widget* super )
{
    sgui_image* this = (sgui_image*)super;

    if( !this->pixmap )
        return;

    sgui_canvas_draw_pixmap( super->canvas, super->area.left, super->area.top,
                             this->pixmap, NULL, this->blend );
}

static void image_destroy( sgui_widget* super )
{
    sgui_image* this = (sgui_image*)super;

    if( !this->useptr )
        free( this->data );

    if( this->pixmap )
        sgui_pixmap_destroy( this->pixmap );
    free( this );
}

static void image_on_state_change( sgui_widget* super, int change )
{
    sgui_image* this = (sgui_image*)super;
    unsigned int w, h;

    if( change & SGUI_WIDGET_CANVAS_CHANGED )
    {
        sgui_internal_lock_mutex( );

        if( this->pixmap )
        {
            sgui_pixmap_destroy( this->pixmap );
            this->pixmap = NULL;
        }

        if( super->canvas )
        {
            sgui_widget_get_size( super, &w, &h );
            this->pixmap = sgui_canvas_create_pixmap( super->canvas, w, h,
                                                      this->format );

            if( this->pixmap )
            {
                sgui_pixmap_load( this->pixmap, 0, 0, this->data, 0, 0, w, h,
                                  w, this->format );
            }
        }

        sgui_internal_unlock_mutex( );
    }
}

/****************************************************************************/

sgui_widget* sgui_image_create( int x, int y,
                                unsigned int width, unsigned int height,
                                const void* data, int format,
                                int blend, int useptr )
{
    sgui_image* this = calloc( 1, sizeof(sgui_image) );
    sgui_widget* super = (sgui_widget*)this;
    unsigned int num_bytes;

    if( !this )
        return NULL;

    sgui_widget_init( super, x, y, width, height );

    if( !useptr )
    {
        num_bytes = width*height*(format==SGUI_RGBA8 ? 4 :
                                  (format==SGUI_RGB8 ? 3 : 1));
        this->data = malloc( num_bytes );

        if( !this->data )
        {
            free( this );
            return NULL;
        }

        memcpy( this->data, data, num_bytes );
    }
    else
    {
        this->data = (unsigned char*)data;
    }

    super->draw               = image_draw;
    super->state_change_event = image_on_state_change;
    super->destroy            = image_destroy;
    super->flags              = SGUI_WIDGET_VISIBLE;
    this->pixmap  = NULL;
    this->format  = format;
    this->blend   = blend;
    this->useptr  = useptr;

    return super;
}

