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
#include "sgui_internal.h"
#include "sgui_rect.h"



void sgui_rect_repair( sgui_rect* r )
{
    int temp;

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

void sgui_rect_set_size( sgui_rect* r, int left, int top,
                         unsigned int width, unsigned int height )
{
    r->left   = left;
    r->top    = top;
    r->right  = left + (int)width - 1;
    r->bottom = top + (int)height - 1;
}

int sgui_rect_get_intersection( sgui_rect* r, const sgui_rect* a,
                                const sgui_rect* b )
{
    /* check if a is to the right or below b */
    if( (a->left > b->right) || (a->top > b->bottom) )
        goto fail;

    /* check if a is to the left or above b */
    if( (a->right < b->left) || (a->bottom < b->top) )
        goto fail;

    /* set the intersection rectangle */
    if( r )
    {
        r->left   = MAX( a->left,   b->left   );
        r->top    = MAX( a->top,    b->top    );
        r->right  = MIN( a->right,  b->right  );
        r->bottom = MIN( a->bottom, b->bottom );
    }

    return 1;
fail:
    if( r )
    {
        r->left = r->top = r->bottom = r->right = 0;
    }
    return 0;
}

int sgui_rect_join( sgui_rect* acc, const sgui_rect* r, int only_if_touch )
{
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

int sgui_rect_is_point_inside( const sgui_rect* r, int x, int y )
{
    return x>=r->left && x<=r->right && y>=r->top && y<=r->bottom;
}

