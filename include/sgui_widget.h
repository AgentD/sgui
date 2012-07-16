#ifndef SGUI_WIDGET_H
#define SGUI_WIDGET_H



#include "sgui_window.h"
#include "sgui_predef.h"



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
 * \param w The widget to reposition
 * \param x Returns the x component of the position
 * \param y Returns the y component of the position
 */
void sgui_widget_get_position( sgui_widget* w, int* x, int* y );

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
 * \brief Redraw a widget
 *
 * \param widget A pointer to the widget to redraw.
 * \param wnd    The window to draw to.
 * \param x      Window relative x coordinate of a clip region to draw into
 * \param y      Window relative x coordinate of a clip region to draw into
 * \param w      Width of the clip region to draw into
 * \param h      Height of the clip region to draw into
 */
void sgui_widget_draw( sgui_widget* widget, sgui_window* wnd, int x, int y,
                       unsigned int w, unsigned int h );




#ifdef __cplusplus
}
#endif

#endif /* SGUI_WIDGET_H */

