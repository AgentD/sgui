/*
 * group_box.c
 * This file is part of sgio
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
#include "sgui_group_box.h"
#include "sgui_skin.h"
#include "sgui_canvas.h"
#include "sgui_internal.h"
#include "sgui_widget.h"

#include <stdlib.h>
#include <string.h>



typedef struct
{
    sgui_widget widget;
    char* caption;
}
sgui_group_box;



void group_box_draw( sgui_widget* widget )
{
    sgui_skin_draw_group_box( widget->canvas,
                              widget->area.left, widget->area.top,
                              SGUI_RECT_WIDTH(widget->area),
                              SGUI_RECT_HEIGHT(widget->area),
                              ((sgui_group_box*)widget)->caption );
}

static void group_box_destroy( sgui_widget* box )
{
    sgui_group_box* b = (sgui_group_box*)box;

    if( b )
    {
        free( b->caption );
        free( b );
    }
}



sgui_widget* sgui_group_box_create( int x, int y,
                                    unsigned int width, unsigned int height,
                                    const char* caption )
{
    sgui_group_box* b = malloc( sizeof(sgui_group_box) );

    if( !b )
        return NULL;

    /* try to store the caption string */
    b->caption = malloc( strlen( caption ) + 1 );

    if( !b->caption )
    {
        free( b );
        return NULL;
    }

    strcpy( b->caption, caption );

    /* initialize widget base struct */
    sgui_internal_widget_init( (sgui_widget*)b, x, y, width, height );

    b->widget.draw_callback = group_box_draw;
    b->widget.destroy = group_box_destroy;

    return (sgui_widget*)b;
}

