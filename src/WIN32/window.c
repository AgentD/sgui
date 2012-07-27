#include "internal.h"




void clear( sgui_window* wnd, unsigned long color )
{
    unsigned int X, Y;

    for( Y=0; Y<wnd->h; ++Y )
        for( X=0; X<wnd->w; ++X )
        {
            wnd->back_buffer[ (Y*wnd->w + X)*4     ] =  color      & 0xFF;
            wnd->back_buffer[ (Y*wnd->w + X)*4 + 1 ] = (color>>8 ) & 0xFF;
            wnd->back_buffer[ (Y*wnd->w + X)*4 + 2 ] = (color>>16) & 0xFF;
        }
}

void draw_box( sgui_window* wnd, int x, int y,
                                 unsigned int w, unsigned int h,
                                 unsigned long color )
{
    int lr_x = x + (int)w;
    int lr_y = y + (int)h;
    int X, Y;

    x = x<0 ? 0 : x;
    y = y<0 ? 0 : y;

    if( x >= (int)wnd->w || y >= (int)wnd->h || lr_x < 0 || lr_y < 0 )
        return;

    if( lr_x >= (int)wnd->w )
        lr_x = (int)wnd->w - 1;

    if( lr_y >= (int)wnd->h )
        lr_y = (int)wnd->h - 1;

    for( Y=y; Y!=lr_y; ++Y )
        for( X=x; X!=lr_x; ++X )
        {
            wnd->back_buffer[ (Y*wnd->w + X)*4     ] =  color      & 0xFF;
            wnd->back_buffer[ (Y*wnd->w + X)*4 + 1 ] = (color>>8 ) & 0xFF;
            wnd->back_buffer[ (Y*wnd->w + X)*4 + 2 ] = (color>>16) & 0xFF;
        }
}

void draw_line( sgui_window* wnd, int x, int y, int length, int horizontal,
                                  unsigned long color )
{
    int i, endi;

    if( horizontal )
    {
        if( (y<0) || (y>=(int)wnd->h) )
            return;

        if( length < 0 )
        {
            length = -length;
            x -= length;
        }

        if( (x >= (int)wnd->w) || ((x+length)<0) )
            return;

        i = x < 0 ? 0 : x;
        endi = x+length+1;

        if( endi >= (int)wnd->w )
            endi = wnd->w - 1;

        for( ; i!=endi; ++i )
        {
            wnd->back_buffer[ (y*wnd->w + i)*4     ] =  color      & 0xFF;
            wnd->back_buffer[ (y*wnd->w + i)*4 + 1 ] = (color>>8 ) & 0xFF;
            wnd->back_buffer[ (y*wnd->w + i)*4 + 2 ] = (color>>16) & 0xFF;
        }
    }
    else
    {
        if( (x<0) || (x>=(int)wnd->w) )
            return;

        if( length < 0 )
        {
            length = -length;
            y -= length;
        }

        if( (y >= (int)wnd->h) || ((y+length)<0) )
            return;

        i = y < 0 ? 0 : y;
        endi = y+length+1;

        if( endi >= (int)wnd->h )
            endi = wnd->h - 1;

        for( ; i!=endi; ++i )
        {
            wnd->back_buffer[ (i*wnd->w + x)*4     ] = (color>>16) & 0xFF;
            wnd->back_buffer[ (i*wnd->w + x)*4 + 1 ] = (color>>8 ) & 0xFF;
            wnd->back_buffer[ (i*wnd->w + x)*4 + 2 ] =  color      & 0xFF;
        }
    }
}




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
        wnd->info.bmiHeader.biWidth  = wnd->w;
        wnd->info.bmiHeader.biHeight = -((int)wnd->h);

        SelectObject( wnd->dc, wnd->old_bitmap );
        DeleteObject( wnd->bitmap );
        DeleteDC( wnd->dc );

        wnd->dc = CreateCompatibleDC( NULL );
        wnd->bitmap = CreateDIBSection( wnd->dc, &wnd->info, DIB_RGB_COLORS,
                                        (void**)&wnd->back_buffer, 0, 0 );
        wnd->old_bitmap = (HBITMAP)SelectObject( wnd->dc, wnd->bitmap );

        clear( wnd, SGUI_WINDOW_COLOR );

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

        BitBlt( hDC, 0, 0, wnd->w+1, wnd->h, wnd->dc, 0, 0, SRCCOPY );

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
    wnd->info.bmiHeader.biSize        = sizeof(wnd->info.bmiHeader);
    wnd->info.bmiHeader.biBitCount    = 32;
    wnd->info.bmiHeader.biCompression = BI_RGB;
    wnd->info.bmiHeader.biPlanes      = 1;
    wnd->info.bmiHeader.biWidth       = wnd->w;
    wnd->info.bmiHeader.biHeight      = -((int)wnd->h);

    wnd->dc = CreateCompatibleDC( NULL );
    wnd->bitmap = CreateDIBSection( wnd->dc, &wnd->info, DIB_RGB_COLORS,
                                    (void**)&wnd->back_buffer, 0, 0 );
    wnd->old_bitmap = (HBITMAP)SelectObject( wnd->dc, wnd->bitmap );

    clear( wnd, SGUI_WINDOW_COLOR );

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
        wnd->info.bmiHeader.biWidth  = wnd->w;
        wnd->info.bmiHeader.biHeight = -((int)wnd->h);

        SelectObject( wnd->dc, wnd->old_bitmap );
        DeleteObject( wnd->bitmap );
        DeleteDC( wnd->dc );

        wnd->dc = CreateCompatibleDC( NULL );
        wnd->bitmap = CreateDIBSection( wnd->dc, &wnd->info, DIB_RGB_COLORS,
                                        (void**)&wnd->back_buffer, 0, 0 );
        wnd->old_bitmap = (HBITMAP)SelectObject( wnd->dc, wnd->bitmap );

        clear( wnd, SGUI_WINDOW_COLOR );

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



void sgui_window_blit_image( sgui_window* wnd, int x, int y,
                             unsigned int width, unsigned int height,
                             unsigned char* image, int has_a )
{
    unsigned char* dst;
    unsigned int i, j, bpp = has_a ? 4 : 3;

    for( j=0; j<height; ++j )
    {
        dst = wnd->back_buffer + ((y+j)*wnd->w + x) * 4;

        if( (y+(int)j)<0 )
            continue;

        if( (y+(int)j)>=(int)wnd->h )
            break;

        for( i=0; i<width; ++i, image+=bpp, dst+=4 )
        {
            if( (x+(int)i)>0 && (x+(int)i)<(int)wnd->w )
            {
                dst[0] = image[2];
                dst[1] = image[1];
                dst[2] = image[0];
            }
        }
    }
}

void sgui_window_blend_image( sgui_window* wnd, int x, int y,
                              unsigned int width, unsigned int height,
                              unsigned char* image )
{
    unsigned char* dst;
    unsigned int i, j;
    float A;

    for( j=0; j<height; ++j )
    {
        dst = wnd->back_buffer + ((y+j)*wnd->w + x) * 4;

        if( (y+(int)j)<0 )
            continue;

        if( (y+(int)j)>=(int)wnd->h )
            break;

        for( i=0; i<width; ++i, image+=4, dst+=4 )
        {
            if( (x+(int)i)>0 && (x+(int)i)<(int)wnd->w )
            {
                A = ((float)image[3]) / 255.0f;

                dst[0] = image[2] * A + dst[0] * (1.0f-A);
                dst[1] = image[1] * A + dst[1] * (1.0f-A);
                dst[2] = image[0] * A + dst[2] * (1.0f-A);
            }
        }
    }
}

void sgui_window_draw_box( sgui_window* wnd, int x, int y,
                           unsigned int width, unsigned int height,
                           unsigned long bgcolor, int inset )
{
    draw_box( wnd, x, y, width, height, bgcolor );

    if( inset>0 )
    {
        draw_line( wnd, x,       y,        width,  1, SGUI_INSET_COLOR  );
        draw_line( wnd, x,       y,        height, 0, SGUI_INSET_COLOR  );

        draw_line( wnd, x,       y+height, width,  1, SGUI_OUTSET_COLOR );
        draw_line( wnd, x+width, y,        height, 0, SGUI_OUTSET_COLOR );
    }
    else if( inset<0 )
    {
        draw_line( wnd, x,       y,        width,  1, SGUI_OUTSET_COLOR );
        draw_line( wnd, x,       y,        height, 0, SGUI_OUTSET_COLOR );

        draw_line( wnd, x,       y+height, width,  1, SGUI_INSET_COLOR  );
        draw_line( wnd, x+width, y,        height, 0, SGUI_INSET_COLOR  );
    }
}

void sgui_window_draw_fancy_lines( sgui_window* wnd, int x, int y,
                                   int* length, unsigned int num_lines,
                                   int start_horizontal )
{
    unsigned int i;
    int h, oldx = x, oldy = y;

    for( h=start_horizontal, i=0; i<num_lines; ++i, h=!h )
    {
        if( h )
        {
            draw_line( wnd, x, y+1, length[i]+1, 1, SGUI_OUTSET_COLOR );
            x += length[i];
        }
        else
        {
            draw_line( wnd, x+1, y, length[i], 0, SGUI_OUTSET_COLOR );
            y += length[i];
        }
    }

    for( x=oldx, y=oldy, h=start_horizontal, i=0; i<num_lines; ++i, h=!h )
    {
        draw_line( wnd, x, y, length[i], h, SGUI_INSET_COLOR );

        if( h )
            x += length[i];
        else
            y += length[i];
    }
}

