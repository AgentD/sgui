#ifndef SGUI_WIDGET_MANAGER_H
#define SGUI_WIDGET_MANAGER_H



#include "sgui_predef.h"
#include "sgui_event.h"



#ifdef __cplusplus
extern "C"
{
#endif



/**
 * \brief Create a widget manager
 *
 * A widget manager holds and manages widgets.
 */
sgui_widget_manager* sgui_widget_manager_create( void );

/** \brief Destroy a widget manager */
void sgui_widget_manager_destroy( sgui_widget_manager* mgr );

/**
 * \brief Add a widget to a widget manager
 *
 * \param mgr    The widget manager
 * \param widget The widget to add
 */
void sgui_widget_manager_add_widget( sgui_widget_manager* mgr,
                                     sgui_widget* widget );

/**
 * \brief Remove a widget from a widget manager
 *
 * \param mgr    The widget manager
 * \param widget The widget to remove
 */
void sgui_widget_manager_remove_widget( sgui_widget_manager* mgr,
                                        sgui_widget* widget );

/**
 * \brief Update all widgets managed by a widget manager
 *
 * \param mgr The widget manager
 * \param wnd The window to draw to, in case a widget needs redrawing
 */
int sgui_widget_manager_update( sgui_widget_manager* mgr,
                                sgui_window* wnd );

/**
 * \brief Send an event to all widgets held by a widget manager
 *
 * \param mgr   The widget manager
 * \param wnd   The window sending the event
 * \param event The event type to send
 * \param e     The event data to send
 */
void sgui_widget_manager_send_event( sgui_widget_manager* mgr,
                                     sgui_window* wnd, int event,
                                     sgui_event* e );



#ifdef __cplusplus
}
#endif

#endif /* SGUI_WIDGET_MANAGER_H */

