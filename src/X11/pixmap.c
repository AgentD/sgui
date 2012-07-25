#include "internal.h"
#include "sgui_pixmap.h"



sgui_pixmap* sgui_pixmap_create( unsigned int width, unsigned int height,
                                 unsigned char* data )
{
    unsigned char *buffer, *src, *dst;
    unsigned int x, y;
    sgui_pixmap* pix;

    /* sanity check */
    if( !width || !height || !data )
        return NULL;

    /* allocate size for the image */
    pix = malloc( sizeof(sgui_pixmap) );

    if( !pix )
        return NULL;

    /* Open display */
    pix->dpy = XOpenDisplay( NULL );

    if( !pix->dpy )
    {
        free( pix );
        return NULL;
    }

    /* create conversion buffer */
    buffer = malloc( width*height*4 );

    if( !buffer )
    {
        XCloseDisplay( pix->dpy );
        free( pix );
        return NULL;
    }

    /* fill conversion buffer */
    for( src=data, dst=buffer, y=0; y<height; ++y )
    {
        for( x=0; x<width; ++x, src+=3, dst+=4 )
        {
            dst[0] = src[2];
            dst[1] = src[1];
            dst[2] = src[0];
            dst[3] = 0xFF;
        }
    }

    /* create and return image */
    pix->image = XCreateImage( pix->dpy, CopyFromParent, 24, ZPixmap, 0,
                               (char*)buffer, width, height, 32, 0 );

    return pix;
}

void sgui_pixmap_delete( sgui_pixmap* pixmap )
{
    if( pixmap )
    {
        XCloseDisplay( pixmap->dpy );
        XDestroyImage( pixmap->image );
        free( pixmap );
    }
}

