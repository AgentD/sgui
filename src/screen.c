/*
 * screen.c
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
#include "sgui_screen.h"
#include "sgui_widget_manager.h"

#include <stdlib.h>



struct sgui_screen
{
    sgui_canvas* cv;
    sgui_widget_manager* mgr;
};



sgui_screen* sgui_screen_create( sgui_canvas* cv )
{
    sgui_screen* scr;

    /* sanity checks */
    if( !cv )
        return NULL;

    /* allocate space for the screen */
    scr = malloc( sizeof(sgui_screen) );

    if( !scr )
        return NULL;

    /* create a widget manager */
    scr->mgr = sgui_widget_manager_create( );

    if( !scr->mgr )
    {
        free( scr );
        return NULL;
    }

    /* store the canvas */
    scr->cv = cv;

    return scr;
}

void sgui_screen_destroy( sgui_screen* screen )
{
    if( screen )
    {
        sgui_widget_manager_destroy( screen->mgr );
        free( screen );
    }
}

void sgui_screen_add_widget( sgui_screen* screen, sgui_widget* widget )
{
    if( screen )
    {
        sgui_widget_manager_add_widget( screen->mgr, widget );
    }
}

void sgui_screen_remove_widget( sgui_screen* screen, sgui_widget* widget )
{
    if( screen )
    {
        sgui_widget_manager_remove_widget( screen->mgr, widget );
    }
}

void sgui_screen_inject_event( sgui_screen* screen, int type,
                               sgui_event* event )
{
    if( screen )
    {
        sgui_widget_manager_send_window_event( screen->mgr, type, event );
    }
}

void sgui_screen_update( sgui_screen* screen )
{
    if( screen && sgui_widget_manager_num_dirty_rects( screen->mgr ) )
    {
        sgui_widget_manager_draw( screen->mgr, screen->cv );
        sgui_widget_manager_clear_dirty_rects( screen->mgr );
    }
}

