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
#include "platform.h"

#include "sgui_config.h"
#include "sgui_event.h"
#include "sgui_context.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>


static void xlib_window_size_hints(Window wnd, sgui_lib_x11 *lib,
					unsigned int w, unsigned int h,
					unsigned int min_w, unsigned int min_h,
					unsigned int max_w, unsigned int max_h)
{
	XSizeHints hints;

	memset(&hints, 0, sizeof(hints));
	hints.flags = PSize | PMinSize | PMaxSize;
	hints.min_width = min_w;
	hints.min_height = min_h;
	hints.base_width = w;
	hints.base_height = h;
	hints.max_width = max_w;
	hints.max_height = max_h;

	XSetWMNormalHints(lib->dpy, wnd, &hints);
}

static void xlib_window_get_mouse_position(sgui_window *this, int *x, int *y)
{
	sgui_lib_x11 *lib = (sgui_lib_x11 *)this->lib;
	Window t1, t2;		/* values we are not interested */
	int t3, t4;		/* into but xlib does not accept */
	unsigned int t5;	/* a NULL pointer for these */

	sgui_internal_lock_mutex();
	XQueryPointer(lib->dpy, TO_X11(this)->wnd, &t1, &t2, &t3, &t4,
			x, y, &t5);
	sgui_internal_unlock_mutex();
}

static void xlib_window_set_mouse_position(sgui_window *this, int x, int y)
{
	sgui_lib_x11 *lib = (sgui_lib_x11 *)this->lib;

	sgui_internal_lock_mutex();
	XWarpPointer(lib->dpy, None, TO_X11(this)->wnd, 0, 0,
			this->w, this->h, x, y);
	XFlush(lib->dpy);

	++(TO_X11(this)->mouse_warped);
	sgui_internal_unlock_mutex();
}

static int xlib_window_toggle_flags(sgui_window *super, int diff)
{
	sgui_window_xlib *this = (sgui_window_xlib *)super;
	sgui_lib_x11 *lib = (sgui_lib_x11 *)super->lib;
	int ret = 1;

	sgui_internal_lock_mutex();
	if (diff & SGUI_DOUBLEBUFFERED) {
		ret = 0;
		goto out;
	}

	if (diff & SGUI_VISIBLE) {
		if (super->flags & SGUI_VISIBLE) {
			XUnmapWindow(lib->dpy, this->wnd);
		} else {
			XMapWindow(lib->dpy, this->wnd);
		}
	}

	if (diff & SGUI_FIXED_SIZE) {
		if (super->flags & SGUI_FIXED_SIZE) {
			xlib_window_size_hints(this->wnd, lib,
						super->w, super->h,
						0, 0, INT_MAX, INT_MAX);
		} else {
			xlib_window_size_hints(this->wnd, lib,
						super->w, super->h,
						super->w, super->h,
						super->w, super->h);
		}
	}

	super->flags ^= diff;
out:
	sgui_internal_unlock_mutex();
	return ret;
}

static void xlib_window_set_title(sgui_window *this, const char *title)
{
	sgui_lib_x11 *lib = (sgui_lib_x11 *)this->lib;

	sgui_internal_lock_mutex();
	XStoreName(lib->dpy, TO_X11(this)->wnd, title);
	sgui_internal_unlock_mutex();
}

static void xlib_window_set_size(sgui_window *this, unsigned int width,
				unsigned int height)
{
	sgui_lib_x11 *lib = (sgui_lib_x11 *)this->lib;
	XWindowAttributes attr;

	sgui_internal_lock_mutex();

	if (this->flags & SGUI_FIXED_SIZE) {
		xlib_window_size_hints(TO_X11(this)->wnd, lib, width, height,
					width, height, width, height);
	}

	XResizeWindow(lib->dpy, TO_X11(this)->wnd, width, height);
	XFlush(lib->dpy);

	/* get the real geometry as the window manager is free to change it */
	XGetWindowAttributes(lib->dpy, TO_X11(this)->wnd, &attr);
	this->w = (unsigned int)attr.width;
	this->h = (unsigned int)attr.height;

	sgui_internal_unlock_mutex();
}

static void xlib_window_move_center(sgui_window *this)
{
	sgui_lib_x11 *lib = (sgui_lib_x11 *)this->lib;

	sgui_internal_lock_mutex();
	this->x = (DPY_WIDTH / 2) - (int)(this->w / 2);
	this->y = (DPY_HEIGHT / 2) - (int)(this->h / 2);

	XMoveWindow(lib->dpy, TO_X11(this)->wnd, this->x, this->y);
	sgui_internal_unlock_mutex();
}

static void xlib_window_move(sgui_window *this, int x, int y)
{
	sgui_lib_x11 *lib = (sgui_lib_x11 *)this->lib;

	sgui_internal_lock_mutex();
	XMoveWindow(lib->dpy, TO_X11(this)->wnd, x, y);
	sgui_internal_unlock_mutex();
}

static void xlib_window_force_redraw(sgui_window *this, sgui_rect *r)
{
	sgui_lib_x11 *lib = (sgui_lib_x11 *)this->lib;
	XExposeEvent exp;

	memset(&exp, 0, sizeof(exp));
	exp.type = Expose;
	exp.send_event = 1;
	exp.display = lib->dpy;
	exp.window = TO_X11(this)->wnd;
	exp.x = r->left;
	exp.y = r->top;
	exp.width = SGUI_RECT_WIDTH_V(r);
	exp.height = SGUI_RECT_HEIGHT_V(r);

	sgui_internal_lock_mutex();
	XSendEvent(lib->dpy, TO_X11(this)->wnd, False,
			ExposureMask, (XEvent *)&exp);
	sgui_internal_unlock_mutex();
}

static void xlib_window_destroy(sgui_window *this)
{
	sgui_lib_x11 *lib = (sgui_lib_x11 *)this->lib;

	sgui_internal_lock_mutex();
	sgui_internal_remove_window(this->lib, this);

	if (this->backend == SGUI_NATIVE) {
		sgui_canvas_destroy(this->ctx.canvas);
	} else if (this->backend != SGUI_CUSTOM) {
		this->ctx.ctx->destroy(this->ctx.ctx);
	}

	XDestroyIC(TO_X11(this)->ic);
	XDestroyWindow(lib->dpy, TO_X11(this)->wnd);
	sgui_internal_unlock_mutex();

	free(this);
}

static void xlib_window_get_platform_data(const sgui_window *this,
					void *window)
{
	*((Window*)window) = TO_X11(this)->wnd;
}

static void xlib_window_make_topmost(sgui_window *this)
{
	sgui_lib_x11 *lib = (sgui_lib_x11 *)this->lib;

	sgui_internal_lock_mutex();
	if (this->flags & SGUI_VISIBLE)
		XRaiseWindow(lib->dpy, TO_X11(this)->wnd);
	sgui_internal_unlock_mutex();
}

/****************************************************************************/

static void handle_key_release(sgui_window_xlib *this, XEvent *e)
{
	sgui_window *super = (sgui_window *)this;
	sgui_event se;
	XEvent nev;
	KeySym sym;

	/*
	    Mimic Windows(R) behaviour: Generate a sequence of keydown events
	    when a key is held down. X11 generates a sequence of KeyPress and
	    KeyRelease events. Remove the additional KeyRelease events.
	*/
	if (XPending(e->xkey.display) > 0) {
		XPeekEvent(e->xkey.display, &nev);

		if ((nev.type == KeyPress) &&
			(nev.xkey.keycode == e->xkey.keycode) &&
			(nev.xkey.time - e->xkey.time) < 2) {
			return;
		}
	}

	sym = XLookupKeysym(&e->xkey, 0);
	se.type = SGUI_KEY_RELEASED_EVENT;
	se.src.window = super;
	se.arg.i = key_entries_translate(sym);
	sgui_internal_window_fire_event(super, &se);
}

static void handle_key_press(sgui_window_xlib *this, XEvent *e)
{
	sgui_window *super = (sgui_window *)this;
	sgui_event se;
	Status stat;
	KeySym sym;

	se.src.window = super;

	memset(se.arg.utf8, 0, sizeof(se.arg.utf8));

	Xutf8LookupString(this->ic, &e->xkey, (char*)se.arg.utf8,
				sizeof(se.arg.utf8), &sym, &stat);

	if (stat == XLookupChars || stat == XLookupBoth) {
		if (!iscntrl(se.arg.utf8[0])) {
			se.type = SGUI_CHAR_EVENT;
			sgui_internal_window_fire_event(super, &se);
		}
	}

	if (stat == XLookupKeySym || stat == XLookupBoth) {
		se.arg.i = key_entries_translate(sym);
		se.type = SGUI_KEY_PRESSED_EVENT;
		sgui_internal_window_fire_event(super, &se);
	}
}

static void handle_mouse_button(sgui_window_xlib *this, XEvent *e)
{
	sgui_window *super = (sgui_window *)this;
	int double_click = 0;
	sgui_event se;

	se.type = (e->type == ButtonPress) ? SGUI_MOUSE_PRESS_EVENT :
						SGUI_MOUSE_RELEASE_EVENT;
	se.arg.i3.x = e->xbutton.x;
	se.arg.i3.y = e->xbutton.y;
	se.src.window = super;

	switch (e->xbutton.button) {
	case Button1:
		if (e->type == ButtonRelease && check_double_click(this))
			double_click = 1;

		se.arg.i3.z = SGUI_MOUSE_BUTTON_LEFT;
		break;
	case Button2:
		se.arg.i3.z = SGUI_MOUSE_BUTTON_MIDDLE;
		break;
	case Button3:
		se.arg.i3.z = SGUI_MOUSE_BUTTON_RIGHT;
		break;
	case Button4:
	case Button5:
		if (e->type != ButtonPress)
			return;

		se.type = SGUI_MOUSE_WHEEL_EVENT;
		se.arg.i = (e->xbutton.button == Button4) ? 1 : -1;
		break;
	default:
		return;
	}

	sgui_internal_window_fire_event(super, &se);

	if (double_click) {
		se.type = SGUI_DOUBLE_CLICK_EVENT;
		sgui_internal_window_fire_event(super, &se);
	}
}

static void handle_mouse_move(sgui_window_xlib *this, XEvent *e)
{
	sgui_window *super = (sgui_window *)this;
	sgui_event se;

	interrupt_double_click(super->lib);

	if (this->mouse_warped) {
		--(this->mouse_warped);
	} else {
		se.type = SGUI_MOUSE_MOVE_EVENT;
		se.src.window = super;
		se.arg.i2.x = e->xmotion.x < 0 ? 0 : e->xmotion.x;
		se.arg.i2.y = e->xmotion.y < 0 ? 0 : e->xmotion.y;
		sgui_internal_window_fire_event(super, &se);
	}
}

static void handle_resize(sgui_window_xlib *this, XEvent *e)
{
	sgui_window *super = (sgui_window *)this;
	sgui_event se;

	se.type = SGUI_SIZE_CHANGE_EVENT;
	se.src.window = super;
	se.arg.ui2.x = e->xconfigure.width;
	se.arg.ui2.y = e->xconfigure.height;

	super->x = e->xconfigure.x;
	super->y = e->xconfigure.y;

	if (!se.arg.ui2.x || !se.arg.ui2.y)
		return;

	if (se.arg.ui2.x == super->w && se.arg.ui2.y == super->h)
		return;

	super->w = (unsigned int)e->xconfigure.width;
	super->h = (unsigned int)e->xconfigure.height;

	if (super->backend == SGUI_NATIVE)
		sgui_canvas_resize(super->ctx.canvas, super->w, super->h);

	sgui_internal_window_fire_event(super, &se);
}

static void handle_map(sgui_window_xlib *this, XEvent *e)
{
	sgui_window *super = (sgui_window *)this;
	sgui_event se;

	se.src.window = super;

	if (e->type == MapNotify) {
		if (super->backend == SGUI_NATIVE)
			return;

		sgui_rect_set_size(&se.arg.rect, 0, 0, super->w, super->h);
		se.type = SGUI_EXPOSE_EVENT;
	} else {
		if (!(super->flags & IS_CHILD))
			return;
		if (e->xunmap.window != this->wnd)
			return;

		se.type = SGUI_USER_CLOSED_EVENT;
		super->flags &= ~SGUI_VISIBLE;
	}

	sgui_internal_window_fire_event(super, &se);
}

static void handle_client_message(sgui_window_xlib *this, XEvent *e)
{
	sgui_window *super = (sgui_window *)this;
	sgui_lib_x11 *lib = (sgui_lib_x11 *)super->lib;
	sgui_event se;

	if (e->xclient.data.l[0] == (long)lib->atom_wm_delete) {
		se.type = SGUI_USER_CLOSED_EVENT;
		se.src.window = super;
		super->flags &= ~SGUI_VISIBLE;
		XUnmapWindow(lib->dpy, this->wnd);
		XUnmapSubwindows(lib->dpy, this->wnd);
		sgui_internal_window_fire_event(super, &se);
	}
}

static void handle_expose(sgui_window_xlib *this, XEvent *e)
{
	sgui_window *super = (sgui_window *)this;
	sgui_event se;
	sgui_rect r;

	if (super->backend == SGUI_NATIVE) {
		sgui_rect_set_size(&r, e->xexpose.x, e->xexpose.y,
					e->xexpose.width, e->xexpose.height);
		sgui_canvas_redraw_area(super->ctx.canvas, &r, 1);
	} else {
		se.type = SGUI_EXPOSE_EVENT;
		se.src.window = super;
		sgui_rect_set_size(&se.arg.rect, 0, 0, super->w, super->h);
		sgui_internal_window_fire_event(super, &se);
	}
}

static void handle_focus_change(sgui_window_xlib *this, XEvent *e)
{
	sgui_window *super = (sgui_window *)this;
	sgui_event se;

	se.type = (e->type == FocusIn) ? SGUI_FOCUS_EVENT :
					SGUI_FOCUS_LOSE_EVENT;
	se.src.window = super;
	sgui_internal_window_fire_event(super, &se);
}

void handle_window_events(sgui_window_xlib *this, XEvent *e)
{
	switch (e->type) {
	case KeyRelease:
		handle_key_release(this, e);
		break;
	case KeyPress:
		handle_key_press(this, e);
		break;
	case ButtonPress:
	case ButtonRelease:
		handle_mouse_button(this, e);
		break;
	case MotionNotify:
		handle_mouse_move(this, e);
		break;
	case ConfigureNotify:
		handle_resize(this, e);
		break;
	case MapNotify:
	case UnmapNotify:
		handle_map(this, e);
		break;
	case ClientMessage:
		handle_client_message(this, e);
		break;
	case Expose:
		handle_expose(this, e);
		break;
	case FocusIn:
	case FocusOut:
		handle_focus_change(this, e);
		break;
	case DestroyNotify:
		((sgui_window *)this)->flags &= ~SGUI_VISIBLE;
		this->wnd = 0;
		break;
	}
}
/****************************************************************************/
sgui_window *x11_window_create(sgui_lib *lib,
				const sgui_window_description *desc)
{
	sgui_lib_x11 *lib_x11 = (sgui_lib_x11 *)lib;
	unsigned long color = 0;
	XWindowAttributes attr;
	sgui_window_xlib *this;
	sgui_window *super;
	sgui_color rgb;
	Window x_parent;

	if (!desc->width || !desc->height)
		return NULL;

	if (desc->flags & (~SGUI_ALL_WINDOW_FLAGS))
		return NULL;

	if (desc->backend == SGUI_OPENGL_CORE ||
		desc->backend == SGUI_OPENGL_COMPAT) {
        	this = calloc(1, sizeof(sgui_window_glx));
	} else {
		this = calloc(1, sizeof(sgui_window_xlib));
	}

	super = (sgui_window*)this;

	if (!this)
		return NULL;

	sgui_internal_lock_mutex();
	x_parent = desc->parent ? TO_X11(desc->parent)->wnd : lib_x11->root;

	switch (desc->backend) {
	case SGUI_OPENGL_CORE:
	case SGUI_OPENGL_COMPAT:
		this->wnd = create_glx_window(super, lib, desc, x_parent);
		break;
	default:
		rgb = sgui_skin_get()->window_color;
		color = (rgb.c.r << 16) | (rgb.c.g << 8) | (rgb.c.b);

		this->wnd = XCreateSimpleWindow(lib_x11->dpy, x_parent, 0, 0,
						desc->width, desc->height,
						0, 0, color);
		break;
	}

	if (!this->wnd)
		goto fail;

	if (desc->flags & SGUI_FIXED_SIZE) {
		xlib_window_size_hints(this->wnd, (sgui_lib_x11 *)lib,
					desc->width, desc->height,
					desc->width, desc->height,
					desc->width, desc->height);
	}

	XGetWindowAttributes(lib_x11->dpy, this->wnd, &attr);

	XSelectInput(lib_x11->dpy, this->wnd, X11_EVENT_MASK);
	XSetWMProtocols(lib_x11->dpy, this->wnd, &lib_x11->atom_wm_delete, 1);
	XFlush(lib_x11->dpy);

	this->ic = XCreateIC(lib_x11->im, XNInputStyle,
				XIMPreeditNothing | XIMStatusNothing,
				XNClientWindow, this->wnd, XNFocusWindow,
				this->wnd, NULL);

	if (!this->ic)
		goto failic;

	switch (desc->backend) {
	case SGUI_NATIVE:
		super->ctx.canvas = canvas_x11_create(lib, this->wnd,
							attr.width,
							attr.height, 1);
		if (!super->ctx.canvas)
			goto failcv;
		break;
	case SGUI_OPENGL_CORE:
	case SGUI_OPENGL_COMPAT:
		super->ctx.ctx = gl_context_create(super, lib, desc->backend,
							desc->share);

		if (!super->ctx.ctx)
			goto failcv;
		break;
	}

	if (desc->flags & SGUI_VISIBLE)
		XMapWindow(lib_x11->dpy, this->wnd);

	super->x = attr.x;
	super->y = attr.y;
	super->flags = desc->flags | (desc->parent != NULL ? IS_CHILD : 0);

	super->get_mouse_position = xlib_window_get_mouse_position;
	super->set_mouse_position = xlib_window_set_mouse_position;
	super->toggle_flags = xlib_window_toggle_flags;
	super->set_title = xlib_window_set_title;
	super->set_size = xlib_window_set_size;
	super->move_center = xlib_window_move_center;
	super->move = xlib_window_move;
	super->force_redraw = xlib_window_force_redraw;
	super->get_platform_data = xlib_window_get_platform_data;
	super->write_clipboard = xlib_window_clipboard_write;
	super->read_clipboard = xlib_window_clipboard_read;
	super->make_topmost = xlib_window_make_topmost;
	super->destroy = xlib_window_destroy;
	super->w = attr.width;
	super->h = attr.height;
	super->backend = desc->backend;
	super->modmask = 0;
	super->lib = lib;

	if (desc->backend == SGUI_NATIVE) {
		sgui_canvas_begin(super->ctx.canvas, NULL);
		sgui_canvas_clear(super->ctx.canvas, NULL);
		sgui_canvas_end(super->ctx.canvas);
	}

	sgui_internal_add_window(lib, super);
	sgui_internal_unlock_mutex();
	return (sgui_window *)this;
failcv:
	XDestroyIC(this->ic);
failic:
	XDestroyWindow(lib_x11->dpy, this->wnd);
fail:
	free(this);
	sgui_internal_unlock_mutex();
	return NULL;
}
