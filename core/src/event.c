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



typedef struct listener
{
    int event;                  /* event to listen for */
    void* sender;               /* sender to listen for */

    void* receiver;             /* receiver */
    sgui_function callback;

    int type;                   /* type of function argument */

    union
    {
        char c; 
        short s;
        int i3[3];
        long l;
        void* p;

#ifndef SGUI_NO_FLOAT
        float f;
        double d;
#endif
    }
    value;

    struct listener* next;      /* linked list pointer */
}
listener;



static sgui_event* queue = NULL;
static int queue_top = 0;
static int queue_size = 0;
static listener* listeners = NULL;



void sgui_event_connect( void* sender, int eventtype, ... )
{
    sgui_function callback;
    listener* l;
    va_list va;

    va_start( va, eventtype );
    callback = va_arg( va, sgui_function );

    if( !callback || !(l = malloc( sizeof(listener) )) )
        goto done;

    l->event    = eventtype;
    l->sender   = sender;
    l->callback = callback;
    l->receiver = va_arg( va, void* );
    l->type     = va_arg( va, int );

    switch( l->type )
    {
    case SGUI_CHAR:          l->value.c = va_arg( va, int    ); break;
    case SGUI_SHORT:         l->value.s = va_arg( va, int    ); break;
    case SGUI_LONG:          l->value.l = va_arg( va, long   ); break;
    case SGUI_POINTER:       l->value.p = va_arg( va, void*  ); break;
#ifndef SGUI_NO_FLOAT
    case SGUI_FLOAT:         l->value.f = va_arg( va, double ); break;
    case SGUI_DOUBLE:        l->value.d = va_arg( va, double ); break;
#endif
    case SGUI_FROM_EVENT:
    case SGUI_INT:           l->value.i3[0] = va_arg( va, int ); break;
    case SGUI_INT2:          l->value.i3[0] = va_arg( va, int );
                             l->value.i3[1] = va_arg( va, int ); break;
    case SGUI_INT3:          l->value.i3[0] = va_arg( va, int );
                             l->value.i3[1] = va_arg( va, int );
                             l->value.i3[2] = va_arg( va, int ); break;
    }

    sgui_internal_lock_mutex( );
    SGUI_ADD_TO_LIST( listeners, l );
    sgui_internal_unlock_mutex( );
done:
    va_end( va );
}

void sgui_event_disconnect( void* sender, int eventtype,
                            sgui_function callback, void* receiver )
{
    listener* old = NULL;
    listener* l = listeners;

    sgui_internal_lock_mutex( );

    while( l )
    {
        if( l->event==eventtype && l->callback==callback &&
            l->sender==sender && l->receiver==receiver )
        {
            if( old )
            {
                old->next = l->next;
                free( l );
                l = old->next;
            }
            else
            {
                listeners = listeners->next;
                free( l );
                l = listeners;
            }
        }
        else
        {
            old = l;
            l = l->next;
        }
    }

    sgui_internal_unlock_mutex( );
}

void sgui_event_post( const sgui_event* event )
{
    sgui_event* new_queue;
    int new_size;

    if( event )
    {
        sgui_internal_lock_mutex( );

        if( queue_top == queue_size )
        {
            new_size = queue_size<10 ? 10 : queue_size*2;
            new_queue = realloc( queue, sizeof(sgui_event)*new_size );

            if( new_queue )
            {
                queue_size = new_size;
                queue = new_queue;
            }
        }

        if( queue_top < queue_size )
            queue[ queue_top++ ] = (*event);

        sgui_internal_unlock_mutex( );
    }
}

/****************************************************************************/

void sgui_event_process( void )
{
    sgui_event* local;
    sgui_event* e;
    int i, count;
    listener* l;

    sgui_internal_lock_mutex( );

    /*
        snatch the queue, so modification while processing
        doesn't confuse us
     */
    local = queue;
    count = queue_top;

    queue_top = 0;
    queue_size = 0;
    queue = NULL;

    for( e=local, i=0; i<count; ++i, ++e )  /* for each event in the queue */
    {
        for( l=listeners; l; l=l->next )        /* for each listener */
        {
            if( (l->event!=e->type)||(l->sender && l->sender!=e->src.other) )
                continue;

            switch( l->type )
            {
            case SGUI_FROM_EVENT:
                switch( l->value.i3[0] )
                {
                case SGUI_EVENT: l->callback(l->receiver,e            );break;
                case SGUI_WIDGET:l->callback(l->receiver,e->src.widget);break;
                case SGUI_WINDOW:l->callback(l->receiver,e->src.window);break;
                case SGUI_TYPE:  l->callback(l->receiver,e->type      );break;
                case SGUI_I:     l->callback(l->receiver,e->arg.i     );break;
                case SGUI_I2_X:  l->callback(l->receiver,e->arg.i2.x  );break;
                case SGUI_I2_Y:  l->callback(l->receiver,e->arg.i2.y  );break;
                case SGUI_I3_X:  l->callback(l->receiver,e->arg.i3.x  );break;
                case SGUI_I3_Y:  l->callback(l->receiver,e->arg.i3.y  );break;
                case SGUI_I3_Z:  l->callback(l->receiver,e->arg.i3.z  );break;
                case SGUI_UI2_X: l->callback(l->receiver,e->arg.ui2.x );break;
                case SGUI_UI2_Y: l->callback(l->receiver,e->arg.ui2.y );break;
                case SGUI_UTF8:  l->callback(l->receiver,e->arg.utf8  );break;
                case SGUI_RECT:  l->callback(l->receiver,&e->arg.rect );break;
                case SGUI_COLOR: l->callback(l->receiver,e->arg.color );break;
                case SGUI_UI2_XY:
                    l->callback(l->receiver,e->arg.ui2.x,e->arg.ui2.y); break;
                case SGUI_UI2_YX:
                    l->callback(l->receiver,e->arg.ui2.y,e->arg.ui2.x); break;
                case SGUI_I2_XY:
                    l->callback(l->receiver,e->arg.i2.x,e->arg.i2.y); break;
                case SGUI_I2_YX:
                    l->callback(l->receiver,e->arg.i2.y,e->arg.ui2.x); break;
                case SGUI_I3_XY:
                    l->callback(l->receiver,e->arg.i3.x,e->arg.i3.y); break;
                case SGUI_I3_XZ:
                    l->callback(l->receiver,e->arg.i3.x,e->arg.i3.z); break;
                case SGUI_I3_YX:
                    l->callback(l->receiver,e->arg.i3.y,e->arg.i3.x); break;
                case SGUI_I3_YZ:
                    l->callback(l->receiver,e->arg.i3.y,e->arg.i3.z); break;
                case SGUI_I3_ZX:
                    l->callback(l->receiver,e->arg.i3.z,e->arg.i3.x); break;
                case SGUI_I3_ZY:
                    l->callback(l->receiver,e->arg.i3.z,e->arg.i3.y); break;
                case SGUI_I3_XYZ:
                    l->callback( l->receiver,
                                 e->arg.i3.x, e->arg.i3.y, e->arg.i3.z );
                    break;
                case SGUI_I3_XZY:
                    l->callback( l->receiver,
                                 e->arg.i3.x, e->arg.i3.z, e->arg.i3.y );
                    break;
                case SGUI_I3_YXZ:
                    l->callback( l->receiver,
                                 e->arg.i3.y, e->arg.i3.x, e->arg.i3.z );
                    break;
                case SGUI_I3_YZX:
                    l->callback( l->receiver,
                                 e->arg.i3.y, e->arg.i3.z, e->arg.i3.x );
                    break;
                case SGUI_I3_ZXY:
                    l->callback( l->receiver,
                                 e->arg.i3.z, e->arg.i3.x, e->arg.i3.y );
                    break;
                case SGUI_I3_ZYX:
                    l->callback( l->receiver,
                                 e->arg.i3.z, e->arg.i3.y, e->arg.i3.x );
                    break;
                }
                break;
            case SGUI_VOID:   l->callback(l->receiver);             break;
            case SGUI_CHAR:   l->callback(l->receiver, l->value.c); break;
            case SGUI_SHORT:  l->callback(l->receiver, l->value.s); break;
            case SGUI_INT:    l->callback(l->receiver, l->value.i3[0]); break;
            case SGUI_LONG:   l->callback(l->receiver, l->value.l); break;
            case SGUI_POINTER:l->callback(l->receiver, l->value.p); break;
#ifndef SGUI_NO_FLOAT
            case SGUI_FLOAT:  l->callback(l->receiver, l->value.f); break;
            case SGUI_DOUBLE: l->callback(l->receiver, l->value.d); break;
#endif
            case SGUI_INT2:
                l->callback(l->receiver, l->value.i3[0], l->value.i3[1]);
                break;
            case SGUI_INT3:
                l->callback( l->receiver, l->value.i3[0], l->value.i3[1],
                             l->value.i3[2] );
                break;
            }
        }
    }

    sgui_internal_unlock_mutex( );

    free( local );
}

void sgui_event_reset( void )
{
    listener* l;

    sgui_internal_lock_mutex( );

    while( listeners )
    {
        l = listeners;
        listeners = listeners->next;
        free( l );
    }

    free( queue );

    queue = NULL;
    queue_size = 0;
    queue_top = 0;
    sgui_internal_unlock_mutex( );
}

unsigned int sgui_event_queued( void )
{
    unsigned int count;

    sgui_internal_lock_mutex( );
    count = queue_top;
    sgui_internal_unlock_mutex( );

    return count;
}

