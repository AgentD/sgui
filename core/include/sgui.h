/*
 * sgui.h
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
 * \file sgui.h
 *
 * \brief This is the global sgui header
 *
 * This file includes all other sgui headers and declares the initialization,
 * cleanup and main loop functions.
 */
#ifndef SGUI_H
#define SGUI_H



#include "sgui_canvas.h"
#include "sgui_context.h"
#include "sgui_event.h"
#include "sgui_font.h"
#include "sgui_icon_cache.h"
#include "sgui_keycodes.h"
#include "sgui_model.h"
#include "sgui_pixmap.h"
#include "sgui_predef.h"
#include "sgui_rect.h"
#include "sgui_skin.h"
#include "sgui_utf8.h"
#include "sgui_window.h"

#include "sgui_widget.h"
#include "sgui_button.h"
#include "sgui_color_picker.h"
#include "sgui_edit_box.h"
#include "sgui_frame.h"
#include "sgui_group_box.h"
#include "sgui_icon_view.h"
#include "sgui_image.h"
#include "sgui_label.h"
#include "sgui_numeric_edit.h"
#include "sgui_pass_box.h"
#include "sgui_progress_bar.h"
#include "sgui_scroll_bar.h"
#include "sgui_slider.h"
#include "sgui_subview.h"
#include "sgui_tab.h"

#include "sgui_dialog.h"
#include "sgui_color_dialog.h"
#include "sgui_messagebox.h"



#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Initialise sgui
 *
 * Call this before any other sgui function.
 *
 * \return Non-zero on success, zero on error.
 */
SGUI_DLL int sgui_init( void );

/**
 * \brief Uninitialise sgui
 *
 * Call this once you are done using sgui.
 */
SGUI_DLL void sgui_deinit( void );

/**
 * \brief Enter the sgui main loop
 *
 * This function processes window system messages, relays them to windows and
 * asks them to update. The function does not return as long as there are
 * windows visible.
 */
SGUI_DLL void sgui_main_loop( void );

/**
 * \brief Execute a single step of the main loop
 *
 * This function executes a single step of the main loop inside the
 * sgui_main_loop function, but returns after processing a single system
 * message.
 * Unlike the main loop function, it does not wait for system messages and
 * returns immediately if there are no messages left.
 *
 * \return Non-zero if there is at least one window visible, zero if there
 *         are no more visible windows.
 */
SGUI_DLL int sgui_main_loop_step( void );

#ifdef __cplusplus
}
#endif

#endif /* SGUI_H */

