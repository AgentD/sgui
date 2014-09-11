/*
 * sgui_model.h
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
#ifndef SGUI_MODEL_H
#define SGUI_MODEL_H



#include "sgui_predef.h"



/**
 * \struct sgui_item
 *
 * \brief Represents an abstract item entry stored in an sgui_model
 *
 * Items are aranged in a linked list. An item can have multiple columns,
 * each containing text and an icon of an asociated sgui_icon_cache.
 * An item can also have a linked list of children, implementing a hirachical
 * data representation.
 *
 * Items are accessed through an sgui_model wich can only return a list of
 * items on the same level, for each of which a list of children can be
 * obtained.
 */
struct sgui_item
{
    sgui_item* next;      /**< \brief Next item on same hirarchical level */
};

/**
 * \struct sgui_model
 *
 * \brief Represents an abstract model in an MVC pattern
 *
 * The sgui_model is abstract enough to hold data organized in lists, tables
 * or hirachical. A model can be queried for a linked list of items on a same
 * hirarchical leve. Each item can have multiple columns containing text and
 * an icon of an asociated sgui_icon_cache. An item can also have a linked
 * list of children, which can be queried via the model, implementing a
 * hirachical data representation.
 *
 * The sgui_model was designed with implementations in mind that might not
 * have all the data in memory at once. Concrete implementations might also,
 * for instance, retrieve data from an SQL database or represent a directroy
 * tree in a filesystem.
 */
struct sgui_model
{
    sgui_icon_cache* cache; /**< \brief Cache for item icons or NULL */
    unsigned int cols;      /**< \brief Column count */

    /** \copydoc sgui_model_destroy */
    void (* destroy )( sgui_model* model );

    /** \copydoc sgui_model_query_items */
    const sgui_item* (* query_items )( const sgui_model* model,
                                       const sgui_item* parent,
                                       unsigned int start,
                                       unsigned int count );

    /**
     * \copydoc sgui_model_free_item_list
     *
     * Can be NULL if not implemented.
     */
    void (* free_item_list )( sgui_model* model, const sgui_item* start );

    /** \copydoc sgui_model_item_children_count */
    unsigned int (* item_children_count )( const sgui_model* model,
                                           const sgui_item* item );

    /**
     * \copydoc sgui_item_text
     *
     * Can be NULL if not implemented, only called if item is not NULL
     * and column is less than cols.
     */
    const char* (* item_text )( const sgui_model* model,
                                const sgui_item* item,
                                unsigned int column );

    /**
     * \copydoc sgui_item_icon
     *
     * Can be NULL if not implemented, only called if item is not NULL,
     * column is less than cols and cache is not NULL.
     */
    sgui_icon* (* item_icon )( const sgui_model* model, const sgui_item* item,
                               unsigned int column );
};



/**
 * \struct sgui_simple_item
 *
 * \implements sgui_item
 *
 * \brief An item implementation used by sgui_simple_model
 */

/**
 * \struct sgui_simple_model
 *
 * \implements sgui_model
 *
 * \brief A simple, in-memory implementation of an sgui_model
 */



#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Destroy an item model
 *
 * \memberof sgui_model
 *
 * \param model A pointer to an item model
 */
SGUI_DLL void sgui_model_destroy( sgui_model* model );

/**
 * \brief Get the sgui_icon_cache used for item icons
 *
 * \memberof sgui_model
 *
 * \param model A pointer to an item model
 *
 * \return The icon cache used for item icons, or NULL if none
 */
SGUI_DLL sgui_icon_cache* sgui_model_get_icon_cache(const sgui_model* model);

/**
 * \brief Get the total number of data entries (columns) for each item
 *
 * \memberof sgui_model
 *
 * \param model A pointer to an item model
 *
 * \return The number of data columns per item
 */
SGUI_DLL unsigned int sgui_model_column_count( const sgui_model* model );

/**
 * \brief Get a list of items from an sgui_model
 *
 * \memberof sgui_model
 *
 * \note The returned list has to be at least the requested length, but may
 *       be longer. If, for instance, the requested list is a prefix of an
 *       already cached list, a model might as well return the entire cached
 *       list instead.
 *
 * \param model  A pointer to a model object
 * \param parent The parent item, or NULL for the topmost in the hirarchy
 * \param start  An offset into the list of children
 * \param count  The number of children to retrieve, 0 for all
 *
 * \return A pointer to the first item in the list, or NULL if empty
 */
SGUI_DLL const sgui_item* sgui_model_query_items( const sgui_model* model,
                                                  const sgui_item* parent,
                                                  unsigned int start,
                                                  unsigned int count );
/**
 * \brief Free a list of items obtained from sgui_model_query_items
 *
 * \param model A pointer to a model object
 * \param start A pointer to the start of the list
 */
SGUI_DLL void sgui_model_free_item_list( sgui_model* model,
                                         const sgui_item* start );

/**
 * \brief Get the number of children an item has
 *
 * \memberof sgui_model
 *
 * \param model A pointer to a model object
 * \param item  The parent item, or NULL for the topmost in the hirarchy,
 *              i.e. all items that don't have a parent
 *
 * \return The number of children a given item has
 */
SGUI_DLL unsigned int sgui_model_item_children_count( const sgui_model* model,
                                                      const sgui_item* item );

/**
 * \brief Get the text of an item column
 *
 * \memberof sgui_model
 *
 * \param model  A pointer to the model owning the item
 * \param item   A pointer to an item
 * \param column The column index to get the text from
 *
 * \return A pointer to a text string, or NULL if empty
 */
SGUI_DLL const char* sgui_item_text( const sgui_model* model,
                                     const sgui_item* item,
                                     unsigned int column );

/**
 * \brief Get the asociated icon of an item entry
 *
 * \memberof sgui_model
 *
 * \param model  A pointer to the model owning the item
 * \param item   A pointer to an item
 * \param column The column index to get the icon from
 *
 * \return A pointer to an icon or NULL if there is no icon
 */
SGUI_DLL const sgui_icon* sgui_item_icon( const sgui_model* model,
                                          const sgui_item* item,
                                          unsigned int column );

/**
 * \brief Create a simple, in memory model
 *
 * \memberof sgui_simple_model
 *
 * \param columns The number of columns per entry
 * \param cache   A pointer to an icon cache or NULL if not used
 *
 * \return A pointer to a model implementation on success, NULL on failure
 */
SGUI_DLL sgui_model* sgui_simple_model_create( unsigned int columns,
                                               sgui_icon_cache* cache );

/**
 * \brief Add an item to an sgui_simple_model
 *
 * \memberof sgui_simple_model
 *
 * \param model  A pointer to the model object to add the item to
 * \param parent A pointer to the parent item, or NULL to add it to the root
 *
 * \return A pointer to an item on success, NULL on failure
 */
SGUI_DLL sgui_item* sgui_simple_model_add_item( sgui_model* model,
                                                sgui_item* parent );

/**
 * \brief Set the icon of an entry in an sgui_simple_item
 *
 * \memberof sgui_simple_model
 *
 * \param model  A pointer to an sgui_simple_model
 * \param item   A pointer to an item of that model to set the data of
 * \param column A column index
 * \param icon   The new icon to set
 */
SGUI_DLL void sgui_simple_item_set_icon( sgui_model* model, sgui_item* item,
                                         unsigned int column,
                                         const sgui_icon* icon );

/**
 * \brief Set the text of an entry in an sgui_simple_item
 *
 * \memberof sgui_simple_model
 *
 * \param model  A pointer to an sgui_simple_model
 * \param item   A pointer to an item of that model to set the data of
 * \param column A column index
 * \param text   The new text to set
 */
SGUI_DLL void sgui_simple_item_set_text( sgui_model* model, sgui_item* item,
                                         unsigned int column,
                                         const char* text );

#ifdef __cplusplus
}
#endif

#endif /* SGUI_MODEL_H */

