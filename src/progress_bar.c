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
#include "sgui_colors.h"

#include "widget_internal.h"

#include <stdlib.h>
#include <string.h>



typedef struct
{
    sgui_widget widget;

    float progress;
    int continuous;
    int offset;
    unsigned long color;
    int vertical;
}
sgui_progress_bar;



void sgui_progress_bar_on_event( sgui_widget* widget, sgui_window* wnd,
                                 int type, sgui_event* event )
{
    sgui_progress_bar* b = (sgui_progress_bar*)widget;
    unsigned int width, height, segments, i;
    int ox, oy;
    (void)wnd;
    (void)event;

    if( type != SGUI_DRAW_EVENT )
        return;

    /* draw background box */
    sgui_window_draw_box( wnd, widget->x, widget->y,
                               widget->width, widget->height,
                               SGUI_INSET_FILL_COLOR_L1, 1 );

    /* draw bar */
    if( b->offset )
        ox = oy = 5;
    else
        ox = oy = 1;

    if( b->vertical )
    {
        height = (widget->height - 2*oy) * b->progress;
        width  =  widget->width  - 2*ox;

        if( height )
        {
            if( b->continuous )
            {
                sgui_window_draw_box( wnd, widget->x+ox,
                                      widget->y+widget->height-oy-height,
                                      width, height-1, b->color, 0 );
            }
            else
            {
                segments = height / 12;

                for( i=0; i<segments; ++i )
                {
                    sgui_window_draw_box( wnd, widget->x+ox,
                                          widget->y+widget->height-oy -
                                          (int)i*12 - 7,
                                          width, 7, b->color, 0 );
                }
            }
        }
    }
    else
    {
        width  = (widget->width - 2*ox) * b->progress;
        height = widget->height - 2*oy;

        if( width )
        {
            if( b->continuous )
            {
                sgui_window_draw_box( wnd, widget->x+ox, widget->y+oy,
                                      width, height, b->color, 0 );
            }
            else
            {
                segments = width / 12;

                for( i=0; i<segments; ++i )
                {
                    sgui_window_draw_box( wnd, widget->x+ox+(int)i*12,
                                               widget->y+oy,
                                               7, height, b->color, 0 );
                }
            }
        }
    }
}



sgui_widget* sgui_progress_bar_create( int x, int y, unsigned int width,
                                       unsigned int height, float progress )
{
    sgui_progress_bar* b;

    progress = (progress>1.0f) ? 1.0f : ((progress<0.0f) ? 0.0f : progress);

    width  = (width< 15) ? 15 : width;
    height = (height<15) ? 15 : height;

    b = malloc( sizeof(sgui_progress_bar) );

    memset( b, 0, sizeof(sgui_progress_bar) );

    b->widget.x                     = x;
    b->widget.y                     = y;
    b->widget.width                 = width;
    b->widget.height                = height;
    b->widget.window_event_callback = sgui_progress_bar_on_event;
    b->widget.need_redraw           = 1;
    b->progress                     = progress;
    b->continuous                   = SGUI_PROGRESS_BAR_STIPPLED;
    b->offset                       = SGUI_PROGRESS_BAR_OFFSET;
    b->color                        = SGUI_WHITE;

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

void sgui_progress_bar_set_style( sgui_widget* bar, int continuous,
                                  int offset )
{
    sgui_progress_bar* b = (sgui_progress_bar*)bar;

    if( b )
    {
        b->continuous         = continuous;
        b->offset             = offset;
        b->widget.need_redraw = 1;
    }
}

void sgui_progress_bar_set_color( sgui_widget* bar, unsigned long color )
{
    sgui_progress_bar* b = (sgui_progress_bar*)bar;

    if( b )
    {
        b->color              = color;
        b->widget.need_redraw = 1;
    }
}

void sgui_progress_bar_set_direction( sgui_widget* bar, int vertical )
{
    sgui_progress_bar* b = (sgui_progress_bar*)bar;

    if( b )
    {
        b->vertical           = vertical;
        b->widget.need_redraw = 1;
    }
}

void sgui_progress_bar_destroy( sgui_widget* bar )
{
    free( bar );
}

