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

    cv->scissor_stack_pointer = 0;
    cv->offset_stack_pointer = 0;
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
            canvas->scissor_stack_pointer = 0;
            canvas->offset_stack_pointer = 0;

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
    sgui_rect r1;

    if( canvas && canvas->began )
    {
        if( r )
        {
            COPY_RECT_OFFSET( r1, r );

            if( canvas->scissor_stack_pointer == SGUI_CANVAS_STACK_DEPTH )
                return;

            /* push current scissor rect */
            canvas->sc_stack[ canvas->scissor_stack_pointer ] = canvas->sc;

            ++(canvas->scissor_stack_pointer);

            /* new scissor rect = intersection with given and existing */
            sgui_rect_get_intersection( &canvas->sc, &canvas->sc, &r1 );
        }
        else if( canvas->scissor_stack_pointer )    /* r=NULL means disable */
        {
            /* pop old scissor rect from stack */
            --(canvas->scissor_stack_pointer);

            canvas->sc = canvas->sc_stack[ canvas->scissor_stack_pointer ];
        }
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
        if( canvas->offset_stack_pointer == SGUI_CANVAS_STACK_DEPTH )
            return;

        /* push existing offset */
        canvas->offset_stack_x[ canvas->offset_stack_pointer ] = canvas->ox;
        canvas->offset_stack_y[ canvas->offset_stack_pointer ] = canvas->oy;

        ++(canvas->offset_stack_pointer);

        /* set new offset, offset by the existing one -> add */
        canvas->ox += x;
        canvas->oy += y;
    }
}

void sgui_canvas_restore_offset( sgui_canvas* canvas )
{
    if( canvas && canvas->began && canvas->offset_stack_pointer )
    {
        --(canvas->offset_stack_pointer);

        canvas->ox = canvas->offset_stack_x[ canvas->offset_stack_pointer ];
        canvas->oy = canvas->offset_stack_y[ canvas->offset_stack_pointer ];
    }
}

void sgui_canvas_blit( sgui_canvas* canvas, int x, int y, unsigned int width,
                       unsigned int height, SGUI_COLOR_FORMAT format,
                       const void* data )
{
    unsigned char* src;
    sgui_rect r, r0;

    /* sanity check */
    if( !canvas || !width || !height || !data || !canvas->began )
        return;

    /* get actual destination rect */
    sgui_rect_set_size( &r0, x+canvas->ox, y+canvas->oy, width, height );

    if( !sgui_rect_get_intersection( &r, &canvas->sc, &r0 ) )
        return;

    /* get a pointer to the first pixel, taking clipping into account */
    src = (unsigned char*)data +
          ((r.top-r0.top)*width + r.left-r0.left)*(format==SCF_RGBA8 ? 4 : 3);

    /* do the blitting */
    canvas->blit( canvas, r.left, r.top, r.right-r.left, r.bottom-r.top,
                  width, format, src );
}

void sgui_canvas_blend( sgui_canvas* canvas, int x, int y, unsigned int width,
                        unsigned int height, SGUI_COLOR_FORMAT format,
                        const void* data )
{
    unsigned char* src;
    sgui_rect r, r0;

    /* sanity check */
    if( !canvas || !width || !height || !data || format!=SCF_RGBA8 ||
        !canvas->began )
        return;

    /* get actual destination rect */
    sgui_rect_set_size( &r0, x+canvas->ox, y+canvas->oy, width, height );

    if( !sgui_rect_get_intersection( &r, &canvas->sc, &r0 ) )
        return;

    /* get a pointer to the first pixel, taking clipping into account */
    src = (unsigned char*)data + ((r.top-r0.top)*width + r.left-r0.left)*4;

    /* do the blending */
    canvas->blend( canvas, r.left, r.top, r.right-r.left, r.bottom-r.top,
                   width, src );
}

void sgui_canvas_draw_box( sgui_canvas* canvas, sgui_rect* r,
                           unsigned char* color, SGUI_COLOR_FORMAT format )
{
    sgui_rect r1;

    /* sanity check */
    if( !canvas || !color || !canvas->began || !r )
        return;

    if( format==SCF_RGBA8 && color[3]==0xFF )
        format = SCF_RGB8;

    /* offset and clip the given rectangle */
    COPY_RECT_OFFSET( r1, r );

    if( !sgui_rect_get_intersection( &r1, &canvas->sc, &r1 ) )
        return;

    canvas->draw_box( canvas, &r1, color, format );
}

void sgui_canvas_draw_line( sgui_canvas* canvas, int x, int y,
                            unsigned int length, int horizontal,
                            unsigned char* color, SGUI_COLOR_FORMAT format )
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
    int bearing, oldx;
    unsigned int i, w, h, len = 0;
    unsigned long character, previous=0;
    unsigned char* buffer;
    sgui_rect r;
    sgui_font* font_face = sgui_skin_get_default_font( bold, italic );

    x += canvas->ox;
    y += canvas->oy;
    oldx = x;

    /* sanity check */
    if( !canvas || !font_face || !color || !text || !canvas->began )
        return 0;

    /* for each character */
    for( i=0; i<length && (*text) && (*text!='\n'); text+=len, i+=len )
    {
        /* load the next glyph */
        character = sgui_utf8_decode( text, &len );
        sgui_font_load_glyph( font_face, character );

        /* apply kerning */
        x += sgui_font_get_kerning_distance( font_face, previous, character );

        /* blend onto destination buffer */
        sgui_font_get_glyph_metrics( font_face, &w, &h, &bearing );
        buffer = sgui_font_get_glyph( font_face );

        sgui_rect_set_size( &r, x, y + bearing, w, h );

        if( buffer && sgui_rect_get_intersection( &r, &canvas->sc, &r ) )
        {
            buffer += (r.top - (y + bearing)) * w + (r.left - x);

            canvas->blend_stencil( canvas, buffer, r.left, r.top,
                                   SGUI_RECT_WIDTH( r ),
                                   SGUI_RECT_HEIGHT( r ),
                                   w, color );
        }

        /* advance cursor */
        x += w + 1;

        /* store previous glyph index for kerning */
        previous = character;
    }

    return x - oldx;
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

