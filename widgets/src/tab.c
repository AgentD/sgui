/*
 * tab.c
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
#include "sgui_tab.h"
#include "sgui_skin.h"
#include "sgui_widget_manager.h"
#include "sgui_event.h"
#include "sgui_canvas.h"
#include "sgui_widget.h"
#include "sgui_internal.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>



typedef struct _sgui_tab
{
    struct _sgui_tab* next; /* linked list */

    sgui_widget_manager* mgr;
    char* caption;
    unsigned int caption_width;
}
sgui_tab;

typedef struct
{
    sgui_widget widget;

    sgui_tab* tabs;
    sgui_tab* selected;
    unsigned int tab_cap_height;
}
sgui_tab_group;




/* widget manager callback that passes widget events to the parent manager */
void tab_pass_event( sgui_widget* widget, int type, void* user )
{
    sgui_widget_manager_fire_widget_event( ((sgui_widget*)user)->mgr,
                                           widget, type );
}



void sgui_tab_group_on_event( sgui_widget* widget, int type,
                              sgui_event* event )
{
    sgui_tab_group* g = (sgui_tab_group*)widget;
    sgui_rect r;
    sgui_tab* i;
    int x, num;

    if( type == SGUI_MOUSE_PRESS_EVENT &&
        event->mouse_press.y < (int)g->tab_cap_height )
    {
        /* determine which tab caption was clicked */
        for( x=0, i=g->tabs; i!=NULL; i=i->next )
        {
            if( event->mouse_press.x>x &&
                event->mouse_press.x < (x + (int)i->caption_width) )
            {
                break;
            }

            x += i->caption_width;
        }

        /* select coresponding tab and mark widget area dirty */
        if( i!=g->selected )
        {
            sgui_widget_manager_add_dirty_rect( widget->mgr, &widget->area );

            if( g->selected )
            {
                sgui_widget_manager_on_event( g->selected->mgr, NULL, NULL );
                sgui_widget_manager_send_window_event( g->selected->mgr,
                                                       SGUI_TAB_DESELECTED,
                                                       NULL );
            }

            if( i )
            {
                sgui_widget_manager_on_event( i->mgr, tab_pass_event, widget );
                sgui_widget_manager_send_window_event( i->mgr,
                                                       SGUI_TAB_SELECTED,
                                                       NULL );
            }

            g->selected = i;
        }
    }
    else if( g->selected )
    {
        /* send event to selected tab */
        sgui_widget_manager_send_window_event(g->selected->mgr, type, event);

        /* transfer dirty rects from tab manager to parent widget manager */
        num = sgui_widget_manager_num_dirty_rects( g->selected->mgr );

        for( x=0; x<num; ++x )
        {
            sgui_widget_manager_get_dirty_rect( g->selected->mgr, &r, x );

            r.left += widget->area.left; r.right  += widget->area.left;
            r.top  += widget->area.top;  r.bottom += widget->area.top;

            sgui_widget_manager_add_dirty_rect( widget->mgr, &r );
        }

        sgui_widget_manager_clear_dirty_rects( g->selected->mgr );
    }
}

void sgui_tab_group_draw( sgui_widget* widget, sgui_canvas* cv )
{
    sgui_tab_group* g = (sgui_tab_group*)widget;
    unsigned int gap, gap_w;
    int x = widget->area.left, y = widget->area.top, old_ox, old_oy;
    sgui_rect r;
    sgui_tab* i;

    /* draw tab captions */
    for( i=g->tabs; i!=NULL; x += i->caption_width, i=i->next )
    {
        sgui_skin_draw_tab_caption( cv, x, y, i->caption_width, i->caption );
    }

    /* draw selected tab */
    if( g->selected )
    {
        /* draw tab frame */
        gap_w = g->selected->caption_width;

        for( i=g->tabs, gap=0; i && i!=g->selected; i=i->next )
            gap += i->caption_width;

        r = widget->area;
        r.top += g->tab_cap_height;
        r.bottom -= g->tab_cap_height + g->tab_cap_height;

        sgui_canvas_clear( cv, &r );

        sgui_skin_draw_tab( cv, widget->area.left,
                                widget->area.top + g->tab_cap_height, 
                                SGUI_RECT_WIDTH(widget->area),
                                SGUI_RECT_HEIGHT(widget->area) -
                                g->tab_cap_height, gap, gap_w );

        /* adjust offset and draw the tab area */
        sgui_canvas_get_offset( cv, &old_ox, &old_oy );
        sgui_canvas_add_offset( cv, widget->area.left, widget->area.top );
        sgui_widget_manager_draw_all( g->selected->mgr, cv );
        sgui_canvas_set_offset( cv, old_ox, old_oy );
    }
}



sgui_widget* sgui_tab_group_create( int x, int y,
                                    unsigned int width, unsigned int height )
{
    sgui_tab_group* g = malloc( sizeof(sgui_tab_group) );

    if( !g )
        return NULL;

    sgui_internal_widget_init( (sgui_widget*)g, x, y, width, height );

    g->widget.draw_callback         = sgui_tab_group_draw;
    g->widget.window_event_callback = sgui_tab_group_on_event;
    g->tabs                         = NULL;
    g->selected                     = NULL;
    g->tab_cap_height               = sgui_skin_get_tab_caption_height( );

    return (sgui_widget*)g;
}

void sgui_tab_group_destroy( sgui_widget* tab )
{
    sgui_tab_group* t = (sgui_tab_group*)tab;
    sgui_tab *i, *old;

    if( t )
    {
        i = t->tabs;

        while( i!=NULL )
        {
            old = i;
            i = i->next;
            sgui_widget_manager_destroy( old->mgr );
            free( old->caption );
            free( old );
        }

        free( t );
    }
}

int sgui_tab_group_add_tab( sgui_widget* tab, const char* caption )
{
    sgui_tab_group* g = (sgui_tab_group*)tab;
    sgui_tab* i;
    sgui_tab* t;
    int index = -1;

    if( !g || !caption )
        return -1;

    /* allocate a new tab */
    t = malloc( sizeof(sgui_tab) );

    if( !t )
        return -1;

    /* initialise the tab */
    t->next = NULL;
    t->caption = malloc( strlen(caption) + 1 );

    if( !t->caption )
    {
        free( t );
        return -1;
    }

    strcpy( t->caption, caption );
    t->caption_width = sgui_skin_get_tab_caption_width( caption );

    t->mgr = sgui_widget_manager_create( );

    if( !t->mgr )
    {
        free( t->caption );
        free( t );
        return -1;
    }

    /* add the tab */
    if( g->tabs )
    {
        for( index=0, i=g->tabs; i->next; i=i->next, ++index );

        i->next = t;
        return index + 1;
    }

    g->tabs = t;
    g->selected = t;
    sgui_widget_manager_on_event( t->mgr, tab_pass_event, tab );

    if( g->selected )
        sgui_widget_manager_send_window_event( g->selected->mgr,
                                               SGUI_TAB_DESELECTED, NULL );

    return 0;
}

void sgui_tab_group_add_widget( sgui_widget* tab, int index, sgui_widget* w )
{
    sgui_tab* i;
    int count;

    if( tab && index>=0 && w )
    {
        i = ((sgui_tab_group*)tab)->tabs;

        for( count=0; count<index && i; i=i->next, ++count );

        if( count==index && i )
        {
            sgui_widget_manager_add_widget( i->mgr, w );

            if( i != ((sgui_tab_group*)tab)->selected )
                sgui_widget_send_window_event( w, SGUI_TAB_DESELECTED, NULL );
            else
                sgui_widget_send_window_event( w, SGUI_TAB_SELECTED, NULL );
        }
    }
}

void sgui_tab_group_remove_widget( sgui_widget* tab, sgui_widget* w )
{
    if( tab && w )
        sgui_widget_manager_remove_widget( w->mgr, w );
}

