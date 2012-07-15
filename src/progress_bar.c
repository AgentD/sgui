#include "sgui_progress_bar.h"
#include "sgui_colors.h"

#include <stdlib.h>



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



void sgui_progress_bar_draw( sgui_widget* widget, sgui_window* wnd,
                             int x, int y, unsigned int w, unsigned int h )
{
    sgui_progress_bar* b = (sgui_progress_bar*)widget;
    unsigned int width, height, segments, i;
    int ox, oy;

    (void)x; (void)y; (void)w; (void)h;

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

    b->widget.x             = x;
    b->widget.y             = y;
    b->widget.width         = width;
    b->widget.height        = height;
    b->widget.draw_callback = sgui_progress_bar_draw;
    b->progress             = progress;
    b->continuous           = SGUI_PROGRESS_BAR_STIPPLED;
    b->offset               = SGUI_PROGRESS_BAR_OFFSET;
    b->color                = SGUI_WHITE;
    b->vertical             = 0;

    return (sgui_widget*)b;
}

void sgui_progress_bar_set_progress( sgui_widget* bar, float progress )
{
    sgui_progress_bar* b = (sgui_progress_bar*)bar;

    progress = (progress>1.0f) ? 1.0f : ((progress<0.0f) ? 0.0f : progress);

    if( b )
        b->progress = progress;
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
        b->continuous = continuous;
        b->offset     = offset;
    }
}

void sgui_progress_bar_set_color( sgui_widget* bar, unsigned long color )
{
    sgui_progress_bar* b = (sgui_progress_bar*)bar;

    if( b )
        b->color = color;
}

void sgui_progress_bar_set_direction( sgui_widget* bar, int vertical )
{
    sgui_progress_bar* b = (sgui_progress_bar*)bar;

    if( b )
        b->vertical = vertical;
}

void sgui_progress_bar_delete( sgui_widget* bar )
{
    free( bar );
}

