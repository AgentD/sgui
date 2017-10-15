/*
 * platform.c
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
#include "sgui.h"
#include "platform.h"

#include <sys/select.h>
#include <pthread.h>
#include <time.h>

sgui_lib_x11 x11;

static int resize_clipboard_buffer(sgui_lib_x11 *lib, unsigned int additional)
{
	char *new;

	new = realloc(lib->clipboard_buffer, lib->clipboard_size + additional);

	if (!new)
		return 0;

	lib->clipboard_buffer = new;
	lib->clipboard_size += additional;
	return 1;
}

/* used by wait_for_event to filter for a certain event type */
static Bool filter_event(Display *display, XEvent *event, XPointer arg)
{
	int type = *((int *)arg);
	(void)display; (void)arg;
	return event->type == type;
}

/* receives Xlib events and returns when a certain event is in the queue */
static void wait_for_event(sgui_lib_x11 *lib, XEvent *ret, int type)
{
	XIfEvent(lib->dpy, ret, filter_event, (XPointer)&type);
}

/* Xlib error callback to prevent xlib from crashing our program on error */
static int xlib_swallow_errors(Display *display, XErrorEvent *event)
{
	(void)display; (void)event;
	return 0;
}

static void handle_selection_event(sgui_lib_x11 *lib, XEvent *e)
{
	XEvent respond;
	long data[2];

	if (e->xselectionrequest.target == lib->atom_UTF8) {
		XChangeProperty(lib->dpy, e->xselectionrequest.requestor,
				e->xselectionrequest.property,
				e->xselectionrequest.target, 8,
				PropModeReplace,
				(unsigned char *)lib->clipboard_buffer,
				lib->clipboard_strlen);
		respond.xselection.property = e->xselectionrequest.property;
	} else if (e->xselectionrequest.target == lib->atom_targets) {
		data[0] = lib->atom_text;
		data[1] = lib->atom_UTF8;

		XChangeProperty(lib->dpy, e->xselectionrequest.requestor,
				e->xselectionrequest.property,
				e->xselectionrequest.target, 8,
				PropModeReplace, (unsigned char *)&data,
				sizeof(data));
		respond.xselection.property = e->xselectionrequest.property;
	} else {
		respond.xselection.property = None;
	}

	respond.xselection.type = SelectionNotify;
	respond.xselection.display = e->xselectionrequest.display;
	respond.xselection.requestor = e->xselectionrequest.requestor;
	respond.xselection.selection = e->xselectionrequest.selection;
	respond.xselection.target = e->xselectionrequest.target;
	respond.xselection.time = e->xselectionrequest.time;
	XSendEvent(lib->dpy, e->xselectionrequest.requestor, 0, 0, &respond);
	XFlush(lib->dpy);
}

/* fetch and process the next Xlib event */
static void handle_events(sgui_lib_x11 *lib)
{
	sgui_window *i;
	XEvent e;

	while (XPending(lib->dpy) > 0) {
		XNextEvent(lib->dpy, &e);

		if (e.type == SelectionRequest) {
			handle_selection_event(lib, &e);
			continue;
		}

		if (XFilterEvent(&e, None))
			continue;

		for (i = ((sgui_lib *)lib)->wndlist; i != NULL; i = i->next) {
			if (((sgui_window_xlib *)i)->wnd == e.xany.window)
				break;
		}

		if (i != NULL)
			handle_window_events((sgui_window_xlib *)i, &e);
	}
}

/****************************************************************************/

static unsigned int sgui_utf8_from_latin1_length(const char *in)
{
	unsigned int length = 0;

	while (*in) {
		length += (*(in++) & 0x80) ? 2 : 1;
	}

	return length;
}

static void sgui_utf8_from_latin1(char *out, const char *in)
{
	for (; *in; ++in) {
		if (*in & 0x80) {
			*(out++) = 0xC2 + ((unsigned char)*in > 0xBF);
			*(out++) = 0x80 + (*in & 0x3F);
		} else {
			*(out++) = *in;
		}
	}

	*out = '\0';
}

void xlib_window_clipboard_write(sgui_window *this, const char *text,
				unsigned int length)
{
	sgui_lib_x11 *lib = (sgui_lib_x11 *)this->lib;

	sgui_internal_lock_mutex();

	lib->clipboard_strlen = length;

	if (lib->clipboard_size <= lib->clipboard_strlen) {
		lib->clipboard_size = lib->clipboard_strlen + 1;
		lib->clipboard_buffer = realloc(lib->clipboard_buffer,
						lib->clipboard_size);
	}

	strncpy(lib->clipboard_buffer, text, lib->clipboard_strlen);
	lib->clipboard_buffer[lib->clipboard_strlen] = '\0';

	XSetSelectionOwner(lib->dpy, lib->atom_clipboard,
				TO_X11(this)->wnd, CurrentTime);
	XFlush(lib->dpy);

	sgui_internal_unlock_mutex();
}

const char* xlib_window_clipboard_read(sgui_window *this)
{
	sgui_lib_x11 *lib = (sgui_lib_x11 *)this->lib;
	unsigned long pty_size, pty_items;
	int pty_format, convert = 0;
	Atom target = lib->atom_UTF8;
	unsigned char *buffer;
	Atom pty_type;
	Window owner;
	XEvent evt;

	sgui_internal_lock_mutex();

	owner = XGetSelectionOwner(lib->dpy, lib->atom_clipboard);

	if (owner == TO_X11(this)->wnd)
		goto done;

	if (owner==None)
		goto fail;

	XConvertSelection(lib->dpy, lib->atom_clipboard, target, lib->atom_pty,
				TO_X11(this)->wnd, CurrentTime);
	XFlush(lib->dpy);

	wait_for_event(&x11, &evt, SelectionNotify);

	if (evt.xselection.property == None) {
		target = XA_STRING;
		XConvertSelection(lib->dpy, lib->atom_clipboard, target,
					lib->atom_pty, TO_X11(this)->wnd,
					CurrentTime);
		XFlush(lib->dpy);
		wait_for_event(&x11, &evt, SelectionNotify);
		convert = 1;
	}

	XGetWindowProperty(lib->dpy, TO_X11(this)->wnd, lib->atom_pty, 0, 0,
				False, AnyPropertyType, &pty_type, &pty_format,
				&pty_items, &pty_size, &buffer);
	XFree(buffer);
	lib->clipboard_strlen = 0;

	if (pty_type == lib->atom_inc) {
		XDeleteProperty(lib->dpy, TO_X11(this)->wnd, lib->atom_pty);
		XFlush(lib->dpy);

		while (1) {
			wait_for_event(&x11, &evt, PropertyNotify);

			if (evt.xproperty.state != PropertyNewValue)
				continue;

			XGetWindowProperty(lib->dpy, TO_X11(this)->wnd,
						lib->atom_pty, 0, 0, False,
						AnyPropertyType, &pty_type,
						&pty_format, &pty_items,
						&pty_size, &buffer);

			XFree(buffer);

			if (pty_format != 8) {
				XDeleteProperty(lib->dpy, TO_X11(this)->wnd,
						lib->atom_pty);
				continue;
			}

			if (pty_size == 0) {
				XDeleteProperty(lib->dpy, TO_X11(this)->wnd,
						lib->atom_pty);
				break;
			}

			XGetWindowProperty(lib->dpy, TO_X11(this)->wnd,
						lib->atom_pty, 0, pty_size,
						False, AnyPropertyType,
						&pty_type, &pty_format,
						&pty_items, &pty_size,
						&buffer);

			if (((lib->clipboard_strlen + pty_items+1) >
				lib->clipboard_size) &&
				!resize_clipboard_buffer(lib, pty_items + 1)) {
				goto fail;
			}

			memcpy(lib->clipboard_buffer + lib->clipboard_strlen,
				buffer, pty_items);
			lib->clipboard_strlen += pty_items;
			lib->clipboard_buffer[lib->clipboard_strlen] = '\0';
			XFree(buffer);

			XDeleteProperty(lib->dpy, TO_X11(this)->wnd,
					lib->atom_pty);
			XFlush(lib->dpy);
		}
	} else {
		if (pty_format != 8)
			goto fail;

		XGetWindowProperty(lib->dpy, TO_X11(this)->wnd, lib->atom_pty,
					0, pty_size, False, AnyPropertyType,
					&pty_type, &pty_format, &pty_items,
					&pty_size, &buffer);

		XDeleteProperty(lib->dpy, TO_X11(this)->wnd, lib->atom_pty);

		if (((pty_items + 1) > lib->clipboard_size) &&
			!resize_clipboard_buffer(lib, pty_items + 1)) {
			goto fail;
		}

		lib->clipboard_strlen = pty_items;
		memcpy(lib->clipboard_buffer, buffer, pty_items);
		lib->clipboard_buffer[lib->clipboard_strlen] = '\0';
		XFree(buffer);
	}

	if (convert) {
		pty_size = sgui_utf8_from_latin1_length(lib->clipboard_buffer);

		buffer = malloc(pty_size + 1);
		if (!buffer)
			goto fail;

		sgui_utf8_from_latin1((char *)buffer, lib->clipboard_buffer);
		free(lib->clipboard_buffer);
		lib->clipboard_buffer = (char *)buffer;
		lib->clipboard_size = pty_size + 1;
		lib->clipboard_strlen = pty_size;
	}
done:
	sgui_internal_unlock_mutex();
	return lib->clipboard_buffer;
fail:
	sgui_internal_unlock_mutex();
	return NULL;
}

/****************************************************************************/

static unsigned long get_time_ms(void)
{
	struct timespec ts;

#ifdef CLOCK_MONOTONIC_RAW
	clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
#else
	clock_gettime(CLOCK_MONOTONIC, &ts);
#endif

	return ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

int check_double_click(sgui_window_xlib *window)
{
	sgui_lib_x11 *lib = (sgui_lib_x11 *)(((sgui_window *)window)->lib);
	unsigned long delta, current;

	if (lib->clicked == window) {
		current = get_time_ms();
		delta = current >= lib->click_time ?
				(current - lib->click_time) :
				DOUBLE_CLICK_MS * 64;

		if (delta <= DOUBLE_CLICK_MS)
			return 1;
	}

	lib->clicked = window;
	lib->click_time = get_time_ms();
	return 0;
}

void interrupt_double_click(sgui_lib *slib)
{
	sgui_lib_x11 *lib = (sgui_lib_x11 *)slib;

	lib->clicked = NULL;
	lib->click_time = 0;
}

/****************************************************************************/

void sgui_internal_lock_mutex(void)
{
	pthread_mutex_lock(&x11.mutex);
}

void sgui_internal_unlock_mutex(void)
{
	pthread_mutex_unlock(&x11.mutex);
}

static void destroy_x11(sgui_lib *lib)
{
	sgui_lib_x11 *libx11 = (sgui_lib_x11 *)lib;

	if (lib->ev)
		lib->ev->destroy(lib->ev);

	canvas_cleanup_skin_pixmap();
	sgui_internal_memcanvas_cleanup();
	sgui_interal_skin_deinit_default();
	pthread_mutex_destroy(&libx11->mutex);

	if (libx11->im)
		XCloseIM(libx11->im);

	if (libx11->dpy)
		XCloseDisplay(libx11->dpy);

	free(libx11->clipboard_buffer);
	memset(libx11, 0, sizeof(*libx11));
}

static int main_loop_step_x11(sgui_lib *lib)
{
	sgui_lib_x11 *libx11 = (sgui_lib_x11 *)lib;

	sgui_internal_lock_mutex();
	handle_events(libx11);
	XFlush(libx11->dpy);
	sgui_internal_unlock_mutex();

	sgui_event_process(lib->ev);

	return sgui_lib_have_active_windows(lib) || sgui_event_queued(lib->ev);
}

static void main_loop_x11(sgui_lib *lib)
{
	sgui_lib_x11 *libx11 = (sgui_lib_x11 *)lib;
	int x11_fd = XConnectionNumber(libx11->dpy);
	struct timeval tv;
	fd_set in_fds;

	while (sgui_lib_have_active_windows(lib)) {
		sgui_internal_lock_mutex();
		handle_events(libx11);
		XFlush(libx11->dpy);
		sgui_internal_unlock_mutex();

		sgui_event_process(lib->ev);

		/* wait for X11 events, one second time out */
		FD_ZERO(&in_fds);
		FD_SET(x11_fd, &in_fds);

		tv.tv_usec = 0;
		tv.tv_sec = 1;
		select(x11_fd + 1, &in_fds, 0, 0, &tv);
	}

	while (sgui_event_queued(lib->ev))
		sgui_event_process(lib->ev);
}

sgui_lib *sgui_init(void *arg)
{
	pthread_mutexattr_t attr;

	if (arg)
		return NULL;

	memset(&x11, 0, sizeof(x11));

	pthread_mutexattr_init(&attr);
	pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
	pthread_mutex_init(&x11.mutex, &attr);

	((sgui_lib *)&x11)->ev = sgui_event_queue_create();
	if (!((sgui_lib *)&x11)->ev)
		goto fail;

	XInitThreads();

	if (!(x11.dpy = XOpenDisplay(0)))
		goto fail;

	XSetErrorHandler(xlib_swallow_errors);

	if (!(x11.im = XOpenIM(x11.dpy, NULL, NULL, NULL)))
		goto fail;

	init_keycodes();
	sgui_interal_skin_init_default();

	x11.atom_wm_delete = XInternAtom(x11.dpy, "WM_DELETE_WINDOW", True);
	x11.atom_pty = XInternAtom(x11.dpy, "SGUI_CLIP", False);
	x11.atom_targets = XInternAtom(x11.dpy, "TARGETS", False);
	x11.atom_text = XInternAtom(x11.dpy, "TEXT", False);
	x11.atom_inc = XInternAtom(x11.dpy, "INCR", False);
	x11.atom_UTF8 = XInternAtom(x11.dpy, "UTF8_STRING", False);
	x11.atom_clipboard = XInternAtom(x11.dpy, "CLIPBOARD", False);
	x11.root = DefaultRootWindow(x11.dpy);
	x11.screen = DefaultScreen(x11.dpy);

	((sgui_lib *)&x11)->destroy = destroy_x11;
	((sgui_lib *)&x11)->main_loop = main_loop_x11;
	((sgui_lib *)&x11)->main_loop_step = main_loop_step_x11;
	((sgui_lib *)&x11)->create_window = x11_window_create;
	return (sgui_lib *)&x11;
fail:
	destroy_x11((sgui_lib *)&x11);
	return NULL;
}
