/*
 * icon_view.c
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
#include "sgui_scroll_bar.h"
#include "sgui_icon_cache.h"
#include "sgui_icon_view.h"
#include "sgui_internal.h"
#include "sgui_widget.h"
#include "sgui_event.h"
#include "sgui_model.h"
#include "sgui_skin.h"

#include <stdlib.h>
#include <string.h>

#if !defined(SGUI_NO_ICON_CACHE) && !defined(SGUI_NO_ICON_VIEW) &&\
    !defined(SGUI_NO_MODEL)
#define IV_MULTISELECT 0x01
#define IV_DRAG        0x02
#define IV_SELECTBOX   0x04
#define IV_DRAW_BG     0x08



typedef struct
{
    const sgui_item* item;  /* underlying model item */
    sgui_rect icon_area;    /* area of the icon inside the view */
    sgui_rect text_area;    /* area of the subtext inside the view */
    int selected;           /* non-zero if the icon is selected */
}
icon;

typedef struct
{
    sgui_widget super;
    sgui_model* model;      /* underlying model */
    sgui_widget* v_bar;     /* vertical scroll bar */
    icon* icons;            /* an array of icons */
    unsigned int num_icons; /* number of icons */
    unsigned int v_bar_dist;/* distance of the vertical scroll bar */
    sgui_rect selection;    /* selection rect */
    unsigned int icon_col;
    unsigned int txt_col;
    int flags;
    int offset;             /* the offset from the border of the view */

    const sgui_item* itemlist;
}
icon_view;



#define SELECT( icon, state )\
        (icon).selected = (state);\
        get_icon_bounding_box( this, &(icon), &r );\
        r.bottom -= offset;\
        r.top -= offset;\
        sgui_canvas_add_dirty_rect( this->super.canvas, &r )


static int compare( icon_view* this, sgui_item_compare_fun fun,
                    icon* a, icon* b )
{
    if( fun )
        return fun( this->model, a->item, b->item );

    return strcmp( sgui_item_text( this->model, a->item, this->txt_col ),
                   sgui_item_text( this->model, b->item, this->txt_col ) );
}

static void sink( icon_view* this, sgui_item_compare_fun fun,
                  icon* pq, unsigned int k, unsigned int n )
{
    unsigned int j;
    icon temp;

    for( j=2*k; j<=n; k=j, j*=2 )
    {
        if( j<n && compare( this, fun, pq+j-1, pq+j )<0 )
            ++j;

        if( compare( this, fun, pq+k-1, pq+j-1 )>=0 )
            break;

        temp=pq[k-1]; pq[k-1]=pq[j-1]; pq[j-1]=temp;
    }
}

static void draw_icon( icon_view* this, icon* i, sgui_skin* skin,
                       unsigned int offset )
{
    sgui_rect r;

    sgui_icon_cache_draw_icon( sgui_model_get_icon_cache(this->model),
                           sgui_item_icon(this->model,i->item,this->icon_col),
                           this->super.area.left + i->icon_area.left,
                           this->super.area.top  + i->icon_area.top - offset);

    if( i->selected )
    {
        r = sgui_item_text(this->model,i->item,this->txt_col) ?
            i->text_area : i->icon_area;

        r.top -= offset;
        r.bottom -= offset;

        sgui_rect_add_offset(&r, this->super.area.left, this->super.area.top);
        skin->draw_focus_box(skin, this->super.canvas, &r);
    }

    sgui_skin_draw_text( this->super.canvas,
                         this->super.area.left + i->text_area.left,
                         this->super.area.top  + i->text_area.top - offset,
                         sgui_item_text(this->model,i->item,this->txt_col) );
}

static void ideal_grid_size( icon_view* this,
                             unsigned int* grid_w, unsigned int* grid_h )
{
    unsigned int w, h, i;

    *grid_w = 0;
    *grid_h = 0;

    for( i=0; i<this->num_icons; ++i )
    {
        w = SGUI_RECT_WIDTH( this->icons[i].icon_area );
        *grid_w = MAX(*grid_w, w);
        w = SGUI_RECT_WIDTH( this->icons[i].text_area );
        *grid_w = MAX(*grid_w, w);
        h = SGUI_RECT_HEIGHT(this->icons[i].icon_area) +
            SGUI_RECT_HEIGHT(this->icons[i].text_area);
        *grid_h = MAX(*grid_h, h);
    }

    *grid_w += *grid_w/5;
    *grid_h += *grid_h/10;
}

static void gridify( icon_view* this )
{
    unsigned int x, y, grid_w, grid_h, total_w, txt_w, txt_h, img_w, img_h, i;
    sgui_rect r;
    int dx, dy;

    ideal_grid_size( this, &grid_w, &grid_h );

    x = y = this->offset;
    total_w = SGUI_RECT_WIDTH(this->super.area) - 2*this->offset;

    for( i=0; i<this->num_icons; ++i )
    {
        if( (x + grid_w) >= total_w )
        {
            x  = this->offset;
            y += grid_h;
        }

        txt_w = SGUI_RECT_WIDTH(this->icons[i].text_area);
        txt_h = SGUI_RECT_HEIGHT(this->icons[i].text_area);
        img_w = SGUI_RECT_WIDTH(this->icons[i].icon_area);
        img_h = SGUI_RECT_HEIGHT(this->icons[i].icon_area);

        dx = img_w < grid_w ? (grid_w - img_w)/2 : 0;
        dy = (img_h + txt_h) < grid_h ? (grid_h - img_h - txt_h)/2 : 0;
        sgui_rect_set_size( &this->icons[i].icon_area,
                            x + dx, y + dy, img_w, img_h );

        dx = txt_w < grid_w ? (grid_w - txt_w)/2 : 0;
        sgui_rect_set_size( &this->icons[i].text_area, x + dx,
                            this->icons[i].icon_area.bottom, txt_w, txt_h );

        x += grid_w;
    }

    if( this->super.canvas )
    {
        sgui_widget_get_absolute_rect( &(this->super), &r );
        sgui_canvas_add_dirty_rect( this->super.canvas, &r );
    }
}

static void get_icon_bounding_box( icon_view* this, icon* i, sgui_rect* r )
{
    int x, y;

    *r = i->text_area;
    sgui_rect_join( r, &i->icon_area, 0 );
    sgui_widget_get_absolute_position( &(this->super), &x, &y );
    sgui_rect_add_offset( r, x, y );
}

static void set_all_icons( icon_view* this, int state )
{
    unsigned int i;
    sgui_rect r;

    for( i=0; i<this->num_icons; ++i )
        this->icons[i].selected = state;

    if( this->super.canvas )
    {
        sgui_widget_get_absolute_rect( (sgui_widget*)this, &r );
        sgui_canvas_add_dirty_rect( this->super.canvas, &r );
    }
}

static void view_on_scroll_v( void* userptr, int new_offset, int delta )
{
    sgui_widget* this = userptr;
    sgui_rect r;
    (void)new_offset;
    (void)delta;

    if( this->canvas )
    {
        sgui_widget_get_absolute_rect( this, &r );
        sgui_canvas_add_dirty_rect( this->canvas, &r );
    }
}

static icon* icon_from_point( icon_view* this, int x, int y )
{
    unsigned int i;
    icon* top;

    for( top=NULL, i=0; i<this->num_icons; ++i )
    {
        if( this->icons[i].selected )
            continue;

        if( sgui_rect_is_point_inside( &(this->icons[i].icon_area), x, y ) ||
            sgui_rect_is_point_inside( &(this->icons[i].text_area), x, y ) )
        {
            top = &this->icons[i];
        }
    }

    for( i=0; i<this->num_icons; ++i )
    {
        if( !this->icons[i].selected )
            continue;

        if( sgui_rect_is_point_inside( &(this->icons[i].icon_area), x, y ) ||
            sgui_rect_is_point_inside( &(this->icons[i].text_area), x, y ) )
        {
            top = &this->icons[i];
        }
    }

    return top;
}

static void generate_event_for_each_selected( icon_view* this, int type )
{
    unsigned int i;
    sgui_event ev;

    for( i=0; i<this->num_icons; ++i )
    {
        if( this->icons[i].selected )
        {
            ev.src.other = (void*)this->icons[i].item;
            ev.type = type;
            sgui_event_post( &ev );
        }
    }
}

static void update_scroll_area( icon_view* this )
{
    unsigned int i, length, displength;
    sgui_rect r;

    displength = SGUI_RECT_HEIGHT(this->super.area);

    if( this->num_icons )
    {
        r = this->icons[0].icon_area;
        sgui_rect_join( &r, &this->icons[0].text_area, 0 );

        for( i=1; i<this->num_icons; ++i )
        {
            sgui_rect_join( &r, &this->icons[i].icon_area, 0 );
            sgui_rect_join( &r, &this->icons[i].text_area, 0 );
        }

        r.top = 0;
        length = SGUI_RECT_HEIGHT( r ) + 10;
        length = MAX(length, displength);
    }
    else
    {
        length = displength;
    }

    sgui_scroll_bar_set_area( this->v_bar, length, displength );

    if( length==displength )
    {
        sgui_scroll_bar_set_offset( this->v_bar, 0 );
        sgui_widget_set_visible( this->v_bar, 0 );
    }
    else
    {
        sgui_widget_set_visible( this->v_bar, 1 );
    }
}

static void icon_view_on_event( sgui_widget* super, const sgui_event* e )
{
    icon_view* this = (icon_view*)super;
    int x, y, dx, dy, hit, offset;
    sgui_rect r, r1;
    unsigned int i;
    sgui_event ev;
    icon* new;

    if( !this->icons )
        return;

    sgui_internal_lock_mutex( );

    if(e->type==SGUI_MOUSE_PRESS_EVENT && e->arg.i3.z==SGUI_MOUSE_BUTTON_LEFT)
    {
        offset = sgui_scroll_bar_get_offset( this->v_bar );

        x = e->arg.i3.x;
        y = e->arg.i3.y + offset;
        sgui_rect_set_size(&this->selection, x, y, 0, 0);
        new = icon_from_point( this, x, y );

        if( !(this->flags & IV_MULTISELECT) && !(new && new->selected) )
            set_all_icons( this, 0 );

        if( new )
        {
            this->flags |= IV_DRAG;
            new->selected = !(new->selected && (this->flags&IV_MULTISELECT));
            get_icon_bounding_box( this, new, &r );
            r.top -= offset;
            r.bottom -= offset;
            sgui_canvas_add_dirty_rect( super->canvas, &r );
        }
        else if( !(this->flags & IV_MULTISELECT) )
        {
            this->flags = (this->flags|IV_SELECTBOX) & (~IV_DRAG);
        }
    }
    else if( e->type==SGUI_DOUBLE_CLICK_EVENT )
    {
        x = e->arg.i2.x;
        y = e->arg.i2.y + sgui_scroll_bar_get_offset( this->v_bar );
        set_all_icons( this, 0 );
        this->flags &= ~IV_SELECTBOX;
        new = icon_from_point( this, x, y );

        if( new )
        {
            new->selected = 1;
            ev.type = SGUI_ICON_SELECTED_EVENT;
            ev.src.other = (void*)new->item;
            sgui_event_post( &ev );
        }
    }
    else if( e->type==SGUI_MOUSE_WHEEL_EVENT )
    {
        dy = SGUI_RECT_HEIGHT(super->area) / 4;
        offset = sgui_scroll_bar_get_offset( this->v_bar );

        if( e->arg.i > 0 )
        {
            dy = (dy>offset) ? offset : dy;
            sgui_scroll_bar_set_offset( this->v_bar, offset - dy );
            view_on_scroll_v( this, offset - dy, -dy );
        }
        else if( e->arg.i < 0 )
        {
            sgui_scroll_bar_set_offset( this->v_bar, offset + dy );
            view_on_scroll_v( this, offset + dy, dy );
        }
    }
    else if( e->type==SGUI_MOUSE_MOVE_EVENT && (this->flags & IV_SELECTBOX) )
    {
        offset = sgui_scroll_bar_get_offset( this->v_bar );

        /* get affected area */
        r1 = this->selection;
        sgui_rect_repair( &r1 );

        this->selection.right  = e->arg.i2.x;
        this->selection.bottom = e->arg.i2.y +
                                 sgui_scroll_bar_get_offset( this->v_bar );

        r = this->selection;
        sgui_rect_repair( &r );
        sgui_rect_join( &r1, &r, 0 );

        /* flag area as dity */
        sgui_widget_get_absolute_position( &(this->super), &x, &y );
        sgui_rect_add_offset( &r1, x, y );
        sgui_rect_extend( &r1, 1, 1 );
        r1.top -= offset;
        r1.bottom -= offset;
        sgui_canvas_add_dirty_rect( super->canvas, &r1 );

        /* get selected icons */
        for( i=0; i<this->num_icons; ++i )
        {
            hit=sgui_rect_get_intersection(0,&r,&(this->icons[i].icon_area))||
                sgui_rect_get_intersection(0,&r,&(this->icons[i].text_area));

            if( hit != this->icons[i].selected )
            {
                this->icons[i].selected = hit;
                get_icon_bounding_box( this, this->icons+i, &r );
                r.top -= offset;
                r.bottom -= offset;
                sgui_canvas_add_dirty_rect( super->canvas, &r );
            }
        }
    }
    else if( e->type==SGUI_MOUSE_MOVE_EVENT && (this->flags & IV_DRAG) )
    {
        offset = sgui_scroll_bar_get_offset( this->v_bar );

        x = e->arg.i2.x;
        y = e->arg.i2.y + offset;

        /* get mouse movement difference vector */
        dx = x - this->selection.left;
        dy = y - this->selection.top;
        sgui_rect_set_size( &this->selection, x, y, 0, 0 );

        x = this->v_bar_dist;
        y = SGUI_RECT_HEIGHT(super->area) - this->offset;

        /* clamp movement vector to keep icons inside area */
        for( i=0; i<this->num_icons; ++i )
        {
            if( !this->icons[i].selected )
                continue;

            r = this->icons[i].icon_area;
            sgui_rect_join( &r, &this->icons[i].text_area, 0 );

            if( (r.left   + dx)<this->offset ) dx = this->offset - r.left;
            if( (r.top    + dy)<this->offset ) dy = this->offset - r.top;
            if( (r.right  + dx)>x            ) dx = x - r.right;
        }

        /* move all selected icons by difference vector */
        r1.left = r1.top = r1.right = r1.bottom = 0;

        for( i=0; i<this->num_icons; ++i )
        {
            if( this->icons[i].selected )
            {
                /* accumulate dirty area */
                sgui_rect_join( &r, &(this->icons[i].text_area), 0 );
                sgui_rect_join( &r, &(this->icons[i].icon_area), 0 );

                if( r1.left==r1.right )
                    r1 = r;
                else
                    sgui_rect_join( &r1, &r, 0 );

                sgui_rect_add_offset( &r, dx, dy );
                sgui_rect_join( &r1, &r, 0 );

                /* move */
                sgui_rect_add_offset( &this->icons[i].icon_area, dx, dy );
                sgui_rect_add_offset( &this->icons[i].text_area, dx, dy );
            }
        }

        if( r1.bottom!=r1.top )
        {
            sgui_widget_get_absolute_position( super, &x, &y );
            sgui_rect_add_offset( &r1, x, y-offset );
            sgui_canvas_add_dirty_rect( super->canvas, &r1 );
        }

        update_scroll_area( this );
    }
    else if( e->type==SGUI_KEY_PRESSED_EVENT )
    {
        if( e->arg.i==SGUI_KC_CONTROL || e->arg.i==SGUI_KC_LCONTROL ||
            e->arg.i==SGUI_KC_RCONTROL )
        {
            this->flags |= IV_MULTISELECT;
        }

        if( e->arg.i==SGUI_KC_SELECT_ALL )
            set_all_icons( this, 1 );
    }
    else if( e->type==SGUI_KEY_RELEASED_EVENT )
    {
        offset = sgui_scroll_bar_get_offset( this->v_bar );

        switch( e->arg.i )
        {
        case SGUI_KC_CONTROL:
        case SGUI_KC_LCONTROL:
        case SGUI_KC_RCONTROL:
            this->flags &= ~IV_MULTISELECT;
            break;
        case SGUI_KC_HOME:
            this->flags &= ~IV_MULTISELECT;
            set_all_icons( this, 0 );
            this->icons[0].selected = 1;
            break;
        case SGUI_KC_END:
            this->flags &= ~IV_MULTISELECT;
            set_all_icons( this, 0 );
            this->icons[this->num_icons-1].selected = 1;
            break;
        case SGUI_KC_LEFT:
        case SGUI_KC_UP:
            this->flags &= ~IV_MULTISELECT;
            if( this->icons[0].selected )
            {
                for( i=1; i<this->num_icons; ++i )
                {
                    if( this->icons[i].selected ) {SELECT(this->icons[i],0);}
                }
            }
            else
            {
                for( new=NULL, i=0; i<this->num_icons; ++i )
                {
                    if(!new&&(i+1)<this->num_icons&&this->icons[i+1].selected)
                        new = this->icons + i;
                    if( this->icons[i].selected ) {SELECT(this->icons[i],0);}
                }
                if( new ) { SELECT( *new, 1 ); }
            }
            break;
        case SGUI_KC_RIGHT:
        case SGUI_KC_DOWN:
            this->flags &= ~IV_MULTISELECT;
            for( new=NULL, i=0; i<this->num_icons; ++i )
            {
                if( this->icons[i].selected && (i+1)<this->num_icons )
                {
                    SELECT(this->icons[i], 0);
                    new = this->icons + i + 1;
                }
            }
            if( new ) { SELECT( *new, 1 ); }
            break;
        case SGUI_KC_RETURN:
        case SGUI_KC_SPACE:
            generate_event_for_each_selected(this, SGUI_ICON_SELECTED_EVENT);
            break;
        case SGUI_KC_COPY:
            generate_event_for_each_selected( this, SGUI_ICON_COPY_EVENT );
            break;
        case SGUI_KC_PASTE:
            ev.src.other = this;
            ev.type = SGUI_ICON_PASTE_EVENT;
            sgui_event_post( &ev );
            break;
        case SGUI_KC_CUT:
            generate_event_for_each_selected( this, SGUI_ICON_CUT_EVENT );
            break;
        case SGUI_KC_DELETE:
            generate_event_for_each_selected( this, SGUI_ICON_DELETE_EVENT );
            break;
        }
    }
    else if( (e->type==SGUI_MOUSE_RELEASE_EVENT &&
              e->arg.i3.z==SGUI_MOUSE_BUTTON_LEFT) ||
             e->type==SGUI_MOUSE_LEAVE_EVENT )
    {
        if( this->flags & IV_DRAG )
        {
            sgui_widget_get_absolute_rect( super, &r );
            sgui_canvas_add_dirty_rect( super->canvas, &r );
        }
        else if( this->flags & IV_SELECTBOX )
        {
            r = this->selection;
            sgui_rect_repair( &r );
            dy = sgui_scroll_bar_get_offset( this->v_bar );
            r.top -= dy;
            r.bottom -= dy;
            sgui_widget_get_absolute_position( &(this->super), &x, &y );
            sgui_rect_add_offset( &r, x, y );
            sgui_rect_extend( &r, 1, 1 );
            sgui_canvas_add_dirty_rect( super->canvas, &r );
        }

        this->flags &= ~(IV_DRAG|IV_SELECTBOX);
    }
    else if( e->type==SGUI_FOCUS_LOSE_EVENT )
    {
        set_all_icons( this, 0 );
        this->flags &= ~(IV_MULTISELECT|IV_DRAG|IV_SELECTBOX);
    }

    sgui_internal_unlock_mutex( );
}

static void icon_view_draw( sgui_widget* super )
{
    icon_view* this = (icon_view*)super;
    sgui_skin* skin = sgui_skin_get( );
    unsigned int i, offset;
    sgui_rect r, sc;

    offset = sgui_scroll_bar_get_offset( this->v_bar );

    if( this->flags & IV_DRAW_BG )
        skin->draw_frame( skin, super->canvas, &(super->area) );

    if( this->flags & IV_SELECTBOX )
    {
        r = this->selection;
        sgui_rect_repair( &r );
        r.top -= offset;
        r.bottom -= offset;
        sgui_rect_add_offset( &r, super->area.left, super->area.top );
        skin->draw_focus_box( skin, super->canvas, &r );
    }

    /* draw selected icons on top of non-selected */
    sgui_widget_get_absolute_rect( super, &r );
    sgui_rect_extend( &r, -this->offset, -this->offset );
    sgui_canvas_get_scissor_rect( super->canvas, &sc );
    sgui_canvas_set_scissor_rect( super->canvas, &r );

    for( i=0; i<this->num_icons; ++i )
    {
        if( !this->icons[i].selected )
            draw_icon( this, this->icons+i, skin, offset );
    }

    for( i=0; i<this->num_icons; ++i )
    {
        if( this->icons[i].selected )
            draw_icon( this, this->icons + i, skin, offset );
    }

    sgui_canvas_set_scissor_rect( super->canvas, &sc );
}

static void icon_view_destroy( sgui_widget* super )
{
    icon_view* this = (icon_view*)super;
    sgui_widget* i;

    /* recursive destroy might have already destroyed the scrollbar */
    if( super->children )
    {
        for( i=super->children; i!=NULL; i=i->next )
        {
            if( i->next == this->v_bar )
            {
                i->next = i->next->next;
                sgui_widget_destroy( this->v_bar );
                break;
            }
        }
    }

    sgui_model_free_item_list( this->model, this->itemlist );
    free( this->icons );
    free( this );
}

/***************************************************************************/

sgui_widget* sgui_icon_view_create( int x, int y, unsigned width,
                                    unsigned int height,
                                    sgui_model* model,
                                    int background,
                                    unsigned int icon_col,
                                    unsigned int txt_col )
{
    sgui_skin* skin = sgui_skin_get( );
    sgui_widget* super;
    icon_view* this;
    unsigned int w;
    sgui_rect r;

    if( !model )
        return NULL;

    this = malloc( sizeof(icon_view) );
    super = (sgui_widget*)this;

    if( !this )
        return NULL;

    memset( this, 0, sizeof(icon_view) );
    sgui_widget_init( super, x, y, width, height );

    this->model    = model;
    this->flags    = background ? IV_DRAW_BG : 0;
    this->offset   = background ? skin->get_frame_border_width(skin):0;
    this->icon_col = icon_col;
    this->txt_col  = txt_col;




    skin->get_scroll_bar_button_extents( skin, &r );
    w = SGUI_RECT_WIDTH( r );

    this->v_bar_dist = width - w - this->offset;
    this->v_bar = sgui_scroll_bar_create( this->v_bar_dist, this->offset, 0,
                                          height-2*this->offset,
                                          height-2*this->offset,
                                          height-2*this->offset );

    if( !this->v_bar )
    {
        free( this );
        return NULL;
    }

    sgui_scroll_bar_on_scroll( this->v_bar, view_on_scroll_v, this );
    sgui_widget_set_visible( this->v_bar, 0 );
    sgui_widget_add_child( super, this->v_bar );




    super->window_event = icon_view_on_event;
    super->draw         = icon_view_draw;
    super->destroy      = icon_view_destroy;
    super->flags        = SGUI_FOCUS_ACCEPT|SGUI_FOCUS_DROP_ESC|
                          SGUI_FOCUS_DROP_TAB|SGUI_WIDGET_VISIBLE;
    return super;
}

void sgui_icon_view_populate( sgui_widget* super, sgui_item* root )
{
    icon_view* this = (icon_view*)super;
    const sgui_item* i;
    const char* subtext;
    const sgui_icon* ic;
    unsigned int j;

    sgui_internal_lock_mutex( );
    sgui_model_free_item_list( this->model, this->itemlist );
    this->itemlist = sgui_model_query_items( this->model, root, 0, 0 );

    if( !this->itemlist )
        goto fail;

    this->num_icons = sgui_model_item_children_count( this->model, root );
    this->icons = realloc( this->icons, this->num_icons*sizeof(icon) );

    if( !this->icons )
        goto fail;

    memset( this->icons, 0, this->num_icons*sizeof(icon) );

    for( i=this->itemlist, j=0; i && j<this->num_icons; i=i->next, ++j )
    {
        ic      = sgui_item_icon( this->model, i, this->icon_col );
        subtext = sgui_item_text( this->model, i, this->txt_col );

        sgui_icon_get_area( ic, &this->icons[j].icon_area );
        sgui_skin_get_text_extents( subtext, &this->icons[j].text_area );

        this->icons[j].item = i;
    }

    gridify( this );
    update_scroll_area( this );
    sgui_internal_unlock_mutex( );
    return;
fail:
    free( this->icons );
    sgui_model_free_item_list( this->model, this->itemlist );
    this->itemlist = NULL;
    this->num_icons = 0;
    sgui_internal_unlock_mutex( );
}

void sgui_icon_view_snap_to_grid( sgui_widget* super )
{
    unsigned int grid_w, grid_h, txt_w, txt_h, img_w, img_h;
    icon_view* this = (icon_view*)super;
    int x, y, dx, dy;
    unsigned int i;
    sgui_rect r;

    sgui_internal_lock_mutex( );
    ideal_grid_size( this, &grid_w, &grid_h );

    for( i=0; i<this->num_icons; ++i )
    {
        x = this->icons[i].icon_area.left;
        y = this->icons[i].icon_area.top;
        if( (x % grid_w) > grid_w/2 ) x += grid_w;
        if( (y % grid_h) > grid_h/2 ) y += grid_h;
        x -= (x % grid_w) - this->offset;
        y -= (y % grid_h) - this->offset;

        txt_w = SGUI_RECT_WIDTH(this->icons[i].text_area);
        txt_h = SGUI_RECT_HEIGHT(this->icons[i].text_area);
        img_w = SGUI_RECT_WIDTH(this->icons[i].icon_area);
        img_h = SGUI_RECT_HEIGHT(this->icons[i].icon_area);

        dx = img_w < grid_w ? (grid_w - img_w)/2 : 0;
        dy = (img_h + txt_h) < grid_h ? (grid_h - img_h - txt_h)/2 : 0;
        sgui_rect_set_size( &this->icons[i].icon_area,x+dx,y+dy,img_w,img_h );

        dx = txt_w < grid_w ? (grid_w - txt_w)/2 : 0;
        sgui_rect_set_size( &this->icons[i].text_area, x + dx,
                            this->icons[i].icon_area.bottom, txt_w, txt_h );
    }

    update_scroll_area( this );
    if( super->canvas )
    {
        sgui_widget_get_absolute_rect( super, &r );
        sgui_canvas_add_dirty_rect( super->canvas, &r );
    }
    sgui_internal_unlock_mutex( );
}

void sgui_icon_view_sort( sgui_widget* super, sgui_item_compare_fun fun )
{
    icon_view* this = (icon_view*)super;
    unsigned int k, n;
    icon temp, *last;

    if( !this->icons )
        return;

    sgui_internal_lock_mutex( );
    for( k=this->num_icons/2; k>=1; --k )
        sink( this, fun, this->icons, k, this->num_icons );

    for( n=this->num_icons, last=this->icons+n-1; n>1; --last )
    {
        temp = this->icons[0]; this->icons[0] = *last; *last = temp;
        sink( this, fun, this->icons, 1, --n );
    }

    gridify( this );
    update_scroll_area( this );
    sgui_internal_unlock_mutex( );
}
#elif defined(SGUI_NOP_IMPLEMENTATIONS)
sgui_widget* sgui_icon_view_create( int x, int y, unsigned width,
                                    unsigned int height, sgui_model* model,
                                    int bg, unsigned int icon_col,
                                    unsigned int txt_col )
{
    (void)x; (void)y; (void)width; (void)height; (void)model; (void)bg;
    (void)icon_col; (void)txt_col;
    return NULL;
}
void sgui_icon_view_sort( sgui_widget* super, sgui_item_compare_fun fun )
{
    (void)super; (void)fun;
}
void sgui_icon_view_populate( sgui_widget* view, sgui_item* root )
{
    (void)view; (void)root;
}
void sgui_icon_view_snap_to_grid( sgui_widget* super )
{
    (void)super;
}
#endif /* !defined(SGUI_NO_ICON_CACHE) && !defined(SGUI_NO_ICON_VIEW) */

