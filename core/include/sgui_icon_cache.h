/*
 * sgui_icon_cache.h
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
 * \file sgui_icon_cache.h
 *
 * \brief Contains the declarations of the icon cache data type.
 */
#ifndef SGUI_ICON_CACHE_H
#define SGUI_ICON_CACHE_H



#include "sgui_predef.h"
#include "sgui_rect.h"



/**
 * \struct sgui_icon
 *
 * \brief Represents an icon stored inside an sgui_icon_cache
 *
 * This holds the location and size of an icon on a pixmap and is stored in a
 * red-black tree. A subclass of sgui_icon_cache adds its own key to a
 * subclass and implements the comparison method in sgui_icon_cache.
 */
struct sgui_icon
{
    sgui_rect area;     /**< \brief area on pixmap */
    int red;            /**< \brief non-zero if red, zero if black */
    sgui_icon* left;    /**< \brief pointer to left child node */
    sgui_icon* right;   /**< \brief pointer to right child node */
};

/**
 * \struct sgui_icon_cache
 *
 * \brief Allocates rectangles icons on a pixmap and maps them to keys
 *
 * An sgui_icon_cache manages a pixmap and allocates rectangular areas on the
 * pixmap (the icons, represented by sgui_icon implementations). The icons are
 * internally stored in a red-black tree to allow for mapping of an arbitrary
 * key to an icon area in linearithmic time.
 *
 * The sgui_font_cache is an implementation of an sgui_icon_cache that uses
 * a font-codepoint pair as a key.
 *
 * The sgui_icon_map is an implementation of an sgui_icon_cache that allows
 * mapping arbitrary (but unique) integer IDs to icon areas.
 */
struct sgui_icon_cache
{
    sgui_canvas* owner;         /**< the canvas that created the pixmap */
    sgui_pixmap* pixmap;        /**< the icon pixmap */

    sgui_icon* root;            /**< the root of the icon tree */

    int next_x, next_y;         /**< next insertion position on pixmap */
    unsigned int row_height;    /**< height of the current icon row */

    unsigned int width, height; /**< width of the current icon row */

    int format;                 /**< color format of the pixmap */

    /**
     * \brief Compare two icons
     *
     * \note If this is NULL, the IDs are used for comparison
     *
     * \return 0 if both are equal, a positive number if left &gt; right,
     *         a negative number if left &lt; right
     */
    int (* icon_compare )( const sgui_icon* left, const sgui_icon* right );

    /**
     * \brief Destroy icons created by an icon cache implementation
     *
     * \note If this is NULL, free is used for deletion
     *
     * \param icon A pointer to an icon to destroy
     */
    void (* icon_destroy )( sgui_icon* icon );

    /** \brief Called to cleanup an icon cache. free( ) is used if NULL */
    void (* destroy )( sgui_icon_cache* cache );
};



/**
 * \struct sgui_icon_map
 *
 * \implements sgui_icon_cache
 *
 * \brief An implementation of an sgui_icon_cache that uses integer IDs
 *
 * An sgui_icon_map manages a pixmap and allocates rectangular areas on the
 * pixmap (the icons). The icons have unique integer IDs assigned to them that
 * can be used to look them up later on.
 */



#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Get the area occupied by an icon on the icon pixmap
 *
 * \memberof sgui_icon
 *
 * \param icon A pointer to an icon
 * \param out  Returns the area of the icon on the pixmap
 */
SGUI_DLL void sgui_icon_get_area( const sgui_icon* icon, sgui_rect* out );



/**
 * \brief Destroy an icon cache object
 *
 * \memberof sgui_icon_cache
 *
 * \param cache A pointer to an icon cache object
 */
SGUI_DLL void sgui_icon_cache_destroy( sgui_icon_cache* cache );

/**
 * \brief Insert an icon into the icon tree of an icon cache
 *
 * \memberof sgui_icon_cache
 * \protected
 *
 * \note This funcion is used by sgui_icon_cache_add_icon and should only be
 *       used by subclasses of sgui_icon_cache that extend sgui_icon
 *
 * \param cache  A pointer to an icon cache object
 * \param root   The root of the subtree to insert at
 * \param insert The icon to insert into the subtree. The icon cache takes
 *               over ownership of this icon.
 *
 * \return A pointer to the root node if the sub-tree was not rotated, a
 *         pointer to the new root if it was, the value of the new node if
 *         either cache or root is NULL.
 */
SGUI_DLL sgui_icon* sgui_icon_cache_tree_insert( sgui_icon_cache* cache,
                                                 sgui_icon* root,
                                                 sgui_icon* insert );

/**
 * \brief Load icon data into an icon cache
 *
 * \memberof sgui_icon_cache
 *
 * \param cache  A pointer to an icon cache object
 * \param icon   A pointer to an icon object
 * \param data   A pointer to the image data to load
 * \param scan   Number of pixels to skip to get to the first pixel in the
 *               next line of the image.
 * \param format The color format of the image data.
 */
SGUI_DLL void sgui_icon_cache_load_icon( sgui_icon_cache* cache,
                                         sgui_icon* icon,
                                         const unsigned char* data,
                                         unsigned int scan,
                                         int format );

/**
 * \brief Draw an icon from an icon cache onto the coresponding canvas
 *
 * \memberof sgui_icon_cache
 *
 * \param cache A pointer to an icon cache object
 * \param icon  A pointer to an icon object
 * \param x     Distance of the left of the icon from the left of the canvas
 * \param y     Distance of the top of the icon from the top of the canvas
 */
SGUI_DLL void sgui_icon_cache_draw_icon( const sgui_icon_cache* cache,
                                         const sgui_icon* icon,
                                         int x, int y );

/**
 * \brief Allocate a rectangular area on the underlying pixmap
 *
 * \memberof sgui_icon_cache
 *
 * \param cache  A pointer to an icon cache object
 * \param width  The width of the area to allocate
 * \param height The height of the area to allocate
 * \param out    Returns the exact outline of the allocated area on the pixmap
 *
 * \return Non-zero on success, zero on failure (i.e. no enough space)
 */
SGUI_DLL int sgui_icon_cache_alloc_area( sgui_icon_cache* cache,
                                         unsigned int width,
                                         unsigned int height,
                                         sgui_rect* out );

/**
 * \brief Get a pointer to the pixmap used by an icon cache object
 *
 * \memberof sgui_icon_cache
 *
 * \param cache A pointer to an icon cache object
 *
 * \return A pointer to a pixmap object on success, NULL otherwise.
 */
SGUI_DLL sgui_pixmap* sgui_icon_cache_get_pixmap( sgui_icon_cache* cache );

/**
 * \brief Search for an icon in an icon cache
 *
 * \memberof sgui_icon_cache
 *
 * \param cache A pointer to an icon cache object
 * \param icon  A pointer to an icon to compare against, using the comparion
 *              function
 *
 * \return A pointer to the icon if found, NULL if not
 */
SGUI_DLL sgui_icon* sgui_icon_cache_find( const sgui_icon_cache* cache,
                                          const sgui_icon* icon );






/**
 * \brief Create an icon map object
 *
 * \memberof sgui_icon_map
 *
 * \param canvas The canvas to create the pixmap from
 * \param width  The width of the icon pixmap
 * \param height The height of the icon pixmap
 * \param alpha  Non-zero if the underlying pixmap should have an alpha
 *               channel, zero for RGB only.
 *
 * \return On success, a pointer to an icon map object, NULL on failure
 */
SGUI_DLL sgui_icon_cache* sgui_icon_map_create( sgui_canvas* canvas,
                                                unsigned int width,
                                                unsigned int height,
                                                int alpha );

/**
 * \brief Add an icon to an icon map object
 *
 * \memberof sgui_icon_map
 *
 * \param map    A pointer to an icon map object
 * \param id     A unique id to asociate with the icon
 * \param width  The width of the icon
 * \param height The height of the icon
 *
 * \return Non-zero on success, zero on failure (e.g. out of space on pixmap)
 */
SGUI_DLL int sgui_icon_map_add_icon( sgui_icon_cache* map,
                                     unsigned int id,
                                     unsigned int width,
                                     unsigned int height );

/**
 * \brief Search for an icon in an icon map
 *
 * \memberof sgui_icon_map
 *
 * \param map A pointer to an icon map object
 * \param id  The unique ID of the icon
 *
 * \return A pointer to the icon if found, NULL if not
 */
SGUI_DLL sgui_icon* sgui_icon_map_find( const sgui_icon_cache* map,
                                        unsigned int id );

#ifdef __cplusplus
}
#endif

#endif /* SGUI_ICON_CACHE_H */

