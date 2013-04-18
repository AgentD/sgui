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

    sgui_widget* widgets;
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



static void set_canvas( sgui_widget* i, sgui_canvas* canvas )
{
    for( ; i!=NULL; i=i->next )
    {
        i->canvas = canvas;
        set_canvas( i->children, canvas );
    }
}

static void reparent( sgui_widget* i, sgui_widget* parent )
{
    for( ; i!=NULL; i=i->next )
    {
        i->parent = parent;
        i->canvas = parent ? parent->canvas : NULL;
        set_canvas( i->children, parent ? parent->canvas : NULL );
    }
}

static void tab_group_destroy( sgui_widget* tab )
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
            free( old->caption );
            free( old );
        }

        free( t );
    }
}



void sgui_tab_group_on_event( sgui_widget* widget, int type,
                              sgui_event* event )
{
    sgui_tab_group* g = (sgui_tab_group*)widget;
    sgui_tab* i;
    sgui_widget* j;
    int x;

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
        if( i && i!=g->selected )
        {
            sgui_canvas_add_dirty_rect( widget->canvas, &widget->area );

            if( g->selected )
            {
                for( j=g->selected->widgets; j!=NULL; j=j->next )
                {
                    sgui_widget_send_event( j, SGUI_TAB_DESELECTED, NULL, 1 );
                }
            }

            if( i )
            {
                for( j=i->widgets; j!=NULL; j=j->next )
                {
                    sgui_widget_send_event( j, SGUI_TAB_SELECTED, NULL, 1 );
                }
            }

            reparent( g->selected->widgets, NULL );
            g->selected = i;

            widget->children = g->selected->widgets;
            reparent( g->selected->widgets, widget );
        }
    }
}

void sgui_tab_group_draw( sgui_widget* widget )
{
    sgui_tab_group* g = (sgui_tab_group*)widget;
    unsigned int gap, gap_w;
    int x = widget->area.left, y = widget->area.top;
    sgui_tab* i;

    /* draw tab captions */
    for( i=g->tabs; i!=NULL; x += i->caption_width, i=i->next )
    {
        sgui_skin_draw_tab_caption( widget->canvas, x, y, i->caption_width,
                                    i->caption );
    }

    /* draw selected tab */
    if( g->selected )
    {
        /* draw tab frame */
        gap_w = g->selected->caption_width;

        for( i=g->tabs, gap=0; i && i!=g->selected; i=i->next )
            gap += i->caption_width;

        sgui_skin_draw_tab( widget->canvas, widget->area.left,
                            widget->area.top + g->tab_cap_height, 
                            SGUI_RECT_WIDTH(widget->area),
                            SGUI_RECT_HEIGHT(widget->area) -
                            g->tab_cap_height, gap, gap_w );
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
    g->widget.destroy               = tab_group_destroy;
    g->tabs                         = NULL;
    g->selected                     = NULL;
    g->tab_cap_height               = sgui_skin_get_tab_caption_height( );

    return (sgui_widget*)g;
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
    t->widgets = NULL;
    t->next = NULL;
    t->caption = malloc( strlen(caption) + 1 );

    if( !t->caption )
    {
        free( t );
        return -1;
    }

    strcpy( t->caption, caption );
    t->caption_width = sgui_skin_get_tab_caption_width( caption );

    /* add the tab to the end, if we already have tabs */
    if( g->tabs )
    {
        for( index=0, i=g->tabs; i->next; i=i->next, ++index );

        i->next = t;
        return index + 1;
    }

    /* we don't have any tabs yet, make it the first one */
    g->tabs = t;
    g->selected = t;
    tab->children = NULL;

    return 0;
}

void sgui_tab_group_add_widget( sgui_widget* tab, int index, sgui_widget* w )
{
    sgui_rect r;
    sgui_tab* i;
    int count;

    if( tab && index>=0 && w )
    {
        /* find the tab for the number */
        i = ((sgui_tab_group*)tab)->tabs;
        for( count=0; count<index && i; i=i->next, ++count );

        if( count!=index || !i )
            return;

        /* add the widget */
        w->canvas = tab->canvas;
        w->next = i->widgets;
        w->parent = tab;
        i->widgets = w;

        /* send a tab select/deselect event and flag the area dirty */
        if( i == ((sgui_tab_group*)tab)->selected )
        {
            reparent( w, tab );

            tab->children = i->widgets;
            sgui_widget_send_event( w, SGUI_TAB_SELECTED, NULL, 1 );

            sgui_widget_get_absolute_rect( w, &r );
            sgui_canvas_add_dirty_rect( tab->canvas, &r );
        }
        else
        {
            reparent( w, NULL );

            sgui_widget_send_event( w, SGUI_TAB_DESELECTED, NULL, 1 );
        }
    }
}

