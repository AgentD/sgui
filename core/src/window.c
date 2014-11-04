/*
 * window.c
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
#include "sgui_internal.h"
#include "sgui_window.h"
#include "sgui_event.h"
#include "sgui_canvas.h"
#include "sgui_skin.h"
#include "sgui_context.h"
#include "sgui_widget.h"

#include <stddef.h>



#define COPY_KEY SGUI_KC_C
#define SELECT_KEY SGUI_KC_A
#define PASTE_KEY SGUI_KC_V
#define CUT_KEY SGUI_KC_X



void sgui_internal_window_post_init( sgui_window* this, unsigned int width,
                                     unsigned int height, int backend )
{
    if( this )
    {
        this->w       = width;
        this->h       = height;
        this->backend = backend;
        this->modmask = 0;

        if( this->backend == SGUI_NATIVE )
        {
            sgui_canvas_begin( this->ctx.canvas, NULL );
            sgui_canvas_clear( this->ctx.canvas, NULL );
            sgui_canvas_end( this->ctx.canvas );
        }
    }
}

void sgui_internal_window_fire_event( sgui_window* this, const sgui_event* e )
{
    sgui_event ev;

    if( this )
    {
        if( e->type == SGUI_KEY_PRESSED_EVENT )
        {
            switch( e->arg.i )
            {
            case SGUI_KC_SHIFT:
            case SGUI_KC_LSHIFT:
            case SGUI_KC_RSHIFT:    this->modmask |= SGUI_MOD_SHIFT; break;
            case SGUI_KC_CONTROL:
            case SGUI_KC_LCONTROL:
            case SGUI_KC_RCONTROL:  this->modmask |= SGUI_MOD_CTRL; break;
            case SGUI_KC_ALT:
            case SGUI_KC_LALT:
            case SGUI_KC_RALT:      this->modmask |= SGUI_MOD_ALT; break;
            case SGUI_KC_LSUPER:
            case SGUI_KC_RSUPER:    this->modmask |= SGUI_MOD_SUPER; break;
            }
        }
        else if( e->type == SGUI_KEY_RELEASED_EVENT )
        {
            switch( e->arg.i )
            {
            case SGUI_KC_SHIFT:
            case SGUI_KC_LSHIFT:
            case SGUI_KC_RSHIFT:    this->modmask &= ~SGUI_MOD_SHIFT; break;
            case SGUI_KC_CONTROL:
            case SGUI_KC_LCONTROL:
            case SGUI_KC_RCONTROL:  this->modmask &= ~SGUI_MOD_CTRL; break;
            case SGUI_KC_ALT:
            case SGUI_KC_LALT:
            case SGUI_KC_RALT:      this->modmask &= ~SGUI_MOD_ALT; break;
            case SGUI_KC_LSUPER:
            case SGUI_KC_RSUPER:    this->modmask &= ~SGUI_MOD_SUPER; break;
            }
        }

        if( this->event_fun )
            this->event_fun( this->userptr, e );

        sgui_event_post( e );

        if( this->backend==SGUI_NATIVE )
        {
            sgui_canvas_send_window_event( this->ctx.canvas, e );
        }

        /* generate events for special key combinations */
        if( (this->modmask==SGUI_MOD_CTRL) &&
            (e->type==SGUI_KEY_PRESSED_EVENT ||
             e->type==SGUI_KEY_RELEASED_EVENT) )
        {
            ev.type = e->type;
            ev.src.window = e->src.window;

            switch( e->arg.i )
            {
            case COPY_KEY:   ev.arg.i = SGUI_KC_COPY;       break;
            case SELECT_KEY: ev.arg.i = SGUI_KC_SELECT_ALL; break;
            case PASTE_KEY:  ev.arg.i = SGUI_KC_PASTE;      break;
            case CUT_KEY:    ev.arg.i = SGUI_KC_CUT;        break;
            }

            if( this->event_fun )
                this->event_fun( this->userptr, &ev );

            sgui_event_post( &ev );

            if( this->backend==SGUI_NATIVE )
            {
                sgui_canvas_send_window_event( this->ctx.canvas, &ev );
            }
        }
    }
}

/****************************************************************************/

sgui_window* sgui_window_create( sgui_window* parent, unsigned int width,
                                 unsigned int height, int flags )
{
    sgui_window_description desc;

    desc.parent         = parent;
    desc.share          = NULL;
    desc.width          = width;
    desc.height         = height;
    desc.flags          = flags;
    desc.backend        = SGUI_NATIVE;
    desc.bits_per_pixel = 32;
    desc.depth_bits     = 24;
    desc.stencil_bits   = 8;

    return sgui_window_create_desc( &desc );
}

void sgui_window_get_mouse_position( sgui_window* this, int* x, int* y )
{
    int mx = 0, my = 0;

    if( this )
    {
        this->get_mouse_position( this, &mx, &my );

        mx = mx<0 ? 0 : (mx>=(int)this->w ? ((int)this->w-1) : mx);
        my = my<0 ? 0 : (my>=(int)this->h ? ((int)this->h-1) : my);
    }

    if( x ) *x = mx;
    if( y ) *y = my;
}

void sgui_window_set_mouse_position( sgui_window* this, int x, int y,
                                     int send_event )
{
    sgui_event e;

    if( this && this->visible )
    {
        sgui_internal_lock_mutex( );
        x = x<0 ? 0 : (x>=(int)this->w ? ((int)this->w-1) : x);
        y = y<0 ? 0 : (y>=(int)this->h ? ((int)this->h-1) : y);

        this->set_mouse_position( this, x, y );

        if( send_event )
        {
            e.arg.i2.x = x;
            e.arg.i2.y = y;
            e.src.window = this;
            e.type = SGUI_MOUSE_MOVE_EVENT;
            sgui_internal_window_fire_event( this, &e );
        }
        sgui_internal_unlock_mutex( );
    }
}

void sgui_window_set_visible( sgui_window* this, int visible )
{
    sgui_event ev;
 
    if( !this || (this->visible==visible) )
        return;

    sgui_internal_lock_mutex( );
    this->set_visible( this, visible );
    this->visible = visible;

    if( !visible )
    {
        ev.src.window = this;
        ev.type = SGUI_API_INVISIBLE_EVENT;
        sgui_internal_window_fire_event( this, &ev );
    }

    sgui_internal_unlock_mutex( );
}

void sgui_window_set_title( sgui_window* this, const char* title )
{
    if( this && title )
        this->set_title( this, title );
}

void sgui_window_set_size( sgui_window* this,
                           unsigned int width, unsigned int height )
{
    if( this && width && height && (width!=this->w || height!=this->h) )
    {
        this->set_size( this, width, height );

        /* resize the canvas */
        if( this->backend == SGUI_NATIVE )
        {
            sgui_canvas_resize( this->ctx.canvas, this->w, this->h );
            sgui_canvas_draw_widgets( this->ctx.canvas, 1 );
        }
    }
}

void sgui_window_move_center( sgui_window* this )
{
    if( this )
        this->move_center( this );
}

void sgui_window_move( sgui_window* this, int x, int y )
{
    if( this )
    {
        sgui_internal_lock_mutex( );
        this->move( this, x, y );
        this->x = x;
        this->y = y;
        sgui_internal_unlock_mutex( );
    }
}

void sgui_window_make_current( sgui_window* this )
{
    if( this && this->backend!=SGUI_NATIVE )
    {
        sgui_context_make_current( this->ctx.ctx, this );
    }
}

void sgui_window_release_current( sgui_window* this )
{
    if( this && this->backend!=SGUI_NATIVE )
    {
        sgui_context_release_current( this->ctx.ctx );
    }
}

void sgui_window_swap_buffers( sgui_window* this )
{
    if( this && this->swap_buffers )
        this->swap_buffers( this );
}

void sgui_window_set_vsync( sgui_window* this, int vsync_on )
{
    if( this && this->set_vsync )
        this->set_vsync( this, vsync_on ? 1 : 0 );
}

void sgui_window_destroy( sgui_window* this )
{
    sgui_event ev;

    if( this )
    {
        ev.src.window = this;
        ev.type = SGUI_API_DESTROY_EVENT;
        sgui_internal_window_fire_event( this, &ev );

        this->destroy( this );
    }
}

void sgui_window_force_redraw( sgui_window* this, sgui_rect* r )
{
    sgui_rect r0;

    if( this && r )
    {
        r0.left   = MIN( r->left,                  0 );
        r0.top    = MIN( r->top,                   0 );
        r0.right  = MAX( r->right,  (int)this->w - 1 );
        r0.bottom = MAX( r->bottom, (int)this->h - 1 );

        this->force_redraw( this, &r0 );
    }
}

void sgui_window_write_clipboard( sgui_window* this, const char* text,
                                  unsigned int length )
{
    if( this && text && this->write_clipboard )
        this->write_clipboard( this, text, length );
}

const char* sgui_window_read_clipboard( sgui_window* this )
{
    return (this && this->read_clipboard) ? this->read_clipboard( this ) : 0;
}

void sgui_window_get_platform_data( const sgui_window* this, void* window )
{
    if( this && window )
        this->get_platform_data( this, window );
}

/****************************************************************************/

int sgui_window_get_modifyer_mask( const sgui_window* this )
{
    return this ? this->modmask : 0;
}

int sgui_window_is_visible( const sgui_window* this )
{
    return this ? this->visible : 0;
}

void sgui_window_get_position( const sgui_window* this, int* x, int* y )
{
    if( this )
    {
        if( x ) *x = this->x;
        if( y ) *y = this->y;
    }
    else
    {
        if( x ) *x = 0;
        if( y ) *y = 0;
    }
}

void sgui_window_get_size( const sgui_window* this, unsigned int* width,
                           unsigned int* height )
{
    if( this )
    {
        if( width  ) *width  = this->w;
        if( height ) *height = this->h;
    }
    else
    {
        if( width  ) *width  = 0;
        if( height ) *height = 0;
    }
}

void sgui_window_add_widget( sgui_window* this, sgui_widget* widget )
{
    if( this && this->backend==SGUI_NATIVE )
        sgui_widget_add_child(sgui_canvas_get_root(this->ctx.canvas), widget);
}

void sgui_window_on_event( sgui_window* this, sgui_window_callback fun )
{
    if( this )
        this->event_fun = fun;
}

sgui_canvas* sgui_window_get_canvas( const sgui_window* this )
{
    if( this && this->backend==SGUI_NATIVE )
    {
        return this->ctx.canvas;
    }

    return NULL;
}

void sgui_window_set_userptr( sgui_window* this, void* ptr )
{
    if( this )
        this->userptr = ptr;
}

void* sgui_window_get_userptr( const sgui_window* this )
{
    return this ? this->userptr : NULL;
}

sgui_context* sgui_window_get_context( const sgui_window* this )
{
    if( this && this->backend!=SGUI_NATIVE )
        return this->ctx.ctx;

    return NULL;
}

