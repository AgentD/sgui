#include "sgui_filesystem.h"

#include <stdio.h>



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

    pos = ftell( file );
    fseek( file, 0, SEEK_END );

    if( pos < 0 )
        return 0;

    end = ftell( file );
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

