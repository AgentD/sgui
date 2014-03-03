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



#define INVALID_CHARACTER '?'



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

unsigned int sgui_utf8_encode( unsigned int cp, char* str )
{
    /* 0x00000000 - 0x00000080: 0aaaaaaa */
    if( cp < 0x80 )
    {
        *str = cp;
        return 1;
    }

    /* invalid characters */
    if( cp > 0x10FFFF || cp==0xFFFE || cp==0xFFFF )
    {
        *str = INVALID_CHARACTER;
        return 1;
    }

    /* UTF16 surrogate pairs */
    if( cp==0xD800 || cp==0xDB7F || cp==0xDB80 || cp==0xDBFF ||
        cp==0xDC00 || cp==0xDF80 || cp==0xDFFF )
    {
        *str = INVALID_CHARACTER;
        return 1;
    }

    /* 0x00000080 - 0x000007FF: 110aaaaa 10bbbbbb */
    if( cp < 0x800 )
    {
        *(str++) = (0xC0 | (cp >> 6));
        *(str++) = 0x80 | (cp & 0x3F);
        return 2;
    }

    /* 0x00000800 - 0x0000FFFF: 1110aaaa 10bbbbbb 10cccccc */
    if( cp < 0x10000 )
    {
        *(str++) = 0xE0 | (cp >> 12);
        *(str++) = 0x80 | ((cp >> 6) & 0x3F);
        *(str++) = 0x80 | (cp & 0x3F);
        return 3;
    }

    /* 0x00010000 - 0x0010FFFF: 11110aaa 10bbbbbb 10cccccc 10dddddd */
    *(str++) = 0xF0 | (cp >> 18);
    *(str++) = 0x80 | ((cp >> 12) & 0x3F);
    *(str++) = 0x80 | ((cp >> 6) & 0x3F);
    *(str++) = 0x80 | (cp & 0x3F);
    return 4;
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

unsigned int sgui_utf8_from_latin1_length( const char* in )
{
    unsigned int length = 0;

    while( *in )
    {
        length += (*(in++) & 0x80) ? 2 : 1;
    }

    return length;
}

void sgui_utf8_from_latin1( char* out, const char* in )
{
    for( ; *in; ++in )
    {
        if( *in & 0x80 )
        {
            *out++ = 0xC2 + ((unsigned char)*in > 0xBF);
            *out++ = 0x80 + (*in & 0x3F);
        }
        else
        {
            *out++ = *in;
        }
    }

    *out = '\0';
}

unsigned int sgui_utf8_strncpy( char* dst, const char* src,
                                unsigned int buffersize,
                                unsigned int characters )
{
    unsigned int i, j;
    char* old;

    if( !dst || !src )
        return 0;

    for( i=0, j=0; j<buffersize && i<characters && *src; ++i )
    {
        old = dst;
        *(dst++) = *(src++);
        ++j;

        while( (*dst & 0xC0)==0x80 && j<buffersize )
        {
            *(dst++) = *(src++);
            ++j;
        }

        if( (*dst & 0xC0)==0x80 && j==buffersize )
        {
            dst = old;
            break;
        }
    }

    *dst = 0;
    return i;
}

