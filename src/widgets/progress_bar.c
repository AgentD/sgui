/*
 * progress_bar.c
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
#include "sgui_progress_bar.h"
#include "sgui_skin.h"
#include "sgui_internal.h"

#include <stdlib.h>
#include <string.h>



typedef struct
{
    sgui_widget widget;

    unsigned int progress;
    int style;
    int vertical;
    unsigned int length;
}
sgui_progress_bar;



void sgui_progress_draw( sgui_widget* widget, sgui_canvas* cv )
{
    sgui_progress_bar* b = (sgui_progress_bar*)widget;

    sgui_skin_draw_progress_bar( cv, widget->area.left, widget->area.top,
                                 b->length, b->vertical, b->style,
                                 b->progress );
}



sgui_widget* sgui_progress_bar_create( int x, int y, int style, int vertical,
                                       unsigned int progress,
                                       unsigned int length )
{
    sgui_progress_bar* b;
    unsigned int w, h;

    if( progress > 100 )
        progress = 100;

    b = malloc( sizeof(sgui_progress_bar) );

    if( !b )
        return NULL;

    sgui_skin_get_progress_bar_extents( length, style, vertical, &w, &h );

    sgui_internal_widget_init( (sgui_widget*)b, x, y, w, h );

    b->widget.draw_callback = sgui_progress_draw;
    b->vertical             = vertical;
    b->progress             = progress;
    b->style                = style;
    b->length               = length;

    return (sgui_widget*)b;
}

void sgui_progress_bar_set_progress( sgui_widget* bar, unsigned int progress )
{
    sgui_progress_bar* b = (sgui_progress_bar*)bar;

    if( b )
    {
        sgui_widget_manager_add_dirty_rect( bar->mgr, &bar->area );

        b->progress = progress > 100 ? 100 : progress;
    }
}

unsigned int sgui_progress_bar_get_progress( sgui_widget* bar )
{
    sgui_progress_bar* b = (sgui_progress_bar*)bar;

    return b ? b->progress : 0;
}

void sgui_progress_bar_destroy( sgui_widget* bar )
{
    if( bar )
        free( bar );
}

