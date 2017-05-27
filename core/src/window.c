/*
 * window.c
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
#include "sgui_window.h"
#include "sgui_event.h"
#include "sgui_canvas.h"
#include "sgui_skin.h"
#include "sgui_context.h"
#include "sgui_widget.h"
#include "sgui_lib.h"

#include <stddef.h>
#include <string.h>


#define COPY_KEY SGUI_KC_C
#define SELECT_KEY SGUI_KC_A
#define PASTE_KEY SGUI_KC_V
#define CUT_KEY SGUI_KC_X


static void send_raw_event(sgui_window *this, const sgui_event *e)
{
	if (this->event_fun)
		this->event_fun(this->userptr, e);

	sgui_event_post(e);

	if (this->canvas)
		sgui_canvas_send_window_event(this->canvas, e);
}

void sgui_internal_window_fire_event(sgui_window *this, const sgui_event *e)
{
	sgui_event ev;
	int mask;

	if (e->type == SGUI_KEY_PRESSED_EVENT ||
		e->type == SGUI_KEY_RELEASED_EVENT) {
		mask = 0;

		switch (e->arg.i) {
		case SGUI_KC_SHIFT:
		case SGUI_KC_LSHIFT:
		case SGUI_KC_RSHIFT:
			mask = SGUI_MOD_SHIFT;
			break;
		case SGUI_KC_CONTROL:
		case SGUI_KC_LCONTROL:
		case SGUI_KC_RCONTROL:
			mask = SGUI_MOD_CTRL;
			break;
		case SGUI_KC_ALT:
		case SGUI_KC_LALT:
		case SGUI_KC_RALT:
			mask = SGUI_MOD_ALT;
			break;
		case SGUI_KC_LSUPER:
		case SGUI_KC_RSUPER:
			mask = SGUI_MOD_SUPER;
			break;
		}

		if (e->type == SGUI_KEY_PRESSED_EVENT) {
			this->modmask |= mask;
		} else {
			this->modmask &= ~mask;
		}
	}

	send_raw_event(this, e);

	/* generate events for special key combinations */
	if ((this->modmask == SGUI_MOD_CTRL) &&
		(e->type == SGUI_KEY_PRESSED_EVENT ||
		e->type == SGUI_KEY_RELEASED_EVENT)) {
		ev.type = e->type;
		ev.src.window = e->src.window;

		switch (e->arg.i) {
		case COPY_KEY:
			ev.arg.i = SGUI_KC_COPY;
			break;
		case SELECT_KEY:
			ev.arg.i = SGUI_KC_SELECT_ALL;
			break;
		case PASTE_KEY:
			ev.arg.i = SGUI_KC_PASTE;
			break;
		case CUT_KEY:
			ev.arg.i = SGUI_KC_CUT;
			break;
		}

		send_raw_event(this, &ev);
	}
}

/****************************************************************************/

sgui_window *sgui_window_create_simple(sgui_lib *lib, sgui_window *parent,
				unsigned int width, unsigned int height,
				int flags)
{
	sgui_window_description desc;

	memset(&desc, 0, sizeof(desc));
	desc.parent = parent;
	desc.share = NULL;
	desc.width = width;
	desc.height = height;
	desc.flags = flags;
	desc.backend = SGUI_NATIVE;
	desc.bits_per_pixel = 32;
	desc.depth_bits = 24;
	desc.stencil_bits = 8;

	return lib->create_window(lib, &desc);
}

void sgui_window_set_mouse_position(sgui_window *this, int x, int y,
					int send_event)
{
	sgui_event e;

	this->set_mouse_position(this, x, y);

	if (send_event) {
		e.arg.i2.x = x;
		e.arg.i2.y = y;
		e.src.window = this;
		e.type = SGUI_MOUSE_MOVE_EVENT;
		sgui_internal_window_fire_event(this, &e);
	}
}

/****************************************************************************/

void sgui_window_pack(sgui_window *this)
{
	unsigned int width, height;
	sgui_rect r, acc;
	sgui_widget *w;

	sgui_internal_lock_mutex();

	if (this->canvas)
		goto done;

	if (!(w = this->canvas->root.children))
		goto done;

	/* get bounding rectangle for all widgets */
	acc = w->area;

	for (w = w->next; w != NULL; w = w->next) {
		r = w->area;
		sgui_rect_join(&acc, &r, 0);
	}

	/* compute window size */
	width = SGUI_RECT_WIDTH(acc);
	height = SGUI_RECT_HEIGHT(acc);

	if (acc.left > 0)
		width += 2 * acc.left;

	if (acc.top > 0)
		height += 2 * acc.top;

	sgui_window_set_size(this, width, height);
done:
	sgui_internal_unlock_mutex();
}
