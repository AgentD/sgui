#ifndef SGUI_PIXMAP_H
#define SGUI_PIXMAP_H



#include "sgui_predef.h"



#ifdef __cplusplus
extern "C"
{
#endif



/**
 * \brief Create a pixmap
 *
 * \param width  The width of the pixmap
 * \param height The height of the pixmap
 * \param data   The RGB image data to load into the pixmap
 *
 * \return A pixmap object
 */
sgui_pixmap* sgui_pixmap_create( unsigned int width, unsigned int height,
                                 unsigned char* data );

/**
 * \brief Delete a pixmap
 *
 * \param pixmap The pixmap to delete
 */
void sgui_pixmap_delete( sgui_pixmap* pixmap );



#ifdef __cplusplus
}
#endif

#endif /* SGUI_PIXMAP_H */

