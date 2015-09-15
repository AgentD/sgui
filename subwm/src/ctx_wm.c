/*
 * ctx_wm.c
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
#include "sgui_skin.h"
#include "sgui_event.h"
#include "sgui_ctx_wm.h"
#include "sgui_internal.h"
#include "sgui_ctx_window.h"

#include "GL/gl_wm.h"
#include "D3D9/d3d9_wm.h"
#include "D3D11/d3d11_wm.h"

#include <stdlib.h>
#include <string.h>




static void clamp_window_area( sgui_ctx_wm* this, sgui_window* wnd )
{
    unsigned int ww, wh, maxw, maxh;
    int wx, wy;

    sgui_window_get_position( wnd, &wx, &wy );
    sgui_window_get_size( wnd, &ww, &wh );              /* own size */
    sgui_window_get_size( this->wnd, &maxw, &maxh );    /* parent size */

    wx = wx<0 ? 0 : ((wx + ww)>=maxw ? (int)(maxw - ww) : wx);
    wy = wy<0 ? 0 : ((wy + wh)>=maxh ? (int)(maxh - wh) : wy);

    sgui_window_move( wnd, wx, wy );
}

static void window_move_delta( sgui_window* wnd, int dx, int dy )
{
    int x, y;

    sgui_window_get_position( wnd, &x, &y );
    sgui_window_move( wnd, x+dx, y+dy );
}

static void bring_to_front( sgui_ctx_wm* this, sgui_ctx_window* wnd )
{
    sgui_ctx_window* it;

    SGUI_REMOVE_FROM_LIST( this->list, it, wnd );

    for( it=this->list; it->next!=NULL; it=it->next ) { }

    it->next = wnd;
    wnd->next = NULL;
}

/****************************************************************************/

sgui_ctx_wm* sgui_ctx_wm_create( sgui_window* wnd )
{
    if( !wnd )
        return NULL;

    switch( wnd->backend )
    {
#ifndef SGUI_NO_OPENGL
    case SGUI_OPENGL_CORE:
        return gl_wm_create_core( wnd );
    case SGUI_OPENGL_COMPAT:
        return gl_wm_create( wnd );
#endif
#if defined(SGUI_WINDOWS) && !defined(SGUI_NO_D3D9)
    case SGUI_DIRECT3D_9:
        return d3d9_wm_create( wnd );
#endif
#if defined(SGUI_WINDOWS) && !defined(SGUI_NO_D3D11)
    case SGUI_DIRECT3D_11:
        return d3d11_wm_create( wnd );
#endif
    }

    return NULL;
}

void sgui_ctx_wm_destroy( sgui_ctx_wm* this )
{
    sgui_ctx_window* w;

    for( w=this->list; w!=NULL; w=w->next )
        sgui_window_destroy( (sgui_window*)w );

    this->list = NULL;
    this->destroy( this );
}

sgui_window* sgui_ctx_wm_create_window( sgui_ctx_wm* this,
                                        unsigned int width,
                                        unsigned int height,
                                        int flags )
{
    sgui_ctx_window* wnd = NULL;

    wnd = (sgui_ctx_window*)sgui_ctx_window_create( this->wnd, width, height,
                                                    flags );

    if( wnd )
    {
        wnd->wm = this;
        wnd->next = this->list;
        this->list = wnd;
    }
    return (sgui_window*)wnd;
}

void sgui_ctx_wm_remove_window( sgui_ctx_wm* this, sgui_window* wnd )
{
    sgui_ctx_window* it;
    SGUI_REMOVE_FROM_LIST( this->list, it, (sgui_ctx_window*)wnd );
}

void sgui_ctx_wm_draw_gui( sgui_ctx_wm* this )
{
    sgui_ctx_window* wnd;

    for( wnd=this->list; wnd!=NULL; wnd=wnd->next )
        sgui_ctx_window_update_canvas( (sgui_window*)wnd );

    this->draw_gui( this );
}

sgui_ctx_window* sgui_ctx_wm_window_from_point( sgui_ctx_wm* this,
                                                int x, int y )
{
    sgui_ctx_window *wnd, *result=NULL;
    unsigned int ww, wh;
    int wx, wy;

    for( wnd=this->list; wnd!=NULL; wnd=wnd->next )
    {
        sgui_window_get_position( (sgui_window*)wnd, &wx, &wy );

        if( x<wx || y<wy )
            continue;

        sgui_window_get_size( (sgui_window*)wnd, &ww, &wh );
        wx += ww;
        wy += wh;

        if( x>=wx || y>=wy )
            continue;

        result = wnd;
    }
    return result;
}

void sgui_ctx_wm_inject_event( sgui_ctx_wm* this, const sgui_event* event )
{
    sgui_ctx_window* wnd;
    sgui_event ev;
    int x, y;

    switch( event->type )
    {
    case SGUI_MOUSE_MOVE_EVENT:
        if( this->draging && this->mouseover )
        {
            window_move_delta( (sgui_window*)this->mouseover,
                               event->arg.i2.x - this->grabx,
                               event->arg.i2.y - this->graby );

            clamp_window_area( this, (sgui_window*)this->mouseover );

            this->grabx = event->arg.i2.x;
            this->graby = event->arg.i2.y;
        }
        else
        {
            wnd = sgui_ctx_wm_window_from_point( this,
                                                 event->arg.i2.x,
                                                 event->arg.i2.y );
            this->mouseover = wnd;
            if( this->mouseover )
                sgui_ctx_window_inject_event( (sgui_window*)this->mouseover,
                                              event );
        }
        break;
    case SGUI_MOUSE_PRESS_EVENT:
        if( this->mouseover )
        {
            if( this->mouseover->next )
                bring_to_front( this, this->mouseover );

            sgui_window_get_position( (sgui_window*)this->mouseover, &x, &y );
            this->grabx = event->arg.i3.x;
            this->graby = event->arg.i3.y;

            if( (event->arg.i3.y - y)<20 )
            {
                this->draging = 1;
                break;
            }
            sgui_ctx_window_inject_event((sgui_window*)this->mouseover,event);
        }
        break;
    case SGUI_MOUSE_RELEASE_EVENT:
        if( this->draging )
            this->draging = 0;
        else if( this->mouseover )
            sgui_ctx_window_inject_event((sgui_window*)this->mouseover,event);

        if( this->focus != this->mouseover )
        {
            if( this->focus )
            {
                ev.type = SGUI_FOCUS_LOSE_EVENT;
                sgui_ctx_window_inject_event((sgui_window*)this->focus,&ev);
            }
            this->focus = this->mouseover;
            if( this->focus )
            {
                ev.type = SGUI_FOCUS_EVENT;
                sgui_ctx_window_inject_event((sgui_window*)this->focus,&ev);
            }
        }
        break;
    case SGUI_MOUSE_WHEEL_EVENT:
        if( this->mouseover )
            sgui_ctx_window_inject_event((sgui_window*)this->mouseover,event);
        break;
    case SGUI_KEY_PRESSED_EVENT:
    case SGUI_KEY_RELEASED_EVENT:
    case SGUI_CHAR_EVENT:
        if( this->focus )
            sgui_ctx_window_inject_event( (sgui_window*)this->focus, event );
        break;
    case SGUI_D3D9_DEVICE_LOST:
        for( wnd=this->list; wnd!=NULL; wnd=wnd->next )
            sgui_ctx_window_inject_event( (sgui_window*)wnd, event );
        break;
    }
}

