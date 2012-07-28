#ifndef SGUI_SKIN_H
#define SGUI_SKIN_H



#include "sgui_predef.h"



#ifdef __cplusplus
extern "C"
{
#endif



void sgui_skin_init( void );

void sgui_skin_deinit( void );



/**
 * \brief Override the default font for the skin
 *
 * \param normal      Font face for normal text.
 * \param bold        Font face for bold text.
 * \param italic      Font face for italic text.
 * \param bold_italic Font face for both bold and italic text.
 * \param height      Default font height in pixels.
 */
void sgui_skin_set_default_font( sgui_font* normal, sgui_font* bold,
                                 sgui_font* italic, sgui_font* bold_italic,
                                 unsigned int height );



/**
 * \brief Get a the default window background color
 */
void sgui_skin_get_window_background_color( unsigned char* color );

/**
 * \brief Predict the size of a button when rendered
 *
 * \param text   The UTF8 text to display on the button
 * \param width  Returns the width of the button in pixels
 * \param height Returns the height of the button in pixels
 * \param text_w Returns the width of the text on the button in pixels
 */
void sgui_skin_get_button_extents( const unsigned char* text,
                                   unsigned int* width,
                                   unsigned int* height,
                                   unsigned int* text_w );

/**
 * \brief Draw a text onto a window
 *
 * \param text   The UTF8 text to print. The LF ('\n') character can be used
 *               for line wraps, the \<b\> \</b\> and \<i\> \</i\> for writing
 *               text bold or italic. A \<color="#RRGGBB"\> tag can be used to
 *               switch text color, where the value "default" for color
 *               switches back to default color.
 * \param width  Returns the width of the rendered text.
 * \param height Returns the height of the rendered text.
 */
void sgui_skin_get_text_extents( const unsigned char* text,
                                 unsigned int* width, unsigned int* height );



/**
 * \brief Draw a progress bar onto a window
 *
 * \param wnd    The window to draw to.
 * \param x      The distance from the left side of the bar to the left of
 *               the window.
 * \param y      The distance from the top of the bar to the top of
 *               the window.
 * \param width  The horizontal extent of the bar.
 * \param height The vertical extent of the bar.
 * \param style  The style of the bar (stippled or continuous).
 * \param value  The progress value to indicate (value between 0.0 and 1.0).
 */
void sgui_skin_draw_progress_bar( sgui_window* wnd, int x, int y,
                                  unsigned int width, unsigned int height,
                                  int horizontal, int style, float value );

/**
 * \brief Draw a button widget onto a window
 *
 * \param wnd    The window to draw to.
 * \param x      The distance from the left side of the button to the left of
 *               the window.
 * \param y      The distance from the top of the button to the top of
 *               the window.
 * \param width  Width of the button area.
 * \param text_w Width of the text string in pixels.
 * \param height Height of the button area.
 * \param state  Zero if the button is in default state, non-zero if it is
 *               pressed.
 * \param text   UTF8 text to print onto the button.
 */
void sgui_skin_draw_button( sgui_window* wnd, int x, int y, int state,
                            unsigned int width, unsigned int text_w,
                            unsigned int height, const unsigned char* text );

/**
 * \brief Draw a text onto a window
 *
 * \param wnd  The window to draw to.
 * \param x    Distance from the left of the text to the left of the window.
 * \param y    Distance from the top of the text to the top of the window.
 * \param text The UTF8 text to print. The LF ('\n') character can be used
 *             for line wraps, the \<b\> \</b\> and \<i\> \</i\> for writing
 *             text bold or italic. A \<color="#RRGGBB"\> tag can be used to
 *             switch text color, where the value "default" for color switches
 *             back to default color.
 */
void sgui_skin_draw_text( sgui_window* wnd, int x, int y,
                          unsigned int width, unsigned int height,
                          const unsigned char* text );



#ifdef __cplusplus
}
#endif

#endif /* SGUI_SKIN_H */

