#ifndef SGUI_STATIC_TEXT_H
#define SGUI_STATIC_TEXT_H



#include "sgui_predef.h"



#ifdef __cplusplus
extern "C"
{
#endif



/**
 * \brief Create a static text widget
 *
 * \param x    The x component of the widget position.
 * \param y    The y component of the widget position.
 * \param text The UTF8 text to print. The LF ('\n') character can be used
 *             for line wraps, the \<b\> \</b\> and \<i\> \</i\> for writing
 *             text bold or italic. A \<color="#RRGGBB"\> tag can be used to
 *             switch text color. The default text color is SGUI_DEFAULT_TEXT.
 * \param norm The font face to use for rendering the text.
 * \param bold The font face to use for rendering bold parts of the text.
 * \param ital The font face to use for rendering italic parts of the text.
 * \param boit The font face to use for rendering bold and italic parts of
 *             the text.
 * \praam size The font size in pixels.
 */
sgui_widget* sgui_static_text_create( int x, int y, const unsigned char* text,
                                      sgui_font* norm, sgui_font* bold,
                                      sgui_font* ital, sgui_font* boit,
                                      unsigned int size );

/** \brief Delete a static text widget */
void sgui_static_text_delete( sgui_widget* widget );



#ifdef __cplusplus
}
#endif

#endif /* SGUI_STATIC_TEXT_H */

