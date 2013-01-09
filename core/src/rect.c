/*
 * sgui_rect.h
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
#include "sgui_rect.h"



#ifndef MIN
    #define MIN( a, b ) (((a)<(b)) ? (a) : (b))
#endif

#ifndef MAX
    #define MAX( a, b ) (((a)>(b)) ? (a) : (b))
#endif

void sgui_rect_set_size( sgui_rect* r, int left, int top,
                         unsigned int width, unsigned int height )
{
    if( r )
    {
        r->left   = left;
        r->top    = top;
        r->right  = left + (int)width - 1;
        r->bottom = top + (int)height - 1;
    }
}

void sgui_rect_set( sgui_rect* r, int left, int top, int right, int bottom )
{
    if( r )
    {
        r->left   = left;
        r->top    = top;
        r->right  = right;
        r->bottom = bottom;
    }
}

void sgui_rect_copy( sgui_rect* dst, sgui_rect* src )
{
    if( dst && src )
    {
        dst->left   = src->left;
        dst->right  = src->right;
        dst->top    = src->top;
        dst->bottom = src->bottom;
    }
}

void sgui_rect_normalize( sgui_rect* r )
{
    int temp;

    if( r )
    {
        if( r->left > r->right )
        {
            temp = r->left;
            r->left = r->right;
            r->right = temp;
        }

        if( r->top > r->bottom )
        {
            temp = r->top;
            r->top = r->bottom;
            r->bottom = temp;
        }
    }
}

int sgui_rect_get_intersection( sgui_rect* r, sgui_rect* a, sgui_rect*b )
{
    do
    {
        if( !a || !b )  /* both must exist */
            break;

        /* check if a is to the right or below b */
        if( (a->left > b->right) || (a->top > b->bottom) )
            break;

        /* check if a is to the left or above b */
        if( (a->right < b->left) || (a->bottom < b->top) )
            break;

        /* set the intersection rectangle */
        if( r )
        {
            r->left   = MAX( a->left,   b->left   );
            r->top    = MAX( a->top,    b->top    );
            r->right  = MIN( a->right,  b->right  );
            r->bottom = MIN( a->bottom, b->bottom );
        }

        return 1;
    }
    while( 0 );     /* Don't use goto, they say.
                       Loads of nested ifs are unreadable, they say. */

    if( r )
    {
        r->left = r->top = r->right = r->bottom = 0;
    }

    return 0;
}

int sgui_rect_clip_line( sgui_rect* r, int horizontal, int* x, int* y,
                         unsigned int* length )
{
    int X, Y, L, rL, rR, rB, rT;

    /* all paramters must exist */
    if( !r || !x || !y || !length )
        return 0;

    /* probalby more readable than loads of dereferentiations? */
    X = *x;
    Y = *y;
    L = (int)*length;
    rL= r->left;
    rT= r->top;
    rR= r->right;
    rB= r->bottom;

    if( horizontal )
    {
        /* line starts above, below, to the right,
           or doesn't reach into the rect */
        if( Y < rT || Y > rB || X > rR || (X+L) < rL )
            return 0;

        /* starts left of the rect, but reaches into the rect */
        if( X < rL )
        {
            L -= rL - X;    /* adjust length */
            X  = rL;        /* clip X */
        }

        /* reaches outside the rect */
        if( (X+L) > rR )
            L = rR - X;     /* adjust length */
    }
    else
    {
        /* line is left, right or below the rect or it starts
           above and doesn't reach into the rect */
        if( X < rL || X > rR || Y > rB || (Y+L) < rT )
            return 0;

        /* starts above the rect, but reaches into the rect */
        if( Y < rT )
        {
            L -= rT - Y;    /* adjust length */
            Y  = rT;        /* clip Y */
        }

        /* reaches outside the rect */
        if( (Y+L) > rB )
            L = rB - Y;     /* adjust the length */
    }

    /* write back adjusted parameters */
    *x      = X;
    *y      = Y;
    *length = L;

    return 1;
}

int sgui_rect_join( sgui_rect* acc, sgui_rect* r, int only_if_touch )
{
    if( !acc )      /* joining to a non-existant rectangle always fails */
        return 0;

    if( !r )        /* joining a non-existant rectangle always succeeds */
        return 1;

    if( only_if_touch )
    {
        if( r->left  > acc->right || r->top    > acc->bottom ||
            r->right < acc->left  || r->bottom < acc->top )
        {
            return 0;
        }
    }

    acc->left   = MIN( acc->left,   r->left   );
    acc->top    = MIN( acc->top,    r->top    );
    acc->right  = MAX( acc->right,  r->right  );
    acc->bottom = MAX( acc->bottom, r->bottom );

    return 1;
}
