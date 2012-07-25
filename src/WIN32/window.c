#include "internal.h"



LRESULT CALLBACK WindowProcFun( HWND hWnd, UINT msg, WPARAM wp, LPARAM lp )
{
    sgui_window* wnd;
    sgui_event e;
    PAINTSTRUCT ps;
    HDC hDC;

    wnd = (sgui_window*)GetWindowLong( hWnd, GWL_USERDATA );

    if( !wnd )
        return DefWindowProc( hWnd, msg, wp, lp );

    switch( msg )
    {
    case WM_DESTROY:
        wnd->visible = 0;
        SEND_EVENT( wnd, SGUI_USER_CLOSED_EVENT, NULL );
        break;
    case WM_MOUSEMOVE:
        e.mouse_move.x = LOWORD( lp );
        e.mouse_move.y = HIWORD( lp );
        SEND_EVENT( wnd, SGUI_MOUSE_MOVE_EVENT, &e );
        break;
    case WM_MOUSEWHEEL:
        e.mouse_wheel.direction = GET_WHEEL_DELTA_WPARAM( wp )/120;
        SEND_EVENT( wnd, SGUI_MOUSE_WHEEL_EVENT, &e );
        break;
    case WM_LBUTTONDOWN:
        e.mouse_press.pressed = 1;
        e.mouse_press.button = SGUI_MOUSE_BUTTON_LEFT;
        SEND_EVENT( wnd, SGUI_MOUSE_PRESS_EVENT, &e );
        break;
    case WM_LBUTTONUP:
        e.mouse_press.pressed = 0;
        e.mouse_press.button = SGUI_MOUSE_BUTTON_LEFT;
        SEND_EVENT( wnd, SGUI_MOUSE_PRESS_EVENT, &e );
        break;
    case WM_MBUTTONDOWN:
        e.mouse_press.pressed = 1;
        e.mouse_press.button = SGUI_MOUSE_BUTTON_MIDDLE;
        SEND_EVENT( wnd, SGUI_MOUSE_PRESS_EVENT, &e );
        break;
    case WM_MBUTTONUP:
        e.mouse_press.pressed = 0;
        e.mouse_press.button = SGUI_MOUSE_BUTTON_MIDDLE;
        SEND_EVENT( wnd, SGUI_MOUSE_PRESS_EVENT, &e );
        break;
    case WM_RBUTTONDOWN:
        e.mouse_press.pressed = 1;
        e.mouse_press.button = SGUI_MOUSE_BUTTON_RIGHT;
        SEND_EVENT( wnd, SGUI_MOUSE_PRESS_EVENT, &e );
        break;
    case WM_RBUTTONUP:
        e.mouse_press.pressed = 0;
        e.mouse_press.button = SGUI_MOUSE_BUTTON_RIGHT;
        SEND_EVENT( wnd, SGUI_MOUSE_PRESS_EVENT, &e );
        break;
    case WM_SIZE:
        wnd->w = LOWORD( lp );
        wnd->h = HIWORD( lp );

        /* resize the double buffering context/bitmap */
        SelectObject( wnd->dc, wnd->old_bitmap );
        DeleteObject( wnd->bitmap );
        DeleteDC( wnd->dc );

        wnd->dc = CreateCompatibleDC( NULL );
        wnd->bitmap = CreateBitmap( wnd->w, wnd->h, 1, 32, NULL );
        wnd->old_bitmap = (HBITMAP)SelectObject( wnd->dc, wnd->bitmap );

        sgui_window_draw_box( wnd, 0, 0, wnd->w, wnd->h,
                              SGUI_WINDOW_COLOR, 0 );

        /* send size change event */
        e.size.new_width  = wnd->w;
        e.size.new_height = wnd->h;

        SEND_EVENT( wnd, SGUI_SIZE_CHANGE_EVENT, &e );

        /* redraw everything */
        e.draw.x = 0;
        e.draw.y = 0;
        e.draw.w = wnd->w;
        e.draw.h = wnd->h;

        SEND_EVENT( wnd, SGUI_DRAW_EVENT, &e );
        break;
    case WM_PAINT:
        hDC = BeginPaint( hWnd, &ps );

        BitBlt( hDC, 0, 0, wnd->w, wnd->h, wnd->dc, 0, 0, SRCCOPY );

        EndPaint( hWnd, &ps );
        break;
    default:
        return DefWindowProc( hWnd, msg, wp, lp );
    }

    return 0;
}





sgui_window* sgui_window_create( unsigned int width, unsigned int height,
                                 int resizeable )
{
    sgui_window* wnd;
    WNDCLASSEX wc;
    DWORD style;
    RECT r;

    if( !width || !height )
        return NULL;

    /* allocate space for the window structure */
    wnd = malloc( sizeof(sgui_window) );

    if( !wnd )
        return NULL;

    memset( wnd, 0, sizeof(sgui_window) );

    /* create a widget manager */
    wnd->mgr = sgui_widget_manager_create( );

    if( !wnd->mgr )
    {
        free( wnd );
        return NULL;
    }

    /* get HINSTANCE, window style and actual window size */
    wnd->hInstance = GetModuleHandle( NULL );

    style = resizeable ? WS_OVERLAPPEDWINDOW : (WS_CAPTION | WS_SYSMENU);

    SetRect( &r, 0, 0, width, height );
    AdjustWindowRect( &r, style, FALSE );

    /* setup and register a window class if not done yet */
    if( !GetClassInfoEx( wnd->hInstance, "sgui_wnd_class", &wc ) )
    {
        memset( &wc, 0, sizeof(WNDCLASSEX) );

        wc.cbSize        = sizeof(WNDCLASSEX);
        wc.style         = CS_HREDRAW | CS_VREDRAW;
        wc.lpfnWndProc   = WindowProcFun;
        wc.hInstance     = wnd->hInstance;
        wc.lpszClassName = "sgui_wnd_class";
        wc.hCursor       = LoadCursor( NULL, IDC_ARROW );

        if( RegisterClassEx( &wc ) == 0 )
        {
            sgui_window_destroy( wnd );
            return NULL;
        }
    }

    /* create a window */
    wnd->hWnd = CreateWindowEx( 0, "sgui_wnd_class", "", style, 0, 0,
                                r.right-r.left, r.bottom-r.top,
                                0, 0, wnd->hInstance, NULL );

    if( !wnd->hWnd )
    {
        sgui_window_destroy( wnd );
        return NULL;
    }

    SetWindowLong( wnd->hWnd, GWL_USERDATA, (LONG)wnd );

    wnd->w = width;
    wnd->h = height;

    /* create double buffering context/bitmap */
    wnd->dc = CreateCompatibleDC(NULL);
    wnd->bitmap = CreateBitmap( wnd->w, wnd->h, 1, 32, NULL );
    wnd->old_bitmap = (HBITMAP)SelectObject( wnd->dc, wnd->bitmap );

    sgui_window_draw_box( wnd, 0, 0, wnd->w, wnd->h, SGUI_WINDOW_COLOR, 0 );

    return wnd;
}

void sgui_window_destroy( sgui_window* wnd )
{
    MSG msg;

    if( wnd )
    {
        SEND_EVENT( wnd, SGUI_API_DESTROY_EVENT, NULL );

        if( wnd->dc )
        {
            SelectObject( wnd->dc, wnd->old_bitmap );
            DeleteObject( wnd->bitmap );
            DeleteDC( wnd->dc );
        }

        if( wnd->hWnd )
        {
            DestroyWindow( wnd->hWnd );
            PeekMessage( &msg, wnd->hWnd, WM_QUIT, WM_QUIT, PM_REMOVE );
        }

        sgui_widget_manager_destroy( wnd->mgr );

        free( wnd );
    }
}

void sgui_window_set_visible( sgui_window* wnd, int visible )
{
    if( wnd )
    {
        ShowWindow( wnd->hWnd, visible ? SW_SHOWNORMAL : SW_HIDE );
        wnd->visible = visible;

        if( !visible )
            SEND_EVENT( wnd, SGUI_API_INVISIBLE_EVENT, NULL );
    }
}

int sgui_window_is_visible( sgui_window* wnd )
{
    return wnd ? wnd->visible : 0;
}

void sgui_window_set_title( sgui_window* wnd, const char* title )
{
    if( wnd )
        SetWindowTextA( wnd->hWnd, title );
}

void sgui_window_set_size( sgui_window* wnd,
                           unsigned int width, unsigned int height )
{
    RECT rcClient, rcWindow;
    POINT ptDiff;
    sgui_event e;

    if( wnd )
    {
        /* Resize the client rect of the window */
        GetClientRect( wnd->hWnd, &rcClient );
        GetWindowRect( wnd->hWnd, &rcWindow );

        ptDiff.x = (rcWindow.right  - rcWindow.left) - rcClient.right;
        ptDiff.y = (rcWindow.bottom - rcWindow.top ) - rcClient.bottom;

        MoveWindow( wnd->hWnd, rcWindow.left, rcWindow.top,
                    (int)width + ptDiff.x, (int)height + ptDiff.y, TRUE );

        wnd->w = width;
        wnd->h = height;

        /* resize the double buffering context/bitmap */
        SelectObject( wnd->dc, wnd->old_bitmap );
        DeleteObject( wnd->bitmap );
        DeleteDC( wnd->dc );

        wnd->dc = CreateCompatibleDC( NULL );
        wnd->bitmap = CreateBitmap( wnd->w, wnd->h, 1, 32, NULL );
        wnd->old_bitmap = (HBITMAP)SelectObject( wnd->dc, wnd->bitmap );

        sgui_window_draw_box( wnd, 0, 0, wnd->w, wnd->h,
                              SGUI_WINDOW_COLOR, 0 );

        /* redraw everything */
        e.draw.x = 0;
        e.draw.y = 0;
        e.draw.w = wnd->w;
        e.draw.h = wnd->h;

        SEND_EVENT( wnd, SGUI_DRAW_EVENT, &e );
    }
}

void sgui_window_get_size( sgui_window* wnd,
                           unsigned int* width, unsigned int* height )
{
    if( wnd )
    {
        if( width  ) *width  = wnd->w;
        if( height ) *height = wnd->h;
    }
}

void sgui_window_move_center( sgui_window* wnd )
{
    RECT desktop, window;
    int w, h, dw, dh;

    if( wnd )
    {
        GetClientRect( GetDesktopWindow( ), &desktop );
        GetWindowRect( wnd->hWnd,           &window );

        w = window.right  - window.left;
        h = window.bottom - window.top;

        dw = desktop.right  - desktop.left;
        dh = desktop.bottom - desktop.top;

        MoveWindow( wnd->hWnd, (dw>>1)-(w>>1), (dh>>1)-(h>>1), w, h, TRUE );
    }
}

void sgui_window_move( sgui_window* wnd, int x, int y )
{
    RECT outer, inner;
    int w, h, dx, dy;

    if( wnd )
    {
        GetClientRect( wnd->hWnd, &inner );
        GetWindowRect( wnd->hWnd, &outer );

        w = outer.right  - outer.left;
        h = outer.bottom - outer.top;

        dx = inner.left - outer.left;
        dy = inner.top  - outer.top;

        MoveWindow( wnd->hWnd, x+dx, y+dy, w, h, TRUE );
    }
}

void sgui_window_get_position( sgui_window* wnd, int* x, int* y )
{
    RECT r;

    if( wnd )
    {
        GetClientRect( wnd->hWnd, &r );

        if( x ) *x = r.left;
        if( y ) *y = r.top;
    }
}

int sgui_window_update( sgui_window* wnd )
{
    MSG msg;
    RECT r;

    if( !wnd || !wnd->visible )
        return 0;

    /* update the widgets, redraw window if there was any change */
    if( sgui_widget_manager_update( wnd->mgr, wnd ) )
    {
        SetRect( &r, 0, 0, wnd->w-1, wnd->h-1 );
        InvalidateRect( wnd->hWnd, &r, TRUE );
    }

    /* message loop */
    while( PeekMessage( &msg, wnd->hWnd, 0, 0, PM_REMOVE ) )
    {
        TranslateMessage( &msg );
        DispatchMessage( &msg );
    }

    return 1;
}

void sgui_window_on_event( sgui_window* wnd, sgui_window_callback fun )
{
    if( wnd )
        wnd->event_fun = fun;
}


void sgui_window_add_widget( sgui_window* wnd, sgui_widget* widget )
{
    if( wnd )
        sgui_widget_manager_add_widget( wnd->mgr, widget );
}

void sgui_window_remove_widget( sgui_window* wnd, sgui_widget* widget )
{
    if( wnd )
        sgui_widget_manager_remove_widget( wnd->mgr, widget );
}



void sgui_window_draw_pixmap( sgui_window* wnd, sgui_pixmap* pixmap,
                              int x, int y )
{
    if( pixmap && wnd )
    {
        HDC memdc = CreateCompatibleDC( wnd->dc );
        SelectObject( memdc, pixmap->bitmap );

        BitBlt( wnd->dc, x, y, pixmap->width, pixmap->height,
                memdc, 0, 0, SRCCOPY );

        DeleteDC( memdc );
    }
}

void sgui_window_draw_box( sgui_window* wnd, int x, int y,
                           unsigned int width, unsigned int height,
                           unsigned long bgcolor, int inset )
{
    RECT r;
    HBRUSH brush;
    POINT points[3];
    HPEN pen;

    SetRect( &r, x, y, x + (int)width, y + (int)height );

    brush = CreateSolidBrush( SGUI_COLORREF( bgcolor ) );

    FillRect( wnd->dc, &r, brush );

    DeleteObject( brush );

    if( inset == 0 )
        return;

    points[0].x = x;
    points[0].y = y + (int)height - 1;

    points[1].x = x;
    points[1].y = y;

    points[2].x = x + (int)width - 1;
    points[2].y = y;

    if( inset > 0 )
        pen = CreatePen( PS_SOLID, 1, SGUI_COLORREF( SGUI_INSET_COLOR ) );
    else
        pen = CreatePen( PS_SOLID, 1, SGUI_COLORREF( SGUI_OUTSET_COLOR ) );

    SelectObject( wnd->dc, pen );
    Polyline( wnd->dc, points, 3 );
    DeleteObject( pen );


    points[0].x = x + (int)width - 1;
    points[0].y = y;

    points[1].x = x + (int)width - 1;
    points[1].y = y + (int)height - 1;

    points[2].x = x;
    points[2].y = y + (int)height - 1;

    if( inset > 0 )
        pen = CreatePen( PS_SOLID, 1, SGUI_COLORREF( SGUI_OUTSET_COLOR ) );
    else
        pen = CreatePen( PS_SOLID, 1, SGUI_COLORREF( SGUI_INSET_COLOR ) );

    SelectObject( wnd->dc, pen );
    Polyline( wnd->dc, points, 3 );
    DeleteObject( pen );
}

void sgui_window_draw_fancy_lines( sgui_window* wnd, int x, int y,
                                   int* length, unsigned int num_lines,
                                   int start_horizontal )
{
    POINT p[100];
    unsigned int i;
    int h = start_horizontal;
    HPEN pen;

    /* draw bright lines with offset */
    p[0].x = x + 1;
    p[0].y = y + 1;

    for( i=0; i<num_lines; ++i, h=!h )
    {
        p[i+1].x = h ? (p[i].x + length[i]) :  p[i].x;
        p[i+1].y = h ?  p[i].y              : (p[i].y + length[i]);
    }

    pen = CreatePen( PS_SOLID, 1, SGUI_COLORREF( SGUI_OUTSET_COLOR ) );
    SelectObject( wnd->dc, pen );
    Polyline( wnd->dc, p, (int)num_lines + 1 );
    DeleteObject( pen );

    /* draw dark lines without offset */
    p[0].x = x;
    p[0].y = y;

    for( i=0; i<num_lines; ++i, h=!h )
    {
        p[i+1].x = h ? (p[i].x + length[i]) :  p[i].x;
        p[i+1].y = h ?  p[i].y              : (p[i].y + length[i]);
    }

    pen = CreatePen( PS_SOLID, 1, SGUI_COLORREF( SGUI_INSET_COLOR ) );
    SelectObject( wnd->dc, pen );
    Polyline( wnd->dc, p, (int)num_lines + 1 );
    DeleteObject( pen );
}

void sgui_window_draw_radio_button( sgui_window* wnd, int x, int y,
                                    int selected )
{
    HBRUSH brush;
    RECT r;

    SetPixel( wnd->dc, x+4, y, SGUI_COLORREF( SGUI_INSET_COLOR ) );
    SetPixel( wnd->dc, x+5, y, SGUI_COLORREF( SGUI_INSET_COLOR ) );
    SetPixel( wnd->dc, x+6, y, SGUI_COLORREF( SGUI_INSET_COLOR ) );
    SetPixel( wnd->dc, x+7, y, SGUI_COLORREF( SGUI_INSET_COLOR ) );

    SetPixel( wnd->dc, x+2, y+1, SGUI_COLORREF( SGUI_INSET_COLOR ) );
    SetPixel( wnd->dc, x+3, y+1, SGUI_COLORREF( SGUI_INSET_COLOR ) );

    SetPixel( wnd->dc, x+8, y+1, SGUI_COLORREF( SGUI_INSET_COLOR ) );
    SetPixel( wnd->dc, x+9, y+1, SGUI_COLORREF( SGUI_INSET_COLOR ) );

    SetPixel( wnd->dc, x+1, y+2, SGUI_COLORREF( SGUI_INSET_COLOR ) );
    SetPixel( wnd->dc, x+1, y+3, SGUI_COLORREF( SGUI_INSET_COLOR ) );

    SetPixel( wnd->dc, x, y+4, SGUI_COLORREF( SGUI_INSET_COLOR ) );
    SetPixel( wnd->dc, x, y+5, SGUI_COLORREF( SGUI_INSET_COLOR ) );
    SetPixel( wnd->dc, x, y+6, SGUI_COLORREF( SGUI_INSET_COLOR ) );
    SetPixel( wnd->dc, x, y+7, SGUI_COLORREF( SGUI_INSET_COLOR ) );

    SetPixel( wnd->dc, x+1, y+8, SGUI_COLORREF( SGUI_INSET_COLOR ) );
    SetPixel( wnd->dc, x+1, y+9, SGUI_COLORREF( SGUI_INSET_COLOR ) );



    SetPixel( wnd->dc, x+2, y+10, SGUI_COLORREF( SGUI_OUTSET_COLOR ) );
    SetPixel( wnd->dc, x+3, y+10, SGUI_COLORREF( SGUI_OUTSET_COLOR ) );

    SetPixel( wnd->dc, x+4, y+11, SGUI_COLORREF( SGUI_OUTSET_COLOR ) );
    SetPixel( wnd->dc, x+5, y+11, SGUI_COLORREF( SGUI_OUTSET_COLOR ) );
    SetPixel( wnd->dc, x+6, y+11, SGUI_COLORREF( SGUI_OUTSET_COLOR ) );
    SetPixel( wnd->dc, x+7, y+11, SGUI_COLORREF( SGUI_OUTSET_COLOR ) );

    SetPixel( wnd->dc, x+8, y+10, SGUI_COLORREF( SGUI_OUTSET_COLOR ) );
    SetPixel( wnd->dc, x+9, y+10, SGUI_COLORREF( SGUI_OUTSET_COLOR ) );

    SetPixel( wnd->dc, x+10, y+8, SGUI_COLORREF( SGUI_OUTSET_COLOR ) );
    SetPixel( wnd->dc, x+10, y+9, SGUI_COLORREF( SGUI_OUTSET_COLOR ) );

    SetPixel( wnd->dc, x+11, y+4, SGUI_COLORREF( SGUI_OUTSET_COLOR ) );
    SetPixel( wnd->dc, x+11, y+5, SGUI_COLORREF( SGUI_OUTSET_COLOR ) );
    SetPixel( wnd->dc, x+11, y+6, SGUI_COLORREF( SGUI_OUTSET_COLOR ) );
    SetPixel( wnd->dc, x+11, y+7, SGUI_COLORREF( SGUI_OUTSET_COLOR ) );

    SetPixel( wnd->dc, x+10, y+2, SGUI_COLORREF( SGUI_OUTSET_COLOR ) );
    SetPixel( wnd->dc, x+11, y+3, SGUI_COLORREF( SGUI_OUTSET_COLOR ) );



    brush = CreateSolidBrush( SGUI_COLORREF( SGUI_INSET_FILL_COLOR_L1 ) );

    SetRect( &r, x+2, y+2, x + 10, y + 10 );
    FillRect( wnd->dc, &r, brush );

    SetRect( &r, x+1, y+4, x + 11, y + 8 );
    FillRect( wnd->dc, &r, brush );

    SetRect( &r, x+4, y+1, x + 8, y + 11 );
    FillRect( wnd->dc, &r, brush );

    DeleteObject( brush );

    if( selected )
    {
        brush = CreateSolidBrush( SGUI_COLORREF( SGUI_RADIO_BUTTON_COLOR ) );

        SetRect( &r, x+3, y+4, x + 9, y + 8 );
        FillRect( wnd->dc, &r, brush );

        SetRect( &r, x+4, y+3, x + 8, y + 9 );
        FillRect( wnd->dc, &r, brush );

        DeleteObject( brush );
    }
}

void sgui_window_draw_checkbox( sgui_window* wnd, int x, int y,
                                int selected )
{
    int i;
    COLORREF c = SGUI_COLORREF(SGUI_CHECKBOX_TICK_COLOR);

    sgui_window_draw_box( wnd, x, y, 12, 12, SGUI_INSET_FILL_COLOR_L1, 1 );

    if( selected )
    {
        for( i=0; i<3; ++i )
        {
            SetPixel( wnd->dc, x+2, y+4+i, c );
            SetPixel( wnd->dc, x+3, y+5+i, c );
            SetPixel( wnd->dc, x+4, y+6+i, c );
            SetPixel( wnd->dc, x+5, y+5+i, c );
            SetPixel( wnd->dc, x+6, y+4+i, c );
            SetPixel( wnd->dc, x+7, y+3+i, c );
            SetPixel( wnd->dc, x+8, y+2+i, c );
        }
    }
}

