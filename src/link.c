/*
 * link.c
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
#include "sgui_link.h"

#include <stdlib.h>



typedef struct
{
    int event;
    void* function;
    void* this_ptr;
    sgui_variant arg;
}
sgui_link;



struct sgui_link_list
{
    sgui_link* links;
    unsigned int num_links;
    unsigned int num_avail;
};




sgui_link_list* sgui_link_list_create( void )
{
    sgui_link_list* l;

    l = malloc( sizeof(sgui_link_list) );

    l->links     = malloc( sizeof(sgui_link) * 10 );
    l->num_links = 0;
    l->num_avail = 10;

    return l;
}

void sgui_link_list_destroy( sgui_link_list* list )
{
    if( list )
    {
        free( list->links );
        free( list );
    }
}

void sgui_link_list_add( sgui_link_list* list, int event, void* function,
                         void* this_ptr, sgui_variant arg )
{
    if( list && function )
    {
        if( list->num_avail == list->num_links )
        {
            list->num_avail *= 2;

            list->links = realloc( list->links,
                                   sizeof(sgui_link) * list->num_avail );
        }

        list->links[ list->num_links ].event    = event;
        list->links[ list->num_links ].function = function;
        list->links[ list->num_links ].this_ptr = this_ptr;
        list->links[ list->num_links ].arg      = arg;

        ++list->num_links;
    }
}

void sgui_link_list_process( sgui_link_list* list, int event )
{
    unsigned int i;
    sgui_link* l;

    union
    {
        void* vp;

        void(* f_void )(void*);
        void(* f_float )(void*,float);
        void(* f_int )(void*,int);
        void(* f_uint )(void*,unsigned int);
        void(* f_int2 )(void*,int,int);
        void(* f_uint2 )(void*,unsigned int,unsigned int);
    }
    fun;

    if( !list )
        return;

    for( i=0; i<list->num_links; ++i )
    {
        l = &list->links[ i ];

        if( l->event != event )
            continue;

        fun.vp = l->function;

        switch( l->arg.type )
        {
        case SGUI_VOID:
            fun.f_void( l->this_ptr );
            break;
        case SGUI_FLOAT:
            fun.f_float( l->this_ptr, l->arg.data.f );
            break;
        case SGUI_INT:
            fun.f_int( l->this_ptr, l->arg.data.i );
            break;
        case SGUI_UINT:
            fun.f_uint( l->this_ptr, l->arg.data.ui );
            break;
        case SGUI_INT_VEC2:
            fun.f_int2( l->this_ptr, l->arg.data.ivec2.x,
                                     l->arg.data.ivec2.y );
            break;
        case SGUI_UINT_VEC2:
            fun.f_uint2( l->this_ptr, l->arg.data.uivec2.x,
                                      l->arg.data.uivec2.y );
            break;
        }
    }
}


