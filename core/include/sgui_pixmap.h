/*
 * sgui_pixmap.h
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

/**
 * \file sgui_pixmap.h
 *
 * This file contains the interface functions for the abstract pixmap
 * datatype.
 */
#ifndef SGUI_PIXMAP_H
#define SGUI_PIXMAP_H



#include "sgui_predef.h"
#include "sgui_window.h"



struct sgui_pixmap
{
    unsigned int width, height;     /**< \brief Size of the pixmap */

    /** \copydoc sgui_pixmap_destroy */
    void(* destroy )( sgui_pixmap* pixmap );

    /**
     * \brief Gets called by sgui_pixmap_load
     *
     * \param pixmap A pointer to the pixmap
     * \param dstx   Offset from the left of the pixmap
     * \param dsty   Offset from the top of the pixmap
     * \param data   A pointer to the first pixel
     * \param scan   The number of pixels to skip to get to the same pixel in
     *               the next row
     * \param width  The number of pixels per row to upload
     * \param height The number of rows to upload
     */
    void(* load )( sgui_pixmap* pixmap, int dstx, int dsty,
                   const unsigned char* data, unsigned int scan,
                   unsigned int width, unsigned int height, int format );
};



#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Upload data to a pixmap
 *
 * \memberof sgui_pixmap
 *
 * \param pixmap   The pixmap to upload data to.
 * \param dstx     Offset from the left of the destination.
 * \param dsty     Offset from the top of the destination.
 * \param dstrect  A subrect within the pixmap to draw to.
 * \param data     A pointer to the data buffer of which to upload a portion.
 * \param srcx     Offset from the left of the source buffer to start reading.
 * \param srcy     Offset from the top of the source buffer to start reading.
 * \param width    The width of the source buffer.
 * \param height   The height of the source buffer.
 * \param scan     The length of one scane line in the source buffer.
 * \param format   The color format of the source data (SGUI_RGB8, etc...)
 */
SGUI_DLL void sgui_pixmap_load( sgui_pixmap* pixmap, int dstx, int dsty,
                                const unsigned char* data, int srcx, int srcy,
                                unsigned int width, unsigned int height,
                                unsigned int scan, int format );

/**
 * \brief Get the size of a pixmap
 *
 * \memberof sgui_pixmap
 *
 * \param pixmap The pixmap from wich to get the size
 * \param width  Returns the width of the pixmap
 * \param height Returns the height of the pixmap
 */
SGUI_DLL void sgui_pixmap_get_size( const sgui_pixmap* pixmap,
                                    unsigned int* width,
                                    unsigned int* height );

/**
 * \brief Destroy a pixmap
 *
 * \memberof sgui_pixmap
 */
SGUI_DLL void sgui_pixmap_destroy( sgui_pixmap* pixmap );

#ifdef __cplusplus
}
#endif

#endif /* SGUI_PIXMAP_H */

