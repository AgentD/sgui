#ifndef SGUI_FONT_MANAGER_H
#define SGUI_FONT_MANAGER_H



#include "sgui_predef.h"



#ifdef __cplusplus
extern "C"
{
#endif



/**
 * \brief Initialise the font rasterisation subsystem
 *
 * \note Call this before any other sgui_font_* function!
 *
 * \return non-zero on success, zero on failure.
 */
int sgui_font_init( void );

/**
 * \brief Uninitialise the font rasterisation subsystem
 *
 * \note Call this after you're done with font rasterisation and do not call
 *       any other sgui_font_* function afterwards!
 */
void sgui_font_deinit( void );



/**
 * \brief Load a font face from a file using STDIO
 *
 * \param filename The path to the font file
 *
 * \returns A font object
 */
sgui_font* sgui_font_load_from_file( const char* filename );

/**
 * \brief Load a font from a memory buffer
 *
 * This is usefull when loading fonts from other resources than the standard
 * file system (e.g. custom virtual filesystems like physicsFS et al.)
 *
 * \note This function takes ownership of the given buffer. Do NOT delete it
 *       on your own afterwards.
 *
 * \param buffer     Pointer to a buffer containing the loaded font file data
 * \param buffersize The size of the buffer in bytes
 *
 * \returns A font object
 */
sgui_font* sgui_font_load_from_mem( void* buffer, unsigned int buffersize );

/**
 * \brief Delete a font object
 */
void sgui_font_delete( sgui_font* font );


/**
 * \brief Print a string of text to a buffer
 *
 * \param text        The UTF8 text to print. Line wraps are treated as end of
 *                    string.
 * \param font_face   The font face to use
 * \param font_height The height of the text in texels
 * \param buffer      The buffer to print the text to
 * \param x           The x offset of the text in the buffer in pixels
 * \param y           The y offset of the text in the buffer in pixels
 * \param width       The width of the target buffer in texels
 * \param height      The height of the target buffer in texels
 * \param color       The 8 bit per channel RGB text color
 * \param length      The number of characters to read from the text string.
 * \param alpha       Non-zero if the image has an additional alpha channel.
 *                    The alpha channel will be left untouched.
 */
void sgui_font_print( const unsigned char* text, sgui_font* font_face,
                      unsigned int font_height, unsigned char* buffer,
                      int x, int y, unsigned int width, unsigned int height,
                      unsigned char* color, unsigned int length, int alpha );

/**
 * \brief Print a string of text to a buffer with alpha channel from the text
 *
 * Instead of blending the text onto the buffer (like sgui_font_print), the
 * text will overwrite the buffer with the given color and write the fonts
 * transparency value into the alpha channel.
 *
 * \param text        The UTF8 text to print. Line wraps are treated as end of
 *                    string.
 * \param font_face   The font face to use
 * \param font_height The height of the text in texels
 * \param buffer      The buffer to print the text to
 * \param x           The x offset of the text in the buffer in pixels
 * \param y           The y offset of the text in the buffer in pixels
 * \param width       The width of the target buffer in texels
 * \param height      The height of the target buffer in texels
 * \param color       The 8 bit per channel RGB text color
 * \param length      The number of characters to read from the text string.
 */
void sgui_font_print_alpha( const unsigned char* text, sgui_font* font_face,
                            unsigned int font_height, unsigned char* buffer,
                            int x, int y, unsigned int width,
                            unsigned int height, unsigned char* color,
                            unsigned int length );

/**
 * \brief Predict the length of a string in pixels for a given font
 *
 * \param text      The UTF8 text to calculate the size of in printed form.
 *                  Linefeed characters are treeted as the end of the string.
 * \param font_face The font face to use
 * \param height    The height of the text in texels
 * \param length    The number of characters to read from the text string.
 *
 * \returns The horizontal length of the printed string in pixels
 */
unsigned int sgui_font_extents( const unsigned char* text,
                                sgui_font* font_face,
                                unsigned int height, unsigned int length );



#ifdef __cplusplus
}
#endif

#endif /* SGUI_FONT_MANAGER_H */

