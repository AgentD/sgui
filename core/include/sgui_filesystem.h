#ifndef SGUI_FILESYSTEM_H
#define SGUI_FILESYSTEM_H



#include <stddef.h>

#include "sgui_predef.h"



#ifdef __cplusplus
extern "C" {
#endif



struct sgui_filesystem
{
    /**
     * \brief Open a file for reading
     *
     * \param filename The path to the file to open (utf8 encoded)
     *
     * \return An opaque file handle
     */
    void* (* file_open_read )( const char* filename );

    /**
     * \brief Close a previously opened file
     *
     * \param file An opqaue file handle
     *
     * \return non-zero on success, zero on error
     */
    int (* file_close )( void* file );

    /** \brief Get the length of a file in bytes */
    size_t (* file_get_length )( void* file );

    /**
     * \brief Read data from a file
     *
     * \param file     An opaque file handle returned by file_open_read
     * \param buffer   A buffer to read into
     * \param itemsize Size of one "item"
     * \param items    The number of items to read from the file
     *
     * \return The actual number of items successfully read from the file
     */
    size_t (* file_read )( void* file, void* buffer,
                           size_t itemsize, size_t items );
};


/** \brief Get a pointer to a default stdio filesystem implementation */
const sgui_filesystem* SGUI_DLL sgui_filesystem_get_default( void );



#ifdef __cplusplus
}
#endif

#endif /* SGUI_FILESYSTEM_H */

