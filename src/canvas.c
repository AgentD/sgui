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
#include "sgui_canvas.h"
#include "sgui_internal.h"



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
    if( canvas )
    {
        if( !canvas->began )
        {
            canvas->ox = 0;
            canvas->oy = 0;
            canvas->scissor_stack_pointer = 0;
            canvas->offset_stack_pointer = 0;

            sgui_rect_copy( &canvas->sc, r );

            canvas->begin( canvas, r );

            if( canvas->sc.left < 0 )
                canvas->sc.left = 0;

            if( canvas->sc.top < 0 )
                canvas->sc.top = 0;

            if( canvas->sc.right >= (int)canvas->width )
                canvas->sc.right = canvas->width - 1;

            if( canvas->sc.bottom >= (int)canvas->height )
                canvas->sc.bottom = canvas->height - 1;
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

    if( r )
    {
        COPY_RECT_OFFSET( r1, r );
    }
    else
    {
        sgui_rect_set_size( &r1, 0, 0, canvas->width, canvas->height );
    }

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

            /* merge rectangles */
            sgui_rect_get_intersection( &canvas->sc, &canvas->sc, &r1 );
        }
        else if( canvas->scissor_stack_pointer )
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

        canvas->offset_stack_x[ canvas->offset_stack_pointer ] = canvas->ox;
        canvas->offset_stack_y[ canvas->offset_stack_pointer ] = canvas->oy;

        canvas->ox += x;
        canvas->oy += y;

        ++(canvas->offset_stack_pointer);
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
    sgui_rect r, r0;

    if( !canvas || !width || !height || !data || !canvas->began )
        return;

    sgui_rect_set_size( &r0, x+canvas->ox, y+canvas->oy, width, height );

    if( !sgui_rect_get_intersection( &r, &canvas->sc, &r0 ) )
        return;

    data = (unsigned char*)data +
           ((r.top-r0.top)*width + r.left-r0.left)*
           (format==SCF_RGBA8 ? 4 : 3);

    canvas->blit( canvas, r.left, r.top, r.right - r.left + 1,
                  r.bottom - r.top + 1, width, format, data );
}

void sgui_canvas_blend( sgui_canvas* canvas, int x, int y, unsigned int width,
                        unsigned int height, SGUI_COLOR_FORMAT format,
                        const void* data )
{
    sgui_rect r, r0;

    if( !canvas || !width || !height || !data || format!=SCF_RGBA8 ||
        !canvas->began )
        return;

    sgui_rect_set_size( &r0, x+canvas->ox, y+canvas->oy, width, height );

    if( !sgui_rect_get_intersection( &r, &canvas->sc, &r0 ) )
        return;

    data = (unsigned char*)data + ((r.top-r0.top)*width + r.left-r0.left)*4;

    canvas->blend( canvas, r.left, r.top, r.right - r.left + 1,
                   r.bottom - r.top + 1, width, data );
}

void sgui_canvas_draw_box( sgui_canvas* canvas, sgui_rect* r,
                           unsigned char* color, SGUI_COLOR_FORMAT format )
{
    sgui_rect r1;

    if( !canvas || !color || !canvas->began || !r )
        return;

    COPY_RECT_OFFSET( r1, r );

    if( !sgui_rect_get_intersection( &r1, &canvas->sc, &r1 ) )
        return;

    canvas->draw_box( canvas, &r1, color, format );
}


void sgui_canvas_draw_line( sgui_canvas* canvas, int x, int y,
                            unsigned int length, int horizontal,
                            unsigned char* color, SGUI_COLOR_FORMAT format )
{
    if( !canvas || !canvas->began )
        return;

    x += canvas->ox;
    y += canvas->oy;

    if( !sgui_rect_clip_line( &canvas->sc, horizontal, &x, &y, &length ) )
        return;

    canvas->draw_line( canvas, x, y, length, horizontal, color, format );
}

int sgui_canvas_blend_stencil( sgui_canvas* canvas, unsigned char* buffer,
                               int x, int y, unsigned int w, unsigned int h,
                               unsigned char* color )
{
    sgui_rect r;

    if( !canvas || !buffer || !color || !w || !h || !canvas->began )
        return 0;

    x += canvas->ox;
    y += canvas->oy;

    if( (x+(int)w-1)<canvas->sc.left || (y+(int)h-1)<canvas->sc.top )
        return -1;

    if( x>canvas->sc.right || y>canvas->sc.bottom )
        return 1;

    sgui_rect_set_size( &r, x, y, w, h );
    sgui_rect_get_intersection( &r, &canvas->sc, &r );

    buffer += (r.top - y) * w + (r.left - x);

    canvas->blend_stencil( canvas, buffer, r.left, r.top,
                           r.right-r.left+1, r.bottom-r.top+1, w, color );

    return 0;
}

