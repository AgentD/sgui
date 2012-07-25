#ifndef INTERNAL_H
#define INTERNAL_H



#include "sgui_window.h"
#include "sgui_pixmap.h"
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
    HBITMAP bitmap;
    HBITMAP old_bitmap;

    unsigned int w, h;

    int visible;

    sgui_widget_manager* mgr;
    sgui_window_callback event_fun;
};

struct sgui_pixmap
{
    HBITMAP bitmap;
    unsigned int width, height;
};



#define SGUI_COLORREF( c ) RGB( ((c>>16) & 0xFF), (((c)>>8) & 0xFF),\
                                ((c) & 0xFF) )

#define SEND_EVENT( wnd, event, e )\
            if( wnd->event_fun )\
                wnd->event_fun( wnd, event, e );\
            sgui_widget_manager_send_event( wnd->mgr, wnd, event, e );



#endif /* INTERNAL_H */

