/*
 * sgui_internal.h
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
#ifndef SGUI_INTERNAL_H
#define SGUI_INTERNAL_H



#include "sgui_widget.h"
#include "sgui_widget_manager.h"
#include "sgui_link.h"



#define SGUI_CANVAS_STACK_DEPTH 10

struct sgui_canvas
{
    int allow_clear;
    int ox, oy;

    unsigned int width, height;

    sgui_rect sc;

    sgui_rect sc_stack[ SGUI_CANVAS_STACK_DEPTH ];
    unsigned int scissor_stack_pointer;

    int offset_stack_x[ SGUI_CANVAS_STACK_DEPTH ];
    int offset_stack_y[ SGUI_CANVAS_STACK_DEPTH ];
    unsigned int offset_stack_pointer;

    int began;

    unsigned char bg_color[3];


    void(* begin )( sgui_canvas* canvas, sgui_rect* r );

    void(* end )( sgui_canvas* canvas );

    void(* clear )( sgui_canvas* canvas, sgui_rect* r );


    void(* blit )( sgui_canvas* canvas, int x, int y, unsigned int width,
                   unsigned int height, unsigned int scanline_length,
                   SGUI_COLOR_FORMAT format, const void* data );

    void(* blend )( sgui_canvas* canvas, int x, int y,
                    unsigned int width, unsigned int height,
                    unsigned int scanline_length, const void* data );

    void(* draw_box )( sgui_canvas* canvas, sgui_rect* r,
                       unsigned char* color, SGUI_COLOR_FORMAT format );

    void(* draw_line )( sgui_canvas* canvas, int x, int y,
                        unsigned int length, int horizontal,
                        unsigned char* color, SGUI_COLOR_FORMAT format );

    void(* blend_stencil )( sgui_canvas* canvas, unsigned char* buffer,
                            int x, int y, unsigned int w, unsigned int h,
                            unsigned int scan, unsigned char* color );
};

void sgui_internal_canvas_init( sgui_canvas* cv, unsigned int width,
                                unsigned int height );




struct sgui_widget
{
    int x;                  /**< \brief x component of widget position */
    int y;                  /**< \brief y component of widget position */
    unsigned int width;     /**< \brief widget width */
    unsigned int height;    /**< \brief widget height */

    int visible;            /**< \brief zero if the widget should not be
                                        rendered */

    /** \brief The widget manager responsible for that widget */
    sgui_widget_manager* mgr;

    /**
     * \brief List of links to implement signals and slots like widget
     *        intercommunication.
     */
    sgui_link_list* links;

    /**
     * \brief Callback that is called to draw a widget
     *
     * \param widget A pointer to the widget to update (for C++
     *               people: practically a this pointer).
     * \param cv     A pointer to a canvas to draw to.
     */
    void (* draw_callback )( sgui_widget* widget, sgui_canvas* cv );

    /**
     * \brief Callback that is called to update a widget
     *
     * \param widget A pointer to the widget to update (for C++
     *               people: practically a this pointer).
     */
    void (* update_callback )( sgui_widget* widget );

    /**
     * \brief Callback that is called to inject window events
     *
     * \param widget A pointer to the widget to update (for C++
     *               people: practically a this pointer).
     * \param type   The event type.
     * \param event  The window event that occoured.
     */
    void (* window_event_callback )( sgui_widget* widget, int type,
                                     sgui_event* event );
};



void sgui_internal_widget_init( sgui_widget* widget, int x, int y,
                                unsigned int width, unsigned int height,
                                int triggers_events );

void sgui_internal_widget_deinit( sgui_widget* widget );

void sgui_internal_widget_fire_event( sgui_widget* widget, int event );



#endif /* SGUI_WIDGET_INTERNAL_H */

