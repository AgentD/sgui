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
     * \brief Callback that is called to redraw a widget
     *
     * \param widget A pointer to the widget to redraw (for C++
     *               people: practically a this pointer).
     * \param wnd    The window to draw to.
     * \param x      Window relative x coordinate of a region to draw into
     * \param y      Window relative x coordinate of a region to draw into
     * \param w      Width of the region to draw into
     * \param h      Height of the region to draw into
     */
    void (* draw_callback )( sgui_widget* widget, sgui_window* wnd,
                             int x, int y, unsigned int w, unsigned int h );

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
     * \param type   The event type.
     * \param event  The window event that occoured.
     */
    void (* window_event_callback )( sgui_widget* widget, int type,
                                     sgui_event* event );
};



#endif /* SGUI_WIDGET_INTERNAL_H */

