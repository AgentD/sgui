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
#include "sgui_filesystem.h"

#include <stdio.h>



static void* fs_open_read( sgui_filesystem* fs, const char* filename )
{
    (void)fs;

    return fopen( filename, "rb" );
}

static unsigned int fs_read( void* file, void* ptr,
                             unsigned int size, unsigned int nmemb )
{
    return fread( ptr, size, nmemb, file );
}

static int fs_eof( void* file )
{
    return feof( file );
}

static void fs_close( void* file )
{
    fclose( file );
}



static sgui_filesystem stdio =
{
    fs_open_read,
    fs_read,
    fs_eof,
    fs_close
};



sgui_filesystem* sgui_filesystem_get_default( void )
{
    return &stdio;
}

void sgui_filesystem_read_line( sgui_filesystem* fs, void* file,
                                char* buffer, unsigned int size )
{
    if( !fs || !file || !buffer || !size )
        return;

    /* read until EOF or at only one character left (for terminator) */
    while( !fs->file_eof( file ) && (size >= 1) )
    {
        /* maybe the punched tape reader is not ready yet */
        if( fs->file_read( file, buffer, 1, 1 ) == 1 )
        {
            /* plainly ignore carriage returns */
            if( *buffer == '\r' )
                continue;

            /* stop when we found a line feed */
            if( *buffer == '\n' )
                break;

            /* go to next buffer position */
            --size;
            ++buffer;
        }
    }

    /* insert terminator, so nobody gets hurt */
    *buffer = '\0';
}

