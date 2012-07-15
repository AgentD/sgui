#include "sgui_progress_bar.h"
#include "sgui_colors.h"

#include <stdlib.h>



typedef struct
{
    sgui_widget widget;

    float progress;
}
sgui_progress_bar;



void sgui_progress_bar_draw( sgui_widget* widget, sgui_window* wnd,
                             int x, int y, unsigned int w, unsigned int h )
{
    sgui_progress_bar* b;
    unsigned int width;

    (void)x; (void)y; (void)w; (void)h;

    b = (sgui_progress_bar*)widget;

    /* draw background box */
    sgui_window_draw_box( wnd, widget->x, widget->y,
                               widget->width, widget->height,
                               SGUI_INSET_FILL_COLOR_L1, 1 );

    /* draw bar */
    width = (unsigned int)(((float)(widget->width - 10)) * b->progress);

    if( width )
    {
        sgui_window_draw_box( wnd, widget->x+5, widget->y+5,
                                   width, widget->height - 10,
                                   SGUI_PROGRESS_BAR_COLOR, 0 );
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

void sgui_progress_bar_delete( sgui_widget* bar )
{
    free( bar );
}

