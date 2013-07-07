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



static void group_box_draw( sgui_widget* widget )
{
    sgui_rect lt, rt, lb, rb, l, r, t, b, dst;
    sgui_canvas* cv = widget->canvas;
    sgui_group_box* gb = (sgui_group_box*)widget;
    sgui_pixmap* skin_pixmap = sgui_canvas_get_skin_pixmap( cv );
    unsigned int fh = sgui_skin_get_default_font_height( );
    unsigned int fw = sgui_skin_default_font_extents( gb->caption, -1, 0, 0 );
    unsigned char color[4];

    int x  = widget->area.left;
    int y  = widget->area.top + (fh>>1);
    int x1 = widget->area.right;
    int y1 = widget->area.bottom;

    /* get required skin elements */
    sgui_skin_get_element( SGUI_GROUPBOX_LEFT_TOP, &lt );
    sgui_skin_get_element( SGUI_GROUPBOX_RIGHT_TOP, &rt );
    sgui_skin_get_element( SGUI_GROUPBOX_LEFT_BOTTOM, &lb );
    sgui_skin_get_element( SGUI_GROUPBOX_RIGHT_BOTTOM, &rb );
    sgui_skin_get_element( SGUI_GROUPBOX_LEFT, &l );
    sgui_skin_get_element( SGUI_GROUPBOX_RIGHT, &r );
    sgui_skin_get_element( SGUI_GROUPBOX_TOP, &t );
    sgui_skin_get_element( SGUI_GROUPBOX_BOTTOM, &b );

    /* draw corners */
    sgui_canvas_blend( cv, x, y, skin_pixmap, &lt );
    sgui_canvas_blend( cv, x1-(rt.right-rt.left), y, skin_pixmap, &rt );
    sgui_canvas_blend( cv, x, y1-(lb.bottom-lb.top), skin_pixmap, &lb );
    sgui_canvas_blend( cv, x1-(rb.right-rb.left), y1-(rb.bottom-rb.top),
                       skin_pixmap, &rb );

    /* draw borders */
    dst.left   = x;
    dst.right  = x  + (l.right - l.left);
    dst.top    = y  + SGUI_RECT_HEIGHT(lt);
    dst.bottom = y1 - SGUI_RECT_HEIGHT(lb);
    sgui_canvas_stretch_blend( cv, skin_pixmap, &l, &dst, 0 );

    dst.left   = x1 - (r.right-r.left);
    dst.right  = x1;
    dst.top    = y  + SGUI_RECT_HEIGHT(rt);
    dst.bottom = y1 - SGUI_RECT_HEIGHT(rb);
    sgui_canvas_stretch_blend( cv, skin_pixmap, &r, &dst, 0 );

    dst.left   = x  + SGUI_RECT_WIDTH(lt) + fw + (fh>>1);
    dst.right  = x1 - SGUI_RECT_WIDTH(rt);
    dst.top    = y;
    dst.bottom = y  + (t.bottom-t.top);
    sgui_canvas_stretch_blend( cv, skin_pixmap, &t, &dst, 0 );

    dst.left   = x  + SGUI_RECT_WIDTH(lb);
    dst.right  = x1 - SGUI_RECT_WIDTH(rb);
    dst.top    = y1 - (b.bottom-b.top);
    dst.bottom = y1;
    sgui_canvas_stretch_blend( cv, skin_pixmap, &b, &dst, 0 );

    /* draw caption */
    sgui_skin_get_default_font_color( color );
    sgui_canvas_draw_text_plain( cv, x + SGUI_RECT_WIDTH(lt) + (fh>>2),
                                 widget->area.top, 0, 0, color,
                                 gb->caption, -1 );
}

static void group_box_destroy( sgui_widget* box )
{
    sgui_group_box* b = (sgui_group_box*)box;

    free( b->caption );
    free( b );
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

