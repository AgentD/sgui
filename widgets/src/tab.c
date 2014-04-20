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

    sgui_widget dummy;
    char* caption;
    unsigned int caption_width;
}
sgui_tab;

typedef struct
{
    sgui_widget super;

    sgui_tab* tabs;
    unsigned int tab_cap_width, tab_cap_height;
}
sgui_tab_group;



static void tab_group_destroy( sgui_widget* super )
{
    sgui_tab_group* this = (sgui_tab_group*)super;
    sgui_tab *i, *old;

    i = this->tabs;

    while( i!=NULL )
    {
        old = i;
        i = i->next;
        free( old->caption );
        free( old );
    }

    free( this );
}

static void tab_group_on_event( sgui_widget* super, const sgui_event* e )
{
    sgui_tab_group* this = (sgui_tab_group*)super;
    sgui_tab *i, *j;
    sgui_event ev;
    int x;

    ev.src.widget = super;

    if( e->type == SGUI_MOUSE_PRESS_EVENT )
    {
        sgui_internal_lock_mutex( );

        /* determine which tab caption was clicked */
        for( x=e->arg.i3.x, i=this->tabs; i!=NULL; i=i->next )
        {
            if( x>=0 && x<(int)i->caption_width )
                break;

            x -= i->caption_width;
        }

        /* select coresponding tab and deselect the current one */
        if( i && !i->dummy.visible )
        {
            for( j=this->tabs; j!=NULL && !j->dummy.visible; j=j->next );

            if( j )
            {
                ev.type = SGUI_TAB_DESELECTED;
                sgui_widget_set_visible( &j->dummy, 0 );
                sgui_widget_send_event( &j->dummy, &ev, 1 );
            }

            ev.type = SGUI_TAB_SELECTED;
            sgui_widget_set_visible( &i->dummy, 1 );
            sgui_widget_send_event( &i->dummy, &ev, 1 );
        }

        sgui_internal_unlock_mutex( );
    }
    else if( e->type == SGUI_KEY_RELEASED_EVENT )
    {
        if( e->arg.i!=SGUI_KC_LEFT && e->arg.i!=SGUI_KC_RIGHT )
            return;

        sgui_internal_lock_mutex( );

        if( e->arg.i==SGUI_KC_LEFT )
        {
            if( this->tabs->dummy.visible )
            {
                sgui_internal_unlock_mutex( );
                return;
            }

            /* find currently selected and tab and its predecessor */
            for(i=this->tabs; i!=NULL && !i->next->dummy.visible; i=i->next);

            j = i->next;
        }
        else
        {
            /* find currently selected and tab and its successor */
            for( j=this->tabs; j!=NULL && !j->dummy.visible; j=j->next );

            i = j->next;
        }

        /* sanity check */
        if( !i || !j )
        {
            sgui_internal_unlock_mutex( );
            return;
        }

        /* deselect the currently selected tab */
        ev.type = SGUI_TAB_DESELECTED;
        sgui_widget_set_visible( &j->dummy, 0 );
        sgui_widget_send_event( &j->dummy, &ev, 1 );

        /* select the other tab */
        ev.type = SGUI_TAB_SELECTED;
        sgui_widget_set_visible( &i->dummy, 1 );
        sgui_widget_send_event( &i->dummy, &ev, 1 );

        sgui_internal_unlock_mutex( );
    }
}

static void tab_group_draw( sgui_widget* super )
{
    sgui_tab_group* this = (sgui_tab_group*)super;
    int x = super->area.left, y = super->area.top;
    sgui_canvas* cv = super->canvas;
    unsigned int gap, gap_w;
    sgui_rect r;
    sgui_tab* i;

    /* draw tab captions. TODO: what if there are too many captions? */
    sgui_internal_lock_mutex( );

    for( i=this->tabs; i!=NULL; i=i->next )
    {
        sgui_skin_draw_tab_caption( cv, x, y, i->caption, i->caption_width );
        x += i->caption_width;
    }

    /* draw selected tab */
    for( i=this->tabs, gap=0; i!=NULL; i=i->next )
    {
        if( i->dummy.visible )
        {
            gap_w = i->caption_width;
            break;
        }

        gap += i->caption_width;
    }

    if( i )
    {
        r = super->area;
        r.top += this->tab_cap_height;
        sgui_skin_draw_tab( cv, &r, gap, gap_w );
    }

    sgui_internal_unlock_mutex( );
}



sgui_widget* sgui_tab_group_create( int x, int y,
                                    unsigned int width, unsigned int height )
{
    sgui_tab_group* g = malloc( sizeof(sgui_tab_group) );
    sgui_rect r;

    if( !g )
        return NULL;

    sgui_internal_widget_init( (sgui_widget*)g, x, y, width, height );
    sgui_skin_get_tap_caption_extents( &r );

    g->super.draw_callback         = tab_group_draw;
    g->super.window_event_callback = tab_group_on_event;
    g->super.destroy               = tab_group_destroy;
    g->tabs                        = NULL;
    g->tab_cap_height              = SGUI_RECT_HEIGHT( r );
    g->tab_cap_width               = SGUI_RECT_WIDTH( r );

    return (sgui_widget*)g;
}

int sgui_tab_group_add_tab( sgui_widget* super, const char* caption )
{
    sgui_tab_group* this = (sgui_tab_group*)super;
    sgui_tab* i;
    sgui_tab* t;
    int idx = -1;

    if( !this || !caption )
        return -1;

    /* allocate a new tab */
    t = malloc( sizeof(sgui_tab) );

    if( !t )
        return -1;

    memset( t, 0, sizeof(sgui_tab) );

    /* initialise the tab */
    t->caption = malloc( strlen(caption) + 1 );

    if( !t->caption )
    {
        free( t );
        return -1;
    }

    strcpy( t->caption, caption );

    sgui_internal_lock_mutex( );
    t->caption_width = this->tab_cap_width +
                       sgui_skin_default_font_extents( caption, -1, 0, 0 );

    t->dummy.area.top = this->tab_cap_height;
    t->dummy.area.right = super->area.right - super->area.left;
    t->dummy.area.bottom = t->dummy.area.top + super->area.bottom -
                           super->area.top;
    t->dummy.visible = (this->tabs==NULL);  /* first is visible by default */
    t->dummy.canvas = super->canvas;
    t->dummy.focus_policy = 0;

    sgui_widget_add_child( super, &t->dummy );

    /* add the tab to the end, if we already have tabs */
    if( this->tabs )
    {
        for( idx=0, i=this->tabs; i->next; i=i->next, ++idx );

        i->next = t;
        sgui_internal_unlock_mutex( );
        return idx + 1;
    }

    this->tabs = t;
    sgui_internal_unlock_mutex( );
    return 0;
}

void sgui_tab_group_add_widget( sgui_widget* this, int idx, sgui_widget* w )
{
    sgui_event ev;
    sgui_tab* i;
    int count;

    if( this && idx>=0 && w )
    {
        sgui_internal_lock_mutex( );

        /* find the tab for the number */
        i = ((sgui_tab_group*)this)->tabs;
        for( count=0; count<idx && i; i=i->next, ++count );

        if( count!=idx || !i )
        {
            sgui_internal_unlock_mutex( );
            return;
        }

        /* add the widget */
        sgui_widget_add_child( &i->dummy, w );

        /* send a tab select/deselect event */
        ev.src.widget = this;
        ev.type = i->dummy.visible ? SGUI_TAB_SELECTED : SGUI_TAB_DESELECTED;

        sgui_widget_send_event( w, &ev, 1 );

        sgui_internal_unlock_mutex( );
    }
}

