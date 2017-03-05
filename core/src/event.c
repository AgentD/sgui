/*
 * event.c
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
#include "sgui_event.h"
#include "sgui_internal.h"

#include <assert.h>
#include <stddef.h>
#include <stdarg.h>
#include <stdlib.h>


struct listener {
	void *sender;
	void *receiver;
	sgui_function callback;
	int type;			/* type of function argument */

	union {
		char c; 
		short s;
		int i3[3];
		long l;
		void* p;

#ifndef SGUI_NO_FLOAT
		float f;
		double d;
#endif
	} value;

	struct listener* next;
};

struct bucket {
	int event;			/* event to listen for */
	struct listener *listeners;	/* linked list of listeneres */
	struct bucket *next;
};


static sgui_event *queue = NULL;
static int queue_top = 0;
static int queue_size = 0;
static struct bucket *buckets = NULL;


static int add_listener(struct listener *l, int eventtype)
{
	struct bucket* b;
	int ret = 0;

	sgui_internal_lock_mutex();

	for (b = buckets; b != NULL; b = b->next) {
		if (b->event == eventtype)
			break;
	}

	if (!b) {
		b = calloc(1, sizeof(*b));
		if (!b)
			goto out;
		b->event = eventtype;
		b->next = buckets;
		buckets = b;
	}

	l->next = b->listeners;
	b->listeners = l;
	ret = 1;
out:
	sgui_internal_unlock_mutex();
	return ret;
}

int sgui_event_connect(void *sender, int eventtype, ...)
{
	sgui_function callback;
	struct listener *l;
	int ret = 0;
	va_list va;

	va_start(va, eventtype);
	callback = va_arg(va, sgui_function);

	if (!callback) {
		ret = 1;
		goto out_va;
	}

	l = calloc(1, sizeof(*l));
	if (!l)
		goto out_va;

	l->sender = sender;
	l->callback = callback;
	l->receiver = va_arg(va, void *);
	l->type = va_arg(va, int);

	switch (l->type) {
	case SGUI_CHAR:
		l->value.c = va_arg(va, int);
		break;
	case SGUI_SHORT:
		l->value.s = va_arg(va, int);
		break;
	case SGUI_LONG:
		l->value.l = va_arg(va, long);
		break;
	case SGUI_POINTER:
		l->value.p = va_arg(va, void *);
		break;
#ifndef SGUI_NO_FLOAT
	case SGUI_FLOAT:
		l->value.f = va_arg(va, double);
		break;
	case SGUI_DOUBLE:
		l->value.d = va_arg(va, double);
		break;
#endif
	case SGUI_FROM_EVENT:
	case SGUI_INT:
		l->value.i3[0] = va_arg(va, int);
		break;
	case SGUI_INT2:
		l->value.i3[0] = va_arg(va, int);
		l->value.i3[1] = va_arg(va, int);
		break;
	case SGUI_INT3:
		l->value.i3[0] = va_arg(va, int);
		l->value.i3[1] = va_arg(va, int);
		l->value.i3[2] = va_arg(va, int);
		break;
	}

	ret = add_listener(l, eventtype);
out_va:
	va_end(va);
	return ret;
}

void sgui_event_disconnect(void *sender, int eventtype,
			sgui_function callback, void *receiver)
{
	struct listener *old = NULL;
	struct listener *l;
	struct bucket *b;

	sgui_internal_lock_mutex();

	for (b = buckets; b != NULL; b = b->next) {
		if (b->event == eventtype)
			break;
	}

	if (!b)
		goto out;

	l = b->listeners;

	while (l) {
		if (l->callback == callback && l->sender == sender &&
			l->receiver == receiver) {
			if (old) {
				old->next = l->next;
				free(l);
				l = old->next;
			} else {
				b->listeners = b->listeners->next;
				free(l);
				l = b->listeners;
			}
		} else {
			old = l;
			l = l->next;
		}
	}
out:
	sgui_internal_unlock_mutex();
}

int sgui_event_post(const sgui_event *event)
{
	sgui_event *new_queue;
	int new_size, ret = 1;
	struct listener *l;
	struct bucket *b;

	sgui_internal_lock_mutex();

	/* if there is no one handling the event, drop it */
	for (b = buckets; b != NULL; b = b->next) {
		if (b->event == event->type)
			break;
	}

	if (!b)
		goto out;

	for (l = b->listeners; l != NULL; l = l->next) {
		if (!l->sender || l->sender == event->src.other)
			break;
	}

	if (!l)
		goto out;

	/* enlarge queue if necessary */
	if (queue_top == queue_size) {
		new_size = queue_size < 10 ? 10 : queue_size * 2;
		new_queue = realloc(queue, sizeof(queue[0]) * new_size);

		if (!new_queue) {
			ret = 0;
			goto out;
		}

		queue_size = new_size;
		queue = new_queue;
	}

	queue[queue_top++] = (*event);
out:
	sgui_internal_unlock_mutex();
	return ret;
}

/****************************************************************************/

static void call_from_event(const struct listener *l, const sgui_event *e)
{
	const void *ptr;
	int x, y, z;

	switch (l->value.i3[0]) {
	case SGUI_UI2_X:
		l->callback(l->receiver,e->arg.ui2.x);
		break;
	case SGUI_UI2_Y:
		l->callback(l->receiver,e->arg.ui2.y);
		break;
	case SGUI_UI2_XY:
		l->callback(l->receiver,e->arg.ui2.x,e->arg.ui2.y);
		break;
	case SGUI_UI2_YX:
		l->callback(l->receiver,e->arg.ui2.y,e->arg.ui2.x);
		break;
	case SGUI_EVENT:  ptr = e;                          goto out_ptr;
	case SGUI_WIDGET: ptr = e->src.widget;              goto out_ptr;
	case SGUI_WINDOW: ptr = e->src.window;              goto out_ptr;
	case SGUI_UTF8:   ptr = e->arg.utf8;                goto out_ptr;
	case SGUI_RECT:   ptr = &e->arg.rect;               goto out_ptr;
	case SGUI_COLOR:  ptr = e->arg.color;               goto out_ptr;
	case SGUI_TYPE:   x = e->type;                      goto out_x;
	case SGUI_I:      x = e->arg.i;                     goto out_x;
	case SGUI_I2_X:   x = e->arg.i2.x;                  goto out_x;
	case SGUI_I2_Y:   x = e->arg.i2.y;                  goto out_x;
	case SGUI_I3_X:   x = e->arg.i3.x;                  goto out_x;
	case SGUI_I3_Y:   x = e->arg.i3.y;                  goto out_x;
	case SGUI_I3_Z:   x = e->arg.i3.z;                  goto out_x;
	case SGUI_I2_XY:  x = e->arg.i2.x; y = e->arg.i2.y; goto out_xy;
	case SGUI_I2_YX:  x = e->arg.i2.y; y = e->arg.i2.x; goto out_xy;
	case SGUI_I3_XY:  x = e->arg.i3.x; y = e->arg.i3.y; goto out_xy;
	case SGUI_I3_XZ:  x = e->arg.i3.x; y = e->arg.i3.z; goto out_xy;
	case SGUI_I3_YX:  x = e->arg.i3.y; y = e->arg.i3.x; goto out_xy;
	case SGUI_I3_YZ:  x = e->arg.i3.y; y = e->arg.i3.z; goto out_xy;
	case SGUI_I3_ZX:  x = e->arg.i3.z; y = e->arg.i3.x; goto out_xy;
	case SGUI_I3_ZY:  x = e->arg.i3.z; y = e->arg.i3.y; goto out_xy;
	case SGUI_I3_XYZ:
		x = e->arg.i3.x; y = e->arg.i3.y; z = e->arg.i3.z;
		goto out_xyz;
	case SGUI_I3_XZY:
		x = e->arg.i3.x; y = e->arg.i3.z; z = e->arg.i3.y;
		goto out_xyz;
	case SGUI_I3_YXZ:
		x = e->arg.i3.y; y = e->arg.i3.x; z = e->arg.i3.z;
		goto out_xyz;
	case SGUI_I3_YZX:
		x = e->arg.i3.y; y = e->arg.i3.z; z = e->arg.i3.x;
		goto out_xyz;
	case SGUI_I3_ZXY:
		x = e->arg.i3.z; y = e->arg.i3.x; z = e->arg.i3.y;
		goto out_xyz;
	case SGUI_I3_ZYX:
		x = e->arg.i3.z; y = e->arg.i3.y; z = e->arg.i3.x;
		goto out_xyz;
	}
	return;
out_ptr:
	l->callback(l->receiver, ptr);
	return;
out_x:
	l->callback(l->receiver, x);
	return;
out_xy:
	l->callback(l->receiver, x, y);
	return;
out_xyz:
	l->callback(l->receiver, x, y, z);
	return;
}

static void call(const struct listener *l, const sgui_event *e)
{
	switch (l->type) {
	case SGUI_FROM_EVENT:
		call_from_event(l, e);
		break;
	case SGUI_VOID:
		l->callback(l->receiver);
		break;
	case SGUI_CHAR:
		l->callback(l->receiver, l->value.c);
		break;
	case SGUI_SHORT:
		l->callback(l->receiver, l->value.s);
		break;
	case SGUI_INT:
		l->callback(l->receiver, l->value.i3[0]);
		break;
	case SGUI_LONG:
		l->callback(l->receiver, l->value.l);
		break;
	case SGUI_POINTER:
		l->callback(l->receiver, l->value.p);
		break;
#ifndef SGUI_NO_FLOAT
	case SGUI_FLOAT:
		l->callback(l->receiver, l->value.f);
		break;
	case SGUI_DOUBLE:
		l->callback(l->receiver, l->value.d);
		break;
#endif
	case SGUI_INT2:
		l->callback(l->receiver, l->value.i3[0], l->value.i3[1]);
		break;
	case SGUI_INT3:
		l->callback(l->receiver, l->value.i3[0], l->value.i3[1],
				l->value.i3[2]);
		break;
	}
}

void sgui_event_process(void)
{
	struct listener *l;
	sgui_event *local;
	struct bucket *b;
	sgui_event *e;
	int i, count;

	sgui_internal_lock_mutex();

	/*
		Handling events can trigger adding of new events, which
		can trigger a realloc() of the event queue.
	*/
	local = queue;
	count = queue_top;

	queue_top = 0;
	queue_size = 0;
	queue = NULL;

	for (e = local, i = 0; i < count; ++i, ++e) {
		for (b = buckets; b != NULL; b = b->next) {
			if (b->event == e->type)
				break;
		}
		if (!b)
			continue;

		for (l = b->listeners; l != NULL; l = l->next) {
			if (!l->sender || l->sender == e->src.other)
				call(l, e);
		}
	}

	sgui_internal_unlock_mutex();

	free(local);
}

void sgui_event_reset(void)
{
	struct listener *l;
	struct bucket *b;

	sgui_internal_lock_mutex();

	while (buckets != NULL) {
		b = buckets;
		buckets = buckets->next;

		while (b->listeners != NULL) {
			l = b->listeners;
			b->listeners = b->listeners->next;
			free(l);
		}

		free(b);
	}

	free(queue);

	queue = NULL;
	queue_size = 0;
	queue_top = 0;
	sgui_internal_unlock_mutex();
}

unsigned int sgui_event_queued(void)
{
	unsigned int count;

	sgui_internal_lock_mutex();
	count = queue_top;
	sgui_internal_unlock_mutex();

	return count;
}
