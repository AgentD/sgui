#ifndef SGUI_WIDGET_INTERNAL_H
#define SGUI_WIDGET_INTERNAL_H



#include "sgui_widget.h"



struct sgui_widget
{
    int x;                  /**< \brief x component of widget position */
    int y;                  /**< \brief y component of widget position */
    unsigned int width;     /**< \brief widget width */
    unsigned int height;    /**< \brief widget height */

    int need_redraw;        /**< \brief non-zero if need for redrawing */

    /**
     * \brief Callback that is called to update a widget
     *
     * \param widget A pointer to the widget to update (for C++
     *               people: practically a this pointer).
     */
    void (* update_callback )( sgui_widget* widget );

    /**
     * \brief Callback that is called to inject window events
     *
     * \param widget A pointer to the widget to update (for C++
     *               people: practically a this pointer).
     * \param wnd    The window that sent the event
     * \param type   The event type.
     * \param event  The window event that occoured.
     */
    void (* window_event_callback )( sgui_widget* widget, sgui_window* wnd,
                                     int type, sgui_event* event );
};



#endif /* SGUI_WIDGET_INTERNAL_H */

