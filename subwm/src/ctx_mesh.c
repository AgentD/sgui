/*
 * ctx_mesh.c
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
#include "sgui_subwm_skin.h"
#include "sgui_rect.h"
#include "ctx_mesh.h"



/*
    0_1__________2_3
    |_|__________|_|
   4| |5        6| |7
    | |          | |
    | |          | |
   8|_|9_______10|_|11
    |_|__________|_|
   12 13       14  15
 */
static unsigned short ibo[] =
{
     0,  1,  4,  1,  5,  4, /* top left */
     1,  2,  5,  2,  6,  5, /* top center */
     2,  3,  6,  3,  7,  6, /* top right */
     4,  5,  8,  5,  9,  8, /* left */
     5,  6,  9,  9,  6, 10, /* center */
     6,  7, 10,  7, 11, 10, /* right */
     8,  9, 12, 12,  9, 13, /* bottom left */
     9, 10, 13, 13, 10, 14, /* bottom center */
    10, 11, 14, 14, 11, 15  /* bottom right */
};



void ctx_get_window_vertices( unsigned int width, unsigned int height,
                              float* vbo, sgui_subwm_skin* skin, int offset )
{
    int a, b, c, d, e, f, g, h;
    sgui_rect tl, tr, bl, br;
    unsigned int tw, th;
    float sx, sy;

    skin->get_ctx_window_corner( skin, &tl, SGUI_WINDOW_TOP_LEFT );
    skin->get_ctx_window_corner( skin, &tr, SGUI_WINDOW_TOP_RIGHT );
    skin->get_ctx_window_corner( skin, &bl, SGUI_WINDOW_BOTTOM_LEFT );
    skin->get_ctx_window_corner( skin, &br, SGUI_WINDOW_BOTTOM_RIGHT );
    skin->get_ctx_skin_texture_size( skin, &tw, &th );
    sx = 1.0f / (float)(tw-1);
    sy = 1.0f / (float)(th-1);

    a = SGUI_RECT_WIDTH(tl);
    b = SGUI_RECT_WIDTH(tr);
    c = SGUI_RECT_HEIGHT(tl);
    d = SGUI_RECT_HEIGHT(tr);
    e = SGUI_RECT_WIDTH(bl);
    f = SGUI_RECT_WIDTH(br);
    g = SGUI_RECT_HEIGHT(bl);
    h = SGUI_RECT_HEIGHT(br);

    /* top row texture coordinates */
    vbo[ 0]=vbo[16]=(float)tl.left*sx;
    vbo[ 1]=vbo[ 5]=(float)tl.top*sy;
    vbo[ 4]=vbo[20]=(float)tl.right*sx;
    vbo[17]=vbo[21]=(float)tl.bottom*sy;

    vbo[ 8]=vbo[24]=(float)tr.left*sx;
    vbo[ 9]=vbo[13]=(float)tr.top*sy;
    vbo[12]=vbo[28]=(float)tr.right*sx;
    vbo[25]=vbo[29]=(float)tr.bottom*sy;

    /* bottom row texture coordinates */
    vbo[32]=vbo[48]=(float)bl.left*sx;
    vbo[36]=vbo[52]=(float)bl.right*sx;
    vbo[33]=vbo[37]=(float)bl.top*sy;
    vbo[49]=vbo[53]=(float)bl.bottom*sy;

    vbo[40]=vbo[56]=(float)br.left*sx;
    vbo[44]=vbo[60]=(float)br.right*sx;
    vbo[41]=vbo[45]=(float)br.top*sy;
    vbo[57]=vbo[61]=(float)br.bottom*sy;

    /* vertex positions */
    vbo[2]=vbo[3]=vbo[7]=vbo[11]=vbo[15]=vbo[18]=vbo[34]=vbo[50]=0.0f;

    vbo[ 6] = vbo[22] = a+offset;
    vbo[10] = vbo[26] = width-b+offset;
    vbo[19] = vbo[23] = c+offset;
    vbo[27] = vbo[31] = d+offset;
    vbo[38] = vbo[54] = e+offset;
    vbo[42] = vbo[58] = width-f+offset;
    vbo[35] = vbo[39] = height-g+offset;
    vbo[43] = vbo[47] = height-h+offset;
    vbo[14] = vbo[30] = vbo[46] = vbo[62] = width+offset;
    vbo[51] = vbo[55] = vbo[59] = vbo[63] = height+offset;
}

unsigned int ctx_get_window_indices( unsigned short** indexbuffer )
{
    if( indexbuffer )
        *indexbuffer = ibo;
    return sizeof(ibo)/sizeof(ibo[0]);
}

