/*
 * messagebox.c
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
#include "sgui_messagebox.h"
#include "sgui_internal.h"
#include "sgui_dialog.h"
#include "sgui_button.h"
#include "sgui_widget.h"
#include "sgui_image.h"
#include "sgui_label.h"
#include "sgui_event.h"
#include "sgui_rect.h"

#include <stdlib.h>
#include <string.h>



#ifndef SGUI_NO_MESSAGEBOX
typedef struct
{
    sgui_dialog super;

    sgui_widget* text;
    sgui_widget* icon;
}
sgui_message_box;



static void message_box_button_pressed( sgui_dialog* super, int button )
{
    sgui_message_box* this = (sgui_message_box*)super;
    struct sgui_event ev;

    ev.src.other = this;

    switch( button )
    {
    case 0:  ev.type = SGUI_MESSAGE_BOX_BUTTON1_EVENT; break;
    case 1:  ev.type = SGUI_MESSAGE_BOX_BUTTON2_EVENT; break;
    case 2:  ev.type = SGUI_MESSAGE_BOX_BUTTON3_EVENT; break;
    default: ev.type = SGUI_DIALOG_REJECTED;           break;
    }

    sgui_window_set_visible( super->window, SGUI_INVISIBLE );
    sgui_event_post( &ev );
}

static void sgui_message_box_destroy( sgui_dialog* super )
{
    sgui_message_box* this = (sgui_message_box*)super;
    sgui_widget_destroy( this->text );
    sgui_widget_destroy( this->icon );
    free( this );
}

/****************************************************************************/

sgui_dialog* sgui_message_box_create( int icon, const char* caption,
                                      const char* text,
                                      const char* button1,
                                      const char* button2,
                                      const char* button3 )
{
    unsigned int text_w, text_h, icon_w, icon_h, w, h, y;
    sgui_rect r, icon_area;
    sgui_message_box* this;
    sgui_dialog* super;
    sgui_skin* skin;
    sgui_canvas* cv;
    const sgui_pixmap* p;

    if( (!button1 && !button2 && !button3) || !text || !caption )
        return NULL;

    /* create dialog structure */
    this = calloc( 1, sizeof(sgui_message_box) );
    super = (sgui_dialog*)this;

    if( !this )
        return NULL;

    super->destroy = sgui_message_box_destroy;
    super->handle_button = message_box_button_pressed;

    /* determine element sizes */
    skin = sgui_skin_get();
    skin->get_icon_area(skin, &icon_area, icon);

    icon_w = SGUI_RECT_WIDTH(icon_area);
    icon_h = SGUI_RECT_HEIGHT(icon_area);

    sgui_skin_get_text_extents( text, &r );
    text_w = SGUI_RECT_WIDTH(r);
    text_h = SGUI_RECT_HEIGHT(r);

    w = 10 + icon_w + 10 + text_w + 10;
    h = 10 + MAX(text_h, icon_h) + 5;

    /* create the message box window */
    super->window = sgui_window_create( NULL, w, h, SGUI_FIXED_SIZE );

    if( !super->window )
        goto fail;

    sgui_window_set_title( super->window, caption );

    /* create widgets */
    y = icon_h > text_h ? (icon_h - text_h)/2 : 0;
    this->text = sgui_label_create( 10 + icon_w + 10, y + 10, text );

    if( !this->text )
        goto fail;

    y = icon_h > text_h ? 0 : (text_h  -icon_h)/2;

    cv = sgui_window_get_canvas(super->window);
    p = cv->get_skin_pixmap(cv);
    this->icon = sgui_image_from_pixmap(10, 10 + y, icon_w, icon_h, p,
                                        icon_area.left, icon_area.top,
                                        SGUI_IMAGE_BLEND);

    if( !this->icon )
        goto fail;

    if( !sgui_dialog_init( super, button1, button2, button3, SGUI_CENTER ) )
        goto fail;

    sgui_window_add_widget( super->window, this->text );
    sgui_window_add_widget( super->window, this->icon );
    return (sgui_dialog*)this;
fail:
    sgui_dialog_destroy( super );
    return NULL;
}
#elif defined(SGUI_NOP_IMPLEMENTATIONS)
sgui_dialog* sgui_message_box_create( int icon, const char* caption,
                                      const char* text, const char* button1,
                                      const char* button2,
                                      const char* button3 )
{
    (void)icon; (void)caption; (void)text;
    (void)button1; (void)button2; (void)button3;
    return NULL;
}
#endif /* !SGUI_NO_MESSAGEBOX */

