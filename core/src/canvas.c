/*
 * canvas.c
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
#include "sgui_canvas.h"
#include "sgui_internal.h"
#include "sgui_utf8.h"
#include "sgui_font.h"
#include "sgui_skin.h"
#include "sgui_pixmap.h"

#include <string.h>
#include <stdlib.h>



#define COPY_RECT_OFFSET( dst, src )\
        dst.left = src->left + canvas->ox;\
        dst.right = src->right + canvas->ox;\
        dst.top = src->top + canvas->oy;\
        dst.bottom = src->bottom + canvas->oy;



void sgui_internal_canvas_init( sgui_canvas* cv, unsigned int width,
                                unsigned int height )
{
    cv->allow_clear = 1;
    cv->ox = cv->oy = 0;

    cv->width = width;
    cv->height = height;
    cv->began = 0;

    cv->bg_color[0] = 0;
    cv->bg_color[1] = 0;
    cv->bg_color[2] = 0;
}




void sgui_canvas_destroy( sgui_canvas* canvas )
{
    if( canvas )
        canvas->destroy( canvas );
}

void sgui_canvas_resize( sgui_canvas* canvas, unsigned int width,
                         unsigned int height )
{
    if( canvas && (canvas->width!=width || canvas->height!=height) )
    {
        canvas->resize( canvas, width, height );

        canvas->width = width;
        canvas->height = height;
    }
}




void sgui_canvas_get_size( sgui_canvas* canvas, unsigned int* width,
                           unsigned int* height )
{
    if( width  ) *width  = canvas ? canvas->width  : 0;
    if( height ) *height = canvas ? canvas->height : 0;
}

void sgui_canvas_set_background_color( sgui_canvas* canvas,
                                       unsigned char* color )
{
    if( canvas && color )
    {
        canvas->bg_color[0] = color[0];
        canvas->bg_color[1] = color[1];
        canvas->bg_color[2] = color[2];
    }
}

void sgui_canvas_begin( sgui_canvas* canvas, sgui_rect* r )
{
    sgui_rect r0;

    if( canvas && r )
    {
        if( !canvas->began )
        {
            sgui_rect_copy( &r0, r );

            /* clip region to canvas size */
            if( r0.left < 0 )
                r0.left = 0;

            if( r0.top < 0 )
                r0.top = 0;

            if( r0.right >= (int)canvas->width )
                r0.right = canvas->width - 1;

            if( r0.bottom >= (int)canvas->height )
                r0.bottom = canvas->height - 1;

            /* tell the implementation to begin drawing */
            canvas->begin( canvas, &r0 );
            canvas->sc = r0;
        }

        ++canvas->began;
    }
}

void sgui_canvas_end( sgui_canvas* canvas )
{
    if( canvas )
    {
        if( canvas->began )
            --canvas->began;

        if( !canvas->began )
            canvas->end( canvas );
    }
}

void sgui_canvas_clear( sgui_canvas* canvas, sgui_rect* r )
{
    sgui_rect r1;

    if( !canvas || !canvas->allow_clear )
        return;

    /* if no rect is given, set to the full canvas area */
    if( r )
    {
        COPY_RECT_OFFSET( r1, r );
    }
    else
    {
        sgui_rect_set_size( &r1, 0, 0, canvas->width, canvas->height );
    }

    /* prepare the clearing rect */
    if( canvas->began )
    {
        if( !sgui_rect_get_intersection( &r1, &canvas->sc, &r1 ) )
            return;
    }
    else
    {
        if( r1.left   <  0                   ) r1.left   = 0;
        if( r1.top    <  0                   ) r1.top    = 0;
        if( r1.right  >= (int)canvas->width  ) r1.right  = canvas->width - 1;
        if( r1.bottom >= (int)canvas->height ) r1.bottom = canvas->height - 1;
    }

    canvas->clear( canvas, &r1 );
}

void sgui_canvas_set_scissor_rect( sgui_canvas* canvas, sgui_rect* r )
{
    if( canvas && r )
    {
        sgui_rect_copy( &canvas->sc, r );
    }
}

void sgui_canvas_get_scissor_rect( sgui_canvas* canvas, sgui_rect* r )
{
    if( canvas && r )
    {
        sgui_rect_copy( r, &canvas->sc );
    }
}

void sgui_canvas_merge_scissor_rect( sgui_canvas* canvas, sgui_rect* r )
{
    sgui_rect r1;

    if( canvas && canvas->began && r )
    {
        COPY_RECT_OFFSET( r1, r );
        sgui_rect_get_intersection( &canvas->sc, &canvas->sc, &r1 );
    }
}

void sgui_canvas_allow_clear( sgui_canvas* canvas, int clear )
{
    if( canvas )
        canvas->allow_clear = clear;
}

int sgui_canvas_is_clear_allowed( sgui_canvas* canvas )
{
    return canvas ? canvas->allow_clear : 0;
}

void sgui_canvas_set_offset( sgui_canvas* canvas, int x, int y )
{
    if( canvas && canvas->began )
    {
        canvas->ox = x;
        canvas->oy = y;
    }
}

void sgui_canvas_add_offset( sgui_canvas* canvas, int x, int y )
{
    if( canvas && canvas->began )
    {
        canvas->ox += x;
        canvas->oy += y;
    }
}

void sgui_canvas_get_offset( sgui_canvas* canvas, int* x, int* y )
{
    if( canvas && canvas->began )
    {
        if( x ) *x = canvas->ox;
        if( y ) *y = canvas->oy;
    }
    else
    {
        if( x ) *x = 0;
        if( y ) *y = 0;
    }
}

/**************************** drawing functions ****************************/

void sgui_canvas_blit( sgui_canvas* canvas, int x, int y,
                       sgui_pixmap* pixmap, sgui_rect* srcrect )
{
    unsigned int w, h;
    sgui_rect r, clip;

    /* sanity check */
    if( !canvas || !pixmap || !canvas->began )
        return;

    x += canvas->ox;
    y += canvas->oy;

    sgui_pixmap_get_size( pixmap, &w, &h );
    sgui_rect_set_size( &r, 0, 0, w, h );

    if( srcrect )
        sgui_rect_get_intersection( &r, &r, srcrect );

    /* get the scissor clipping rectangle in image local coordinates */
    clip = canvas->sc;
    clip.left -= x; clip.right  -= x;
    clip.top  -= y; clip.bottom -= y;

    /* clip the source rect against the image local scissor rectangle */
    sgui_rect_get_intersection( &r, &r, &clip );

    /* do the blitting */
    canvas->blit( canvas, x, y, pixmap, &r );
}

void sgui_canvas_blend( sgui_canvas* canvas, int x, int y,
                        sgui_pixmap* pixmap, sgui_rect* srcrect )
{
    unsigned int w, h;
    sgui_rect r, clip;

    /* sanity check */
    if( !canvas || !pixmap || !canvas->began )
        return;

    x += canvas->ox;
    y += canvas->oy;

    sgui_pixmap_get_size( pixmap, &w, &h );
    sgui_rect_set_size( &r, 0, 0, w, h );

    if( srcrect )
        sgui_rect_get_intersection( &r, &r, srcrect );

    /* get the scissor clipping rectangle in image local coordinates */
    clip = canvas->sc;
    clip.left -= x; clip.right  -= x;
    clip.top  -= y; clip.bottom -= y;

    /* clip the source rect against the image local scissor rectangle */
    sgui_rect_get_intersection( &r, &r, &clip );

    /* do the blending */
    canvas->blend( canvas, x, y, pixmap, &r );
}

void sgui_canvas_draw_box( sgui_canvas* canvas, sgui_rect* r,
                           unsigned char* color, int format )
{
    sgui_rect r1;

    /* sanity check */
    if( !canvas || !color || !canvas->began || !r )
        return;

    if( format==SGUI_RGBA8 && color[3]==0xFF )
        format = SGUI_RGB8;

    /* offset and clip the given rectangle */
    COPY_RECT_OFFSET( r1, r );

    if( !sgui_rect_get_intersection( &r1, &canvas->sc, &r1 ) )
        return;

    canvas->draw_box( canvas, &r1, color, format );
}

void sgui_canvas_draw_line( sgui_canvas* canvas, int x, int y,
                            unsigned int length, int horizontal,
                            unsigned char* color, int format )
{
    sgui_rect r;

    /* santiy check */
    if( !canvas || !canvas->began )
        return;

    if( horizontal )
        sgui_rect_set_size( &r, x, y, length, 1 );
    else
        sgui_rect_set_size( &r, x, y, 1, length );

    sgui_canvas_draw_box( canvas, &r, color, format );
}

int sgui_canvas_draw_text_plain( sgui_canvas* canvas, int x, int y,
                                 int bold, int italic,
                                 unsigned char* color,
                                 const char* text, unsigned int length )
{
    sgui_font* font = sgui_skin_get_default_font( bold, italic );

    /* sanity check */
    if( !canvas || !font || !color || !text || !canvas->began || !length )
        return 0;

    x += canvas->ox;
    y += canvas->oy;

    if( x>=canvas->sc.right || y>= canvas->sc.bottom )
        return 0;

    return canvas->draw_string( canvas, x, y, font, color, text, length );
}

void sgui_canvas_draw_text( sgui_canvas* canvas, int x, int y,
                            const char* text )
{
    int i = 0, X = 0, font_stack_index = 0, font_height;
    unsigned char col[3], font_stack[10], f = 0;
    long c;

    /* sanity check */
    if( !canvas || !text )
        return;

    sgui_skin_get_default_font_color( col );

    font_height = sgui_skin_get_default_font_height( );

    for( ; text && text[ i ]; ++i )
    {
        if( text[ i ] == '<' )  /* we encountered a tag */
        {
            /* draw what we got so far with the current settings */
            X += sgui_canvas_draw_text_plain( canvas, x+X, y, f & 0x02, f & 0x01,
                                              col, text, i );

            if( !strncmp( text+i+1, "color=", 6 ) ) /* it's a color tag */
            {
                if( !strncmp( text+i+9, "default", 7 ) )
                {
                    sgui_skin_get_default_font_color( col );
                }
                else
                {
                    c = strtol( text+i+9, NULL, 16 );

                    col[0] = (c>>16) & 0xFF;
                    col[1] = (c>>8 ) & 0xFF;
                    col[2] =  c      & 0xFF;
                }
            }
            else if( text[ i+1 ] == 'b' )   /* it's a <b> tag */
            {
                font_stack[ font_stack_index++ ] = f;
                f |= 0x02;
            }
            else if( text[ i+1 ] == 'i' )   /* it's an <i> tag */
            {
                font_stack[ font_stack_index++ ] = f;
                f |= 0x01;
            }
            else if( text[ i+1 ] == '/' && font_stack_index )   /* end tag */
            {
                f = font_stack[ --font_stack_index ];   /* pop from stack */
            }

            /* skip to the end of the tag */
            if( (text = strchr( text+i, '>' )) )
                ++text;

            i = -1; /* reset i to 0 at next iteration */
        }
        else if( text[ i ] == '\n' )
        {
            /* draw what we got so far */
            sgui_canvas_draw_text_plain( canvas, x+X, y, f&0x02, f&0x01,
                                         col, text, i );

            text += i + 1;    /* skip to next line */
            i = -1;           /* reset i to 0 at next iteration */
            X = 0;            /* adjust move cursor */
            y += font_height;
        }
    }

    /* draw what is still left */
    sgui_canvas_draw_text_plain(canvas, x+X, y, f&0x02, f&0x01, col, text, i);
}

