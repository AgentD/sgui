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



/* FIXME: somehow handle tabs with more captions than displayable? */
#ifndef SGUI_NO_TABS
typedef struct
{
    sgui_widget super;

    char* caption;
    unsigned int caption_width;
}
sgui_tab;

typedef struct
{
    sgui_widget super;

    sgui_tab* current;
    unsigned int tab_cap_width, tab_cap_height, count;
}
sgui_tab_group;



static void tab_on_state_change( sgui_widget* super, int change )
{
    sgui_tab_group* g = (sgui_tab_group*)super->parent;
    sgui_tab *this = (sgui_tab*)super, *old;
    sgui_event ev;

    if( change == SGUI_WIDGET_VISIBILLITY_CHANGED )
    {
        /* set tab to invisible is not allowed */
        if( !super->visible )
        {
            super->visible = 1;
            return;
        }

        if( g && this==g->current )
            return;

        /* turn previously visible tab invisible */
        if( g )
        {
            old = g->current;
            g->current = this;

            if( old )
            {
                old->super.visible = 0;

                ev.src.widget = (sgui_widget*)old;
                ev.type = SGUI_TAB_DESELECTED;
                sgui_widget_send_event( (sgui_widget*)old, &ev, 1 );
            }
        }

        /* generate select event for current tab */
        ev.src.widget = (sgui_widget*)this;
        ev.type = SGUI_TAB_SELECTED;
        sgui_widget_send_event( super, &ev, 1 );
    }
}

static void tab_on_draw( sgui_widget* super )
{
    sgui_tab* this = ((sgui_tab*)super);
    sgui_skin* skin = sgui_skin_get( );
    unsigned int gap = 0;
    sgui_widget* i;

    sgui_internal_lock_mutex( );
    for( i=super->parent->children; i!=super; i=i->next )
        gap += ((sgui_tab*)i)->caption_width;

    skin->draw_tab(skin,super->canvas,&super->area,gap,this->caption_width);
    sgui_internal_unlock_mutex( );
}

static void tab_destroy( sgui_widget* super )
{
    free( ((sgui_tab*)super)->caption );
    free( super );
}

/****************************************************************************/

static void tab_group_on_state_change( sgui_widget* super, int change )
{
    sgui_tab_group* this = (sgui_tab_group*)super;
    sgui_widget* i;

    if( change==SGUI_WIDGET_CHILD_ADDED && !super->children->next )
    {
        ((sgui_tab_group*)super)->current = (sgui_tab*)super->children;
        sgui_widget_set_visible( super->children, 1 );
    }
    else if( change==SGUI_WIDGET_CHILD_REMOVED )
    {
        for( i=super->children; i!=NULL; i=i->next )
        {
            if( i==(sgui_widget*)this->current )
                return;
        }

        this->current = NULL;
        sgui_widget_set_visible( super->children, 1 );
    }
}

static void tab_group_on_event( sgui_widget* super, const sgui_event* e )
{
    sgui_tab_group* this = (sgui_tab_group*)super;
    sgui_widget* i;
    int x;

    if( e->type == SGUI_MOUSE_PRESS_EVENT )
    {
        sgui_internal_lock_mutex( );

        for( x=e->arg.i3.x, i=super->children; i; i=i->next )
        {
            if( x<(int)(((sgui_tab*)i)->caption_width) )
                break;
            x -= ((sgui_tab*)i)->caption_width;
        }

        sgui_widget_set_visible( i, 1 );
        sgui_internal_unlock_mutex( );
    }
    else if( e->type==SGUI_KEY_RELEASED_EVENT && this->current &&
             (e->arg.i==SGUI_KC_LEFT || e->arg.i==SGUI_KC_RIGHT) )
    {
        sgui_internal_lock_mutex( );
        if( e->arg.i==SGUI_KC_LEFT )
        {
            for( i=super->children; i; i=i->next )
            {
                if( i->next==(sgui_widget*)this->current )
                {
                    sgui_widget_set_visible( i, 1 );
                    break;
                }
            }
        }
        else
        {
            sgui_widget_set_visible( this->current->super.next, 1 );
        }
        sgui_internal_unlock_mutex( );
    }
}

static void tab_group_draw( sgui_widget* super )
{
    int x = super->area.left, y = super->area.top;
    sgui_skin* skin = sgui_skin_get( );
    sgui_widget* i;

    sgui_internal_lock_mutex( );

    for( i=super->children; i!=NULL; i=i->next )
    {
        skin->draw_tab_caption( skin, super->canvas, x, y,
                                ((sgui_tab*)i)->caption,
                                ((sgui_tab*)i)->caption_width );
        x += ((sgui_tab*)i)->caption_width;
    }

    sgui_internal_unlock_mutex( );
}

/****************************************************************************/

sgui_widget* sgui_tab_group_create( int x, int y,
                                    unsigned int width, unsigned int height )
{
    sgui_tab_group* this = malloc( sizeof(sgui_tab_group) );
    sgui_skin* skin = sgui_skin_get( );
    sgui_rect r;

    if( !this )
        return NULL;

    memset( this, 0, sizeof(sgui_tab_group) );
    sgui_widget_init( (sgui_widget*)this, x, y, width, height );
    skin->get_tap_caption_extents( skin, &r );

    this->super.state_change_callback = tab_group_on_state_change;
    this->super.draw_callback         = tab_group_draw;
    this->super.window_event_callback = tab_group_on_event;
    this->super.destroy               = (void(*)(sgui_widget*))free;
    this->tab_cap_height              = SGUI_RECT_HEIGHT( r );
    this->tab_cap_width               = SGUI_RECT_WIDTH( r );

    return (sgui_widget*)this;
}

sgui_widget* sgui_tab_create( sgui_widget* parent, const char* caption )
{
    sgui_tab* this = malloc( sizeof(sgui_tab) );
    sgui_widget* super = (sgui_widget*)this;
    sgui_tab_group* g = (sgui_tab_group*)parent;

    if( !this )
        return NULL;

    memset( this, 0, sizeof(sgui_tab) );

    if( !(this->caption = malloc( strlen(caption) + 1 )) )
    {
        free( this );
        return NULL;
    }

    strcpy( this->caption, caption );

    sgui_internal_lock_mutex( );
    sgui_widget_init( super, 0, g->tab_cap_height,
                      SGUI_RECT_WIDTH(parent->area),
                      SGUI_RECT_HEIGHT(parent->area) - g->tab_cap_height );
    sgui_internal_unlock_mutex( );

    this->caption_width = g->tab_cap_width +
                          sgui_skin_default_font_extents( caption, -1, 0, 0 );
    super->focus_policy          = 0;
    super->visible               = 0;
    super->state_change_callback = tab_on_state_change;
    super->draw_callback         = tab_on_draw;
    super->destroy               = tab_destroy;

    return super;
}
#elif defined(SGUI_NOP_IMPLEMENTATIONS)
sgui_widget* sgui_tab_create( sgui_widget* parent, const char* caption )
{
    (void)parent; (void)caption;
    return NULL;
}
sgui_widget* sgui_tab_group_create( int x, int y,
                                    unsigned int width, unsigned int height )
{
    (void)x; (void)y; (void)width; (void)height;
    return NULL;
}
#endif /* !SGUI_NO_TABS */

