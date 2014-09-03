/*
 * color_picker.c
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
#include "sgui_color_picker.h"
#include "sgui_internal.h"
#include "sgui_widget.h"
#include "sgui_canvas.h"
#include "sgui_pixmap.h"
#include "sgui_event.h"

#include <stdlib.h>
#include <string.h>



#define IMAGE_W 128
#define IMAGE_H 128
#define DELTA_H 2
#define DELTA_S 2
#define DELTA_V 2
#define DELTA_A 2

#define BAR_W 10

#define DISP_W 30
#define DISP_H 20
#define DISP_GAP 10
#define DISP_GAP_H 10



typedef struct
{
    sgui_widget super;
    unsigned char* hsdata;      /* static hue-saturation picker image */
    unsigned char* vbardata;    /* dynamic bar slider image */
    unsigned char* abardata;    /* dynamic alpha slider image */
    sgui_pixmap* hs;            /* static hue-saturation picker */
    sgui_pixmap* vbar;          /* dynamic value slider */
    sgui_pixmap* abar;          /* dynamic alpha slider */

    unsigned char hsva[4];      /* currently set color */
}
sgui_color_picker;



/* source: http://web.mit.edu/storborg/Public/hsvtorgb.c */
static void hsv_to_rgb( int h, int s, int v, unsigned char* rgb )
{
    int region, fpart, p, q, t;

    /* grayscale */
    if( s==0 )
    {
        rgb[0] = rgb[1] = rgb[2] = v;
        return;
    }

    region = h / 43;                    /* circle sector index (0-5) */
    fpart = (h - (region * 43)) * 6;    /* fraction scaled to range 0-255 */

    p = (v * (255 - s)) >> 8;
    q = (v * (255 - ((s * fpart) >> 8))) >> 8;
    t = (v * (255 - ((s * (255 - fpart)) >> 8))) >> 8;

    switch( region )
    {
    case 0:  rgb[0] = v; rgb[1] = t; rgb[2] = p; break;
    case 1:  rgb[0] = q; rgb[1] = v; rgb[2] = p; break;
    case 2:  rgb[0] = p; rgb[1] = v; rgb[2] = t; break;
    case 3:  rgb[0] = p; rgb[1] = q; rgb[2] = v; break;
    case 4:  rgb[0] = t; rgb[1] = p; rgb[2] = v; break;
    default: rgb[0] = v; rgb[1] = p; rgb[2] = q; break;
    }

    return;
}

static void generate_v_bar( sgui_color_picker* this )
{
    unsigned char* ptr, rgb[3];
    int x, y, v;

    for( v=0xFF, ptr=this->vbardata, y=0; y<IMAGE_H; ++y, v-=DELTA_V )
    {
        hsv_to_rgb( this->hsva[0], this->hsva[1], v, rgb );

        for( x=0; x<BAR_W; ++x, ptr+=3 )
        {
            ptr[0] = rgb[0];
            ptr[1] = rgb[1];
            ptr[2] = rgb[2];
        }
    }
}

static void generate_a_bar( sgui_color_picker* this )
{
    unsigned char* ptr, rgb[3];
    int x, y, a, c;

    hsv_to_rgb( this->hsva[0], this->hsva[1], this->hsva[2], rgb );

    for( a=0xFF, ptr=this->abardata, y=0; y<IMAGE_H; ++y, a-=DELTA_A )
    {
        for( x=0; x<BAR_W; ++x, ptr+=3 )
        {
            c = (((x&4)==0) ^ ((y&4)==0)) * 0x80 + 0x80;

            ptr[0] = (c * (0xFF-a) + rgb[0] * a) >> 8;
            ptr[1] = (c * (0xFF-a) + rgb[1] * a) >> 8;
            ptr[2] = (c * (0xFF-a) + rgb[2] * a) >> 8;
        }
    }
}

static void color_picker_draw( sgui_widget* super )
{
    sgui_color_picker* this = (sgui_color_picker*)super;
    unsigned char black[4] = { 0x00, 0x00, 0x00, 0xFF };
    unsigned char white[4] = { 0xFF, 0xFF, 0xFF, 0xFF };
    unsigned char gray[4] = { 0x80, 0x80, 0x80, 0xFF };
    unsigned char rgba[4];
    sgui_rect r, r0;
    int x, y;

    /* background pixmaps */
    sgui_canvas_draw_pixmap( super->canvas, super->area.left, super->area.top,
                             this->hs, NULL, 1 );

    sgui_canvas_draw_pixmap( super->canvas, super->area.left+IMAGE_W+BAR_W/2,
                             super->area.top, this->vbar, NULL, 1 );

    sgui_canvas_draw_pixmap( super->canvas, super->area.left+IMAGE_W+2*BAR_W,
                             super->area.top, this->abar, NULL, 1 );

    /* cross hair */
    sgui_canvas_draw_line( super->canvas,
                           super->area.left + this->hsva[0]/DELTA_H - 5,
                           super->area.top + (0xFF-this->hsva[1])/DELTA_S,
                           11, 1, black, SGUI_RGB8 );

    sgui_canvas_draw_line( super->canvas,
                           super->area.left + this->hsva[0]/DELTA_H,
                           super->area.top + (0xFF-this->hsva[1])/DELTA_S - 5,
                           11, 0, black, SGUI_RGB8 );

    /* value selector */
    sgui_canvas_draw_line( super->canvas,
                           super->area.left + IMAGE_W + BAR_W/2 - BAR_W/4,
                           super->area.top + (0xFF-this->hsva[2])/DELTA_V,
                           BAR_W + BAR_W/2, 1, black, SGUI_RGB8 );

    /* alpha selector */
    sgui_canvas_draw_line( super->canvas,
                           super->area.left + IMAGE_W + 2*BAR_W - BAR_W/4,
                           super->area.top + (0xFF-this->hsva[3])/DELTA_A,
                           BAR_W + BAR_W/2, 1, black, SGUI_RGB8 );

    /* color display */
    hsv_to_rgb( this->hsva[0], this->hsva[1], this->hsva[2], rgba );
    rgba[3] = this->hsva[3];

    r.left   = super->area.left;
    r.right  = super->area.left + DISP_W - 1;
    r.top    = super->area.top + IMAGE_H + DISP_GAP;
    r.bottom = super->area.bottom;
    sgui_canvas_draw_box( super->canvas, &r, rgba, SGUI_RGB8 );

    white[0] = (white[0]*(0xFF-rgba[3]) + rgba[0]*rgba[3])>>8;
    white[1] = (white[1]*(0xFF-rgba[3]) + rgba[1]*rgba[3])>>8;
    white[2] = (white[2]*(0xFF-rgba[3]) + rgba[2]*rgba[3])>>8;

    gray[0] = (gray[0]*(0xFF-rgba[3]) + rgba[0]*rgba[3])>>8;
    gray[1] = (gray[1]*(0xFF-rgba[3]) + rgba[1]*rgba[3])>>8;
    gray[2] = (gray[2]*(0xFF-rgba[3]) + rgba[2]*rgba[3])>>8;

    r.left += DISP_W + DISP_GAP_H;
    r.right += DISP_W + DISP_GAP_H;
    sgui_canvas_draw_box( super->canvas, &r, white, SGUI_RGB8 );

    for( y=0; y<DISP_H; y+=4 )
    {
        for( x=0; x<DISP_W; x+=4 )
        {
            r0.left   = r.left + x;
            r0.right  = r.left + ((x+3)>=DISP_W ? (DISP_W-1) : (x+3));
            r0.top    = r.top + y;
            r0.bottom = r.top + ((y+3)>=DISP_H ? (DISP_H-1) : (y+3));

            if( ((x&4)==0) ^ ((y&4)==0) )
                sgui_canvas_draw_box( super->canvas, &r0, gray, SGUI_RGB8 );
        }
    }
}

static void color_picker_destroy( sgui_widget* super )
{
    sgui_color_picker* this = (sgui_color_picker*)super;

    free( this->vbardata );
    free( this->hsdata );
    sgui_pixmap_destroy( this->hs );
    sgui_pixmap_destroy( this->vbar );
    sgui_pixmap_destroy( this->abar );
    free( this );
}

static void color_picker_on_event( sgui_widget* super, const sgui_event* e )
{
    sgui_color_picker* this = (sgui_color_picker*)super;
    unsigned char hsva[4];
    int fire_event = 0;
    sgui_event ev;

    if( e->type!=SGUI_MOUSE_PRESS_EVENT && e->type!=SGUI_MOUSE_RELEASE_EVENT )
        return;

    if( e->arg.i3.y>=IMAGE_H )
        return;

    sgui_internal_lock_mutex( );
    memcpy( hsva, this->hsva, 4 );

    /* hue-saturation selector */
    if( e->arg.i3.x<IMAGE_W )
    {
        hsva[0] = e->arg.i3.x * DELTA_H;
        hsva[1] = 0xFF - (e->arg.i3.y * DELTA_S);
        fire_event = 1;
    }

    /* value slider */
    if( e->arg.i3.x>=(IMAGE_W+BAR_W/2) && e->arg.i3.x<=(IMAGE_W+3*BAR_W/2) )
    {
        hsva[2] = 0xFF - (e->arg.i3.y * DELTA_V);
        fire_event = 1;
    }

    /* alpha slider */
    if( e->arg.i3.x>=(IMAGE_W+2*BAR_W) && e->arg.i3.x<=(IMAGE_W+3*BAR_W) )
    {
        hsva[3] = 0xFF - (e->arg.i3.y * DELTA_A);
        fire_event = 1;
    }

    sgui_color_picker_set_hsv( super, hsva );

    if( fire_event )
    {
        memcpy( ev.arg.color, this->hsva, 4 );
        ev.type = SGUI_HSVA_CHANGED_EVENT;
        ev.src.widget = super;
        sgui_event_post( &ev );

        hsv_to_rgb( this->hsva[0], this->hsva[1], this->hsva[2],
                    ev.arg.color );

        ev.type = SGUI_RGBA_CHANGED_EVENT;
        ev.src.widget = super;
        ev.arg.color[3] = this->hsva[3];
        sgui_event_post( &ev );
    }

    sgui_internal_unlock_mutex( );
}

static void color_picker_on_state_change( sgui_widget* super, int change )
{
    sgui_color_picker* this = (sgui_color_picker*)super;
    unsigned int w, h;

    if( change & SGUI_WIDGET_CANVAS_CHANGED )
    {
        sgui_internal_lock_mutex( );
        sgui_widget_get_size( super, &w, &h );

        /* destroy pixmaps on old canvas */
        sgui_pixmap_destroy( this->hs );
        sgui_pixmap_destroy( this->vbar );
        sgui_pixmap_destroy( this->abar );

        /* create pixmaps on new canvas */
        this->hs = sgui_canvas_create_pixmap( super->canvas, IMAGE_W, IMAGE_H,
                                              SGUI_RGB8 );

        this->vbar = sgui_canvas_create_pixmap( super->canvas, BAR_W, IMAGE_H,
                                                SGUI_RGB8 );

        this->abar = sgui_canvas_create_pixmap( super->canvas, BAR_W, IMAGE_H,
                                                SGUI_RGB8 );

        /* upload data to new pixmaps */
        sgui_pixmap_load( this->hs, 0, 0, this->hsdata, 0, 0,
                          IMAGE_W, IMAGE_H, IMAGE_W, SGUI_RGB8 );

        sgui_pixmap_load( this->vbar, 0, 0, this->vbardata, 0, 0,
                          BAR_W, IMAGE_H, BAR_W, SGUI_RGB8 );

        sgui_pixmap_load( this->abar, 0, 0, this->abardata, 0, 0,
                          BAR_W, IMAGE_H, BAR_W, SGUI_RGB8 );
        sgui_internal_unlock_mutex( );
    }
}

/****************************************************************************/

sgui_widget* sgui_color_picker_create( int x, int y )
{
    sgui_color_picker* this = malloc( sizeof(sgui_color_picker) );
    sgui_widget* super = (sgui_widget*)this;
    unsigned char* ptr;
    int i, j, h, s;

    if( !this )
        return NULL;

    memset( this, 0, sizeof(sgui_color_picker) );
    sgui_widget_init( super, x, y, IMAGE_W + 3*BAR_W + BAR_W/4,
                                   IMAGE_H + DISP_H + DISP_GAP );

    /* allocate image data */
    if( !(this->hsdata = malloc( IMAGE_W*IMAGE_H*3 )) )
        goto fail;

    if( !(this->vbardata = malloc( IMAGE_H*BAR_W*3 )) )
        goto fail;

    if( !(this->abardata = malloc( IMAGE_H*BAR_W*3 )) )
        goto fail;

    /* fill images */
    this->hsva[0] = 0x80;
    this->hsva[1] = 0x80;
    this->hsva[2] = 0x80;
    this->hsva[3] = 0x80;

    for( s=0xFF, ptr=this->hsdata, i=0; i<IMAGE_H; ++i, s-=DELTA_S )
    {
        for( h=0, j=0; j<IMAGE_W; ++j, ptr+=3, h+=DELTA_H )
            hsv_to_rgb( h, s, 0xFF, ptr );
    }

    generate_v_bar( this );
    generate_a_bar( this );

    /* init */
    super->draw_callback         = color_picker_draw;
    super->state_change_callback = color_picker_on_state_change;
    super->destroy               = color_picker_destroy;
    super->window_event_callback = color_picker_on_event;
    super->focus_policy          = 0;
    return super;
fail:
    free( this->vbardata );
    free( this->hsdata );
    free( this );
    return NULL;
}

void sgui_color_picker_set_hsv( sgui_widget* super,
                                unsigned char* hsva )
{
    sgui_color_picker* this = (sgui_color_picker*)super;
    unsigned char oldhsva[4];
    sgui_rect r;

    if( this && hsva )
    {
        sgui_internal_lock_mutex( );
        memcpy( oldhsva, this->hsva, 4 );
        memcpy( this->hsva, hsva, 4 );

        if( !memcmp( oldhsva, hsva, 4 ) )
        {
            sgui_internal_unlock_mutex( );
            return;
        }

        /* regenerate slider images if neccessary */
        if( oldhsva[0]!=hsva[0] || oldhsva[1]!=hsva[1] )
        {
            generate_v_bar( this );

            sgui_pixmap_load( this->vbar, 0, 0, this->vbardata, 0, 0,
                              BAR_W, IMAGE_H, BAR_W, SGUI_RGB8 );
        }

        if(oldhsva[0]!=hsva[0] || oldhsva[1]!=hsva[1] || oldhsva[2]!=hsva[2])
        {
            generate_a_bar( this );

            sgui_pixmap_load( this->abar, 0, 0, this->abardata, 0, 0,
                              BAR_W, IMAGE_H, BAR_W, SGUI_RGB8 );
        }

        /* flag dirty */
        sgui_widget_get_absolute_rect( super, &r );

        if( oldhsva[0]==hsva[0] && oldhsva[1]==hsva[1] )
        {
            r.left += IMAGE_W + BAR_W/4;

            if( oldhsva[2]==hsva[2] )
                r.left += BAR_W + BAR_W/2;

            sgui_canvas_add_dirty_rect( super->canvas, &r );

            sgui_widget_get_absolute_rect( super, &r );
            r.right = r.left + 2*DISP_W + DISP_GAP_H - 1;
            r.top   = r.bottom - DISP_H - 1;
            sgui_canvas_add_dirty_rect( super->canvas, &r );
        }
        else
        {
            sgui_canvas_add_dirty_rect( super->canvas, &r );
        }

        sgui_internal_unlock_mutex( );
    }
}

