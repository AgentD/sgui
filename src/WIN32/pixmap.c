#include "internal.h"



sgui_pixmap* sgui_pixmap_create( unsigned int width, unsigned int height,
                                 unsigned char* data )
{
    unsigned int x, y;
    unsigned char *buffer, *dst, *src;
    sgui_pixmap* pixmap;

    pixmap = malloc( sizeof(sgui_pixmap) );

    if( !pixmap )
        return NULL;

    /* allocate conversion buffer */
    buffer = malloc( width*height*4 );

    if( !buffer )
    {
        free( pixmap );
        return NULL;
    }

    /* convert data */
    for( src = data, dst = buffer, y=0; y<height; ++y )
    {
        for( x=0; x<width; ++x, dst+=4, src+=3 )
        {
            dst[0] = src[2];
            dst[1] = src[1];
            dst[2] = src[0];
            dst[3] = 0xFF;
        }
    }

    /* create bitmap and upload data */
    pixmap->bitmap = CreateBitmap( width, height, 1, 32, buffer );

    pixmap->width = width;
    pixmap->height = height;

    /* cleanup */
    free( buffer );

    return pixmap;
}

void sgui_pixmap_delete( sgui_pixmap* pixmap )
{
    if( pixmap )
    {
        DeleteObject( pixmap->bitmap );
        free( pixmap );
    }
}

