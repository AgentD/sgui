#ifndef INTERNAL_H
#define INTERNAL_H



#include "sgui_window.h"
#include "sgui_colors.h"
#include "sgui_widget_manager.h"

#define WIN32_LEAN_AND_MEAN
#define VC_EXTRA_LEAN
#define NOMINMAX

#include <windows.h>

#include <stdlib.h>



struct sgui_window
{
    HWND hWnd;
    HINSTANCE hInstance;
    HDC dc;
    BITMAPINFO info;
    HBITMAP bitmap;
    HBITMAP old_bitmap;
    unsigned char* back_buffer;

    unsigned int w, h;

    int visible;

    sgui_widget_manager* mgr;
    sgui_window_callback event_fun;
};


#define SEND_EVENT( wnd, event, e )\
            if( wnd->event_fun )\
                wnd->event_fun( wnd, event, e );\
            sgui_widget_manager_send_event( wnd->mgr, wnd, event, e );



#endif /* INTERNAL_H */

