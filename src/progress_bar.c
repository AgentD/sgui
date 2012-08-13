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

#include "widget_internal.h"

#include <stdlib.h>
#include <string.h>



typedef struct
{
    sgui_widget widget;

    float progress;
    int style;
    int vertical;
    unsigned int length;
}
sgui_progress_bar;



void sgui_progress_bar_on_event( sgui_widget* widget, sgui_window* wnd,
                                 int type, sgui_event* event )
{
    sgui_progress_bar* b = (sgui_progress_bar*)widget;
    (void)event;

    if( type == SGUI_DRAW_EVENT )
    {
        sgui_skin_draw_progress_bar( wnd, widget->x, widget->y, b->length,
                                     b->vertical, b->style, b->progress );
    }
}



sgui_widget* sgui_progress_bar_create( int x, int y, int style, int vertical,
                                       float progress, unsigned int length )
{
    sgui_progress_bar* b;
    unsigned int w, h;

    progress = (progress>1.0f) ? 1.0f : ((progress<0.0f) ? 0.0f : progress);

    b = malloc( sizeof(sgui_progress_bar) );

    sgui_skin_get_progress_bar_extents( length, style, vertical, &w, &h );

    sgui_internal_widget_init( (sgui_widget*)b, x, y, w, h, 0 );

    b->widget.window_event_callback = sgui_progress_bar_on_event;
    b->vertical                     = vertical;
    b->progress                     = progress;
    b->style                        = style;
    b->length                       = length;

    return (sgui_widget*)b;
}

void sgui_progress_bar_set_progress( sgui_widget* bar, float progress )
{
    sgui_progress_bar* b = (sgui_progress_bar*)bar;

    progress = (progress>1.0f) ? 1.0f : ((progress<0.0f) ? 0.0f : progress);

    if( b )
    {
        b->widget.need_redraw = 1;
        b->progress           = progress;
    }
}

float sgui_progress_bar_get_progress( sgui_widget* bar )
{
    sgui_progress_bar* b = (sgui_progress_bar*)bar;

    return b ? b->progress : 0.0f;
}

void sgui_progress_bar_destroy( sgui_widget* bar )
{
    if( bar )
    {
        sgui_internal_widget_deinit( bar );
        free( bar );
    }
}

