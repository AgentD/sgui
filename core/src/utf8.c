/*
 * sgui_utf8.c
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
#include "sgui_utf8.h"



unsigned int sgui_utf8_decode( const char* utf8, unsigned int* length )
{
    unsigned int ch = 0, i = 0, len = 0;

    if( utf8 )
    {
        len = 1;
        ch = *(utf8++);

             if( (ch & 0xFE) == 0xFC ) { len = 6; ch &= 0x01; }
        else if( (ch & 0xFC) == 0xF8 ) { len = 5; ch &= 0x03; }
        else if( (ch & 0xF8) == 0xF0 ) { len = 4; ch &= 0x07; }
        else if( (ch & 0xF0) == 0xE0 ) { len = 3; ch &= 0x0F; }
        else if( (ch & 0xE0) == 0xC0 ) { len = 2; ch &= 0x1F; }

        for( i=1; i<len; ++i, ++utf8 )
        {
            ch <<= 6;
            ch |= (*utf8) & 0x3F;
        }
    }

    if( length )
        *length = len;

    return ch;
}

unsigned int sgui_utf8_strlen( const char* utf8 )
{
    unsigned int len;

    for( len=0; *utf8; ++utf8 )
    {
        len += ((*utf8) & 0xC0)!=0x80;
    }

    return len;
}

