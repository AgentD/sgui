#ifndef SGUI_PROGRESS_BAR_H
#define SGUI_PROGRESS_BAR_H



#include "sgui_widget.h"



/**
 * \brief Create a progress bar widget
 *
 * \param x        The x component of the position of the progress bar.
 * \param y        The y component of the position of the progress bar.
 * \param width    Width of the progress bar in pixels.
 * \param height   Height of the progress bar in pixels.
 * \param progress Initial progress for the bar (value between 0.0 and 1.0)
 */
sgui_widget* sgui_progress_bar_create( int x, int y, unsigned int width,
                                       unsigned int height, float progress );

/**
 * \brief Set the progress on a progress bar
 *
 * \param bar      The progress bar to alter.
 * \param progress Value between 0.0 and 1.0 for the bar to display.
 */
void sgui_progress_bar_set_progress( sgui_widget* bar, float progress );

/**
 * \brief Get the progress on a progress bar
 *
 * \param bar The progress bar to alter.
 */
float sgui_progress_bar_get_progress( sgui_widget* bar );

/**
 * \brief Delete a progress bar widget
 *
 * \param par The progress bar widget to delete
 */
void sgui_progress_bar_delete( sgui_widget* bar );



#endif /* SGUI_PROGRESS_BAR_H */

