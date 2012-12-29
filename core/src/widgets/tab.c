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
#include "sgui_internal.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>



typedef struct
{
    sgui_widget_manager* mgr;
    char* caption;
    unsigned int caption_width;
}
sgui_tab;

typedef struct
{
    sgui_widget widget;

    sgui_tab* tabs;
    unsigned int num_tabs;
    unsigned int tabs_avail;
    unsigned int tab_cap_height;

    int selected;
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
    sgui_widget_manager* mgr;
    sgui_rect r;
    unsigned int i, num;
    int x;

    if( type == SGUI_MOUSE_PRESS_EVENT &&
        event->mouse_press.y < (int)g->tab_cap_height )
    {
        /* determine which tab caption was clicked */
        for( x=0, i=0; i<g->num_tabs; ++i )
        {
            if( event->mouse_press.x>x &&
                event->mouse_press.x < (x + (int)g->tabs[i].caption_width) )
            {
                break;
            }

            x += g->tabs[i].caption_width;
        }

        /* select coresponding tab and mark widget area dirty */
        if( g->selected>=0 )
            sgui_widget_manager_on_event( g->tabs[ g->selected ].mgr,
                                          NULL, NULL );

        if( i<g->num_tabs && (int)i!=g->selected )
        {
            g->selected = i;
            sgui_widget_manager_add_dirty_rect( widget->mgr, &widget->area );
            sgui_widget_manager_on_event( g->tabs[ g->selected ].mgr,
                                          tab_pass_event, widget );
        }
    }
    else if( g->selected>=0 && g->selected<(int)g->num_tabs )
    {
        mgr = g->tabs[ g->selected ].mgr;

        /* send event to selected tab */
        sgui_widget_manager_send_window_event( mgr, type, event );

        /* transfer dirty rects from tab manager to parent widget manager */
        num = sgui_widget_manager_num_dirty_rects( mgr );

        for( i=0; i<num; ++i )
        {
            sgui_widget_manager_get_dirty_rect( mgr, &r, i );

            r.left += widget->area.left; r.right  += widget->area.left;
            r.top  += widget->area.top;  r.bottom += widget->area.top;

            sgui_widget_manager_add_dirty_rect( widget->mgr, &r );
        }

        sgui_widget_manager_clear_dirty_rects( mgr );
    }
}

void sgui_tab_group_draw( sgui_widget* widget, sgui_canvas* cv )
{
    sgui_tab_group* g = (sgui_tab_group*)widget;
    unsigned int i, gap, gap_w;
    int x = widget->area.left, y = widget->area.top;
    sgui_rect r;

    /* draw tab captions */
    for( i=0; i<g->num_tabs; ++i )
    {
        sgui_skin_draw_tab_caption( cv, x, y, g->tabs[i].caption_width,
                                    g->tabs[i].caption );

        x += g->tabs[i].caption_width;
    }

    /* draw selected tab */
    if( g->selected >= 0 && (unsigned int)g->selected < g->num_tabs )
    {
        /* draw tab frame */
        gap_w = g->tabs[ g->selected ].caption_width;

        for( i=0, gap=0; i<(unsigned int)g->selected; ++i )
            gap += g->tabs[i].caption_width;

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
        sgui_canvas_set_offset( cv, widget->area.left, widget->area.top );
        sgui_widget_manager_draw_all( g->tabs[g->selected].mgr, cv );
        sgui_canvas_restore_offset( cv );
    }
}



sgui_widget* sgui_tab_group_create( int x, int y,
                                    unsigned int width, unsigned int height )
{
    sgui_tab_group* g = malloc( sizeof(sgui_tab_group) );

    if( !g )
        return NULL;

    g->tabs = malloc( sizeof(sgui_tab)*10 );

    if( !g->tabs )
    {
        free( g );
        return NULL;
    }

    sgui_internal_widget_init( (sgui_widget*)g, x, y, width, height );

    g->widget.draw_callback         = sgui_tab_group_draw;
    g->widget.window_event_callback = sgui_tab_group_on_event;
    g->num_tabs                     = 0;
    g->tabs_avail                   = 10;
    g->tab_cap_height               = sgui_skin_get_tab_caption_height( );
    g->selected                     = -1;

    return (sgui_widget*)g;
}

void sgui_tab_group_destroy( sgui_widget* tab )
{
    sgui_tab_group* t = (sgui_tab_group*)tab;
    unsigned int i;

    if( t )
    {
        for( i=0; i<t->num_tabs; ++i )
        {
            sgui_widget_manager_destroy( t->tabs[ i ].mgr );
            free( t->tabs[ i ].caption );
        }

        free( t->tabs );
        free( t );
    }
}

int sgui_tab_group_add_tab( sgui_widget* tab, const char* caption )
{
    sgui_tab_group* g = (sgui_tab_group*)tab;
    sgui_tab* new_tabs = NULL;
    int index = -1;

    if( g && caption )
    {
        /* resize tab aray if required and acquire a new tab */
        if( g->num_tabs < g->tabs_avail )
        {
            index = g->num_tabs;
            g->num_tabs += 1;
        }
        else
        {
            new_tabs = realloc( g->tabs, sizeof(sgui_tab)*(g->tabs_avail*2) );

            if( new_tabs )
            {
                index = g->num_tabs;

                g->tabs = new_tabs;
                g->num_tabs += 1;
                g->tabs_avail *= 2;
            }
        }

        /* initialise new tab */
        if( index>=0 )
        {
            /* try to create a widget manager for the tab */
            g->tabs[ index ].mgr = sgui_widget_manager_create( );

            if( !g->tabs[ index ].mgr )
            {
                g->num_tabs -= 1;
                return -1;
            }

            /* if it's the first tab added, select it */
            if( g->selected < 0 )
            {
                g->selected = 0;
                sgui_widget_manager_on_event( g->tabs[ g->selected ].mgr,
                                              tab_pass_event, tab );
            }

            /* try to store the caption string */
            g->tabs[ index ].caption = malloc( strlen(caption) + 1 );

            if( !g->tabs[ index ].caption )
            {
                sgui_widget_manager_destroy( g->tabs[ index ].mgr );
                g->num_tabs -= 1;
                return -1;
            }

            strcpy( g->tabs[ index ].caption, caption );

            g->tabs[ index ].caption_width =
            sgui_skin_get_tab_caption_width( caption );
        }
    }

    return index;
}

void sgui_tab_group_add_widget( sgui_widget* tab, int index, sgui_widget* w )
{
    sgui_tab_group* g = (sgui_tab_group*)tab;

    if( g && index>=0 && index<(int)g->num_tabs )
        sgui_widget_manager_add_widget( g->tabs[ index ].mgr, w );
}

void sgui_tab_group_remove_widget( sgui_widget* tab, int index,
                                   sgui_widget* w )
{
    sgui_tab_group* g = (sgui_tab_group*)tab;

    if( g && index>=0 && index<(int)g->num_tabs )
        sgui_widget_manager_remove_widget( g->tabs[ index ].mgr, w );
}

