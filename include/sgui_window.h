#ifndef SGUI_WINDOW_H
#define SGUI_WINDOW_H



#include "sgui_event.h"



#ifdef __cplusplus
extern "C" {
#endif



#define SGUI_RESIZEABLE 1
#define SGUI_FIXED_SIZE 0

#define SGUI_VISIBLE   1
#define SGUI_INVISIBLE 0



typedef struct sgui_window sgui_window;



/**
 * \brief A callback for listening to window events
 *
 * \param wnd   The window that triggered the event
 * \param type  An identifyer describing the event
 * \param event Additional data to the event
 */
typedef void (* sgui_window_callback ) ( sgui_window* wnd, int type,
                                         sgui_event* event );



/**
 * \brief Create a window
 *
 * Creates a window using the platforms native window system. The window has
 * to be destroyed again using sgui_window_destroy( ), thus freeing up it's
 * resources.
 *
 * \note The window is created invisible and has to be made visible by calling
 *       sgui_window_set_visible( ).
 *
 * \param width      The width of the window(without borders and decoration).
 * \param height     The height of the window(without borders and decoration)
 * \param resizeable Non-zero if the window should be resizeable by the user,
 *                   zero if the should remain at a fixed size. The symbolic
 *                   constants SGUI_RESIZEABLE and SGUI_FIXED_SIZE can be used
 *                   to generate more readable code.
 *
 * \return Either a valid pointer to a window or NULL if there was an error
 */
sgui_window* sgui_window_create( unsigned int width, unsigned int height,
                                 int resizeable );

/**
 * \brief Destroy a previously created window
 *
 * The window is closed and all it's resources are freed up, so the window
 * pointer itself is nolonger valid after a call to this function.
 *
 * This function triggers the window close event with SGUI_API_DESTROYED as
 * how paramter, no matter whether the window is visible or not.
 */
void sgui_window_destroy( sgui_window* wnd );

/**
 * \brief Make a window visible or invisible
 *
 * This function shows or hides a window. The window is hidden by default
 * after creation.
 *
 * If the window is set invisible, the window close event is triggered with
 * the parameter SGUI_API_MADE_INVISIBLE.
 *
 * \param wnd     A pointer to the window
 * \param visible Non-zero to turn the window visible, zero to turn it
 *                invisible. The symbolic constants SGUI_VISIBLE and
 *                SGUI_INVISIBLE can be used to generat more readable code.
 */
void sgui_window_set_visible( sgui_window* wnd, int visible );

/** \brief Returns non-zero if a given window is visible, zero otherwise */
int sgui_window_is_visible( sgui_window* wnd );

/**
 * \brief Change the title of a window
 *
 * Most windowsystems decorate windows with borders which tend to have a
 * configurable titlebar with a text in it. This function can be used to alter
 * that title.
 *
 * \param wnd   A pointer to a window
 * \param title The new title as a NULL terminated ASCII string
 */
void sgui_window_set_title( sgui_window* wnd, const char* title );

/**
 * \brief Change the size of a window
 *
 * A call to this function will trigger a size change event.
 *
 * \param wnd     A pointer to a window
 * \param width   The width of the window(without borders and decoration).
 *                If zero, the entire screen width is used.
 * \param height  The height of the window(without borders and decoration)
 *                If zero, the entire screen height is used.
 */
void sgui_window_set_size( sgui_window* wnd,
                           unsigned int width, unsigned int height );

/**
 * \brief Get the size of a window
 *
 * \param width  Returns width of the window. Pass NULL if you're not
 *               iteressted in it.
 * \param height Returns height of the window. Pass NULL if you're not
 *               iteressted in it.
 */
void sgui_window_get_size( sgui_window* wnd,
                           unsigned int* width, unsigned int* height );

/** \brief Relocate a window to the center of the screen */
void sgui_window_move_center( sgui_window* wnd );

/**
 * \brief Move a window to a specified position
 *
 * \param wnd A pointer to a window
 * \param x   The distance of the left of the window to the left of the screen
 * \param y   The distance to the top of the window to the top of the screen
 */
void sgui_window_move( sgui_window* wnd, int x, int y );

/**
 * \brief Get the position of a window
 *
 * \param wnd A pointer to a window
 * \param x   The distance of the left of the window to the left of the
 *            screen. Pass NULL if you're not interessted in it.
 * \param y   The distance to the top of the window to the top of the screen.
 *            Pass NULL if you're not interessted in it.
 */
void sgui_window_get_position( sgui_window* wnd, int* x, int* y );

/**
 * \brief Update the window
 *
 * Updates all widgets in the window, redraws them if necessary, handles
 * window system events and generates window events on widgets and callbacks.
 *
 * \return Non-zero if the window is opened, zero if it is invisible(the user
 *         closed it or it is set invisible)
 */
int sgui_window_update( sgui_window* wnd );

/**
 * \brief Set a window's event callback
 *
 * Each window may have ONE callback registered that gets called when an event
 * occours. The callback receives the pointer of the window that triggered it,
 * an event identifyer and a pointer to a structure or other datatype with
 * additional information on the event
 *
 * \param wnd A pointer to a window
 * \param fun The callback function, or NULL to unset
 */
void sgui_window_on_event( sgui_window* wnd, sgui_window_callback fun );

/**
 * \brief Force redrawing of a window region
 *
 * \param wnd    The window to force to redraw.
 * \param x      The x component of the position of the redraw region.
 * \param y      The y component of the position of the redraw region.
 * \param width  The width of the redraw region.
 * \param height The height of the redraw region.
 */
void sgui_window_force_redraw( sgui_window* wnd, int x, int y,
                               unsigned int width, unsigned int height );



/**
 * \brief Draw a rectangle/box
 *
 * \param wnd     The window to draw onto
 * \param x       The distance of the left of the box from the
 *                left of the window
 * \param y       The distance of the top of the box from the
 *                top of the window
 * \param width   The extent of the box in x direction
 * \param height  The extent of the box in y direction
 * \param bgcolor The background fill color of the box
 * \param inset   If the value is non-zero, a border is drawn around the box.
 *                If the value is positive, the border is shaded in a way to
 *                make the box apear to go deeper into the window.
 *                If the value is negative, the border is shaded in a way to
 *                make the box apera to stick out of the window.
 */
void sgui_window_draw_box( sgui_window* wnd, int x, int y,
                           unsigned int width, unsigned int height,
                           unsigned long bgcolor, int inset );

/**
 * \brief Draw fancy lines that are shaded in a way that they apear to be
 *        engraved into the window.
 *
 * The lines are assumed to start off of the end of the last one and to be
 * either horizontally or vertically, where a horizontal line is always
 * followed by a vertical line and vice versa, so everything needed to
 * discribe the fancy line sequence is a start point and an array of integers
 * indicating line lengths and direction using the sign. A positive value
 * means right for horizontal lines and down for vertical lines and a negative
 * value means the opposite.
 * 
 * \param wnd              The window to draw to.
 * \param x                The distance of the start point to the left of the
 *                         window.
 * \param y                The distance of the start point to the top of the
 *                         window.
 * \param length           An array of integers giving lengths and directions
 *                         of the line sequence.
 * \param num_lines        The number of lines to draw, i.e. the number of
 *                         entries in the length array.
 * \param start_horizontal Zero to start with a vertical line, non-zero to
 *                         start with a horizontal line.
 */
void sgui_window_draw_fancy_lines( sgui_window* wnd, int x, int y,
                                   int* length, unsigned int num_lines,
                                   int start_horizontal );

/**
 * \brief Draw a radio button
 *
 * \param wnd      The window to draw to
 * \param x        The distance to the left of the window.
 * \param y        The distance to the top of the window.
 * \param selected If zero the radio button is emtpy, otherwise it contains
 *                 a smaller circle that indicats selection.
 */
void sgui_window_draw_radio_button( sgui_window* wnd, int x, int y,
                                    int selected );

/**
 * \brief Draw a check box
 *
 * \param wnd      The window to draw to
 * \param x        The distance to the left of the window.
 * \param y        The distance to the top of the window.
 * \param selected If zero the check box is emtpy, otherwise it contains
 *                 a tick that indicats selection.
 */
void sgui_window_draw_checkbox( sgui_window* wnd, int x, int y,
                                int selected );




#ifdef __cplusplus
}
#endif

#endif /* SGUI_H */

