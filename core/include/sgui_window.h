/*
 * sgui_window.h
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

/**
 * \file sgui_window.h
 *
 * This file contains the interface functions for the abstract window
 * datatype.
 */
#ifndef SGUI_WINDOW_H
#define SGUI_WINDOW_H



#include "sgui_predef.h"
#include "sgui_canvas.h"



/**
 * \brief A callback for listening to window events
 *
 * \param user  The user pointer data added to the window
 * \param event Additional data to the event
 */
typedef void (* sgui_window_callback ) (void* user, const sgui_event* event);



struct sgui_window
{
    union
    {
        sgui_canvas* canvas;       /**< \brief pointer to a canvas */
        sgui_context* ctx;         /**< \brief Pointer to rendering context */
    }
    ctx;

    sgui_window_callback event_fun; /**< \brief the window event callback */

    int x, y;                   /**< \brief position of the window */
    unsigned int w, h;          /**< \brief the size of the window */

    int visible;                /**< \brief Window visibility */
    int modmask;                /**< \brief Keyboard modifyer mask */
    int backend;                /**< \brief Window backend used */

    void* userptr;

    /**
     * \brief Called by sgui_window_get_mouse_position
     *
     * \param wnd Pointer to the window itself
     * \param x   Pointer to the x return value, never NULL, adjusted to
     *            window dimensions after return
     * \param y   Pointer to the y return value, never NULL, adjusted to
     *            window dimensions after return
     */
    void (* get_mouse_position )( sgui_window* wnd, int* x, int* y );

    /**
     * \brief Called by sgui_window_set_mouse_position
     *
     * \param wnd Pointer to the window itself
     * \param x   The horizontal component of the position, already
     *            adjusted to the window size
     * \param y   The vertical component of the position, already
     *            adjusted to the window size
     */
    void (* set_mouse_position )( sgui_window* wnd, int x, int y );

    /** \copydoc Called by sgui_window_set_visible */
    void (* set_visible )( sgui_window* wnd, int visible );

    /** \copydoc Called by sgui_window_set_visible */
    void (* set_title )( sgui_window* wnd, const char* title );

    /**
     * \brief Called by sgui_window_set_size
     *
     * The sgui_window_set_size function takes care of also resizing the
     * canvas and redrawing the widget, if the backend is SGUI_NATIVE.
     *
     * \param wnd    Pointer to the window itself
     * \param width  New width of the window
     * \param height New height of the window
     */
    void (* set_size )( sgui_window* wnd,
                        unsigned int width, unsigned int height );

    /** \copydoc Called by sgui_window_move_center */
    void (* move_center )( sgui_window* wnd );

    /** \copydoc sgui_window_move */
    void (* move )( sgui_window* wnd, int x, int y );

    /**
     * \copydoc sgui_window_swap_buffers
     *
     * \note Can be set to NULL if not needed by the implementation.
     */
    void (* swap_buffers )( sgui_window* wnd );

    /** \copydoc sgui_window_destroy */
    void (* destroy )( sgui_window* wnd );

    /**
     * \brief Called by sgui_window_force_redraw
     *
     * \param wnd Pointer to the window itself
     * \param r   Pointer to a rect to redraw (clamped to window dimesions)
     */
    void (* force_redraw )( sgui_window* wnd, sgui_rect* r );

    /**
     * \copydoc Called by sgui_window_set_vsync.
     *
     * \note May be NULL if not implemented
     */
    void (* set_vsync )( sgui_window* wnd, int interval );

    /** \copydoc sgui_window_get_platform_data */
    void (* get_platform_data )( const sgui_window* wnd, void* ptr );

    /**
     * \copydoc sgui_window_write_clipboard
     *
     * \note May be NULL if not implemented
     */
    void (* write_clipboard )( sgui_window* wnd, const char* text,
                               unsigned int length );

    /**
     * \copydoc sgui_window_read_clipboard
     *
     * \note May be NULL if not implemented
     */
    const char* (* read_clipboard )( sgui_window* wnd );
};



struct sgui_window_description
{
    /**
     * \brief A pointer to the parent window, or NULL for root window
     *
     * If a window has a parent, it is not decorted by the systems window
     * manager, positioned relative to its parent and only visible within
     * its parent.
     */
    sgui_window* parent;

    /**
     * \brief A pointer to a window with a context to share resources with
     *
     * If a new window is to be created with a rendering context
     * (e.g. OpenGL), this can point to an existing window with a context to
     * share resources with.
     */
    sgui_window* share;

    /** \brief The width of the window (without borders and decoration) */
    unsigned int width;

    /** \brief The height of the window (without borders and decoration) */
    unsigned int height;

    /**
     * \brief Non-zero if the window should be resizeable by the user,
     *        zero if the should remain at a fixed size.
     *
     * The symbolic constants SGUI_RESIZEABLE and SGUI_FIXED_SIZE can be used
     * to generate more readable code.
     */
    int resizeable;

    /**
     * \brief What back end to use
     *
     * 0 or SGUI_NATIVE for native window system back end, SGUI_OPENGL_CORE to
     * create a core profile OpenGL context for the window, SGUI_OPENGL_COMPAT
     * for compatibillity profile. OpenGL contexts are created for the highest
     * version available on the current system.
     */
    int backend;

    /**
     * \brief Non-zero if the window should use double buffering
     *
     * The symbolic constants SGUI_SINGLEBUFFERED and SGUI_DOUBLEBUFFERED can
     * be used to generate more readable code.
     */
    int doublebuffer;

    int bits_per_pixel; /**< \brief The desired number of bits per pixel */
    int depth_bits;   /**< \brief The number of bits for the depth buffer */
    int stencil_bits; /**< \brief The number of bits for the stencil buffer */
    int samples;    /**< \brief Desired number of multisampling samples */
};



#define SGUI_RESIZEABLE 1
#define SGUI_FIXED_SIZE 0

#define SGUI_DOUBLEBUFFERED 1
#define SGUI_SINGLEBUFFERED 0

#define SGUI_VISIBLE   1
#define SGUI_INVISIBLE 0

#define SGUI_NATIVE        0
#define SGUI_OPENGL_CORE   1
#define SGUI_OPENGL_COMPAT 2
#define SGUI_DIRECT3D_9    3
#define SGUI_DIRECT3D_11   4
#define SGUI_CUSTOM        10



#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Create a window
 *
 * Creates a window using the platforms native window system. The window has
 * to be destroyed again using sgui_window_destroy( ), thus freeing up it's
 * resources.
 *
 * This function internally sets up an sgui_window_description structure and
 * calls sgui_window_create_desc( ).
 *
 * \note The window is created invisible and has to be made visible by calling
 *       sgui_window_set_visible( ).
 *
 * \param parent     A pointer to the parent window, or NULL for root window.
 *                   If a window has a parent, it is not decorted by the
 *                   systems window manager, positioned relative to its parent
 *                   and only visible within its parent.
 * \param width      The width of the window(without borders and decoration).
 * \param height     The height of the window(without borders and decoration)
 * \param resizeable Non-zero if the window should be resizeable by the user,
 *                   zero if the should remain at a fixed size. The symbolic
 *                   constants SGUI_RESIZEABLE and SGUI_FIXED_SIZE can be used
 *                   to generate more readable code.
 *
 * \return Either a valid pointer to a window or NULL if there was an error
 */
SGUI_DLL sgui_window* sgui_window_create( sgui_window* parent,
                                          unsigned int width,
                                          unsigned int height,
                                          int resizeable );

/**
 * \brief Create a window using a pointer to a description structure
 *
 * Creates a window using the platforms native window system. The window has
 * to be destroyed again using sgui_window_destroy( ), thus freeing up it's
 * resources.
 *
 * \note The window is created invisible and has to be made visible by calling
 *       sgui_window_set_visible( ).
 *
 * \param desc A pointer to a structure holding a description of the window
 *             that is to be created.
 *
 * \return Either a valid pointer to a window or NULL if there was an error
 */
SGUI_DLL sgui_window* sgui_window_create_desc(
                                        const sgui_window_description* desc
                                             );

/**
 * \brief Make the rendering context for a window current
 *
 * If the window was created with, for instance, an OpenGL rendering context
 * (see sgui_window_create), this function makes the context current for the
 * calling thread.
 *
 * \see sgui_context_make_current
 */
SGUI_DLL void sgui_window_make_current( sgui_window* window );

/**
 * \brief Release the rendering context for a window
 *
 * If the window was created with, for instance, an OpenGL rendering context
 * (see sgui_window_create), and has been made current via
 * sgui_window_make_current( ) or sgui_context_make_current( ), this function
 * releases the context from the calling thread.
 *
 * \see sgui_context_release_current
 */
SGUI_DLL void sgui_window_release_current( sgui_window* window );

/**
 * \brief Swap the back and the front buffer of a window
 *
 * If the window was created with, for instance, an OpenGL rendering context
 * (see sgui_window_create), this function swappes the back buffer of the
 * context with the front buffer.
 */
SGUI_DLL void sgui_window_swap_buffers( sgui_window* window );

/**
 * \brief Force synchronisation of buffer swapping with vertical rectrace
 *
 * For windows with OpenGL or other rendering API backends, this can turn
 * synchronisation of buffer swapping with vertical rectrace of the monitor
 * on or off.
 *
 * \param window   A pointer to a window
 * \param vsync_on Non-zero to turn vsync on, zero to turn it off
 */
SGUI_DLL void sgui_window_set_vsync( sgui_window* window, int vsync_on );

/**
 * \brief Destroy a previously created window
 *
 * The window is closed and all it's resources are freed up, so the window
 * pointer itself is nolonger valid after a call to this function.
 *
 * This function triggers the window close event with SGUI_API_DESTROYED as
 * how paramter, no matter whether the window is visible or not.
 */
SGUI_DLL void sgui_window_destroy( sgui_window* wnd );



/**
 * \brief Get the position of the mouse pointer within a window
 *
 * \param x Returns the distance of the pointer from the left of the window
 * \param y Returns the distance of the pointer from the top of the window
 */
SGUI_DLL void sgui_window_get_mouse_position( sgui_window* wnd,
                                              int* x, int* y );

/**
 * \brief Set the mouse pointer to a position within a window
 *
 * \param x          The distance of the pointer from the left of the window.
 * \param y          The distance of the pointer from the top of the window.
 * \param send_event Non-zero if the function should generate a mouse movement
 *                   event, zero if it shouldn't.
 */
SGUI_DLL void sgui_window_set_mouse_position( sgui_window* wnd, int x, int y,
                                              int send_event );



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
SGUI_DLL void sgui_window_set_visible( sgui_window* wnd, int visible );

/** \brief Returns non-zero if a given window is visible, zero otherwise */
SGUI_DLL int sgui_window_is_visible( const sgui_window* wnd );

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
SGUI_DLL void sgui_window_set_title( sgui_window* wnd, const char* title );

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
SGUI_DLL void sgui_window_set_size( sgui_window* wnd,
                                    unsigned int width, unsigned int height );

/**
 * \brief Get the size of a window
 *
 * \param width  Returns width of the window. Pass NULL if you're not
 *               iteressted in it.
 * \param height Returns height of the window. Pass NULL if you're not
 *               iteressted in it.
 */
SGUI_DLL void sgui_window_get_size( const sgui_window* wnd,
                                    unsigned int* width,
                                    unsigned int* height );

/** \brief Relocate a window to the center of the screen */
SGUI_DLL void sgui_window_move_center( sgui_window* wnd );

/**
 * \brief Move a window to a specified position
 *
 * \param wnd A pointer to a window
 * \param x   The distance of the left of the window to the left of the screen
 * \param y   The distance to the top of the window to the top of the screen
 */
SGUI_DLL void sgui_window_move( sgui_window* wnd, int x, int y );

/**
 * \brief Get the position of a window
 *
 * \param wnd A pointer to a window
 * \param x   The distance of the left of the window to the left of the
 *            screen. Pass NULL if you're not interessted in it.
 * \param y   The distance to the top of the window to the top of the screen.
 *            Pass NULL if you're not interessted in it.
 */
SGUI_DLL void sgui_window_get_position( const sgui_window* wnd,
                                        int* x, int* y );

/**
 * \brief Force redrawing of a portion of a window
 *
 * \param wnd A pointer to a window
 * \param r   A pointer to a rect holding the outlines of the redraw area
 */
SGUI_DLL void sgui_window_force_redraw( sgui_window* wnd, sgui_rect* r );

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
SGUI_DLL void sgui_window_on_event( sgui_window* wnd,
                                    sgui_window_callback fun );

/**
 * \brief Store a user supplied pointer along with a window
 *
 * This function can be used to conviently store a user pointer (e.g. to a C++
 * class managing the window, or a widget managing a sub window, etc...) with
 * a window to be later retrieved on demand.
 *
 * \param wnd A pointer to a window
 * \param ptr The user supplied pointer to store
 */
SGUI_DLL void sgui_window_set_userptr( sgui_window* wnd, void* ptr );

/**
 * \brief Obtain a user supplied pointer from a window
 *
 * \see sgui_window_set_userptr
 *
 * \param wnd A pointer to a window
 *
 * \return The user supplied pointer stored in the window structure
 */
SGUI_DLL void* sgui_window_get_userptr( const sgui_window* wnd );

/**
 * \brief Add a widget to a window
 *
 * Ownership remains with the caller.
 *
 * \param wnd    The window to add the widget to
 * \param widget The widget to add
 */
SGUI_DLL void sgui_window_add_widget( sgui_window* wnd, sgui_widget* widget );

/**
 * \brief Write a fraction of text to the system clipboard
 *
 * \param wnd    A pointer to a window through which to access the clipboard
 * \param text   A pointer to a text to write to the clipboard
 * \param length The number of bytes to read from the text buffer
 */
SGUI_DLL void sgui_window_write_clipboard( sgui_window* wnd,
                                           const char* text,
                                           unsigned int length );

/**
 * \brief Read a text string from the system clipboard
 *
 * \param wnd A pointer to a window through which to access the clipboard
 *
 * \return A pointer to a global text buffer. DO NOT FREE THIS POINTER OR
 *         ALTER THE UNDERLYING DATA
 */
SGUI_DLL const char* sgui_window_read_clipboard( sgui_window* wnd );

/** \brief Get a pointer to the back buffer canvas object of the window */
SGUI_DLL sgui_canvas* sgui_window_get_canvas( const sgui_window* wnd );

/** \brief Get a combination of SGUI_MOD_ flags for the current modifiers */
SGUI_DLL int sgui_window_get_modifyer_mask( const sgui_window* wnd );

/**
 * \brief This function returns the platform specific objects of a window
 *
 * Using this function, it is possible to perform some advanced techniques
 * with a window, like creating an OpenGL context that shares resources, or
 * letting 3rd party libraries work with sgui (sub)windows. (For instance,
 * irrlicht supports using an arbitrary window handle).
 *
 * \param window  On MS Windows, this is asumed to be a pointer to a HWND
 *                and receives the value of the windows HWND. When using the
 *                Xlib backend, this is assumed to be a pointer to a "Window".
 */
SGUI_DLL void sgui_window_get_platform_data( const sgui_window* wnd,
                                             void* window );

/**
 * \brief If a window has a rendering context, get a pointer to the
 *        context object
 */
SGUI_DLL sgui_context* sgui_window_get_context( const sgui_window* wnd );

#ifdef __cplusplus
}
#endif

#endif /* SGUI_H */

