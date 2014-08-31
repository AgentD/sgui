/*
 * sgui_editbox.h
 * This file is part of sgui
 *
 * Copyright (C) 2012 - David Oberhollenzer
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */
#ifndef SGUI_EDIT_BOX_H
#define SGUI_EDIT_BOX_H



#include "sgui_predef.h"
#include "sgui_widget.h"



#define SGUI_EDIT_NORMAL 0
#define SGUI_EDIT_NUMERIC 1



/**
 * \struct sgui_edit_box
 *
 * \extends sgui_widget
 *
 * \brief An edit box with textual, numeric only and password versions
 *
 * \image html edit.png "Different kinds of edit boxes"
 */
typedef struct sgui_edit_box
{
    sgui_widget super;

    /* maximum number of UTF8 characters the user can enter and
       the number of UTF8 characters that have already been entered */
    unsigned int max_chars, num_entered;

    /* BYTE OFFSET of the last character in the text buffer */
    unsigned int end;

    /* BYTE OFFSET of the character after which to draw the cursor */
    unsigned int cursor;

    /* BYTE OFFSET of the cursor before selection started */
    unsigned int selection;

    /* BYTE OFFSET of the first character
       visible at the left side of the box */
    unsigned int offset;

    int selecting;      /* boolean: currently in selection mode? */
    int draw_cursor;    /* boolean: draw the cursor? */
    char* buffer;       /* text buffer */

    /**
     * \brief Insert a piece of text at the current cursor position
     *
     * \param box  A pointer to an edit box
     * \param len  The number of bytes to copy from the source string
     * \param utf8 A pointer to the source string
     *
     * \return Non-zero on success, zero on failure (invalid arguments)
     */
    int (* insert )( struct sgui_edit_box* box, unsigned int len,
                     const char* utf8 );

    /**
     * \brief Remove the currently selected text
     *
     * \param box A pointer to an edit box
     */
    void (* remove_selection )( struct sgui_edit_box* box );

    /**
     * \brief Synchronize cursors if the implementation uses a shadow buffer
     *
     * Some edit box derivatives (e.g. the sgui_pass_box) use a shadow buffer
     * containing the actual text and write something different into the
     * displayed buffer. This function synchronizes the two cursors.
     *
     * \param box A pointer to an edit box
     */
    void (* sync_cursors )( struct sgui_edit_box* box );
}
sgui_edit_box;



#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Create an edit box
 *
 * \memberof sgui_edit_box
 *
 * \param x         Distance from the left of the window.
 * \param y         Distance from the top of the window.
 * \param width     The width of the edit box.
 * \param max_chars The maximum number of characters that can be entered.
 */
SGUI_DLL sgui_widget* sgui_edit_box_create( int x, int y, unsigned int width,
                                            unsigned int max_chars );

/**
 * \brief Initialize an edit box
 *
 * \note This function is used when subclassing an editbox to initialize the
 *       base structure.
 *
 * \param eb        A pointer to the edit box base structure
 * \param x         Distance from the left of the window.
 * \param y         Distance from the top of the window.
 * \param width     The width of the edit box.
 * \param max_chars The maximum number of characters that can be entered.
 *
 * \return Non-zero on success, zero on failure
 */
SGUI_DLL int sgui_edit_box_init( sgui_edit_box* eb, int x, int y,
                                 unsigned int width, unsigned int max_chars );

/**
 * \brief Get a pointer to the text in an edit box
 *
 * \memberof sgui_edit_box
 *
 * \param box The edit box to get the text from
 *
 * \return A pointer to a null-terminated UTF8 string holding the text
 */
SGUI_DLL const char* sgui_edit_box_get_text( sgui_widget* box );

/**
 * \brief Set the text displayed in an edit box
 *
 * \memberof sgui_edit_box
 *
 * \param box  The edit box to set the text to
 * \param text The UTF8 text to display in the edit box
 */
SGUI_DLL void sgui_edit_box_set_text( sgui_widget* box, const char* text );

#ifdef __cplusplus
}
#endif

#endif /* SGUI_EDIT_BOX_H */

