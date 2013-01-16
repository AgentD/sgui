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



unsigned int utf8_char_length( unsigned char c )
{
    c >>= 3;

    if( c == 0x1E )
        return 4;

    c >>= 1;

    if( c == 0x0E )
        return 3;

    c >>= 1;

    return (c==0x06) ? 2 : 1;
}




unsigned int sgui_utf8_decode( const char* utf8, unsigned int* length )
{
    unsigned int ch, i, len = 0;

    if( utf8 )
    {
        len = utf8_char_length( *utf8 );
        ch = *(utf8++);

        switch( len )
        {
        case 4: ch ^= 0xf0; break;
        case 3: ch ^= 0xe0; break;
        case 2: ch ^= 0xc0; break;
        }

        for( i=*length; i>1; --i, ++utf8 )
        {
            ch <<= 6;
            ch |= (((unsigned char)*utf8) ^ 0x80);
        }
    }

    if( length )
        *length = len;

    return ch;
}

