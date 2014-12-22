/*
 * group_box.c
 * This file is part of sgio
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
#include "sgui_group_box.h"
#include "sgui_skin.h"
#include "sgui_canvas.h"
#include "sgui_internal.h"
#include "sgui_widget.h"

#include <stdlib.h>
#include <string.h>



#ifndef SGUI_NO_GROUP_BOX
typedef struct
{
    sgui_widget super;
    char* caption;
}
sgui_group_box;



static void group_box_draw( sgui_widget* super )
{
    sgui_group_box* this = (sgui_group_box*)super;
    sgui_skin* skin = sgui_skin_get( );

    skin->draw_group_box(skin, super->canvas, &(super->area), this->caption);
}

static void group_box_destroy( sgui_widget* super )
{
    sgui_group_box* this = (sgui_group_box*)super;

    free( this->caption );
    free( this );
}



sgui_widget* sgui_group_box_create( int x, int y,
                                    unsigned int width, unsigned int height,
                                    const char* caption )
{
    sgui_group_box* this = malloc( sizeof(sgui_group_box) );
    sgui_widget* super = (sgui_widget*)this;

    if( !this )
        return NULL;

    /* try to store the caption string */
    this->caption = malloc( strlen( caption ) + 1 );

    if( !this->caption )
    {
        free( this );
        return NULL;
    }

    strcpy( this->caption, caption );

    /* initialize widget base struct */
    sgui_widget_init( super, x, y, width, height );

    super->draw = group_box_draw;
    super->destroy = group_box_destroy;
    super->focus_policy = 0;

    return super;
}
#elif defined(SGUI_NOP_IMPLEMENTATIONS)
sgui_widget* sgui_group_box_create( int x, int y,
                                    unsigned int width, unsigned int height,
                                    const char* caption )
{
    (void)x; (void)y; (void)width; (void)height; (void)caption;
    return NULL;
}
#endif /* !SGUI_NO_GROUP_BOX */

