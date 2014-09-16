/*
 * model.c
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
#define SGUI_BUILDING_DLL
#include "sgui_internal.h"
#include "sgui_model.h"

#include <stdlib.h>
#include <string.h>



#ifndef SGUI_NO_MODEL
void sgui_model_destroy( sgui_model* this )
{
    if( this )
        this->destroy( this );
}

sgui_icon_cache* sgui_model_get_icon_cache( const sgui_model* this )
{
    return this ? this->cache : NULL;
}

unsigned int sgui_model_column_count( const sgui_model* this )
{
    return this ? this->cols : 0;
}

const sgui_item* sgui_model_query_items( const sgui_model* this,
                                         const sgui_item* parent,
                                         unsigned int start,
                                         unsigned int count )
{
    if( this )
        return this->query_items( this, parent, start, count );

    return NULL;
}

void sgui_model_free_item_list( sgui_model* this, const sgui_item* start )
{
    if( this && start && this->free_item_list )
        this->free_item_list( this, start );
}

unsigned int sgui_model_item_children_count( const sgui_model* this,
                                             const sgui_item* item )
{
    return this ? this->item_children_count( this, item ) : 0;
}

const char* sgui_item_text( const sgui_model* this, const sgui_item* item,
                            unsigned int column )
{
    if( this && item && (column < this->cols) && this->item_text )
        return this->item_text( this, item, column );

    return NULL;
}

const sgui_icon* sgui_item_icon( const sgui_model* this,
                                 const sgui_item* item,
                                 unsigned int column )
{
    if(this && item && this->cache && (column<this->cols) && this->item_icon)
        return this->item_icon( this, item, column );

    return NULL;
}

/****************************************************************************/

typedef struct
{
    sgui_item super;

    sgui_item* children;
}
sgui_simple_item;

typedef struct
{
    sgui_model super;

    sgui_item* items;
}
sgui_simple_model;



static void destroy_itemlist( sgui_simple_model* this, sgui_item* list )
{
    unsigned char* text;
    unsigned int col;
    sgui_item* old;

    while( list )
    {
        destroy_itemlist( this, ((sgui_simple_item*)list)->children );

        old = list;
        list = list->next;

        text  = (unsigned char*)old;
        text += sizeof(sgui_simple_item);

        for( col=0; col<this->super.cols; ++col )
        {
            free( *((char**)text) );
            text += sizeof(void*)*2;
        }

        free( old );
    }
}



static void simple_destroy( sgui_model* super )
{
    sgui_simple_model* this = (sgui_simple_model*)super;

    destroy_itemlist( this, this->items );
    free( this );
}

static const sgui_item* simple_query_items( const sgui_model* super,
                                            const sgui_item* parent,
                                            unsigned int start,
                                            unsigned int count )
{
    const sgui_simple_model* this = (sgui_simple_model*)super;
    const sgui_simple_item* item = (sgui_simple_item*)parent;
    const sgui_item* list = item ? item->children : this->items;
    unsigned int i;
    (void)count;

    sgui_internal_lock_mutex( );

    for( i=0; list && i<start; ++i )
        list = list->next;

    sgui_internal_unlock_mutex( );
    return list;
}

static unsigned int simple_item_children_count( const sgui_model* super,
                                                const sgui_item* item )
{
    const sgui_simple_model* this = (sgui_simple_model*)super;
    unsigned int count;

    sgui_internal_lock_mutex( );
    item = item ? ((sgui_simple_item*)item)->children : this->items;

    for( count=0; item; ++count )
        item = item->next;

    sgui_internal_unlock_mutex( );

    return count;
}

static const char* simple_item_text( const sgui_model* this,
                                     const sgui_item* item,
                                     unsigned int column )
{
    unsigned char* ptr = (unsigned char*)item;
    (void)this;

    ptr += sizeof(sgui_simple_item);
    ptr += sizeof(void*)*2*column;

    return *((const char**)ptr);
}

static sgui_icon* simple_item_icon( const sgui_model* this,
                                    const sgui_item* item,
                                    unsigned int column )
{
    unsigned char* ptr = (unsigned char*)item;
    (void)this;

    ptr += sizeof(sgui_simple_item);
    ptr += sizeof(void*)*(2*column + 1);

    return *((sgui_icon**)ptr);
}



sgui_model* sgui_simple_model_create( unsigned int columns,
                                      sgui_icon_cache* cache )
{
    sgui_simple_model* this;
    sgui_model* super;

    if( !columns )
        return NULL;

    this = malloc( sizeof(sgui_simple_model) );
    super = (sgui_model*)this;
    
    if( !this )
        return NULL;

    memset( this, 0, sizeof(sgui_simple_model) );

    super->cache               = cache;
    super->cols                = columns;
    super->destroy             = simple_destroy;
    super->query_items         = simple_query_items;
    super->item_children_count = simple_item_children_count;
    super->item_text           = simple_item_text;
    super->item_icon           = simple_item_icon;

    return super;
}

sgui_item* sgui_simple_model_add_item( sgui_model* super, sgui_item* parent )
{
    sgui_simple_model* this = (sgui_simple_model*)super;
    sgui_item* item = NULL;
    unsigned int size;

    if( this )
    {
        size = sizeof(sgui_simple_item) + sizeof(void*)*2*super->cols;
        item = malloc( size );

        if( item )
        {
            memset( item, 0, size );

            sgui_internal_lock_mutex( );

            if( parent )
            {
                item->next = ((sgui_simple_item*)parent)->children;
                ((sgui_simple_item*)parent)->children = item;
            }
            else
            {
                item->next = this->items;
                this->items = item;
            }

            sgui_internal_unlock_mutex( );
        }
    }

    return item;
}

void sgui_simple_item_set_icon( sgui_model* this, sgui_item* item,
                                unsigned int column, const sgui_icon* icon )
{
    unsigned char* ptr = (unsigned char*)item;

    if( ptr && this && column<this->cols && this->cache )
    {
        sgui_internal_lock_mutex( );

        ptr += sizeof(sgui_simple_item);
        ptr += sizeof(void*)*(2*column + 1);

        *((const sgui_icon**)ptr) = icon;

        sgui_internal_unlock_mutex( );
    }
}

void sgui_simple_item_set_text( sgui_model* this, sgui_item* item,
                                unsigned int column, const char* text )
{
    unsigned char* ptr = (unsigned char*)item;

    if( ptr && this && column<this->cols )
    {
        sgui_internal_lock_mutex( );

        ptr += sizeof(sgui_simple_item);
        ptr += sizeof(void*)*2*column;

        free( *((char**)ptr) );

        if( text )
        {
            *((char**)ptr) = malloc( strlen( text )+1 );
            strcpy( *((char**)ptr), text );
        }

        sgui_internal_unlock_mutex( );
    }
}
#elif defined(SGUI_NOP_IMPLEMENTATIONS)
void sgui_model_destroy( sgui_model* model )
{
    (void)model;
}
sgui_icon_cache* sgui_model_get_icon_cache( const sgui_model* model )
{
    (void)model;
    return NULL;
}
unsigned int sgui_model_column_count( const sgui_model* model )
{
    (void)model;
    return 0;
}
const sgui_item* sgui_model_query_items( const sgui_model* model,
                                         const sgui_item* parent,
                                         unsigned int start,
                                         unsigned int count )
{
    (void)model; (void)parent; (void)start; (void)count;
    return NULL;
}
void sgui_model_free_item_list( sgui_model* model, const sgui_item* start )
{
    (void)model; (void)start;
}
unsigned int sgui_model_item_children_count( const sgui_model* model,
                                             const sgui_item* item )
{
    (void)model; (void)item;
    return 0;
}
const char* sgui_item_text( const sgui_model* model, const sgui_item* item,
                            unsigned int column )
{
    (void)model; (void)item; (void)column;
    return NULL;
}
const sgui_icon* sgui_item_icon( const sgui_model* model,
                                 const sgui_item* item,
                                 unsigned int column )
{
    (void)model; (void)item; (void)column;
    return NULL;
}
sgui_model* sgui_simple_model_create( unsigned int columns,
                                      sgui_icon_cache* cache )
{
    (void)columns; (void)cache;
    return NULL;
}
sgui_item* sgui_simple_model_add_item( sgui_model* model, sgui_item* parent )
{
    (void)model; (void)parent;
    return NULL;
}
void sgui_simple_item_set_icon( sgui_model* model, sgui_item* item,
                                unsigned int column, const sgui_icon* icon )
{
    (void)model; (void)item; (void)column; (void)icon;
}
void sgui_simple_item_set_text( sgui_model* model, sgui_item* item,
                                unsigned int column, const char* text )
{
    (void)model; (void)item; (void)column; (void)text;
}
#endif

