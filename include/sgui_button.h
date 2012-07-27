#ifndef SGUI_BUTTON_H
#define SGUI_BUTTON_H



#include "sgui_predef.h"



#ifdef __cplusplus
extern "C"
{
#endif



/**
 * \brief Create a button widget
 *
 * \param x    X component of the buttons position
 * \param x    Y component of the buttons position
 * \param text Text written onto the button
 * \param font The font face to use for rendering the text on the button
 * \param size The font size to use for writing to the button
 *
 * \return A button widget
 */
sgui_widget* sgui_button_create( int x, int y, const unsigned char* text,
                                 sgui_font* font, unsigned int size );

/**
 * \brief Delete a button widget
 *
 * \param button The button widget
 */
void sgui_button_delete( sgui_widget* button );



#ifdef __cplusplus
}
#endif

#endif /* SGUI_BUTTON_H */

