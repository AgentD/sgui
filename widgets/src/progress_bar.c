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
#define SGUI_BUILDING_DLL
#include "sgui_progress_bar.h"
#include "sgui_skin.h"
#include "sgui_internal.h"
#include "sgui_widget.h"

#include <stdlib.h>
#include <string.h>



typedef struct
{
    sgui_widget widget;

    unsigned int progress, stippled;
}
sgui_progress_bar;



static void progress_draw_h( sgui_widget* widget )
{
    sgui_progress_bar* b = (sgui_progress_bar*)widget;
    sgui_canvas* cv = widget->canvas;
    sgui_pixmap* skin_pixmap = sgui_canvas_get_skin_pixmap( cv );
    sgui_rect begin, filled, empty, end, stretch;
    int w, border;

    /* get the required skin elements */
    if( b->stippled )
    {
        sgui_skin_get_element( SGUI_PBAR_H_STIPPLED_START,  &begin  );
        sgui_skin_get_element( SGUI_PBAR_H_STIPPLED_END,    &end    );
        sgui_skin_get_element( SGUI_PBAR_H_STIPPLED_FILLED, &filled );
        sgui_skin_get_element( SGUI_PBAR_H_STIPPLED_EMPTY,  &empty  );
    }
    else
    {
        sgui_skin_get_element( SGUI_PBAR_H_FILLED_START,  &begin );
        sgui_skin_get_element( SGUI_PBAR_H_FILLED_END,    &end   );
        sgui_skin_get_element( SGUI_PBAR_H_FILLED_FILLED, &filled );
        sgui_skin_get_element( SGUI_PBAR_H_FILLED_EMPTY,  &empty  );
    }

    /* draw beginning */
    sgui_canvas_blend( cv, widget->area.left, widget->area.top,
                       skin_pixmap, &begin );

    /* draw filled area */
    stretch = widget->area;
    border = SGUI_RECT_WIDTH(begin) + SGUI_RECT_WIDTH(end);
    w = SGUI_RECT_WIDTH( widget->area );
    w = ((w-border)*b->progress) / 100;
    stretch.left += SGUI_RECT_WIDTH(begin);
    stretch.right = stretch.left + w;

    if( b->stippled )
    {
        stretch.right -= w % SGUI_RECT_WIDTH(filled);

        if( w >= SGUI_RECT_WIDTH(filled) )
            sgui_canvas_stretch_blend(cv, skin_pixmap, &filled, &stretch, 1);
    }
    else
    {
        sgui_canvas_stretch_blend( cv, skin_pixmap, &filled, &stretch, 0 );
    }

    /* draw remaining empty area */
    stretch.left = stretch.right;
    stretch.right = widget->area.right - SGUI_RECT_WIDTH(end);
    sgui_canvas_stretch_blend( cv, skin_pixmap, &empty, &stretch, 0 );

    /* draw end */
    sgui_canvas_blend( cv, widget->area.right-SGUI_RECT_WIDTH(end)+1,
                       widget->area.top, skin_pixmap, &end );
}

static void progress_draw_v( sgui_widget* widget )
{
    sgui_progress_bar* b = (sgui_progress_bar*)widget;
    sgui_canvas* cv = widget->canvas;
    sgui_pixmap* skin_pixmap = sgui_canvas_get_skin_pixmap( cv );
    sgui_rect begin, filled, empty, end, stretch;
    int h, border;

    /* get the required skin elements */
    if( b->stippled )
    {
        sgui_skin_get_element( SGUI_PBAR_V_STIPPLED_START,  &begin  );
        sgui_skin_get_element( SGUI_PBAR_V_STIPPLED_END,    &end    );
        sgui_skin_get_element( SGUI_PBAR_V_STIPPLED_FILLED, &filled );
        sgui_skin_get_element( SGUI_PBAR_V_STIPPLED_EMPTY,  &empty  );
    }
    else
    {
        sgui_skin_get_element( SGUI_PBAR_V_FILLED_START,  &begin  );
        sgui_skin_get_element( SGUI_PBAR_V_FILLED_END,    &end    );
        sgui_skin_get_element( SGUI_PBAR_V_FILLED_FILLED, &filled );
        sgui_skin_get_element( SGUI_PBAR_V_FILLED_EMPTY,  &empty  );
    }

    /* draw beginning */
    sgui_canvas_blend( cv, widget->area.left, widget->area.top,
                       skin_pixmap, &begin );

    /* draw filled area */
    stretch = widget->area;
    border = SGUI_RECT_HEIGHT(begin) + SGUI_RECT_HEIGHT(end);
    h = SGUI_RECT_HEIGHT( widget->area );
    h = ((h-border)*b->progress) / 100;

    if( b->stippled )
    {
        stretch.bottom -= SGUI_RECT_HEIGHT(end)-1;
        stretch.top = stretch.bottom - h + h%SGUI_RECT_HEIGHT(filled);

        if( h >= SGUI_RECT_HEIGHT(filled) )
            sgui_canvas_stretch_blend(cv, skin_pixmap, &filled, &stretch, 1);
    }
    else
    {
        stretch.bottom -= SGUI_RECT_HEIGHT(end);
        stretch.top = stretch.bottom - h;
        sgui_canvas_stretch_blend( cv, skin_pixmap, &filled, &stretch, 0 );
    }

    /* draw remaining empty section */
    stretch.bottom = stretch.top - 1;
    stretch.top = widget->area.top + SGUI_RECT_HEIGHT(begin);
    sgui_canvas_stretch_blend( cv, skin_pixmap, &empty, &stretch, 0 );

    /* draw end */
    sgui_canvas_blend( cv, widget->area.left,
                       widget->area.bottom-SGUI_RECT_HEIGHT(end)+1,
                       skin_pixmap, &end );
}

static void progress_bar_destroy( sgui_widget* bar )
{
    free( bar );
}



sgui_widget* sgui_progress_bar_create( int x, int y, int style, int vertical,
                                       unsigned int progress,
                                       unsigned int length )
{
    unsigned int width, height;
    sgui_progress_bar* b;
    sgui_rect r;

    /* sanity check */
    if( progress > 100 )
        progress = 100;

    /* allocate widget structure */
    b = malloc( sizeof(sgui_progress_bar) );

    if( !b )
        return NULL;

    /* initialise the base widget */
    sgui_internal_widget_init( (sgui_widget*)b, 0, 0, 0, 0 );

    b->widget.draw_callback = vertical ? progress_draw_v : progress_draw_h;
    b->widget.destroy       = progress_bar_destroy;
    b->progress             = progress;
    b->stippled             = style==SGUI_PROGRESS_BAR_STIPPLED;

    /* get the size of the progress bar */
    if( b->stippled )
    {
        sgui_skin_get_element( vertical ? SGUI_PBAR_V_STIPPLED_FILLED :
                                          SGUI_PBAR_H_STIPPLED_FILLED, &r );
    }
    else
    {
        sgui_skin_get_element( vertical ? SGUI_PBAR_V_FILLED_FILLED :
                                          SGUI_PBAR_H_FILLED_FILLED, &r );
    }

    width  = vertical ? (unsigned int)SGUI_RECT_WIDTH( r ) : length;
    height = vertical ? length : (unsigned int)SGUI_RECT_HEIGHT( r );

    sgui_rect_set_size( &b->widget.area, x, y, width, height );

    return (sgui_widget*)b;
}

void sgui_progress_bar_set_progress( sgui_widget* bar, unsigned int progress )
{
    sgui_rect r;

    if( bar )
    {
        sgui_widget_get_absolute_rect( bar, &r );
        sgui_canvas_add_dirty_rect( bar->canvas, &r );

        ((sgui_progress_bar*)bar)->progress = progress > 100 ? 100 : progress;
    }
}

unsigned int sgui_progress_bar_get_progress( sgui_widget* bar )
{
    return bar ? ((sgui_progress_bar*)bar)->progress : 0;
}

