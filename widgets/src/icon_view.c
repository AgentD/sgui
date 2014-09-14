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
#include "sgui_icon_cache.h"
#include "sgui_icon_view.h"
#include "sgui_internal.h"
#include "sgui_widget.h"
#include "sgui_event.h"
#include "sgui_model.h"
#include "sgui_skin.h"

#include <stdlib.h>
#include <string.h>

#if !defined(SGUI_NO_ICON_CACHE) && !defined(SGUI_NO_ICON_VIEW)
#ifdef SGUI_WINDOWS
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
#else
    #include <sys/time.h>
#endif



#define IV_MULTISELECT 0x01
#define IV_DRAG        0x02
#define IV_SELECTBOX   0x04



typedef struct icon
{
    sgui_item* item;        /* underlying model item */
    sgui_rect icon_area;    /* area of the icon inside the view */
    sgui_rect text_area;    /* area of the subtext inside the view */
    int selected;           /* non-zero if the icon is selected */
    struct icon* next;      /* linked list pointer */
}
icon;

typedef struct
{
    sgui_widget super;
    sgui_model* model;      /* underlying model */
    icon* icons;            /* a linked list of icons */
    int draw_background;    /* whether to draw the background */
    int grab_x, grab_y;     /* the last mouse grabbing position */
    int endx, endy;
    int flags;
    int offset;             /* the offset from the border of the view */

    icon* grabed;           /* the last icon that was clicked */
    long grabtime;          /* when the last icon was clicked */

    const sgui_item* itemlist;
}
icon_view;



#define DOUBLE_CLICK_MS 750

#define SELECT( icon, state )\
        (icon)->selected = (state);\
        get_icon_bounding_box( this, (icon), &r );\
        sgui_canvas_add_dirty_rect( this->super.canvas, &r )



static long get_time_ms( void )
{
#ifdef SGUI_WINDOWS
    return GetTickCount( );
#else
    struct timeval tp;

    gettimeofday( &tp, NULL );

    return tp.tv_sec*1000 + tp.tv_usec/1000;
#endif
}

static int compare( icon_view* this, sgui_item_compare_fun fun,
                    icon* a, icon* b )
{
    if( fun )
        return fun( a->item, b->item );

    return strcmp( sgui_item_text( this->model, a->item, 0 ),
                   sgui_item_text( this->model, b->item, 0 ) );
}

static void draw_icon( icon_view* this, icon* i, sgui_skin* skin )
{
    sgui_rect r;

    sgui_icon_cache_draw_icon( sgui_model_get_icon_cache(this->model),
                               sgui_item_icon(this->model,i->item,0),
                               this->super.area.left + i->icon_area.left,
                               this->super.area.top  + i->icon_area.top );

    if( i->selected )
    {
        r = sgui_item_text(this->model,i->item,0) ?
            i->text_area : i->icon_area;

        sgui_rect_add_offset(&r, this->super.area.left, this->super.area.top);
        skin->draw_focus_box(skin, this->super.canvas, &r);
    }

    sgui_skin_draw_text( this->super.canvas,
                         this->super.area.left + i->text_area.left,
                         this->super.area.top  + i->text_area.top,
                         sgui_item_text(this->model,i->item,0) );
}

static void ideal_grid_size( icon_view* this,
                             unsigned int* grid_w, unsigned int* grid_h )
{
    unsigned int w, h;
    icon* i;

    *grid_w = 0;
    *grid_h = 0;

    for( i=this->icons; i!=NULL; i=i->next )
    {
        w = SGUI_RECT_WIDTH( i->icon_area );
        *grid_w = MAX(*grid_w, w);
        w = SGUI_RECT_WIDTH( i->text_area );
        *grid_w = MAX(*grid_w, w);
        h = SGUI_RECT_HEIGHT(i->icon_area) + SGUI_RECT_HEIGHT(i->text_area);
        *grid_h = MAX(*grid_h, h);
    }

    *grid_w += *grid_w/5;
    *grid_h += *grid_h/10;
}

static void gridify( icon_view* this )
{
    unsigned int x, y, grid_w, grid_h, total_w, txt_w, txt_h, img_w, img_h;
    sgui_rect r;
    int dx, dy;
    icon* i;

    ideal_grid_size( this, &grid_w, &grid_h );

    x = y = this->offset;
    total_w = SGUI_RECT_WIDTH(this->super.area) - 2*this->offset;

    for( i=this->icons; i!=NULL; i=i->next )
    {
        if( (x + grid_w) >= total_w )
        {
            x  = this->offset;
            y += grid_h;
        }

        txt_w = SGUI_RECT_WIDTH(i->text_area);
        txt_h = SGUI_RECT_HEIGHT(i->text_area);
        img_w = SGUI_RECT_WIDTH(i->icon_area);
        img_h = SGUI_RECT_HEIGHT(i->icon_area);

        dx = img_w < grid_w ? (grid_w - img_w)/2 : 0;
        dy = (img_h + txt_h) < grid_h ? (grid_h - img_h - txt_h)/2 : 0;
        sgui_rect_set_size( &i->icon_area, x + dx, y + dy, img_w, img_h );

        dx = txt_w < grid_w ? (grid_w - txt_w)/2 : 0;
        sgui_rect_set_size( &i->text_area, x + dx, i->icon_area.bottom,
                                           txt_w, txt_h );

        x += grid_w;
    }

    sgui_widget_get_absolute_rect( &(this->super), &r );
    sgui_canvas_add_dirty_rect( this->super.canvas, &r );
}

static void get_icon_bounding_box( icon_view* this, icon* i, sgui_rect* r )
{
    int x, y;

    sgui_rect_copy( r, &i->text_area );
    sgui_rect_join( r, &i->icon_area, 0 );
    sgui_widget_get_absolute_position( &(this->super), &x, &y );
    sgui_rect_add_offset( r, x, y );
}

static void deselect_all_icons( icon_view* this )
{
    sgui_rect r;
    icon* i;

    for( i=this->icons; i!=NULL; i=i->next )
    {
        if( i->selected )
        {
            SELECT( i, 0 );
        }
    }
}

static icon* icon_from_point( icon_view* this, int x, int y )
{
    icon* top;
    icon* i;

    for( top=NULL, i=this->icons; i!=NULL; i=i->next )
    {
        if( i->selected )
            continue;

        if( sgui_rect_is_point_inside( &(i->icon_area), x, y ) ||
            sgui_rect_is_point_inside( &(i->text_area), x, y ) )
        {
            top = i;
        }
    }

    for( i=this->icons; i!=NULL; i=i->next )
    {
        if( !i->selected )
            continue;

        if( sgui_rect_is_point_inside( &(i->icon_area), x, y ) ||
            sgui_rect_is_point_inside( &(i->text_area), x, y ) )
        {
            top = i;
        }
    }

    return top;
}

static void generate_event_for_each_selected( icon_view* this, int type )
{
    sgui_event ev;
    icon* i;

    for( i=this->icons; i!=NULL; i=i->next )
    {
        if( i->selected )
        {
            ev.src.other = i->item;
            ev.type = type;
            sgui_event_post( &ev );
        }
    }
}

static void icon_view_on_event( sgui_widget* super, const sgui_event* e )
{
    icon_view* this = (icon_view*)super;
    int x, y, dx, dy;
    sgui_rect r, r1;
    sgui_event ev;
    icon* new;
    icon* i;

    sgui_internal_lock_mutex( );

    /*
        stop double click detection if mouse moved, key pressed/released or
        any mouse button other than the left one did something
     */
    if( e->type==SGUI_MOUSE_MOVE_EVENT || e->type==SGUI_KEY_PRESSED_EVENT ||
        e->type==SGUI_KEY_RELEASED_EVENT ||
        (e->type==SGUI_MOUSE_PRESS_EVENT &&
         e->arg.i3.z!=SGUI_MOUSE_BUTTON_LEFT)||
        (e->type==SGUI_MOUSE_RELEASE_EVENT &&
         e->arg.i3.z!=SGUI_MOUSE_BUTTON_LEFT) )
    {
        this->grabed = NULL;
    }

    if(e->type==SGUI_MOUSE_PRESS_EVENT && e->arg.i3.z==SGUI_MOUSE_BUTTON_LEFT)
    {
        this->endx = this->grab_x = e->arg.i3.x;
        this->endy = this->grab_y = e->arg.i3.y;
        new = icon_from_point( this, e->arg.i3.x, e->arg.i3.y );

        if( new )
        {
            if( !new->selected )
            {
                if( !(this->flags & IV_MULTISELECT) )
                    deselect_all_icons( this );

                SELECT( new, 1 );
            }
            else if( this->flags & IV_MULTISELECT )
            {
                SELECT( new, 0 );
            }

            this->flags |= IV_DRAG;
        }
        else if( !(this->flags & IV_MULTISELECT) )
        {
            deselect_all_icons( this );
            this->flags = IV_SELECTBOX;
        }

        /* double click detection timeout */
        if( this->grabed && (get_time_ms( )-this->grabtime)>DOUBLE_CLICK_MS )
        {
            this->grabed = NULL;
        }

        /* double click detection */
        if( !(this->flags&IV_MULTISELECT) )
        {
            if( this->grabed && this->grabed==new )
            {
                ev.src.other = this->grabed->item;
                ev.type = SGUI_ICON_SELECTED_EVENT;
                this->grabed = NULL;
                sgui_event_post( &ev );
            }
            else
            {
                this->grabed = new;
                this->grabtime = get_time_ms( );
            }
        }
    }
    else if( e->type==SGUI_MOUSE_MOVE_EVENT && (this->flags & IV_SELECTBOX) )
    {
        /* get absolute affected bounding area */
        SGUI_RECT_SET(r, this->grab_x, this->grab_y, this->endx, this->endy);
        sgui_rect_repair( &r );

        this->endx = e->arg.i2.x;
        this->endy = e->arg.i2.y;

        SGUI_RECT_SET(r1, this->grab_x, this->grab_y, this->endx, this->endy);
        sgui_rect_repair( &r1 );
        sgui_rect_join( &r, &r1, 0 );

        /* flag area as dity */
        sgui_widget_get_absolute_position( &(this->super), &x, &y );
        sgui_rect_add_offset( &r, x, y );
        sgui_rect_extend( &r, 1, 1 );
        sgui_canvas_add_dirty_rect( super->canvas, &r );

        /* get selected icons */
        for( i=this->icons; i!=NULL; i=i->next )
        {
            if( sgui_rect_get_intersection( NULL, &r1, &(i->icon_area) ) ||
                sgui_rect_get_intersection( NULL, &r1, &(i->text_area) ) )
            {
                if( !i->selected )
                {
                    SELECT( i, 1 );
                }
            }
            else if( i->selected )
            {
                SELECT( i, 0 );
            }
        }
    }
    else if( e->type==SGUI_MOUSE_MOVE_EVENT && (this->flags & IV_DRAG) )
    {
        /* get mouse movement difference vector */
        dx = e->arg.i2.x - this->grab_x;
        dy = e->arg.i2.y - this->grab_y;

        this->grab_x = e->arg.i2.x;
        this->grab_y = e->arg.i2.y;

        x = SGUI_RECT_WIDTH(super->area) - this->offset;
        y = SGUI_RECT_HEIGHT(super->area) - this->offset;

        /* clamp movement vector to keep icons inside area */
        for( i=this->icons; i!=NULL; i=i->next )
        {
            if( !i->selected )
                continue;

            r = i->icon_area;
            sgui_rect_join( &r, &i->text_area, 0 );

            if( (r.left   + dx)<this->offset ) dx = this->offset - r.left;
            if( (r.top    + dy)<this->offset ) dy = this->offset - r.top;
            if( (r.right  + dx)>x            ) dx = x - r.right;
            if( (r.bottom + dy)>y            ) dy = y - r.bottom;
        }

        /* move all selected icons by difference vector */
        for( i=this->icons; i!=NULL; i=i->next )
        {
            if( i->selected )
            {
                /* flag affected area as dirty */
                get_icon_bounding_box( this, i, &r );
                r1 = r;
                sgui_rect_add_offset( &r1, dx, dy );
                sgui_rect_join( &r, &r1, 0 );
                sgui_canvas_add_dirty_rect( super->canvas, &r );

                /* move */
                sgui_rect_add_offset( &i->icon_area, dx, dy );
                sgui_rect_add_offset( &i->text_area, dx, dy );
            }
        }
    }
    else if( e->type==SGUI_KEY_PRESSED_EVENT )
    {
        switch( e->arg.i )
        {
        case SGUI_KC_CONTROL:
        case SGUI_KC_LCONTROL:
        case SGUI_KC_RCONTROL:
            this->flags |= IV_MULTISELECT;
            break;
        case SGUI_KC_SELECT_ALL:
            for( i=this->icons; i!=NULL; i=i->next )
            {
                if( !i->selected )
                {
                    SELECT( i, 1 );
                }
            }
            break;
        }
    }
    else if( e->type==SGUI_KEY_RELEASED_EVENT )
    {
        switch( e->arg.i )
        {
        case SGUI_KC_CONTROL:
        case SGUI_KC_LCONTROL:
        case SGUI_KC_RCONTROL:
            this->flags &= ~IV_MULTISELECT;
            break;
        case SGUI_KC_HOME:
            this->flags &= ~IV_MULTISELECT;
            deselect_all_icons( this );
            if( this->icons ) { SELECT( this->icons, 1 ); }
            break;
        case SGUI_KC_END:
            this->flags &= ~IV_MULTISELECT;
            for( i=this->icons; i->next!=NULL; i=i->next )
            {
                if( i->selected ) { SELECT( i, 0 ); }
            }
            if( i && !i->selected ) { SELECT( i, 1 ); }
            break;
        case SGUI_KC_LEFT:
        case SGUI_KC_UP:
            this->flags &= ~IV_MULTISELECT;
            if( this->icons && this->icons->selected )
            {
                for( i=this->icons->next; i!=NULL; i=i->next )
                {
                    if( i->selected ) { SELECT( i, 0 ); }
                }
            }
            else
            {
                for( new=NULL, i=this->icons; i!=NULL; i=i->next )
                {
                    if( !new && i->next && i->next->selected ) { new = i; }
                    if( i->selected ) { SELECT( i, 0 ); }
                }
                if( new ) { SELECT( new, 1 ); }
            }
            break;
        case SGUI_KC_RIGHT:
        case SGUI_KC_DOWN:
            this->flags &= ~IV_MULTISELECT;
            for( new=NULL, i=this->icons; i!=NULL; i=i->next )
            {
                if( i->selected && i->next ) { SELECT(i, 0); new = i->next; }
            }
            if( new ) { SELECT( new, 1 ); }
            break;
        case SGUI_KC_RETURN:
        case SGUI_KC_SPACE:
            generate_event_for_each_selected( this, SGUI_ICON_SELECTED_EVENT );
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
        if( this->flags & IV_SELECTBOX )
        {
            SGUI_RECT_SET(r,this->grab_x,this->grab_y,this->endx,this->endy);
            sgui_rect_repair( &r );
            sgui_widget_get_absolute_position( &(this->super), &x, &y );
            sgui_rect_add_offset( &r, x, y );
            sgui_rect_extend( &r, 1, 1 );
            sgui_canvas_add_dirty_rect( super->canvas, &r );
        }

        this->flags &= ~(IV_DRAG|IV_SELECTBOX);
    }
    else if( e->type==SGUI_FOCUS_LOSE_EVENT )
    {
        deselect_all_icons( this );
        this->flags &= ~(IV_MULTISELECT|IV_DRAG|IV_SELECTBOX);
    }

    sgui_internal_unlock_mutex( );
}

static void icon_view_draw( sgui_widget* super )
{
    icon_view* this = (icon_view*)super;
    sgui_skin* skin = sgui_skin_get( );
    sgui_rect r;
    icon* i;

    if( this->draw_background )
    {
        skin->draw_frame( skin, super->canvas, &(super->area) );
    }

    if( this->flags & IV_SELECTBOX )
    {
        SGUI_RECT_SET(r, this->grab_x, this->grab_y, this->endx, this->endy);
        sgui_rect_repair( &r );
        sgui_rect_add_offset( &r, super->area.left, super->area.top );
        skin->draw_focus_box( skin, super->canvas, &r );
    }

    /* draw selected icons on top of non-selected */
    for( i=this->icons; i!=NULL; i=i->next )
    {
        if( !i->selected )
            draw_icon( this, i, skin );
    }

    for( i=this->icons; i!=NULL; i=i->next )
    {
        if( i->selected )
            draw_icon( this, i, skin );
    }
}

static void icon_view_destroy( sgui_widget* super )
{
    icon_view* this = (icon_view*)super;
    icon* old;
    icon* i;

    sgui_internal_lock_mutex( );

    sgui_model_free_item_list( this->model, this->itemlist );

    for( i=this->icons; i!=NULL; )
    {
        old = i;
        i = i->next;
        free( old );
    }

    sgui_internal_unlock_mutex( );

    free( this );
}

/***************************************************************************/

sgui_widget* sgui_icon_view_create( int x, int y, unsigned width,
                                    unsigned int height,
                                    sgui_model* model,
                                    int background )
{
    sgui_skin* skin = sgui_skin_get( );
    sgui_widget* super;
    icon_view* this;

    this = malloc( sizeof(icon_view) );
    super = (sgui_widget*)this;

    if( this )
    {
        memset( this, 0, sizeof(icon_view) );

        sgui_widget_init( super, x, y, width, height );

        this->model = model;
        this->draw_background = background;
        this->offset = background ? skin->get_frame_border_width( skin ) : 0;

        super->window_event_callback = icon_view_on_event;
        super->draw_callback = icon_view_draw;
        super->destroy = icon_view_destroy;

        super->focus_policy = SGUI_FOCUS_ACCEPT|
                              SGUI_FOCUS_DROP_ESC|
                              SGUI_FOCUS_DROP_TAB;
    }

    return super;
}

void sgui_icon_view_populate( sgui_widget* super, sgui_item* root )
{
    unsigned int txt_w, txt_h, img_w, img_h;
    icon_view* this = (icon_view*)super;
    const sgui_item *itemlist, *item;
    const char* subtext;
    const sgui_icon* ic;
    icon *recycle, *i;
    sgui_rect r;

    /* sanity check */
    if( !this )
        return;

    sgui_internal_lock_mutex( );

    sgui_model_free_item_list( this->model, this->itemlist );

    /* clear list of existing icons but keep them for reuse (recycle) */
    recycle = this->icons;
    this->icons = NULL;

    /* for each item under the selected root */
    itemlist = sgui_model_query_items( this->model, root, 0, 0 );
    this->itemlist = itemlist;

    for( item=itemlist; item; item=item->next )
    {
        /* get icon an text */
        ic = sgui_item_icon( this->model, item, 0 );
        subtext = sgui_item_text( this->model, item, 0 );

        /* determine size */
        img_w = img_h = txt_w = txt_h = 0;

        if( ic )
        {
            sgui_icon_get_area( ic, &r );
            img_w = SGUI_RECT_WIDTH(r);
            img_h = SGUI_RECT_HEIGHT(r);
        }
        if( subtext )
        {
            sgui_skin_get_text_extents( subtext, &r );
            txt_w = SGUI_RECT_WIDTH(r);
            txt_h = SGUI_RECT_HEIGHT(r);
        }

        /* get an icon */
        if( recycle )
        {
            i = recycle;
            recycle = recycle->next;
        }
        else
        {
            i = malloc( sizeof(icon) );
        }

        if( !i )
            continue;

        /* initialize icon */
        if( img_w>=txt_w )
        {
            sgui_rect_set_size( &(i->icon_area), 0, 0, img_w, img_h );
            sgui_rect_set_size( &(i->text_area),
                                (img_w-txt_w)/2, i->icon_area.bottom,
                                txt_w, txt_h );
        }
        else
        {
            sgui_rect_set_size( &(i->icon_area), (txt_w-img_w)/2, 0,
                                                 img_w, img_h );

            sgui_rect_set_size( &(i->text_area), 0, i->icon_area.bottom,
                                                 txt_w, txt_h );
        }

        i->selected = 0;
        i->item = (sgui_item*)item;
        i->next = this->icons;
        this->icons = i;
    }

    /* cleanup rest of recycle list */
    while( recycle )
    {
        i = recycle;
        recycle = recycle->next;
        free( i );
    }

    gridify( this );
    sgui_internal_unlock_mutex( );
}

void sgui_icon_view_snap_to_grid( sgui_widget* super )
{
    unsigned int grid_w, grid_h, txt_w, txt_h, img_w, img_h;
    icon_view* this = (icon_view*)super;
    int x, y, dx, dy;
    sgui_rect r;
    icon* i;

    if( !this )
        return;

    sgui_internal_lock_mutex( );
    ideal_grid_size( this, &grid_w, &grid_h );

    for( i=this->icons; i!=NULL; i=i->next )
    {
        x = i->icon_area.left;
        y = i->icon_area.top;
        if( (x % grid_w) > grid_w/2 ) x += grid_w;
        if( (y % grid_h) > grid_h/2 ) y += grid_h;
        x -= (x % grid_w) - this->offset;
        y -= (y % grid_h) - this->offset;

        txt_w = SGUI_RECT_WIDTH(i->text_area);
        txt_h = SGUI_RECT_HEIGHT(i->text_area);
        img_w = SGUI_RECT_WIDTH(i->icon_area);
        img_h = SGUI_RECT_HEIGHT(i->icon_area);

        dx = img_w < grid_w ? (grid_w - img_w)/2 : 0;
        dy = (img_h + txt_h) < grid_h ? (grid_h - img_h - txt_h)/2 : 0;
        sgui_rect_set_size( &i->icon_area, x + dx, y + dy, img_w, img_h );

        dx = txt_w < grid_w ? (grid_w - txt_w)/2 : 0;
        sgui_rect_set_size( &i->text_area, x + dx, i->icon_area.bottom,
                                           txt_w, txt_h );
    }

    sgui_widget_get_absolute_rect( super, &r );
    sgui_canvas_add_dirty_rect( super->canvas, &r );
    sgui_internal_unlock_mutex( );
}

void sgui_icon_view_sort( sgui_widget* super, sgui_item_compare_fun fun )
{
    icon_view* this = (icon_view*)super;
    icon *i, *j, *new = NULL;

    if( !this )
        return;

    sgui_internal_lock_mutex( );

    while( this->icons )
    {
        /* remove the head of the list */
        i = this->icons;
        this->icons = this->icons->next;

        /* insert into new list at right position */
        if( !new || compare( this, fun, i, new )<0 )
        {
            i->next = new;
            new = i;
        }
        else
        {
            for( j=new; j!=NULL; j=j->next )
            {
                if( !j->next || compare( this, fun, i, j->next )<0 )
                {
                    i->next = j->next;
                    j->next = i;
                    break;
                }
            }
        }
    }

    this->icons = new;

    gridify( this );
    sgui_internal_unlock_mutex( );
}
#elif defined(SGUI_NOP_IMPLEMENTATIONS)
sgui_widget* sgui_icon_view_create( int x, int y, unsigned width,
                                    unsigned int height, sgui_model* model,
                                    int bg )
{
    (void)x; (void)y; (void)width; (void)height; (void)model; (void)bg;
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

