#ifndef SGUI_WIDGET_H
#define SGUI_WIDGET_H



#include "sgui_window.h"
#include "sgui_predef.h"
#include "sgui_event.h"



#ifdef __cplusplus
extern "C" {
#endif



/**
 * \brief Change the position of a widget
 *
 * \param w The widget to reposition
 * \param x The x component of the position
 * \param y The y component of the position
 */
void sgui_widget_set_position( sgui_widget* w, int x, int y );

/**
 * \brief Get the position of a widget
 *
 * \param w The widget to get the position from
 * \param x Returns the x component of the position
 * \param y Returns the y component of the position
 */
void sgui_widget_get_position( sgui_widget* w, int* x, int* y );

/**
 * \brief Get the size of a widget
 *
 * \param w      The widget to get the size of
 * \param width  Returns the width of the widget
 * \param height Returns the height of the widget
 */
void sgui_widget_get_size( sgui_widget* w,
                           unsigned int* width, unsigned int* height );

/**
 * \brief Returns non-zero if a widget intersects a given area, zero if not
 *
 * \param w      The widget to test
 * \param x      The x component of the position of the region to test
 * \param y      The y component of the position of the region to test
 * \param width  The width of the region to test
 * \param height The height of the region to test
 */
int sgui_widget_intersects_area( sgui_widget* w, int x, int y,
                                 unsigned int width, unsigned int height );

/**
 * \brief Returns non-zero if a point is inside the bounding box of a widget
 *
 * \param w The widget to test
 * \param x The x component of the position to test
 * \param y The y component of the position to test
 */
int sgui_widget_is_point_inside( sgui_widget* w, int x, int y );

/**
 * \brief Update a widget (e.g. advance animations, et cetera)
 *
 * This is called inside the sgui_window_update function to update widgets
 * assigned to a window.
 *
 * \param widget The widget to update
 */
void sgui_widget_update( sgui_widget* widget );

/**
 * \brief Send a window event to a widget
 *
 * This is called inside the sgui_window_update function to make widgets
 * interact with user input.
 *
 * \param widget The widget to send the event to
 * \param wnd    The window that sent the event
 * \param type   The event type
 * \param event  The event to send
 */
void sgui_widget_send_window_event( sgui_widget* widget, sgui_window* wnd,
                                    int type, sgui_event* event );

/**
 * \brief Returns non-zero if a widget needs to be redrawn
 *
 * After calling, the internal state of the widget is reset, so it no longer
 * reports that it needs to be redrawn.
 *
 * This is called inside the sgui_window_update function to check which
 * widgets assigned to a window need to be redrawn.
 *
 * \param widget The widget to test
 */
int sgui_widget_need_redraw( sgui_widget* widget );



#ifdef __cplusplus
}
#endif

#endif /* SGUI_WIDGET_H */

