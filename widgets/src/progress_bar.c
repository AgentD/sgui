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
    sgui_widget super;

    int vertical;
    unsigned int progress, stippled;
}
sgui_progress_bar;



static void progress_draw( sgui_widget* super )
{
    sgui_progress_bar* this = (sgui_progress_bar*)super;
    unsigned int length = this->vertical ? SGUI_RECT_HEIGHT(super->area) :
                                           SGUI_RECT_WIDTH(super->area);

    if( this->stippled )
    {
        sgui_skin_draw_progress_stippled( super->canvas, super->area.left,
                                          super->area.top, length,
                                          this->vertical, this->progress );
    }
    else
    {
        sgui_skin_draw_progress_bar( super->canvas, super->area.left,
                                     super->area.top, length, this->vertical,
                                     this->progress );
    }
}

static void progress_bar_destroy( sgui_widget* this )
{
    free( this );
}



sgui_widget* sgui_progress_bar_create( int x, int y, int style, int vertical,
                                       unsigned int progress,
                                       unsigned int length )
{
    unsigned int width, height;
    sgui_progress_bar* this;
    sgui_widget* super;

    /* sanity check */
    if( progress > 100 )
        progress = 100;

    /* allocate widget structure */
    this = malloc( sizeof(sgui_progress_bar) );
    super = (sgui_widget*)this;

    if( !this )
        return NULL;

    /* initialise the base widget */
    sgui_widget_init( super, 0, 0, 0, 0 );

    super->draw_callback = progress_draw;
    super->focus_policy  = 0;
    super->destroy       = progress_bar_destroy;
    this->progress       = progress;
    this->stippled       = style==SGUI_PROGRESS_BAR_STIPPLED;
    this->vertical       = vertical;

    /* get the size of the progress bar */
    width  = vertical ? sgui_skin_get_progess_bar_width( ) : length;
    height = vertical ? length : sgui_skin_get_progess_bar_width( );

    sgui_rect_set_size( &super->area, x, y, width, height );

    return super;
}

void sgui_progress_bar_set_progress(sgui_widget* this, unsigned int progress)
{
    sgui_rect r;

    if( this )
    {
        sgui_internal_lock_mutex( );
        sgui_widget_get_absolute_rect( this, &r );
        sgui_canvas_add_dirty_rect( this->canvas, &r );

        ((sgui_progress_bar*)this)->progress = progress>100 ? 100 : progress;
        sgui_internal_unlock_mutex( );
    }
}

unsigned int sgui_progress_bar_get_progress( sgui_widget* this )
{
    return this ? ((sgui_progress_bar*)this)->progress : 0;
}

