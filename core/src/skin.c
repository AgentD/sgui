/*
 * skin.c
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
#include "sgui_skin.h"

#include "sgui_progress_bar.h"
#include "sgui_window.h"
#include "sgui_canvas.h"
#include "sgui_pixmap.h"
#include "sgui_font.h"
#include "sgui_rect.h"
#include "sgui_utf8.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>



static sgui_font* font_norm = NULL;
static sgui_font* font_bold = NULL;
static sgui_font* font_ital = NULL;
static sgui_font* font_boit = NULL;
static unsigned int font_height = 0;



void sgui_skin_get_pixmap_size( unsigned int* width, unsigned int* height )
{
    if( width  ) *width  = 128;
    if( height ) *height = 128;
}

void sgui_skin_to_pixmap( sgui_pixmap* pixmap )
{
    unsigned char buffer[ 20*20*4 ];
    unsigned int x, y;

    if( !pixmap )
        return;

    /* draw unchecked checkbox */
    for( y=0; y<12; ++y )
    {
        for( x=0; x<12; ++x )
        {
            buffer[ (y*12 + x)*4     ] = 0x00;
            buffer[ (y*12 + x)*4 + 1 ] = 0x00;
            buffer[ (y*12 + x)*4 + 2 ] = 0x00;
            buffer[ (y*12 + x)*4 + 3 ] = 0x80;
        }
    }

    for( x=0; x<12; ++x )
    {
        buffer[ x*12*4     ] = buffer[ x*4     ] = 0x00;
        buffer[ x*12*4 + 1 ] = buffer[ x*4 + 1 ] = 0x00;
        buffer[ x*12*4 + 2 ] = buffer[ x*4 + 2 ] = 0x00;
        buffer[ x*12*4 + 3 ] = buffer[ x*4 + 3 ] = 0xFF;
    }

    for( x=0; x<12; ++x )
        for( y=0; y<4; ++y )
            buffer[ (x*12 + 11)*4 + y ] = buffer[ (11*12 + x)*4 + y ] = 0xFF;

    sgui_pixmap_load( pixmap, 0, 0, buffer, 0, 0, 12, 12, 12, SGUI_RGBA8 );

    /* draw checked checkbox */
    for( y=0; y<3; ++y )
    {
        for( x=0; x<4; ++x )
        {
            buffer[ ((4+y)*12 + 2)*4 + x ] = 0xFF;
            buffer[ ((5+y)*12 + 3)*4 + x ] = 0xFF;
            buffer[ ((6+y)*12 + 4)*4 + x ] = 0xFF;
            buffer[ ((5+y)*12 + 5)*4 + x ] = 0xFF;
            buffer[ ((4+y)*12 + 6)*4 + x ] = 0xFF;
            buffer[ ((3+y)*12 + 7)*4 + x ] = 0xFF;
            buffer[ ((2+y)*12 + 8)*4 + x ] = 0xFF;
        }
    }

    sgui_pixmap_load( pixmap, 12, 0, buffer, 0, 0, 12, 12, 12, SGUI_RGBA8 );

    /* draw radio button */
    for( y=0; y<12; ++y )
    {
        for( x=0; x<12; ++x )
        {
            buffer[ (y*12 + x)*4     ] = buffer[ (y*12 + x)*4 + 1 ] = 0x00;
            buffer[ (y*12 + x)*4 + 2 ] = buffer[ (y*12 + x)*4 + 3 ] = 0x00;
        }
    }

    for( y=0; y<8; ++y )
    {
        for( x=0; x<8; ++x )
        {
            buffer[ ((y+2)*12 + x+2)*4     ] = 0x00;
            buffer[ ((y+2)*12 + x+2)*4 + 1 ] = 0x00;
            buffer[ ((y+2)*12 + x+2)*4 + 2 ] = 0x00;
            buffer[ ((y+2)*12 + x+2)*4 + 3 ] = 0x80;
        }
    }

    for( x=0; x<4; ++x )
    {
        buffer[ ((x+4)*12)*4     ] = buffer[ (x+4)*4     ] = 0x00;
        buffer[ ((x+4)*12)*4 + 1 ] = buffer[ (x+4)*4 + 1 ] = 0x00;
        buffer[ ((x+4)*12)*4 + 2 ] = buffer[ (x+4)*4 + 2 ] = 0x00;
        buffer[ ((x+4)*12)*4 + 3 ] = buffer[ (x+4)*4 + 3 ] = 0xFF;

        buffer[ ((x+4)*12 + 11)*4     ] = buffer[ (11*12 + x+4)*4     ]=0xFF;
        buffer[ ((x+4)*12 + 11)*4 + 1 ] = buffer[ (11*12 + x+4)*4 + 1 ]=0xFF;
        buffer[ ((x+4)*12 + 11)*4 + 2 ] = buffer[ (11*12 + x+4)*4 + 2 ]=0xFF;
        buffer[ ((x+4)*12 + 11)*4 + 3 ] = buffer[ (11*12 + x+4)*4 + 3 ]=0xFF;

        buffer[ ((x+4)*12+1)*4     ] = buffer[ (12+x+4)*4     ] = 0x00;
        buffer[ ((x+4)*12+1)*4 + 1 ] = buffer[ (12+x+4)*4 + 1 ] = 0x00;
        buffer[ ((x+4)*12+1)*4 + 2 ] = buffer[ (12+x+4)*4 + 2 ] = 0x00;
        buffer[ ((x+4)*12+1)*4 + 3 ] = buffer[ (12+x+4)*4 + 3 ] = 0x80;

        buffer[ ((x+4)*12 + 10)*4     ] = buffer[ (10*12 + x+4)*4     ]=0x00;
        buffer[ ((x+4)*12 + 10)*4 + 1 ] = buffer[ (10*12 + x+4)*4 + 1 ]=0x00;
        buffer[ ((x+4)*12 + 10)*4 + 2 ] = buffer[ (10*12 + x+4)*4 + 2 ]=0x00;
        buffer[ ((x+4)*12 + 10)*4 + 3 ] = buffer[ (10*12 + x+4)*4 + 3 ]=0x80;
    }

    for( x=0; x<2; ++x )
    {
        buffer[ (1*12 + x+2)*4     ] = buffer[ (1*12 + x+8)*4     ] = 0x00;
        buffer[ (1*12 + x+2)*4 + 1 ] = buffer[ (1*12 + x+8)*4 + 1 ] = 0x00;
        buffer[ (1*12 + x+2)*4 + 2 ] = buffer[ (1*12 + x+8)*4 + 2 ] = 0x00;
        buffer[ (1*12 + x+2)*4 + 3 ] = buffer[ (1*12 + x+8)*4 + 3 ] = 0xFF;

        buffer[ ((x+2)*12 + 1)*4     ] = buffer[ ((x+8)*12 + 1)*4     ]=0x00;
        buffer[ ((x+2)*12 + 1)*4 + 1 ] = buffer[ ((x+8)*12 + 1)*4 + 1 ]=0x00;
        buffer[ ((x+2)*12 + 1)*4 + 2 ] = buffer[ ((x+8)*12 + 1)*4 + 2 ]=0x00;
        buffer[ ((x+2)*12 + 1)*4 + 3 ] = buffer[ ((x+8)*12 + 1)*4 + 3 ]=0xFF;

        buffer[ (10*12 + x+2)*4     ] = buffer[ (10*12 + x+8)*4     ] = 0xFF;
        buffer[ (10*12 + x+2)*4 + 1 ] = buffer[ (10*12 + x+8)*4 + 1 ] = 0xFF;
        buffer[ (10*12 + x+2)*4 + 2 ] = buffer[ (10*12 + x+8)*4 + 2 ] = 0xFF;
        buffer[ (10*12 + x+2)*4 + 3 ] = buffer[ (10*12 + x+8)*4 + 3 ] = 0xFF;

        buffer[ ((x+2)*12 + 10)*4     ]=buffer[ ((x+8)*12 + 10)*4     ]=0xFF;
        buffer[ ((x+2)*12 + 10)*4 + 1 ]=buffer[ ((x+8)*12 + 10)*4 + 1 ]=0xFF;
        buffer[ ((x+2)*12 + 10)*4 + 2 ]=buffer[ ((x+8)*12 + 10)*4 + 2 ]=0xFF;
        buffer[ ((x+2)*12 + 10)*4 + 3 ]=buffer[ ((x+8)*12 + 10)*4 + 3 ]=0xFF;
    }

    sgui_pixmap_load( pixmap, 24, 0, buffer, 0, 0, 12, 12, 12, SGUI_RGBA8 );

    /* draw selected radio button */
    for( y=0; y<6; ++y )
    {
        for( x=0; x<4; ++x )
        {
            buffer[ ((y+3)*12 + x+4)*4     ] = 0xFF;
            buffer[ ((y+3)*12 + x+4)*4 + 1 ] = 0xFF;
            buffer[ ((y+3)*12 + x+4)*4 + 2 ] = 0xFF;
            buffer[ ((y+3)*12 + x+4)*4 + 3 ] = 0xFF;
        }
    }

    for( y=0; y<4; ++y )
    {
        for( x=0; x<6; ++x )
        {
            buffer[ ((y+4)*12 + x+3)*4     ] = 0xFF;
            buffer[ ((y+4)*12 + x+3)*4 + 1 ] = 0xFF;
            buffer[ ((y+4)*12 + x+3)*4 + 2 ] = 0xFF;
            buffer[ ((y+4)*12 + x+3)*4 + 3 ] = 0xFF;
        }
    }

    sgui_pixmap_load( pixmap, 36, 0, buffer, 0, 0, 12, 12, 12, SGUI_RGBA8 );

    /* scroll bar down button */
    for( y=0; y<20; ++y )
    {
        for( x=0; x<20; ++x )
        {
            buffer[ (y*20 + x)*4     ] = 0x64;
            buffer[ (y*20 + x)*4 + 1 ] = 0x64;
            buffer[ (y*20 + x)*4 + 2 ] = 0x64;
            buffer[ (y*20 + x)*4 + 3 ] = 0xFF;
        }
    }

    for( x=0; x<20; ++x )
    {
        buffer[ x*4     ] = buffer[ x*20*4     ] = 0xFF;
        buffer[ x*4 + 1 ] = buffer[ x*20*4 + 1 ] = 0xFF;
        buffer[ x*4 + 2 ] = buffer[ x*20*4 + 2 ] = 0xFF;
        buffer[ x*4 + 3 ] = buffer[ x*20*4 + 3 ] = 0xFF;
    }

    for( x=0; x<20; ++x )
    {
        buffer[ (19*20+x)*4     ] = buffer[ (x*20+19)*4     ] = 0x00;
        buffer[ (19*20+x)*4 + 1 ] = buffer[ (x*20+19)*4 + 1 ] = 0x00;
        buffer[ (19*20+x)*4 + 2 ] = buffer[ (x*20+19)*4 + 2 ] = 0x00;
        buffer[ (19*20+x)*4 + 3 ] = buffer[ (x*20+19)*4 + 3 ] = 0xFF;
    }

    for( y=0; y<6; ++y )
    {
        for( x=0; x<(11-2*y); ++x )
        {
            buffer[ ((7+y)*20 + 5+y+x)*4     ] = 0xFF;
            buffer[ ((7+y)*20 + 5+y+x)*4 + 1 ] = 0xFF;
            buffer[ ((7+y)*20 + 5+y+x)*4 + 2 ] = 0xFF;
            buffer[ ((7+y)*20 + 5+y+x)*4 + 3 ] = 0xFF;
        }
    }

    sgui_pixmap_load( pixmap, 48, 0, buffer, 0, 0, 20, 20, 20, SGUI_RGBA8 );

    /* scroll bar down button pressed */
    for( y=0; y<20; ++y )
    {
        for( x=0; x<20; ++x )
        {
            buffer[ (y*20 + x)*4     ] = 0x64;
            buffer[ (y*20 + x)*4 + 1 ] = 0x64;
            buffer[ (y*20 + x)*4 + 2 ] = 0x64;
            buffer[ (y*20 + x)*4 + 3 ] = 0xFF;
        }
    }

    for( x=0; x<20; ++x )
    {
        buffer[ x*4     ] = buffer[ x*20*4     ] = 0x00;
        buffer[ x*4 + 1 ] = buffer[ x*20*4 + 1 ] = 0x00;
        buffer[ x*4 + 2 ] = buffer[ x*20*4 + 2 ] = 0x00;
        buffer[ x*4 + 3 ] = buffer[ x*20*4 + 3 ] = 0xFF;
    }

    for( x=0; x<20; ++x )
    {
        buffer[ (19*20+x)*4     ] = buffer[ (x*20+19)*4     ] = 0xFF;
        buffer[ (19*20+x)*4 + 1 ] = buffer[ (x*20+19)*4 + 1 ] = 0xFF;
        buffer[ (19*20+x)*4 + 2 ] = buffer[ (x*20+19)*4 + 2 ] = 0xFF;
        buffer[ (19*20+x)*4 + 3 ] = buffer[ (x*20+19)*4 + 3 ] = 0xFF;
    }

    for( y=0; y<6; ++y )
    {
        for( x=0; x<(11-2*y); ++x )
        {
            buffer[ ((7+y-1)*20 + 5+y+x-1)*4     ] = 0xFF;
            buffer[ ((7+y-1)*20 + 5+y+x-1)*4 + 1 ] = 0xFF;
            buffer[ ((7+y-1)*20 + 5+y+x-1)*4 + 2 ] = 0xFF;
            buffer[ ((7+y-1)*20 + 5+y+x-1)*4 + 3 ] = 0xFF;
        }
    }

    sgui_pixmap_load( pixmap, 68, 0, buffer, 0, 0, 20, 20, 20, SGUI_RGBA8 );

    /* scroll bar up button */
    for( y=0; y<20; ++y )
    {
        for( x=0; x<20; ++x )
        {
            buffer[ (y*20 + x)*4     ] = 0x64;
            buffer[ (y*20 + x)*4 + 1 ] = 0x64;
            buffer[ (y*20 + x)*4 + 2 ] = 0x64;
            buffer[ (y*20 + x)*4 + 3 ] = 0xFF;
        }
    }

    for( x=0; x<20; ++x )
    {
        buffer[ x*4     ] = buffer[ x*20*4     ] = 0xFF;
        buffer[ x*4 + 1 ] = buffer[ x*20*4 + 1 ] = 0xFF;
        buffer[ x*4 + 2 ] = buffer[ x*20*4 + 2 ] = 0xFF;
        buffer[ x*4 + 3 ] = buffer[ x*20*4 + 3 ] = 0xFF;
    }

    for( x=0; x<20; ++x )
    {
        buffer[ (19*20+x)*4     ] = buffer[ (x*20+19)*4     ] = 0x00;
        buffer[ (19*20+x)*4 + 1 ] = buffer[ (x*20+19)*4 + 1 ] = 0x00;
        buffer[ (19*20+x)*4 + 2 ] = buffer[ (x*20+19)*4 + 2 ] = 0x00;
        buffer[ (19*20+x)*4 + 3 ] = buffer[ (x*20+19)*4 + 3 ] = 0xFF;
    }

    for( y=0; y<6; ++y )
    {
        for( x=0; x<(11-2*y); ++x )
        {
            buffer[ ((12-y)*20 + 5+y+x)*4     ] = 0xFF;
            buffer[ ((12-y)*20 + 5+y+x)*4 + 1 ] = 0xFF;
            buffer[ ((12-y)*20 + 5+y+x)*4 + 2 ] = 0xFF;
            buffer[ ((12-y)*20 + 5+y+x)*4 + 3 ] = 0xFF;
        }
    }

    sgui_pixmap_load( pixmap, 88, 0, buffer, 0, 0, 20, 20, 20, SGUI_RGBA8 );

    /* scroll bar up button pressed */
    for( y=0; y<20; ++y )
    {
        for( x=0; x<20; ++x )
        {
            buffer[ (y*20 + x)*4     ] = 0x64;
            buffer[ (y*20 + x)*4 + 1 ] = 0x64;
            buffer[ (y*20 + x)*4 + 2 ] = 0x64;
            buffer[ (y*20 + x)*4 + 3 ] = 0xFF;
        }
    }

    for( x=0; x<20; ++x )
    {
        buffer[ x*4     ] = buffer[ x*20*4     ] = 0x00;
        buffer[ x*4 + 1 ] = buffer[ x*20*4 + 1 ] = 0x00;
        buffer[ x*4 + 2 ] = buffer[ x*20*4 + 2 ] = 0x00;
        buffer[ x*4 + 3 ] = buffer[ x*20*4 + 3 ] = 0xFF;
    }

    for( x=0; x<20; ++x )
    {
        buffer[ (19*20+x)*4     ] = buffer[ (x*20+19)*4     ] = 0xFF;
        buffer[ (19*20+x)*4 + 1 ] = buffer[ (x*20+19)*4 + 1 ] = 0xFF;
        buffer[ (19*20+x)*4 + 2 ] = buffer[ (x*20+19)*4 + 2 ] = 0xFF;
        buffer[ (19*20+x)*4 + 3 ] = buffer[ (x*20+19)*4 + 3 ] = 0xFF;
    }

    for( y=0; y<6; ++y )
    {
        for( x=0; x<(11-2*y); ++x )
        {
            buffer[ ((12-y-1)*20 + 5+y+x-1)*4     ] = 0xFF;
            buffer[ ((12-y-1)*20 + 5+y+x-1)*4 + 1 ] = 0xFF;
            buffer[ ((12-y-1)*20 + 5+y+x-1)*4 + 2 ] = 0xFF;
            buffer[ ((12-y-1)*20 + 5+y+x-1)*4 + 3 ] = 0xFF;
        }
    }

    sgui_pixmap_load( pixmap, 108, 0, buffer, 0, 0, 20, 20, 20, SGUI_RGBA8 );

    /* scroll bar left button */
    for( y=0; y<20; ++y )
    {
        for( x=0; x<20; ++x )
        {
            buffer[ (y*20 + x)*4     ] = 0x64;
            buffer[ (y*20 + x)*4 + 1 ] = 0x64;
            buffer[ (y*20 + x)*4 + 2 ] = 0x64;
            buffer[ (y*20 + x)*4 + 3 ] = 0xFF;
        }
    }

    for( x=0; x<20; ++x )
    {
        buffer[ x*4     ] = buffer[ x*20*4     ] = 0xFF;
        buffer[ x*4 + 1 ] = buffer[ x*20*4 + 1 ] = 0xFF;
        buffer[ x*4 + 2 ] = buffer[ x*20*4 + 2 ] = 0xFF;
        buffer[ x*4 + 3 ] = buffer[ x*20*4 + 3 ] = 0xFF;
    }

    for( x=0; x<20; ++x )
    {
        buffer[ (19*20+x)*4     ] = buffer[ (x*20+19)*4     ] = 0x00;
        buffer[ (19*20+x)*4 + 1 ] = buffer[ (x*20+19)*4 + 1 ] = 0x00;
        buffer[ (19*20+x)*4 + 2 ] = buffer[ (x*20+19)*4 + 2 ] = 0x00;
        buffer[ (19*20+x)*4 + 3 ] = buffer[ (x*20+19)*4 + 3 ] = 0xFF;
    }

    for( x=0; x<6; ++x )
    {
        for( y=0; y<(11-2*x); ++y )
        {
            buffer[ ((5+y+x)*20 + 12-x)*4     ] = 0xFF;
            buffer[ ((5+y+x)*20 + 12-x)*4 + 1 ] = 0xFF;
            buffer[ ((5+y+x)*20 + 12-x)*4 + 2 ] = 0xFF;
            buffer[ ((5+y+x)*20 + 12-x)*4 + 3 ] = 0xFF;
        }
    }

    sgui_pixmap_load( pixmap, 48, 20, buffer, 0, 0, 20, 20, 20, SGUI_RGBA8 );

    /* scroll bar left button pressed */
    for( y=0; y<20; ++y )
    {
        for( x=0; x<20; ++x )
        {
            buffer[ (y*20 + x)*4     ] = 0x64;
            buffer[ (y*20 + x)*4 + 1 ] = 0x64;
            buffer[ (y*20 + x)*4 + 2 ] = 0x64;
            buffer[ (y*20 + x)*4 + 3 ] = 0xFF;
        }
    }

    for( x=0; x<20; ++x )
    {
        buffer[ x*4     ] = buffer[ x*20*4     ] = 0x00;
        buffer[ x*4 + 1 ] = buffer[ x*20*4 + 1 ] = 0x00;
        buffer[ x*4 + 2 ] = buffer[ x*20*4 + 2 ] = 0x00;
        buffer[ x*4 + 3 ] = buffer[ x*20*4 + 3 ] = 0xFF;
    }

    for( x=0; x<20; ++x )
    {
        buffer[ (19*20+x)*4     ] = buffer[ (x*20+19)*4     ] = 0xFF;
        buffer[ (19*20+x)*4 + 1 ] = buffer[ (x*20+19)*4 + 1 ] = 0xFF;
        buffer[ (19*20+x)*4 + 2 ] = buffer[ (x*20+19)*4 + 2 ] = 0xFF;
        buffer[ (19*20+x)*4 + 3 ] = buffer[ (x*20+19)*4 + 3 ] = 0xFF;
    }

    for( x=0; x<6; ++x )
    {
        for( y=0; y<(11-2*x); ++y )
        {
            buffer[ ((5+y+x-1)*20 + 12-x-1)*4     ] = 0xFF;
            buffer[ ((5+y+x-1)*20 + 12-x-1)*4 + 1 ] = 0xFF;
            buffer[ ((5+y+x-1)*20 + 12-x-1)*4 + 2 ] = 0xFF;
            buffer[ ((5+y+x-1)*20 + 12-x-1)*4 + 3 ] = 0xFF;
        }
    }

    sgui_pixmap_load( pixmap, 68, 20, buffer, 0, 0, 20, 20, 20, SGUI_RGBA8 );

    /* scroll bar right button */
    for( y=0; y<20; ++y )
    {
        for( x=0; x<20; ++x )
        {
            buffer[ (y*20 + x)*4     ] = 0x64;
            buffer[ (y*20 + x)*4 + 1 ] = 0x64;
            buffer[ (y*20 + x)*4 + 2 ] = 0x64;
            buffer[ (y*20 + x)*4 + 3 ] = 0xFF;
        }
    }

    for( x=0; x<20; ++x )
    {
        buffer[ x*4     ] = buffer[ x*20*4     ] = 0xFF;
        buffer[ x*4 + 1 ] = buffer[ x*20*4 + 1 ] = 0xFF;
        buffer[ x*4 + 2 ] = buffer[ x*20*4 + 2 ] = 0xFF;
        buffer[ x*4 + 3 ] = buffer[ x*20*4 + 3 ] = 0xFF;
    }

    for( x=0; x<20; ++x )
    {
        buffer[ (19*20+x)*4     ] = buffer[ (x*20+19)*4     ] = 0x00;
        buffer[ (19*20+x)*4 + 1 ] = buffer[ (x*20+19)*4 + 1 ] = 0x00;
        buffer[ (19*20+x)*4 + 2 ] = buffer[ (x*20+19)*4 + 2 ] = 0x00;
        buffer[ (19*20+x)*4 + 3 ] = buffer[ (x*20+19)*4 + 3 ] = 0xFF;
    }

    for( x=0; x<6; ++x )
    {
        for( y=0; y<(11-2*x); ++y )
        {
            buffer[ ((5+y+x)*20 + 7+x)*4     ] = 0xFF;
            buffer[ ((5+y+x)*20 + 7+x)*4 + 1 ] = 0xFF;
            buffer[ ((5+y+x)*20 + 7+x)*4 + 2 ] = 0xFF;
            buffer[ ((5+y+x)*20 + 7+x)*4 + 3 ] = 0xFF;
        }
    }

    sgui_pixmap_load( pixmap, 88, 20, buffer, 0, 0, 20, 20, 20, SGUI_RGBA8 );

    /* scroll bar right button pressed */
    for( y=0; y<20; ++y )
    {
        for( x=0; x<20; ++x )
        {
            buffer[ (y*20 + x)*4     ] = 0x64;
            buffer[ (y*20 + x)*4 + 1 ] = 0x64;
            buffer[ (y*20 + x)*4 + 2 ] = 0x64;
            buffer[ (y*20 + x)*4 + 3 ] = 0xFF;
        }
    }

    for( x=0; x<20; ++x )
    {
        buffer[ x*4     ] = buffer[ x*20*4     ] = 0x00;
        buffer[ x*4 + 1 ] = buffer[ x*20*4 + 1 ] = 0x00;
        buffer[ x*4 + 2 ] = buffer[ x*20*4 + 2 ] = 0x00;
        buffer[ x*4 + 3 ] = buffer[ x*20*4 + 3 ] = 0xFF;
    }

    for( x=0; x<20; ++x )
    {
        buffer[ (19*20+x)*4     ] = buffer[ (x*20+19)*4     ] = 0xFF;
        buffer[ (19*20+x)*4 + 1 ] = buffer[ (x*20+19)*4 + 1 ] = 0xFF;
        buffer[ (19*20+x)*4 + 2 ] = buffer[ (x*20+19)*4 + 2 ] = 0xFF;
        buffer[ (19*20+x)*4 + 3 ] = buffer[ (x*20+19)*4 + 3 ] = 0xFF;
    }

    for( x=0; x<6; ++x )
    {
        for( y=0; y<(11-2*x); ++y )
        {
            buffer[ ((5+y+x-1)*20 + 7+x-1)*4     ] = 0xFF;
            buffer[ ((5+y+x-1)*20 + 7+x-1)*4 + 1 ] = 0xFF;
            buffer[ ((5+y+x-1)*20 + 7+x-1)*4 + 2 ] = 0xFF;
            buffer[ ((5+y+x-1)*20 + 7+x-1)*4 + 3 ] = 0xFF;
        }
    }

    sgui_pixmap_load( pixmap, 108, 20, buffer, 0, 0, 20, 20, 20, SGUI_RGBA8 );

    /* horizontal progress bar */
    for( y=0; y<29; ++y )
    {
        buffer[ y*4 ] = buffer[ y*4+1 ] = buffer[ y*4+2 ] = 0x00;
        buffer[ y*4+3 ] = 0xFF;
    }

    buffer[ y*4 ] = buffer[ y*4+1 ] = buffer[ y*4+2 ] = 0xFF;
    buffer[ y*4+3 ] = 0xFF;

    sgui_pixmap_load( pixmap, 0, 12, buffer, 0, 0, 1, 30, 1, SGUI_RGBA8 );

    for( x=0; x<12; ++x )
    {
        for( y=0; y<30; ++y )
        {
            buffer[(y*12+x)*4]=buffer[(y*12+x)*4+1]=buffer[(y*12+x)*4+2]=0x00;
            buffer[ (y*12 + x)*4 + 3 ] = 0x80;
        }
    }

    for( x=0; x<12; ++x )
    {
        buffer[ x*4 ] = buffer[ x*4 + 1 ] = buffer[ x*4 + 2 ] = 0x00;
        buffer[ x*4 + 3 ] = 0xFF;
    }

    for( x=0; x<12; ++x )
    {
        buffer[(29*12+x)*4]=buffer[(29*12+x)*4+1]=buffer[(29*12+x)*4+2]=0xFF;
        buffer[(29*12+x)*4+3] = 0xFF;
    }

    sgui_pixmap_load( pixmap, 1, 12, buffer, 0, 0, 12, 30, 12, SGUI_RGBA8 );

    for( x=0; x<7; ++x )
    {
        for( y=0; y<20; ++y )
        {
            buffer[((y+5)*12+x)*4  ] = buffer[((y+5)*12+x)*4+1] = 0xFF;
            buffer[((y+5)*12+x)*4+2] = 0xFF;
            buffer[((y+5)*12+x)*4+3] = 0xFF;
        }
    }

    sgui_pixmap_load( pixmap, 13, 12, buffer, 0, 0, 12, 30, 12, SGUI_RGBA8 );

    for( x=0; x<12; ++x )
    {
        for( y=0; y<28; ++y )
        {
            buffer[((y+1)*12+x)*4]=buffer[((y+1)*12+x)*4+1]=0xFF;
            buffer[((y+1)*12+x)*4+2]=0x00;
            buffer[((y+1)*12+x)*4+3]=0xFF;
        }
    }

    sgui_pixmap_load( pixmap, 26, 12, buffer, 0, 0, 12, 30, 12, SGUI_RGBA8 );

    for( y=0; y<30; ++y )
    {
        buffer[y*4] = buffer[y*4+1] = buffer[y*4+2] = buffer[y*4+3] = 0xFF;
    }

    sgui_pixmap_load( pixmap, 25, 12, buffer, 0, 0, 1, 30, 1, SGUI_RGBA8 );

    /* vertical progress bar */
    for( y=0; y<29; ++y )
    {
        buffer[ y*4 ] = buffer[ y*4+1 ] = buffer[ y*4+2 ] = 0x00;
        buffer[ y*4+3 ] = 0xFF;
    }

    buffer[ y*4 ] = buffer[ y*4+1 ] = buffer[ y*4+2 ] = 0xFF;
    buffer[ y*4+3 ] = 0xFF;

    sgui_pixmap_load( pixmap, 0, 42, buffer, 0, 0, 30, 1, 30, SGUI_RGBA8 );

    for( x=0; x<30; ++x )
    {
        for( y=0; y<12; ++y )
        {
            buffer[(y*30+x)*4]=buffer[(y*30+x)*4+1]=buffer[(y*30+x)*4+2]=0x00;
            buffer[ (y*30 + x)*4 + 3 ] = 0x80;
        }
    }

    for( y=0; y<12; ++y )
    {
        buffer[ y*30*4 ] = buffer[ y*30*4 + 1 ] = buffer[ y*30*4 + 2 ] = 0x00;
        buffer[ y*30*4 + 3 ] = 0xFF;
    }

    for( y=0; y<12; ++y )
    {
        buffer[(y*30+29)*4]=buffer[(y*30+29)*4+1]=buffer[(y*30+29)*4+2]=0xFF;
        buffer[ (y*30+29)*4 + 3 ] = 0xFF;
    }

    sgui_pixmap_load( pixmap, 0, 43, buffer, 0, 0, 30, 12, 30, SGUI_RGBA8 );

    for( y=0; y<7; ++y )
    {
        for( x=0; x<20; ++x )
        {
            buffer[(y*30+x+5)*4  ] = buffer[(y*30+x+5)*4+1] = 0xFF;
            buffer[(y*30+x+5)*4+2] = buffer[(y*30+x+5)*4+3] = 0xFF;
        }
    }

    sgui_pixmap_load( pixmap, 0, 55, buffer, 0, 0, 30, 12, 30, SGUI_RGBA8 );

    for( x=0; x<28; ++x )
    {
        for( y=0; y<12; ++y )
        {
            buffer[(y*30+x+1)*4]=buffer[(y*30+x+1)*4+1]=0xFF;
            buffer[(y*30+x+1)*4+2]=0x00;
            buffer[(y*30+x+1)*4+3]=0xFF;
        }
    }

    sgui_pixmap_load( pixmap, 0, 68, buffer, 0, 0, 30, 12, 30, SGUI_RGBA8 );

    for( x=0; x<30; ++x )
    {
        buffer[x*4] = buffer[x*4+1] = buffer[x*4+2] = buffer[x*4+3] = 0xFF;
    }

    sgui_pixmap_load( pixmap, 0, 67, buffer, 0, 0, 30, 1, 30, SGUI_RGBA8 );
}

void sgui_skin_set_default_font( sgui_font* normal, sgui_font* bold,
                                 sgui_font* italic, sgui_font* bold_italic )
{
    font_norm = normal;
    font_bold = bold;
    font_ital = italic;
    font_boit = bold_italic;
    font_height = sgui_font_get_height( font_norm );
}

void sgui_skin_get_window_background_color( unsigned char* color )
{
    color[0] = 0x64;
    color[1] = 0x64;
    color[2] = 0x64;
}

void sgui_skin_get_default_font_color( unsigned char* color )
{
    color[0] = 0xFF;
    color[1] = 0xFF;
    color[2] = 0xFF;
}

unsigned int sgui_skin_get_default_font_height( void )
{
    return font_height;
}

sgui_font* sgui_skin_get_default_font( int bold, int italic )
{
    if( bold && italic ) return font_boit;
    if( bold           ) return font_bold;
    if( italic         ) return font_ital;

    return font_norm;
}

unsigned int sgui_skin_default_font_extents( const char* text,
                                             unsigned int length,
                                             int bold, int italic )
{
    sgui_font* font_face = sgui_skin_get_default_font( bold, italic );
    unsigned int x = 0, w, len = 0, i;
    unsigned long character, previous = 0;

    /* sanity check */
    if( !text || !font_face || !length )
        return 0;

    /* for each character */
    for( i=0; i<length && (*text) && (*text!='\n'); text+=len, i+=len )
    {
        /* load the next glyph */
        character = sgui_utf8_decode( text, &len );
        sgui_font_load_glyph( font_face, character );

        /* advance cursor */
        x += sgui_font_get_kerning_distance( font_face, previous, character );
        sgui_font_get_glyph_metrics( font_face, &w, NULL, NULL );

        x += w + 1;

        /* store previous glyph character for kerning */
        previous = character;
    }

    return x;
}

void sgui_skin_get_text_extents( const char* text, sgui_rect* r )
{
    unsigned int X = 0, longest = 0, lines = 1, i = 0, font_stack_index = 0;
    char font_stack[10], f = 0;

    /* sanity check */
    if( !text || !r )
        return;

    for( ; text && text[ i ]; ++i )
    {
        if( text[ i ] == '<' )  /* we found a tag */
        {
            /* get extends for what we found so far */
            X += sgui_skin_default_font_extents( text, i, f&0x02, f&0x01 );

            /* change fonts accordingly */
            if( text[ i+1 ] == 'b' )
            {
                font_stack[ font_stack_index++ ] = f;
                f |= 0x02;
            }
            else if( text[ i+1 ] == 'i' )
            {
                font_stack[ font_stack_index++ ] = f;
                f |= 0x01;
            }
            else if( text[ i+1 ] == '/' && font_stack_index )
            {
                f = font_stack[ --font_stack_index ];
            }

            /* skip to tag end */
            text = strchr( text+i, '>' );

            if( text )
                ++text;

            /* reset i to -1, so it starts with 0 in the next iteration */
            i = -1;
        }
        else if( text[ i ] == '\n' )
        {
            /* get extends for what we found so far */
            X += sgui_skin_default_font_extents( text, i, f&0x02, f&0x01 );

            /* store the length of the longest line */
            if( X > longest )
                longest = X;

            ++lines;        /* increment line counter */
            text += i + 1;  /* skip to next line */
            i = -1;         /* restart with 0 at next iteration */
            X = 0;          /* move cursor back to the left */
        }
    }

    /* get the extents of what we didn't get so far */
    X += sgui_skin_default_font_extents( text, i, f&0x02, f&0x01 );

    if( X > longest )
        longest = X;

    /* Add font height/2 as fudge factor to the height, because our crude
       computation here does not take into account that characters can peek
       out below the line */
    r->left   = 0;
    r->top    = 0;
    r->right  = longest - 1;
    r->bottom = lines * font_height + font_height/2 - 1;
}

void sgui_skin_get_widget_extents( int type, sgui_rect* r )
{
    if( r )
    {
        r->left = r->right = r->top = r->bottom = 0;

        switch( type )
        {
        case SGUI_CHECKBOX:
        case SGUI_CHECKBOX_SELECTED:
            r->right = 19;
            r->bottom = 11;
            break;
        case SGUI_RADIO_BUTTON:
        case SGUI_RADIO_BUTTON_SELECTED:
            r->right = 19;
            r->bottom = 11;
            break;
        case SGUI_PROGRESS_BAR_V_STIPPLED:
        case SGUI_PROGRESS_BAR_V_FILLED:
            r->right = 29;
            break;
        case SGUI_PROGRESS_BAR_H_STIPPLED:
        case SGUI_PROGRESS_BAR_H_FILLED:
            r->bottom = 29;
            break;
        case SGUI_EDIT_BOX:
            r->bottom = (font_height + (font_height / 2) + 4);
            break;
        case SGUI_SCROLL_BAR_V:
            r->right = 19;
            break;
        case SGUI_SCROLL_BAR_H:
            r->bottom = 19;
            break;
        case SGUI_SCROLL_BAR_V_BUTTON:
        case SGUI_SCROLL_BAR_H_BUTTON:
            r->right = r->bottom = 19;
            break;
        case SGUI_TAB_CAPTION:
            r->right = 19;
            r->bottom = font_height + font_height / 2 - 1;
            break;
        case SGUI_FRAME_BORDER:
            r->left = r->right = r->top = r->bottom = 1;
            break;
        }
    }
}

/***************************************************************************/

void sgui_skin_draw_progress_bar( sgui_canvas* cv, sgui_rect* area,
                                  int type, unsigned int value )
{
    sgui_pixmap* skin_pixmap;
    sgui_rect r, stretch;
    int w, h;

    if( !cv || !area )
        return;

    w = SGUI_RECT_WIDTH_V( area );
    h = SGUI_RECT_HEIGHT_V( area );

    skin_pixmap = sgui_canvas_get_skin_pixmap( cv );
    sgui_rect_copy( &stretch, area );

    /* draw bar */
    if( type==SGUI_PROGRESS_BAR_V_STIPPLED )
    {
        h = ((h-10)*(100-value)) / 100;

        /* draw top end of progress bar */
        sgui_rect_set_size( &r, 0, 42, 30, 5 );
        sgui_canvas_blend( cv, area->left, area->top, skin_pixmap, &r );

        /* draw stipples */
        if( h >= 12 )
        {
            sgui_rect_set_size( &r, 0, 50, 30, 12 );
            stretch.bottom -= 5;
            stretch.top = stretch.bottom - h + h%12;
            sgui_canvas_stretch_blend( cv, skin_pixmap, &r, &stretch, 1 );
        }

        /* draw remaining empty section */
        sgui_rect_set_size( &r, 0, 43, 30, 12 );
        stretch.bottom = stretch.top - 1;
        stretch.top = area->top + 5;
        sgui_canvas_stretch_blend( cv, skin_pixmap, &r, &stretch, 0 );

        /* draw right end of progress bar */
        sgui_rect_set_size( &r, 0, 63, 30, 5 );
        sgui_canvas_blend(cv, area->left, area->bottom-5,
                          skin_pixmap, &r);
    }
    else if( type==SGUI_PROGRESS_BAR_V_FILLED )
    {
        h = ((h-2)*(100-value)) / 100;

        /* draw top end of progress bar */
        sgui_rect_set_size( &r, 0, 42, 30, 1 );
        sgui_canvas_blend( cv, area->left, area->top, skin_pixmap, &r );

        /* draw filled area */
        sgui_rect_set_size( &r, 0, 68, 30, 12 );
        stretch.bottom -= 2;
        stretch.top = stretch.bottom - h;
        sgui_canvas_stretch_blend( cv, skin_pixmap, &r, &stretch, 0 );

        /* draw remaining empty section */
        sgui_rect_set_size( &r, 0, 43, 30, 12 );
        stretch.bottom = stretch.top - 1;
        stretch.top = area->top + 1;
        sgui_canvas_stretch_blend( cv, skin_pixmap, &r, &stretch, 0 );

        /* draw right end of progress bar */
        sgui_rect_set_size( &r, 0, 67, 30, 1 );
        sgui_canvas_blend(cv, area->left, area->bottom-1,
                          skin_pixmap, &r);
    }
    else if( type==SGUI_PROGRESS_BAR_H_STIPPLED )
    {
        w = ((w-10)*(100-value)) / 100;

        /* draw left end of progress bar */
        sgui_rect_set_size( &r, 0, 12, 5, 30 );
        sgui_canvas_blend( cv, area->left, area->top, skin_pixmap, &r );

        /* draw stipples */
        if( w >= 12 )
        {
            sgui_rect_set_size( &r, 13, 12, 12, 30 );
            stretch.left += 5;
            stretch.right = stretch.left + w - w%12;
            sgui_canvas_stretch_blend( cv, skin_pixmap, &r, &stretch, 1 );
        }

        /* draw remaining empty section */
        sgui_rect_set_size( &r, 1, 12, 12, 30 );
        stretch.left = stretch.right;
        stretch.right = area->right - 6;
        sgui_canvas_stretch_blend( cv, skin_pixmap, &r, &stretch, 0 );

        /* draw right end of progress bar */
        sgui_rect_set_size( &r, 21, 12, 5, 30 );
        sgui_canvas_blend(cv, area->right-5, area->top, skin_pixmap, &r);
    }
    else
    {
        w = ((w-2)*(100-value)) / 100;

        /* draw left end of progress bar */
        sgui_rect_set_size( &r, 0, 12, 1, 30 );
        sgui_canvas_blend( cv, area->left, area->top, skin_pixmap, &r );

        /* draw filled area */
        sgui_rect_set_size( &r, 26, 12, 12, 30 );
        stretch.left += 1;
        stretch.right = stretch.left + w;
        sgui_canvas_stretch_blend( cv, skin_pixmap, &r, &stretch, 0 );

        /* draw empty area */
        sgui_rect_set_size( &r, 1, 12, 12, 30 );
        stretch.left = stretch.right+1;
        stretch.right = area->right-1;
        sgui_canvas_stretch_blend( cv, skin_pixmap, &r, &stretch, 0 );

        /* draw right end of progress bar */
        sgui_rect_set_size( &r, 25, 12, 1, 30 );
        sgui_canvas_blend(cv, area->right-1, area->top, skin_pixmap, &r);
    }
}

void sgui_skin_draw_button( sgui_canvas* cv, sgui_rect* area, int type )
{
    sgui_pixmap* skin_pixmap;
    unsigned char c[4];
    unsigned int w, h;
    int x, y, x1, y1;
    sgui_rect r;

    x = area->left;
    y = area->top;
    x1 = area->right;
    y1 = area->bottom;
    skin_pixmap = sgui_canvas_get_skin_pixmap( cv );

    if( type==SGUI_CHECKBOX || type==SGUI_CHECKBOX_SELECTED )
    {
        if( type==SGUI_CHECKBOX_SELECTED )
            sgui_rect_set_size( &r, 12, 0, 12, 12 );
        else
            sgui_rect_set_size( &r, 0, 0, 12, 12 );

        sgui_canvas_blend( cv, x, y + font_height/4, skin_pixmap, &r );
    }
    else if( type==SGUI_RADIO_BUTTON || type==SGUI_RADIO_BUTTON_SELECTED )
    {
        if( type==SGUI_RADIO_BUTTON_SELECTED )
            sgui_rect_set_size( &r, 36, 0, 12, 12 );
        else
            sgui_rect_set_size( &r, 24, 0, 12, 12 );

        sgui_canvas_blend( cv, x, y + font_height/4, skin_pixmap, &r );
    }
    else
    {
        w = SGUI_RECT_WIDTH_V( area );
        h = SGUI_RECT_HEIGHT_V( area );

        c[0] = c[1] = c[2] = type==SGUI_BUTTON_SELECTED ? 0x00 : 0xFF;
        sgui_canvas_draw_line( cv, x, y, w, 1, c, SGUI_RGB8 );
        sgui_canvas_draw_line( cv, x, y, h, 0, c, SGUI_RGB8 );

        c[0] = c[1] = c[2] = type==SGUI_BUTTON_SELECTED ? 0xFF : 0x00;
        sgui_canvas_draw_line( cv, x,  y1, w, 1, c, SGUI_RGB8 );
        sgui_canvas_draw_line( cv, x1, y,  h, 0, c, SGUI_RGB8 );
    }
}

void sgui_skin_draw_edit_box( sgui_canvas* cv, sgui_rect* area,
                              const char* text, int cursor )
{
    unsigned int cx, width, height;
    unsigned char c[4];

    if( !cv || !area )
        return;

    width = SGUI_RECT_WIDTH_V( area );
    height = SGUI_RECT_HEIGHT_V( area );

    /* draw background box */
    c[0] = c[1] = c[2] = 0x00; c[3] = 0x80;
    sgui_canvas_draw_box( cv, area, c, SGUI_RGBA8 );

    /* draw text */
    c[0] = c[1] = c[2] = c[3] = 0xFF;
    sgui_canvas_draw_text_plain(cv,area->left+2,area->top+2,0,0,c,text,-1);

    /* draw borders */
    c[0] = c[1] = c[2] = 0x00;
    sgui_canvas_draw_line(cv, area->left, area->top, width,  1, c, SGUI_RGB8);
    sgui_canvas_draw_line(cv, area->left, area->top, height, 0, c, SGUI_RGB8);

    c[0] = c[1] = c[2] = 0xFF;
    sgui_canvas_draw_line(cv,area->left, area->bottom,width, 1,c,SGUI_RGB8);
    sgui_canvas_draw_line(cv,area->right,area->top,   height,0,c,SGUI_RGB8);

    /* draw cursor */
    if( cursor >= 0 )
    {
        cx = sgui_skin_default_font_extents( text, cursor, 0, 0 );

        if( cx == 0 )
            cx = 3;

        if( cx < (width-2) )
        {
            c[0] = c[1] = c[2] = 0x7F;
            sgui_canvas_draw_line( cv, area->left+cx, area->top+5,
                                   height-10, 0, c, SGUI_RGB8 );
        }
    }
}

void sgui_skin_draw_frame( sgui_canvas* cv, sgui_rect* area )
{
    unsigned char c[4] = { 0x00, 0x00, 0x00, 0x80 };
    unsigned int w, h;

    if( !cv || !area )
        return;

    w = SGUI_RECT_WIDTH_V( area );
    h = SGUI_RECT_HEIGHT_V( area );

    sgui_canvas_draw_box( cv, area, c, SGUI_RGBA8 );
    sgui_canvas_draw_line( cv, area->left, area->top, w, 1, c, SGUI_RGB8 );
    sgui_canvas_draw_line( cv, area->left, area->top, h, 0, c, SGUI_RGB8 );

    c[0] = c[1] = c[2] = 0xFF;
    sgui_canvas_draw_line(cv, area->left,  area->bottom, w, 1, c, SGUI_RGB8);
    sgui_canvas_draw_line(cv, area->right, area->top,    h, 0, c, SGUI_RGB8);
}

void sgui_skin_draw_scroll_bar( sgui_canvas* cv, int x, int y,
                                int horizontal, unsigned int length,
                                unsigned int p_offset, unsigned int p_length,
                                int inc_button_state, int dec_button_state )
{
    sgui_pixmap* skin_pixmap;
    unsigned char color[4] = { 0x64, 0x64, 0x64, 0xFF };
    sgui_rect r;

    skin_pixmap = sgui_canvas_get_skin_pixmap( cv );

    if( horizontal )
    {
        sgui_rect_set_size( &r, x, y, length, 20 );
        sgui_canvas_draw_box( cv, &r, color, SGUI_RGB8 );

        /* pane */
        color[0] = color[1] = color[2] = 0xFF;
        sgui_rect_set_size( &r, x+20+p_offset, y, p_length, 20 );
        sgui_skin_draw_button( cv, &r, SGUI_BUTTON );

        /* left button */
        sgui_rect_set_size( &r, dec_button_state ? 68 : 48, 20, 20, 20 );
        sgui_canvas_blend( cv, x, y, skin_pixmap, &r );

        /* right button */
        sgui_rect_set_size( &r, inc_button_state ? 108 : 88, 20, 20, 20 );
        sgui_canvas_blend( cv, x+length-20, y, skin_pixmap, &r );
    }
    else
    {
        sgui_rect_set_size( &r, x, y, 20, length );
        sgui_canvas_draw_box( cv, &r, color, SGUI_RGB8 );

        /* pane */
        color[0] = color[1] = color[2] = 0xFF;
        sgui_rect_set_size( &r, x, y+20+p_offset, 20, p_length );
        sgui_skin_draw_button( cv, &r, SGUI_BUTTON );

        /* upper button */
        sgui_rect_set_size( &r, dec_button_state ? 108 : 88, 0, 20, 20 );
        sgui_canvas_blend( cv, x, y, skin_pixmap, &r );

        /* lower button */
        sgui_rect_set_size( &r, inc_button_state ? 68 : 48, 0, 20, 20 );
        sgui_canvas_blend( cv, x, y+length-20, skin_pixmap, &r );
    }
}

void sgui_skin_draw_group_box( sgui_canvas* cv, sgui_rect* area,
                               const char* caption )
{
    unsigned char color[3] = { 0xFF, 0xFF, 0xFF };
    unsigned int len;
    int x, y, x1, y1;

    if( !area || !cv )
        return;

    x   = area->left;
    y   = area->top;
    x1  = area->right;
    y1  = area->bottom;
    len = sgui_skin_default_font_extents( caption, -1, 0, 0 );

    sgui_canvas_draw_text_plain( cv, x+13, y, 0, 0, color, caption, -1 );
    y += font_height/2;

    /* the top line has a gap for the caption */
    sgui_canvas_draw_line(cv,x+1,     y+1, 9,           1, color, SGUI_RGB8);
    sgui_canvas_draw_line(cv,x+16+len,y+1, x1-x-15-len, 1, color, SGUI_RGB8);

    /* left, right and bottom lines */
    sgui_canvas_draw_line(cv, x+1, y+1, y1-y, 0, color, SGUI_RGB8);
    sgui_canvas_draw_line(cv, x1,  y+1, y1-y, 0, color, SGUI_RGB8);
    sgui_canvas_draw_line(cv, x+1, y1,  x1-x, 1, color, SGUI_RGB8);

    color[0] = color[1] = color[2] = 0x00;

    /* again, a gap for the caption */
    sgui_canvas_draw_line(cv, x,        y,          10, 1, color, SGUI_RGB8);
    sgui_canvas_draw_line(cv, x+16+len, y, x1-x-16-len, 1, color, SGUI_RGB8);

    /* left right and bottom lines */
    sgui_canvas_draw_line( cv, x,    y, y1-y, 0, color, SGUI_RGB8 );
    sgui_canvas_draw_line( cv, x1-1, y, y1-y, 0, color, SGUI_RGB8 );
    sgui_canvas_draw_line( cv, x, y1-1, x1-x, 1, color, SGUI_RGB8 );
}

void sgui_skin_draw_tab_caption( sgui_canvas* cv, int x, int y,
                                 unsigned int width, const char* caption )
{
    unsigned char color[4] = { 0xFF, 0xFF, 0xFF, 0xFF };
    unsigned int h = font_height + font_height / 2;

    sgui_canvas_draw_line( cv, x, y, width, 1, color, SGUI_RGB8 );
    sgui_canvas_draw_line( cv, x, y, h,     0, color, SGUI_RGB8 );

    color[0] = color[1] = color[2] = 0x00;
    sgui_canvas_draw_line( cv, x+width-1, y, h, 0, color, SGUI_RGB8 );

    color[0] = color[1] = color[2] = 0xFF;
    sgui_canvas_draw_text_plain( cv, x+10, y, 0, 0, color, caption, -1 );
}

void sgui_skin_draw_tab( sgui_canvas* cv, int x, int y, unsigned int width,
                         unsigned int height, unsigned int gap,
                         unsigned int gap_width )
{
    unsigned char color[4] = { 0xFF, 0xFF, 0xFF, 0xFF };
    unsigned int w_remain = width - gap - gap_width;
    int x_after = x + gap + gap_width;

    sgui_canvas_draw_line( cv, x,       y, gap+1,     1, color, SGUI_RGB8 );
    sgui_canvas_draw_line( cv, x_after, y, w_remain,  1, color, SGUI_RGB8 );
    sgui_canvas_draw_line( cv, x,       y, height,    0, color, SGUI_RGB8 );

    color[0] = color[1] = color[2] = 0x00;
    sgui_canvas_draw_line( cv, x, y+height-1, width, 1, color, SGUI_RGB8 );
    sgui_canvas_draw_line( cv, x+width-1, y, height, 0, color, SGUI_RGB8 );
}

