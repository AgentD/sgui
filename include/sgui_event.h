#ifndef SGUI_EVENT_H
#define SGUI_EVENT_H



#define SGUI_MOUSE_BUTTON_LEFT   0
#define SGUI_MOUSE_BUTTON_MIDDLE 1
#define SGUI_MOUSE_BUTTON_RIGHT  2



#define SGUI_USER_CLOSED_EVENT        0
#define SGUI_API_INVISIBLE_EVENT      1
#define SGUI_API_DESTROY_EVENT        2
#define SGUI_SIZE_CHANGE_EVENT        3
#define SGUI_MOUSE_MOVE_EVENT         4
#define SGUI_MOUSE_PRESS_EVENT        5
#define SGUI_MOUSE_WHEEL_EVENT        6
#define SGUI_MOUSE_ENTER_EVENT        7
#define SGUI_MOUSE_LEAVE_EVENT        8
#define SGUI_DRAW_EVENT               9



typedef union
{
    struct { unsigned int new_width, new_height; } size;

    struct { int x, y; } mouse_move;

    struct { int button, pressed; } mouse_press;

    struct { int direction; } mouse_wheel;

    struct { int x, y; unsigned int w, h; } draw;
}
sgui_event;



#endif /* SGUI_EVENT_H */

