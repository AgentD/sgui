/*
 * filesystem.c
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
#include "sgui_filesystem.h"

#include <stdio.h>



#ifndef SGUI_NO_STDIO
static void* sgui_stdio_open_read( const char* filename )
{
    return fopen( filename, "rb" );
}

static int sgui_stdio_close( void* file )
{
    return fclose( file );
}

static size_t sgui_stdio_get_length( void* file )
{
    long pos, end;

    /* save current position */
    pos = ftell( file );

    if( pos < 0 )
        return 0;

    /* seek to end */
    fseek( file, 0, SEEK_END );

    /* get the position of the end */
    end = ftell( file );

    /* seek back to the original position */
    fseek( file, pos, SEEK_SET );

    return end < 0 ? 0 : end;
}

static size_t sgui_stdio_read( void* file, void* buffer,
                               size_t itemsize, size_t items )
{
    return fread( buffer, itemsize, items, file );
}




static const sgui_filesystem sgui_stdio =
{
    sgui_stdio_open_read,
    sgui_stdio_close,
    sgui_stdio_get_length,
    sgui_stdio_read
};



const sgui_filesystem* sgui_filesystem_get_default( void )
{
    return &sgui_stdio;
}
#else
const sgui_filesystem* sgui_filesystem_get_default( void )
{
    return NULL;
}
#endif

