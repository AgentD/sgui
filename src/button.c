#include "sgui_button.h"
#include "sgui_colors.h"
#include "sgui_font_manager.h"

#include "widget_internal.h"

#include <stdlib.h>
#include <string.h>



typedef struct
{
    sgui_widget widget;

    unsigned char* buffer;
    int state;
}
sgui_button;



void sgui_button_on_event( sgui_widget* widget, sgui_window* wnd,
                           int type, sgui_event* event )
{
    sgui_button* b = (sgui_button*)widget;
    (void)event;

    if( type == SGUI_DRAW_EVENT )
    {
        sgui_window_draw_box( wnd, widget->x, widget->y,
                                   widget->width, widget->height,
                                   SGUI_WINDOW_COLOR,
                                   b->state==0 ? -1 : 1 );

        sgui_window_blend_image( wnd, widget->x-b->state, widget->y-b->state,
                                 widget->width, widget->height,
                                 b->buffer );
    }
    else if( type == SGUI_MOUSE_LEAVE_EVENT )
    {
        b->state = 0;
        b->widget.need_redraw = 1;
    }
    else if( type == SGUI_MOUSE_PRESS_EVENT )
    {
        if( event->mouse_press.button == SGUI_MOUSE_BUTTON_LEFT )
        {
            b->state = event->mouse_press.pressed ? 1 : 0;
            b->widget.need_redraw = 1;
        }
    }
}



sgui_widget* sgui_button_create( int x, int y, const unsigned char* text,
                                 sgui_font* font, unsigned int size )
{
    sgui_button* b;
    unsigned int width, len;
    unsigned char color[3] = { (SGUI_DEFAULT_TEXT>>16) & 0xFF,
                               (SGUI_DEFAULT_TEXT>>8 ) & 0xFF,
                               SGUI_DEFAULT_TEXT & 0xFF };

    len = strlen( (const char*)text );
    width = sgui_font_extents( text, font, size, len );

    b = malloc( sizeof(sgui_button) );

    memset( b, 0, sizeof(sgui_button) );

    b->widget.x                     = x;
    b->widget.y                     = y;
    b->widget.width                 = width + 2*size;
    b->widget.height                = 2*size - size/4;
    b->widget.window_event_callback = sgui_button_on_event;
    b->widget.need_redraw           = 1;

    /* render the text */
    b->buffer = malloc( b->widget.width*b->widget.height*4 );
    b->state  = 0;

    memset( b->buffer, 0, b->widget.width*b->widget.height*4 );

    sgui_font_print_alpha( text, font, size, b->buffer,
                           b->widget.width/2 - width/2,
                           b->widget.height/2 - size/2 - size/8,
                           b->widget.width, b->widget.height, color, len );

    return (sgui_widget*)b;
}



void sgui_button_delete( sgui_widget* button )
{
    free( ((sgui_button*)button)->buffer );
    free( button );
}

