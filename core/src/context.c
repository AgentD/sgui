/*
 * context.c
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
#include "sgui_context.h"
#include "sgui_internal.h"

#include <stddef.h>



sgui_context* sgui_context_create_share( sgui_context* this )
{
    return this ? this->create_share( this ) : NULL;
}

void sgui_context_destroy( sgui_context* this )
{
    if( this )
        this->destroy( this );
}

void sgui_context_make_current( sgui_context* this, sgui_window* wnd )
{
    if( this && this->make_current )
        this->make_current( this, wnd );
}

void sgui_context_release_current( sgui_context* this )
{
    if( this && this->release_current )
        this->release_current( this );
}

sgui_funptr sgui_context_load( sgui_context* this, const char* name )
{
    return (this && name && this->load) ? this->load( this, name ) : NULL;
}

void* sgui_context_get_internal( sgui_context* this )
{
    return this ? this->get_internal( this ) : NULL;
}

