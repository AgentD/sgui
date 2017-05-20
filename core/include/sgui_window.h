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
 * \brief Contains the declarations of the abstract window datatype.
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


/**
 * \enum SGUI_WINDOW_BACKEND
 *
 * \brief Constants specifiying the backend to use for a window
 */
typedef enum
{
    /**
     * \brief Use the native window system
     *
     * Native windows use the native window sytem API and have a canvas
     * implementation that renders widgets to the window.
     */
    SGUI_NATIVE = 0,

    /**
     * \brief OpenGL core profile, highest verstion available
     *
     * Windows with this backend have an asociated OpenGL rendering context.
     * The rendering context uses OpenGL core profile with the hightes GL
     * version available.
     * Windows with this backend do not have a canvas implementation. Instead,
     * they have an sgui_context implementation that abstracts access to the
     * context.
     *
     * A window with this backend generates an SGUI_EXPOSE_EVENT when the
     * window needs to be redrawn.
     */
    SGUI_OPENGL_CORE   = 1,

    /**
     * \brief OpenGL compatibillity profile, highest verstion available
     *
     * Windows with this backend have an asociated OpenGL rendering context.
     * The rendering context uses OpenGL compatibillity profile with the
     * hightes GL version available.
     * Windows with this backend do not have a canvas implementation. Instead,
     * they have an sgui_context implementation that abstracts access to the
     * context.
     *
     * A window with this backend generates an SGUI_EXPOSE_EVENT when the
     * window needs to be redrawn.
     */
    SGUI_OPENGL_COMPAT = 2,

    /**
     * \brief A window with a Direct3D 9 context
     *
     * Windows with this backend have an asociated IDirect3DDevice9 object
     * that can be used to render to the window using Direct3D 9.
     * Windows with this backend do not have a canvas implementation. Instead,
     * they have an sgui_context implementation (more precisely an
     * sgui_d3d9_context) from wich the IDirect3DDevice9 and
     * D3DPRESENT_PARAMETERS can be obtained.
     *
     * A window with this backend generates an SGUI_D3D9_DEVICE_LOST event
     * when the Direct3D device is lost. An SGUI_EXPOSE_EVENT is generated
     * when the window needs to be redrawn.
     *
     * When the window is resized, the D3DPRESENT_PARAMETERS in the context
     * are updated, but resetting the device is left as a task to the user
     * since doing so requires re-uploading all GPU buffers of the application
     * that the window has no knowledge of.
     */
    SGUI_DIRECT3D_9 = 3,

    /**
     * \brief A window with a Direct3D 11 context
     *
     * Windows with this backend have asociated sgui_d3d11_context that can
     * be used to render to the window using Direct3D 11.
     *
     * Windows with this backend do not have a canvas implementation. Instead,
     * they have an sgui_context implementation (more precisely an
     * sgui_d3d11_context) from wich the IDXGISwapChain, ID3D11Device,
     * ID3D11DeviceContext, backbuffer ID3D11RenderTargetView, depth/stencil
     * ID3D11Texture2D and ID3D11DepthStencilView can be obtained.
     *
     * The back buffer render target views and textures are automatically
     * resized and reattached if they were before resizing.
     *
     * A window with this backend generates an SGUI_EXPOSE_EVENT when the
     * window needs to be redrawn.
     */
    SGUI_DIRECT3D_11 = 4,

    /**
     * \brief A window with neither a canvas nor a context
     *
     * A window with this backend does not have a canvas or a context object
     * asociated with it. It simply abstracts a platform dependend window and
     * generates SGUI_EXPOSE_EVENT when the window needs to be redrawn.
     *
     * The function sgui_window_get_platform_data can be used to obtain a
     * platform dependend window handle and do something interesting with the
     * window.
     */
    SGUI_CUSTOM = 10
}
SGUI_WINDOW_BACKEND;

/**
 * \enum SGUI_WINDOW_FLAG
 *
 * \brief Flag values for the sgui_window_description flags field
 */
typedef enum
{
    /** \brief If set, do not allow the user to change the window size */
    SGUI_FIXED_SIZE = 0x01,

    /**
     * \brief If set, create OpenGL or Direct3D contexts with a double
     *        buffered framebuffer
     */
    SGUI_DOUBLEBUFFERED = 0x02,

    /** \brief If set, the window is initially visible */
    SGUI_VISIBLE = 0x04,

    SGUI_ALL_WINDOW_FLAGS = 0x07
}
SGUI_WINDOW_FLAG;

/**
 * \struct sgui_window
 *
 * \brief Represents a window that either has a sgui_canvas or an sgui_context
 */
struct sgui_window
{
    /** \brief Linked list pointer for window list in \ref sgui_lib */
    sgui_window *next;

    /** \brief The \ref sgui_lib implementation that this window belongs to */
    sgui_lib *lib;

    /**
     * \brief A window can have either an sgui_canvas implementation, or an
     *        sgui_context implementation, not both
     */
    union
    {
        sgui_canvas* canvas;       /**< \brief pointer to a canvas */
        sgui_context* ctx;         /**< \brief Pointer to rendering context */
    }
    ctx;

    sgui_window_callback event_fun; /**< \brief the window event callback */

    int x;          /**< \brief The distance from the left of the screen */
    int y;          /**< \brief The distance from the top of the screen */
    unsigned int w; /**< \brief The horizontal extents of the window */
    unsigned int h; /**< \brief The vertical extents of the window */

    int flags;      /**< \brief Window flags. See \ref SGUI_WINDOW_FLAG */
    int modmask;    /**< \brief Set of \ref SGUI_MODIFYER_FLAG flags */

    /**
     * \brief The backend used by the window
     *
     * \see SGUI_WINDOW_BACKEND
     */
    int backend;

    /**
     * \brief A user data pointer asociated with the window
     *
     * \see sgui_window_callback
     */
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

    /**
     * \brief Alter the currently set flags
     *
     * \param wnd   Pointer to the window itself
     * \param flags The new flags to set. A combination
     *              of \ref SGUI_WINDOW_FLAG that are to be \b inverted.
     *
     * \return Non-zero on success, zero on failure.
     */
    int (* toggle_flags )( sgui_window* wnd, int flags );

    /** Called by \ref sgui_window_set_visible */
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

    /** \copydoc sgui_window_move_center */
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
     * \brief sgui_window_force_redraw
     *
     * \param wnd Pointer to the window itself
     * \param r   Pointer to a rect to redraw (clamped to window dimesions)
     */
    void (* force_redraw )( sgui_window* wnd, sgui_rect* r );

    /**
     * \copydoc sgui_window_set_vsync
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

    /** \copydoc sgui_window_make_topmost */
    void (* make_topmost )( sgui_window* wnd );
};



/**
 * \struct sgui_window_description
 *
 * \brief Holds advanced window creation parameters
 */
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
     * \brief A pointer to a rendering context to share resources with
     *
     * If a new window is to be created with a rendering context
     * (e.g. OpenGL), this can point to an existing context to share
     * resources with.
     */
    sgui_context* share;

    /** \brief The width of the window (without borders and decoration) */
    unsigned int width;

    /** \brief The height of the window (without borders and decoration) */
    unsigned int height;

    /** \brief Misc. window flags, see \ref SGUI_WINDOW_FLAG */
    int flags;

    /**
     * \brief An \ref SGUI_WINDOW_BACKEND value specifying what backend to use
     *
     * \see SGUI_WINDOW_BACKEND
     */
    int backend;

    /**
     * \brief The desired number of bits per pixel
     *
     * Valid values are 16, 24 and 32. Invalid values are interpreted as 32.
     */
    int bits_per_pixel;

    /**
     * \brief The number of bits for the depth buffer
     *
     * Valid values are 0, 16 and 24.
     */
    int depth_bits;

    /**
     * \brief The number of bits for the stencil buffer
     *
     * Valid values are 0 and 8.
     */
    int stencil_bits;

    int samples;    /**< \brief Desired number of multisampling samples */
};

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Create a window
 *
 * \memberof sgui_window
 *
 * Creates a window using the platforms native window system. The window has
 * to be destroyed again using sgui_window_destroy( ), thus freeing up it's
 * resources.
 *
 * This function internally sets up an \ref sgui_window_description structure
 * and creates the window through the \ref sgui_lib instance.
 *
 * \note Unless the SGUI_VISIBLE flag is set, the window is created invisible
 *       and has to be made visible by calling \ref sgui_window_set_visible( ).
 *
 * \param lib    A \ref sgui_lib instance
 * \param parent A pointer to the parent window, or NULL for root window.
 *               If a window has a parent, it is not decorted by the systems
 *               window manager, positioned relative to its parent and only
 *               visible within its parent.
 * \param width  The width of the window(without borders and decoration).
 * \param height The height of the window(without borders and decoration)
 * \param flags  Misc. window flas. See \ref SGUI_WINDOW_FLAG
 *
 * \return Either a valid pointer to a window or NULL if there was an error
 */
SGUI_DLL sgui_window* sgui_window_create_simple( sgui_lib *lib,
                                                 sgui_window* parent,
                                                 unsigned int width,
                                                 unsigned int height,
                                                 int flags );

/**
 * \brief Make the rendering context for a window current
 *
 * \memberof sgui_window
 *
 * If the window was created with, for instance, an OpenGL rendering context
 * (see sgui_window_create), this function makes the context current for the
 * calling thread.
 *
 * \see sgui_context
 */
SGUI_DLL void sgui_window_make_current( sgui_window* window );

/**
 * \brief Release the rendering context for a window
 *
 * \memberof sgui_window
 *
 * If the window was created with, for instance, an OpenGL rendering context
 * (see sgui_window_create), and has been made current via
 * sgui_window_make_current( ), this function releases the context from the
 * calling thread.
 */
SGUI_DLL void sgui_window_release_current( sgui_window* window );

/**
 * \brief Swap the back and the front buffer of a window
 *
 * \memberof sgui_window
 *
 * If the window was created with, for instance, an OpenGL rendering context
 * (see sgui_window_create), this function swappes the back buffer of the
 * context with the front buffer.
 */
static SGUI_INLINE void sgui_window_swap_buffers( sgui_window* wnd )
{
    if( wnd->swap_buffers )
        wnd->swap_buffers( wnd );
}

/**
 * \brief Force synchronisation of buffer swapping with vertical rectrace
 *
 * \memberof sgui_window
 *
 * For windows with OpenGL or other rendering API backends, this can turn
 * synchronisation of buffer swapping with vertical rectrace of the monitor
 * on or off.
 *
 * \param wnd      A pointer to a window
 * \param vsync_on Non-zero to turn vsync on, zero to turn it off
 */
static SGUI_INLINE void sgui_window_set_vsync( sgui_window* wnd,
                                               int vsync_on )
{
    if( wnd->set_vsync )
        wnd->set_vsync( wnd, vsync_on );
}

/**
 * \brief Destroy a previously created window
 *
 * \memberof sgui_window
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
 * \memberof sgui_window
 *
 * \param x Returns the distance of the pointer from the left of the window
 * \param y Returns the distance of the pointer from the top of the window
 */
SGUI_DLL void sgui_window_get_mouse_position( sgui_window* wnd,
                                              int* x, int* y );

/**
 * \brief Set the mouse pointer to a position within a window
 *
 * \memberof sgui_window
 *
 * \param x          The distance of the pointer from the left of the window.
 * \param y          The distance of the pointer from the top of the window.
 * \param send_event Non-zero if the function should generate a mouse movement
 *                   event, zero if it shouldn't.
 */
SGUI_DLL void sgui_window_set_mouse_position( sgui_window* wnd, int x, int y,
                                              int send_event );

/**
 * \brief Alter the flags of a window after creation
 *
 * If the window visibility is changed from visible to invisible, a
 * \ref SGUI_API_INVISIBLE_EVENT event is generated.
 *
 * \note This function invertes flags. Calling it a second time with the exact
 *       same values undoes the previous changes.
 *
 * \note This function is not atomic, i.e. even if it fails, \b some changes
 *       may have been applied sucessfully.
 *
 * \param wnd A pointer to a window
 * \param flags A combination of \ref SGUI_WINDOW_FLAG values that are
 *              to be inverted.
 *
 * \return Non-zero on success, zero on failure.
 */
SGUI_DLL int sgui_window_toggle_flags(sgui_window *wnd, int flags);

/**
 * \brief Make a window visible or invisible
 *
 * \memberof sgui_window
 *
 * This function shows or hides a window. The window is hidden by default
 * after creation.
 *
 * If the window is set invisible, a \ref SGUI_API_INVISIBLE_EVENT is
 * generated.
 *
 * \param wnd     A pointer to the window
 * \param visible Non-zero to turn the window visible,
 *                zero to turn it invisible.
 */
static SGUI_INLINE void sgui_window_set_visible(sgui_window *wnd, int visible)
{
	visible = visible ? SGUI_VISIBLE : 0;
	if ((wnd->flags & SGUI_VISIBLE) != visible)
		sgui_window_toggle_flags(wnd, SGUI_VISIBLE);
}

/**
 * \brief Returns non-zero if a given window is visible, zero otherwise
 *
 * \memberof sgui_window
 *
 * \param wnd A pointer to the window
 *
 * \return Non-zero if the window is visible, zero if not
 */
static SGUI_INLINE int sgui_window_is_visible( const sgui_window* wnd )
{
    return (wnd->flags & SGUI_VISIBLE)!=0;
}

/**
 * \brief Change the title of a window
 *
 * \memberof sgui_window
 *
 * Most windowsystems decorate windows with borders which tend to have a
 * configurable titlebar with a text in it. This function can be used to alter
 * that title.
 *
 * \param wnd   A pointer to a window
 * \param title The new title as a NULL terminated ASCII string
 */
static SGUI_INLINE void sgui_window_set_title( sgui_window* wnd,
                                               const char* title )
{
    wnd->set_title( wnd, title );
}

/**
 * \brief Change the size of a window
 *
 * \memberof sgui_window
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
 * \memberof sgui_window
 *
 * \param width  Returns width of the window
 * \param height Returns height of the window
 */
static SGUI_INLINE void sgui_window_get_size( const sgui_window* wnd,
                                              unsigned int* width,
                                              unsigned int* height )
{
    *width  = wnd->w;
    *height = wnd->h;
}

/**
 * \brief Relocate a window to the center of the screen
 *
 * \memberof sgui_window
 */
static SGUI_INLINE void sgui_window_move_center( sgui_window* wnd )
{
    wnd->move_center( wnd );
}

/**
 * \brief Move a window to a specified position
 *
 * \memberof sgui_window
 *
 * \param wnd A pointer to a window
 * \param x   The distance of the left of the window to the left of the screen
 * \param y   The distance to the top of the window to the top of the screen
 */
SGUI_DLL void sgui_window_move( sgui_window* wnd, int x, int y );

/**
 * \brief Get the position of a window
 *
 * \memberof sgui_window
 *
 * \param wnd A pointer to a window
 * \param x   The distance of the left of the window to the left of the
 *            screen. Pass NULL if you're not interessted in it.
 * \param y   The distance to the top of the window to the top of the screen.
 *            Pass NULL if you're not interessted in it.
 */
static SGUI_INLINE void sgui_window_get_position( const sgui_window* wnd,
                                                  int* x, int* y )
{
    *x = wnd->x;
    *y = wnd->y;
}

/**
 * \brief Force redrawing of a portion of a window
 *
 * \memberof sgui_window
 *
 * \param wnd A pointer to a window
 * \param r   A pointer to a rect holding the outlines of the redraw area
 */
SGUI_DLL void sgui_window_force_redraw( sgui_window* wnd, sgui_rect* r );

/**
 * \brief Set a window's event callback
 *
 * \memberof sgui_window
 *
 * Each window may have ONE callback registered that gets called when an event
 * occours. The callback receives the pointer of the window that triggered it,
 * an event identifyer and a pointer to a structure or other datatype with
 * additional information on the event
 *
 * \param wnd A pointer to a window
 * \param fun The callback function, or NULL to unset
 */
static SGUI_INLINE void sgui_window_on_event( sgui_window* wnd,
                                              sgui_window_callback fun )
{
    wnd->event_fun = fun;
}

/**
 * \brief Store a user supplied pointer along with a window
 *
 * \memberof sgui_window
 *
 * This function can be used to conviently store a user pointer (e.g. to a C++
 * class managing the window, or a widget managing a sub window, etc...) with
 * a window to be later retrieved on demand.
 *
 * \param wnd A pointer to a window
 * \param ptr The user supplied pointer to store
 */
static SGUI_INLINE void sgui_window_set_userptr( sgui_window* wnd, void* ptr )
{
    wnd->userptr = ptr;
}

/**
 * \brief Obtain a user supplied pointer from a window
 *
 * \memberof sgui_window
 *
 * \see sgui_window_set_userptr
 *
 * \param wnd A pointer to a window
 *
 * \return The user supplied pointer stored in the window structure
 */
static SGUI_INLINE void* sgui_window_get_userptr( const sgui_window* wnd )
{
    return wnd->userptr;
}

/**
 * \brief Add a widget to a window
 *
 * \memberof sgui_window
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
 * \memberof sgui_window
 *
 * \param wnd    A pointer to a window through which to access the clipboard
 * \param text   A pointer to a text to write to the clipboard
 * \param length The number of bytes to read from the text buffer
 */
static SGUI_INLINE void sgui_window_write_clipboard( sgui_window* wnd,
                                                     const char* text,
                                                     unsigned int length )
{
    if( wnd->write_clipboard )
        wnd->write_clipboard( wnd, text, length );
}

/**
 * \brief Read a text string from the system clipboard
 *
 * \memberof sgui_window
 *
 * \param wnd A pointer to a window through which to access the clipboard
 *
 * \return A pointer to a global text buffer. DO NOT FREE THIS POINTER OR
 *         ALTER THE UNDERLYING DATA
 */
static SGUI_INLINE const char* sgui_window_read_clipboard( sgui_window* wnd )
{
    return wnd->read_clipboard ? wnd->read_clipboard( wnd ) : NULL;
}

/**
 * \brief Get a pointer to the back buffer canvas object of the window
 *
 * \memberof sgui_window
 */
static SGUI_INLINE sgui_canvas* sgui_window_get_canvas(const sgui_window* wnd)
{
    return wnd->backend==SGUI_NATIVE ? wnd->ctx.canvas : NULL;
}

/**
 * \brief Get the currently set keyboard modifiers
 *
 * \memberof sgui_window
 *
 * \return A combination of \ref SGUI_MODIFYER_FLAG
 */
static SGUI_INLINE int sgui_window_get_modifyer_mask( const sgui_window* wnd )
{
    return wnd->modmask;
}

/**
 * \brief This function returns the platform specific objects of a window
 *
 * \memberof sgui_window
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
static SGUI_INLINE void sgui_window_get_platform_data( const sgui_window* wnd,
                                                       void* window )
{
    wnd->get_platform_data( wnd, window );
}

/**
 * \brief If a window has a rendering context, get a pointer to the
 *        context object
 *
 * \memberof sgui_window
 */
static SGUI_INLINE
sgui_context* sgui_window_get_context( const sgui_window* wnd )
{
    return wnd->backend!=SGUI_NATIVE ? wnd->ctx.ctx : NULL;
}

/**
 * \brief Make sure a specific window is shown on top of all its sibblings
 *
 * \memberof sgui_window
 */
static SGUI_INLINE void sgui_window_make_topmost( sgui_window* wnd )
{
    wnd->make_topmost(wnd);
}

/**
 * \brief Adjust the size of a window to fit the widgets inside
 *
 * \memberof sgui_window
 *
 * If the window is too small for all widgets, the window size is increased so
 * all widgets are visible. If it is larger than neccessary, the window is
 * shrinked to fit around the widgets.
 */
SGUI_DLL void sgui_window_pack( sgui_window* wnd );

#ifdef __cplusplus
}
#endif

#endif /* SGUI_H */

