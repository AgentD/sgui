/*
 * sgui_signals.h
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
#ifndef SGUI_SIGNALS_H
#define SGUI_SIGNALS_H

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum
{
    SGUI_VOID = 0,
    SGUI_INT,
    SGUI_UINT,
    SGUI_INT_VEC2,
    SGUI_UINT_VEC2,

    SGUI_INT_FROM_FUNCTION,
    SGUI_UINT_FROM_FUNCTION
}
SGUI_DATA_TYPE;



typedef struct
{
    union
    {
        int i;
        unsigned int ui;

        struct { int x, y; } ivec2;

        struct { unsigned int x, y; } uivec2;

        struct { void* obj; void* fun; } get_fun;
    }
    data;

    SGUI_DATA_TYPE type;
}
sgui_variant;



typedef struct sgui_link_list sgui_link_list;



/**
 * \brief Create a list of links
 *
 * \note This is a list links, not a linked list!! It implements something
 *       signals and slots like.
 */
sgui_link_list* sgui_link_list_create( void );

/** \brief Destroy a list of links */
void sgui_link_list_destroy( sgui_link_list* list );

/**
 * \brief Add a link to a list of links
 *
 * \param list     The list to add the link to.
 * \param event    An integer identifyer that triggers the link.
 * \param function A function to be called when the link triggers.
 * \param this_ptr The object to execute the function on. Passed as first
 *                 argument to the function.
 * \param arg      An argument to pass as second argument to the function.
 */
void sgui_link_list_add( sgui_link_list* list, int event, void* function,
                         void* this_ptr, sgui_variant arg );

/**
 * \brief Process a list of link
 *
 * \param list  The list to process
 * \param event The event that got triggered. All links registered for the
 *              given event will have theire registered functions called with
 *              the registered arguments.
 */
void sgui_link_list_process( sgui_link_list* list, int event );

#ifdef __cplusplus
}
#endif

#endif /* SGUI_SIGNALS_H */

