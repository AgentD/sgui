/*
 * sgui_filesystem.h
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
#ifndef SGUI_FILESYSTEM_H
#define SGUI_FILESYSTEM_H



#include "sgui_predef.h"



struct sgui_filesystem
{
    /**
     * \brief Open a file for reading
     *
     * The underlying implementation should treat a file as binary and MUST
     * NOT perform things like line ending tranlation.
     *
     * \param fs       A pointer to the filesystem structure, so an
     *                 implementation may use information from a derived
     *                 structure.
     * \param filename The path to the file to open
     *
     * \return An opaque file handle on success, NULL on error
     */
    void* (* file_open_read )( sgui_filesystem* fs, const char* filename );

    /**
     * \brief Read from a file opened through file_open_read
     *
     * \param file  An opaque file handle
     * \param ptr   A pointer to a buffer to write the data from the file to
     * \param size  The size of a single block of data to read
     * \param nmemb The number of blocks to read
     *
     * \return The number of blocks successfully read
     */
    unsigned int (* file_read )( void* file, void* ptr,
                                 unsigned int size, unsigned int nmemb );

    /**
     * \brief Determine if the end of a file, opened for reading, was hit
     *
     * \param file An opaque file handle returned by file_open_read
     *
     * \return Non-zero if the end was hit, zero if not
     */
    int (* file_eof )( void* file );

    /**
     * \brief Close a file
     *
     * \param file An opaque file handle
     */
    void (* file_close )( void* file );
};

#ifdef __cplusplus
extern "C" {
#endif

/** \brief Returns a pointer to a default filesystem implementation */
SGUI_DLL sgui_filesystem* sgui_filesystem_get_default( void );

/**
 * \brief Utilit function to read a single line of text from a file
 *
 * The line ending characters are removed and a null terminator is added at
 * the end. If no line ending was found before running out of buffer space,
 * a null terminator will still be added, without loosing data from the file.
 * Whatever happens, a null terminator will always be in place.
 *
 * \param fs     A pointer to a filesystem struct
 * \param file   An opaque file handle, opened for reading
 * \param buffer A pointer to a buffer to read to
 * \param size   The size of the given read buffer
 */
SGUI_DLL void sgui_filesystem_read_line( sgui_filesystem* fs, void* file,
                                         char* buffer, unsigned int size );

#ifdef __cplusplus
}
#endif

#endif /* SGUI_FILESYSTEM_H */

