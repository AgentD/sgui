/*
 * button.c
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
#include "sgui_button.h"
#include "sgui_skin.h"
#include "sgui_event.h"
#include "sgui_canvas.h"
#include "sgui_font.h"
#include "sgui_internal.h"
#include "sgui_widget.h"

#include <stdlib.h>
#include <string.h>



#define SGUI_BUTTON          1
#define SGUI_BUTTON_SELECTED 2



typedef struct sgui_button
{
    sgui_widget widget;

    char* text;
    int type;                 /* button type */
    unsigned int cx, cy;      /* button: text position,
                                 checkbox/radio button: object size */

    struct sgui_button* prev; /* radio button menu: next in menu */
    struct sgui_button* next; /* radio button menu: previous in menu */
}
sgui_button;



static void checkbox_draw( sgui_widget* w )
{
    sgui_button* b = (sgui_button*)w;
    sgui_pixmap* skin_pixmap = sgui_canvas_get_skin_pixmap( w->canvas );
    unsigned int delta = sgui_skin_get_default_font_height( ) / 4;
    int x = w->area.left + b->cx, y = w->area.top;
    sgui_canvas* cv = w->canvas;
    sgui_rect r;

    sgui_skin_get_element( b->type, &r );
    x += b->cx/2;

    sgui_canvas_blend( cv, w->area.left, w->area.top+delta, skin_pixmap, &r );
    sgui_canvas_draw_text( w->canvas, x, y, b->text );
}

static void button_draw( sgui_widget* w )
{
    sgui_button* button = (sgui_button*)w;
    sgui_pixmap* skin_pixmap = sgui_canvas_get_skin_pixmap( w->canvas );
    sgui_rect lt, rt, lb, rb, l, r, t, b, f, dst;
    sgui_canvas* cv = w->canvas;

    int x = w->area.left;
    int y = w->area.top;
    int x1 = w->area.right;
    int y1 = w->area.bottom;

    /* get required skin elements */
    if( button->type==SGUI_BUTTON_SELECTED )
    {
        sgui_skin_get_element( SGUI_BUTTON_IN_LEFT_TOP, &lt );
        sgui_skin_get_element( SGUI_BUTTON_IN_RIGHT_TOP, &rt );
        sgui_skin_get_element( SGUI_BUTTON_IN_LEFT_BOTTOM, &lb );
        sgui_skin_get_element( SGUI_BUTTON_IN_RIGHT_BOTTOM, &rb );
        sgui_skin_get_element( SGUI_BUTTON_IN_LEFT, &l );
        sgui_skin_get_element( SGUI_BUTTON_IN_RIGHT, &r );
        sgui_skin_get_element( SGUI_BUTTON_IN_TOP, &t );
        sgui_skin_get_element( SGUI_BUTTON_IN_BOTTOM, &b );
        sgui_skin_get_element( SGUI_BUTTON_IN_FILL, &f );
    }
    else
    {
        sgui_skin_get_element( SGUI_BUTTON_LEFT_TOP, &lt );
        sgui_skin_get_element( SGUI_BUTTON_RIGHT_TOP, &rt );
        sgui_skin_get_element( SGUI_BUTTON_LEFT_BOTTOM, &lb );
        sgui_skin_get_element( SGUI_BUTTON_RIGHT_BOTTOM, &rb );
        sgui_skin_get_element( SGUI_BUTTON_LEFT, &l );
        sgui_skin_get_element( SGUI_BUTTON_RIGHT, &r );
        sgui_skin_get_element( SGUI_BUTTON_TOP, &t );
        sgui_skin_get_element( SGUI_BUTTON_BOTTOM, &b );
        sgui_skin_get_element( SGUI_BUTTON_FILL, &f );
    }

    /* draw corners */
    sgui_canvas_blend( cv, x, y, skin_pixmap, &lt );
    sgui_canvas_blend( cv, x1-(rt.right-rt.left), y, skin_pixmap, &rt );
    sgui_canvas_blend( cv, x, y1-(lb.bottom-lb.top), skin_pixmap, &lb );
    sgui_canvas_blend( cv, x1-(rb.right-rb.left), y1-(rb.bottom-rb.top),
                       skin_pixmap, &rb );

    /* draw borders */
    dst.left   = x;
    dst.right  = x  + (l.right - l.left);
    dst.top    = y  + SGUI_RECT_HEIGHT(lt);
    dst.bottom = y1 - SGUI_RECT_HEIGHT(lb);
    sgui_canvas_stretch_blend( cv, skin_pixmap, &l, &dst, 0 );

    dst.left   = x1 - (r.right-r.left);
    dst.right  = x1;
    dst.top    = y  + SGUI_RECT_HEIGHT(rt);
    dst.bottom = y1 - SGUI_RECT_HEIGHT(rb);
    sgui_canvas_stretch_blend( cv, skin_pixmap, &r, &dst, 0 );

    dst.left   = x  + SGUI_RECT_WIDTH(lt);
    dst.right  = x1 - SGUI_RECT_WIDTH(rt);
    dst.top    = y;
    dst.bottom = y  + (t.bottom-t.top);
    sgui_canvas_stretch_blend( cv, skin_pixmap, &t, &dst, 0 );

    dst.left   = x  + SGUI_RECT_WIDTH(lb);
    dst.right  = x1 - SGUI_RECT_WIDTH(rb);
    dst.top    = y1 - (b.bottom-b.top);
    dst.bottom = y1;
    sgui_canvas_stretch_blend( cv, skin_pixmap, &b, &dst, 0 );

    /* draw background */
    dst.left   = x  + SGUI_RECT_WIDTH(l);
    dst.right  = x1 - SGUI_RECT_WIDTH(r);
    dst.top    = y  + SGUI_RECT_HEIGHT(t);
    dst.bottom = y1 - SGUI_RECT_HEIGHT(b);
    sgui_canvas_stretch_blend( cv, skin_pixmap, &f, &dst, 0 );

    /* draw text */
    x += button->cx - (button->type==SGUI_BUTTON_SELECTED ? 1 : 0);
    y += button->cy - (button->type==SGUI_BUTTON_SELECTED ? 1 : 0);
    sgui_canvas_draw_text( cv, x, y, button->text );
}

static void button_destroy( sgui_widget* button )
{
    sgui_button* b = (sgui_button*)button;

    /* detatch from linked list */
    if( b->prev ) b->prev->next = b->next;
    if( b->next ) b->next->prev = b->prev;

    /* free memory of text buffer and button */
    free( b->text );
    free( button );
}

static sgui_button* button_create_common( int x, int y, unsigned int width,
                                          unsigned int height,
                                          const char* text, int type )
{
    sgui_button* b = malloc( sizeof(sgui_button) );
    unsigned int len, text_width, text_height;
    sgui_rect r;

    if( !b )
        return NULL;

    /* allocate space for the text */
    len = strlen( text );
    b->text = malloc( len + 1 );

    if( !b->text )
    {
        free( b );
        return NULL;
    }

    /* compute text size */
    sgui_skin_get_text_extents( text, &r );
    text_width = SGUI_RECT_WIDTH( r );
    text_height = SGUI_RECT_HEIGHT( r );

    /* compute size */
    if( type!=SGUI_BUTTON )
    {
        sgui_skin_get_element( type, &r );

        b->cx = SGUI_RECT_WIDTH( r );
        b->cy = SGUI_RECT_HEIGHT( r );

        width  = b->cx + text_width + b->cx/2;
        height = MAX(b->cy, text_height);
    }
    else
    {
        b->cx  = width /2 - text_width/2;
        b->cy  = height/2 - text_height/2;
    }

    sgui_internal_widget_init( (sgui_widget*)b, x, y, width, height );

    /* initialise remaining fields */
    memcpy( b->text, text, len + 1 );

    b->prev = NULL;
    b->next = NULL;
    b->type = type;

    b->widget.destroy = button_destroy;
    b->widget.draw_callback = type==SGUI_BUTTON ? button_draw : checkbox_draw;

    return b;
}

/**************************** radio button code ****************************/
static void radio_button_select( sgui_button* b )
{
    sgui_button* i;
    sgui_rect r;

    b->type = SGUI_RADIO_BUTTON_SELECTED;
    sgui_widget_get_absolute_rect( &b->widget, &r );
    sgui_canvas_add_dirty_rect( b->widget.canvas, &r );

    /* deselect all preceeding radio buttons */
    for( i=b->prev; i!=NULL; i=i->prev )
    {
        if( i->type == SGUI_RADIO_BUTTON_SELECTED )
        {
            sgui_widget_get_absolute_rect( &i->widget, &r );
            sgui_canvas_add_dirty_rect( i->widget.canvas, &r );
            i->type = SGUI_RADIO_BUTTON;
            return;
        }
    }

    /* deselect all following radio buttons */
    for( i=b->next; i!=NULL; i=i->next )
    {
        if( i->type == SGUI_RADIO_BUTTON_SELECTED )
        {
            sgui_widget_get_absolute_rect( &i->widget, &r );
            sgui_canvas_add_dirty_rect( i->widget.canvas, &r );
            i->type = SGUI_RADIO_BUTTON;
            return;
        }
    }
}

static void radio_on_event( sgui_widget* widget, int type, sgui_event* event )
{
    sgui_button* b = (sgui_button*)widget;
    (void)event;

    /* the radio button got clicked and is not selected */
    if( type==SGUI_MOUSE_RELEASE_EVENT && b->type==SGUI_RADIO_BUTTON )
    {
        radio_button_select( b );

        sgui_canvas_fire_widget_event( widget->canvas, widget,
                                       SGUI_RADIO_BUTTON_SELECT_EVENT );
    }
}

sgui_widget* sgui_radio_button_create( int x, int y, const char* text )
{
    sgui_button* b = button_create_common(x,y,0,0,text,SGUI_RADIO_BUTTON);

    if( b )
        b->widget.window_event_callback = radio_on_event;

    return (sgui_widget*)b;
}

void sgui_radio_button_connect( sgui_widget* radio, sgui_widget* previous,
                                sgui_widget* next )
{
    sgui_button* b = (sgui_button*)radio;

    if( b && (b->type==SGUI_RADIO_BUTTON ||
              b->type==SGUI_RADIO_BUTTON_SELECTED) )
    {
        /* disconnect from existing linked list */
        if( b->prev ) b->prev->next = b->next;
        if( b->next ) b->next->prev = b->prev;

        /* store next and previous pointers */
        b->prev = (sgui_button*)previous;
        b->next = (sgui_button*)next;

        /* connect previous and next to the given pointer */
        if( b->prev ) b->prev->next = b;
        if( b->next ) b->next->prev = b;
    }
}

/****************************** checkbox code ******************************/
static void checkbox_on_event( sgui_widget* widget, int type,
                               sgui_event* event )
{
    sgui_button* b = (sgui_button*)widget;
    sgui_rect r;
    int e;
    (void)event;

    if( type == SGUI_MOUSE_RELEASE_EVENT )  /* the check box got clicked */
    {
        /* invert state, set event type */
        if( b->type==SGUI_CHECKBOX )
        {
            b->type = SGUI_CHECKBOX_SELECTED;
            e = SGUI_CHECKBOX_CHECK_EVENT;
        }
        else
        {
            b->type = SGUI_CHECKBOX;
            e = SGUI_CHECKBOX_UNCHECK_EVENT;
        }

        /* flag dirty */
        sgui_widget_get_absolute_rect( widget, &r );
        sgui_canvas_add_dirty_rect( widget->canvas, &r );

        /* fire event */
        sgui_canvas_fire_widget_event( widget->canvas, widget, e );
    }
}

sgui_widget* sgui_checkbox_create( int x, int y, const char* text )
{
    sgui_button* b = button_create_common( x, y, 0, 0, text, SGUI_CHECKBOX );

    if( b )
        b->widget.window_event_callback = checkbox_on_event;

    return (sgui_widget*)b;
}

/******************************* button code *******************************/
static void button_on_event( sgui_widget* widget, int type,
                             sgui_event* event )
{
    sgui_button* b = (sgui_button*)widget;
    sgui_rect r;
    (void)event;

    /* the mouse left a pressed button */
    if( type == SGUI_MOUSE_LEAVE_EVENT && b->type != SGUI_BUTTON )
    {
        sgui_widget_get_absolute_rect( widget, &r );
        sgui_canvas_add_dirty_rect( widget->canvas, &r );
        b->type = SGUI_BUTTON;
    }
    else if( type == SGUI_MOUSE_PRESS_EVENT )   /* a button got pressed */
    {
        b->type = SGUI_BUTTON_SELECTED;
        sgui_widget_get_absolute_rect( widget, &r );
        sgui_canvas_add_dirty_rect( widget->canvas, &r );
    }
    else if( type == SGUI_MOUSE_RELEASE_EVENT && b->type != SGUI_BUTTON )
    {
        /* a pressed button got released */
        sgui_canvas_fire_widget_event( widget->canvas, widget,
                                       SGUI_BUTTON_CLICK_EVENT );

        b->type = SGUI_BUTTON;
        sgui_widget_get_absolute_rect( widget, &r );
        sgui_canvas_add_dirty_rect( widget->canvas, &r );
    }
}

sgui_widget* sgui_button_create( int x, int y,
                                 unsigned int width, unsigned int height,
                                 const char* text )
{
    sgui_button* b = button_create_common(x,y,width,height,text,SGUI_BUTTON);

    if( b )
        b->widget.window_event_callback = button_on_event;

    return (sgui_widget*)b;
}

/******************************* common code *******************************/
void sgui_button_set_text( sgui_widget* button, const char* text )
{
    unsigned int len, text_width, text_height;
    sgui_button* b = (sgui_button*)button;
    sgui_rect r;

    /* sanity check */
    if( !button || !text )
        return;

    len = strlen( text );

    sgui_skin_get_text_extents( text, &r );
    text_width = SGUI_RECT_WIDTH( r );
    text_height = SGUI_RECT_HEIGHT( r );

    /* determine text position */
    if( b->type==SGUI_BUTTON || b->type==SGUI_BUTTON_SELECTED )
    {
        b->cx = (button->area.left + button->area.right - text_width)>>1;
        b->cy = (button->area.top + button->area.bottom - text_height)>>1;
    }
    else
    {
        sgui_skin_get_element( b->type, &r );

        b->cx = SGUI_RECT_WIDTH( r );
        b->cy = SGUI_RECT_HEIGHT( r );

        button->area.right = button->area.left + b->cx + text_width;
    }

    /* adjust buffer size and copy */
    free( b->text );
    b->text = malloc( len + 1 );

    if( b->text )
        memcpy( b->text, text, len + 1 );
}

void sgui_button_set_state( sgui_widget* button, int state )
{
    sgui_button* b = (sgui_button*)button;
    sgui_rect r;

    if( b->type==SGUI_RADIO_BUTTON )
    {
        radio_button_select( b );
    }
    else if( b->type==SGUI_CHECKBOX && state )
    {
        b->type = SGUI_CHECKBOX_SELECTED;
        sgui_widget_get_absolute_rect( button, &r );
        sgui_canvas_add_dirty_rect( button->canvas, &r );
    }
    else if( b->type==SGUI_CHECKBOX_SELECTED && !state )
    {
        b->type = SGUI_CHECKBOX;
        sgui_widget_get_absolute_rect( button, &r );
        sgui_canvas_add_dirty_rect( button->canvas, &r );
    }
}

int sgui_button_get_state( sgui_widget* button )
{
    sgui_button* b = (sgui_button*)button;

    if( button )
    {
        return b->type==SGUI_RADIO_BUTTON_SELECTED ||
               b->type==SGUI_CHECKBOX_SELECTED ||
               b->type==SGUI_BUTTON_SELECTED;
    }

    return 0;
}

