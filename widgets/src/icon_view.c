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
#include "sgui_skin.h"

#include <stdlib.h>
#include <string.h>

#ifdef MACHINE_OS_WINDOWS
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
    unsigned int id;        /* id of the icon */
    sgui_rect icon_area;    /* area inside the view */
    sgui_rect text_area;    /* area of the subtext inside the view */
    char* subtext;          /* the text to write underneath the icon */
    int selected;           /* non-zero if the icon is selected */
    void* user;             /* user data pointer */
    struct icon* next;      /* linked list pointer */
}
icon;

typedef struct
{
    sgui_widget super;
    icon* icons;            /* a linked list of icons */
    sgui_icon_cache* cache; /* the underlying icon cache */
    int draw_background;    /* whether to draw the background */
    int grab_x, grab_y;     /* the last mouse grabbing position */
    int endx, endy;
    int flags;
    int offset;             /* the offset from the border of the view */

    icon* grabed;           /* the last icon that was clicked */
    long grabtime;          /* when the last icon was clicked */
}
icon_view;



#define DOUBLE_CLICK_MS 750



static long get_time_ms( void )
{
#ifdef MACHINE_OS_WINDOWS
    return GetTickCount( );
#else
    struct timeval tp;

    gettimeofday( &tp, NULL );

    return tp.tv_sec*1000 + tp.tv_usec/1000;
#endif
}

static void get_icon_bounding_box( icon_view* this, icon* i, sgui_rect* r )
{
    int x, y;

    r->top    = i->icon_area.top;
    r->left   = MIN(i->icon_area.left,   i->text_area.left);
    r->right  = MAX(i->icon_area.right,  i->text_area.right);
    r->bottom = MAX(i->icon_area.bottom, i->text_area.bottom);

    sgui_widget_get_absolute_position( &(this->super), &x, &y );
    r->left   += x;
    r->right  += x;
    r->top    += y;
    r->bottom += y;
}

static void deselect_all_icons( icon_view* this )
{
    sgui_rect r;
    icon* i;

    for( i=this->icons; i!=NULL; i=i->next )
    {
        if( i->selected )
        {
            i->selected = 0;
            get_icon_bounding_box( this, i, &r );
            sgui_canvas_add_dirty_rect( this->super.canvas, &r );
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
            ev.widget = (sgui_widget*)i->user;
            ev.window = NULL;
            ev.type = type;
            sgui_event_post( &ev );
        }
    }
}

static void icon_view_on_event( sgui_widget* super, const sgui_event* e )
{
    icon_view* this = (icon_view*)super;
    int x, y, dx, dy;
    sgui_event ev;
    sgui_rect r;
    icon* new;
    icon* i;

#define SELECT( icon, state )\
        (icon)->selected = (state);\
        get_icon_bounding_box( this, (icon), &r );\
        sgui_canvas_add_dirty_rect( super->canvas, &r )

#define ADD_OFFSET( r, x, y )\
        (r).left+=(x); (r).top+=(y); (r).right+=(x); (r).bottom+=(y)

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
                ev.widget = (sgui_widget*)this->grabed->user;
                ev.window = NULL;
                ev.type = SGUI_ICON_SELECTED;
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
        r.left   = MIN(this->grab_x, this->endx);
        r.top    = MIN(this->grab_y, this->endy);
        r.right  = MAX(this->grab_x, this->endx);
        r.bottom = MAX(this->grab_y, this->endy);

        dx = e->arg.i2.x - this->endx;
        dy = e->arg.i2.y - this->endy;

             if( r.left  ==this->endx && dx<0 ) r.left   += dx;
        else if( r.right ==this->endx && dx>0 ) r.right  += dx;
             if( r.top   ==this->endy && dy<0 ) r.top    += dy;
        else if( r.bottom==this->endy && dy>0 ) r.bottom += dy;

        this->endx = e->arg.i2.x;
        this->endy = e->arg.i2.y;

        /* flag area as dity */
        sgui_widget_get_absolute_position( &(this->super), &x, &y );
        r.left   += x-1;
        r.top    += y-1;
        r.right  += x+1;
        r.bottom += y+1;
        sgui_canvas_add_dirty_rect( super->canvas, &r );

        /* get selected icons */
        r.left   = MIN(this->grab_x, this->endx);
        r.top    = MIN(this->grab_y, this->endy);
        r.right  = MAX(this->grab_x, this->endx);
        r.bottom = MAX(this->grab_y, this->endy);

        for( i=this->icons; i!=NULL; i=i->next )
        {
            if( sgui_rect_get_intersection( NULL, &r, &(i->icon_area) ) ||
                sgui_rect_get_intersection( NULL, &r, &(i->text_area) ) )
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

            if( (i->icon_area.left + dx)<this->offset )
                dx = this->offset - i->icon_area.left;

            if( (i->icon_area.top + dy)<this->offset )
                dy = this->offset - i->icon_area.top;

            if( (i->text_area.left + dx)<this->offset )
                dx = this->offset - i->text_area.left;

            if( (i->text_area.top + dy)<this->offset )
                dy = this->offset - i->text_area.top;

            if( (i->icon_area.right  + dx)>x ) dx = x - i->icon_area.right;
            if( (i->icon_area.bottom + dy)>y ) dy = y - i->icon_area.bottom;
            if( (i->text_area.right  + dx)>x ) dx = x - i->text_area.right;
            if( (i->text_area.bottom + dy)>y ) dy = y - i->text_area.bottom;
        }

        /* move all selected icons by difference vector */
        for( i=this->icons; i!=NULL; i=i->next )
        {
            if( i->selected )
            {
                /* flag affected area as dirty */
                get_icon_bounding_box( this, i, &r );

                if( dx>0 ) r.right += dx;
                else       r.left  += dx;

                if( dy>0 ) r.bottom += dy;
                else       r.top    += dy;

                sgui_canvas_add_dirty_rect( super->canvas, &r );

                /* move */
                ADD_OFFSET( i->icon_area, dx, dy );
                ADD_OFFSET( i->text_area, dx, dy );
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
        case SGUI_KC_RIGHT:
        case SGUI_KC_DOWN:
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
        case SGUI_KC_LEFT:
        case SGUI_KC_UP:
            this->flags &= ~IV_MULTISELECT;
            for( new=NULL, i=this->icons; i!=NULL; i=i->next )
            {
                if( i->selected && i->next ) { SELECT(i, 0); new = i->next; }
            }
            if( new ) { SELECT( new, 1 ); }
            break;
        case SGUI_KC_RETURN:
        case SGUI_KC_SPACE:
            generate_event_for_each_selected( this, SGUI_ICON_SELECTED );
            break;
        case SGUI_KC_COPY:
            generate_event_for_each_selected( this, SGUI_ICON_COPY );
            break;
        case SGUI_KC_PASTE:
            ev.widget = (sgui_widget*)this;
            ev.window = NULL;
            ev.type = SGUI_ICON_PASTE;
            sgui_event_post( &ev );
            break;
        case SGUI_KC_CUT:
            generate_event_for_each_selected( this, SGUI_ICON_CUT );
            break;
        case SGUI_KC_DELETE:
            generate_event_for_each_selected( this, SGUI_ICON_DELETE );
            break;
        }
    }
    else if( (e->type==SGUI_MOUSE_RELEASE_EVENT &&
              e->arg.i3.z==SGUI_MOUSE_BUTTON_LEFT) ||
             e->type==SGUI_MOUSE_LEAVE_EVENT )
    {
        if( this->flags & IV_SELECTBOX )
        {
            sgui_widget_get_absolute_position( &(this->super), &x, &y );
            r.left   = MIN(this->grab_x, this->endx) + x - 1;
            r.top    = MIN(this->grab_y, this->endy) + y - 1;
            r.right  = MAX(this->grab_x, this->endx) + x + 1;
            r.bottom = MAX(this->grab_y, this->endy) + y + 1;
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
#undef ADD_OFFSET
#undef SELECT
}

static void icon_view_draw( sgui_widget* super )
{
    icon_view* this = (icon_view*)super;
    sgui_rect r;
    icon* i;

    if( this->draw_background )
    {
        sgui_skin_draw_frame( super->canvas, &(super->area) );
    }

    if( this->flags & IV_SELECTBOX )
    {
        r.left   = MIN(this->grab_x, this->endx) + super->area.left;
        r.top    = MIN(this->grab_y, this->endy) + super->area.top;
        r.right  = MAX(this->grab_x, this->endx) + super->area.left;
        r.bottom = MAX(this->grab_y, this->endy) + super->area.top;
        sgui_skin_draw_focus_box( super->canvas, &r );
    }

    /* draw non selected icons */
    for( i=this->icons; i!=NULL; i=i->next )
    {
        if( !i->selected )
        {
            sgui_icon_cache_draw_icon( this->cache, i->id,
                                       super->area.left + i->icon_area.left,
                                       super->area.top  + i->icon_area.top );

            sgui_canvas_draw_text( super->canvas,
                                   super->area.left + i->text_area.left,
                                   super->area.top  + i->text_area.top,
                                   i->subtext );
        }
    }

    /* draw selected icons on top */
    for( i=this->icons; i!=NULL; i=i->next )
    {
        if( i->selected )
        {
            sgui_icon_cache_draw_icon( this->cache, i->id,
                                       super->area.left + i->icon_area.left,
                                       super->area.top  + i->icon_area.top );

            r = i->subtext ? i->text_area : i->icon_area;
            r.left += super->area.left;
            r.right += super->area.left;
            r.top += super->area.top;
            r.bottom += super->area.top;
            sgui_skin_draw_focus_box( super->canvas, &r );

            sgui_canvas_draw_text( super->canvas,
                                   super->area.left + i->text_area.left,
                                   super->area.top  + i->text_area.top,
                                   i->subtext );
        }
    }
}

static void icon_view_destroy( sgui_widget* super )
{
    icon_view* this = (icon_view*)super;
    icon* old;
    icon* i;

    sgui_internal_lock_mutex( );

    for( i=this->icons; i!=NULL; )
    {
        old = i;
        i = i->next;
        free( old->subtext );
        free( old );
    }

    sgui_internal_unlock_mutex( );

    free( this );
}

/***************************************************************************/

sgui_widget* sgui_icon_view_create( int x, int y, unsigned width,
                                    unsigned int height,
                                    sgui_icon_cache* cache,
                                    int background )
{
    sgui_widget* super;
    icon_view* this;

    this = malloc( sizeof(icon_view) );
    super = (sgui_widget*)this;

    if( this )
    {
        memset( this, 0, sizeof(icon_view) );

        sgui_internal_widget_init( super, x, y, width, height );

        this->cache = cache;
        this->draw_background = background;
        this->offset = background ? sgui_skin_get_frame_border_width( ) : 0;

        super->window_event_callback = icon_view_on_event;
        super->draw_callback = icon_view_draw;
        super->destroy = icon_view_destroy;

        super->focus_policy = SGUI_FOCUS_ACCEPT|
                              SGUI_FOCUS_DROP_ESC|
                              SGUI_FOCUS_DROP_TAB;
    }

    return super;
}

void sgui_icon_view_add_icon( sgui_widget* super, int x, int y,
                              const char* subtext, unsigned int id,
                              void* user )
{
    unsigned int txt_w, txt_h, img_w, img_h;
    icon_view* this = (icon_view*)super;
    sgui_rect r;
    icon* i;

    /* sanity check */
    if( !this )
        return;

    /* obtain icon size */
    if( !sgui_icon_cache_get_icon_area( this->cache, id, &r ) )
        return;

    img_w = SGUI_RECT_WIDTH(r);
    img_h = SGUI_RECT_HEIGHT(r);

    /* try to allocate icon object */
    i = malloc( sizeof(icon) );

    if( !i )
        return;

    if( subtext )
    {
        /* try to allocate text buffer */
        i->subtext = malloc( strlen(subtext)+1 );

        if( !i->subtext )
        {
            free( i );
            return;
        }

        /* copy text and get text size */
        strcpy( i->subtext, subtext );
        sgui_skin_get_text_extents( subtext, &r );

        txt_w = SGUI_RECT_WIDTH(r);
        txt_h = SGUI_RECT_HEIGHT(r);
    }
    else
    {
        i->subtext = NULL;
        txt_w = txt_h = 0;
    }

    /* initialize icon & text area */
    if( img_w>=txt_w )
    {
        sgui_rect_set_size( &(i->icon_area), x, y, img_w, img_h );
        sgui_rect_set_size( &(i->text_area),
                            x+((img_w-txt_w)/2),
                            i->icon_area.bottom, txt_w, txt_h );
    }
    else
    {
        sgui_rect_set_size( &(i->icon_area), (x + (txt_w-img_w)/2), y,
                                             img_w, img_h );

        sgui_rect_set_size( &(i->text_area), x, i->icon_area.bottom,
                                             txt_w, txt_h );
    }

    i->selected = 0;
    i->user = user;
    i->id = id;

    sgui_internal_lock_mutex( );
    i->next = this->icons;
    this->icons = i;
    sgui_internal_unlock_mutex( );
}

void sgui_icon_view_snap_to_grid( sgui_widget* super )
{
    unsigned int w, h, grid_w = 0, grid_h = 0;
    icon_view* this = (icon_view*)super;
    sgui_rect r;
    int dx, dy;
    icon* i;

    if( !this )
        return;

    sgui_internal_lock_mutex( );

    /* determine grid size */
    for( i=this->icons; i!=NULL; i=i->next )
    {
        r.top    = i->icon_area.top;
        r.left   = MIN(i->icon_area.left,   i->text_area.left);
        r.right  = MAX(i->icon_area.right,  i->text_area.right);
        r.bottom = MAX(i->icon_area.bottom, i->text_area.bottom);

        w = SGUI_RECT_WIDTH( r );
        h = SGUI_RECT_HEIGHT( r );
        grid_w = MAX(grid_w, w);
        grid_h = MAX(grid_h, h);
    }

    /* arange icons */
    for( i=this->icons; i!=NULL; i=i->next )
    {
        r.top    = i->icon_area.top;
        r.left   = MIN(i->icon_area.left,   i->text_area.left);
        r.right  = MAX(i->icon_area.right,  i->text_area.right);
        r.bottom = MAX(i->icon_area.bottom, i->text_area.bottom);

        dx = r.left % grid_w;
        dy = r.top  % grid_h;

        if( (r.left-dx) <= 2*this->offset ) dx = r.left-2*this->offset;
        if( (r.top -dy) <= 2*this->offset ) dy = r.top -2*this->offset;

        if( dx > (int)grid_w/2 ) dx = -(grid_w - dx);
        if( dy > (int)grid_h/2 ) dy = -(grid_h - dy);

        i->icon_area.left   -= dx;
        i->icon_area.right  -= dx;
        i->text_area.left   -= dx;
        i->text_area.right  -= dx;

        i->icon_area.top    -= dy;
        i->icon_area.bottom -= dy;
        i->text_area.top    -= dy;
        i->text_area.bottom -= dy;
    }

    sgui_internal_unlock_mutex( );

    sgui_widget_get_absolute_rect( super, &r );
    sgui_canvas_add_dirty_rect( super->canvas, &r );
}

void sgui_icon_view_sort( sgui_widget* super, sgui_icon_compare_fun fun )
{
    unsigned int x, y, w, h, grid_w = 0, grid_h = 0;
    icon_view* this = (icon_view*)super;
    icon* new = NULL;
    sgui_rect r;
    int da, db;
    icon* i;
    icon* j;

    sgui_internal_lock_mutex( );

    /* insertion sort the icon list */
    while( this->icons )
    {
        /* remove the head of the list */
        i = this->icons;
        this->icons = this->icons->next;

        /* update grid size */
        r.top    = i->icon_area.top;
        r.left   = MIN(i->icon_area.left,   i->text_area.left);
        r.right  = MAX(i->icon_area.right,  i->text_area.right);
        r.bottom = MAX(i->icon_area.bottom, i->text_area.bottom);

        w = SGUI_RECT_WIDTH( r );
        h = SGUI_RECT_HEIGHT( r );
        grid_w = MAX(grid_w, w);
        grid_h = MAX(grid_h, h);

        /* insert it into the new list */
        if( !new || (!fun && strcmp( i->subtext, new->subtext )<0) ||
            (fun && fun( i->user, new->user )<0) )
        {
            SGUI_ADD_TO_LIST( new, i );
        }
        else
        {
            for( j=new; j!=NULL; j=j->next )
            {
                if( !j->next || (fun && fun(i->user,j->next->user)<0) ||
                    (!fun && strcmp(i->subtext,j->next->subtext)<0) )
                {
                    SGUI_ADD_TO_LIST( j->next, i );
                    break;
                }
            }
        }
    }

    this->icons = new;

    /* recalculate icon positions */
    x = 2*this->offset;
    y = 2*this->offset;

    for( i=this->icons; i!=NULL; i=i->next )
    {
        if( (int)(x+grid_w)>=(SGUI_RECT_WIDTH(super->area)-2*this->offset) )
        {
            x  = 2*this->offset;
            y += grid_h;
        }

        da = i->text_area.top    - i->icon_area.top;
        db = i->text_area.bottom - i->icon_area.top;
        i->icon_area.bottom = y + i->icon_area.bottom - i->icon_area.top;
        i->icon_area.top    = y;
        i->text_area.top    = i->icon_area.top + da;
        i->text_area.bottom = i->icon_area.top + db;

        if( i->icon_area.left < i->text_area.left )
        {
            da = i->text_area.left  - i->icon_area.left;
            db = i->text_area.right - i->icon_area.left;
            i->icon_area.right = x + i->icon_area.right - i->icon_area.left;
            i->icon_area.left  = x;
            i->text_area.left  = i->icon_area.left + da;
            i->text_area.right = i->icon_area.left + db;
        }
        else
        {
            da = i->icon_area.left  - i->text_area.left;
            db = i->icon_area.right - i->text_area.left;
            i->text_area.right = x + i->text_area.right - i->text_area.left;
            i->text_area.left  = x;
            i->icon_area.left  = i->text_area.left + da;
            i->icon_area.right = i->text_area.left + db;
        }

        x += grid_w;
    }

    sgui_internal_unlock_mutex( );

    sgui_widget_get_absolute_rect( super, &r );
    sgui_canvas_add_dirty_rect( super->canvas, &r );
}

