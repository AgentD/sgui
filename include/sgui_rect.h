/*
 * sgui_rect.h
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
#ifndef SGUI_RECT_H
#define SGUI_RECT_H



typedef struct
{
    int left;
    int top;
    int right;
    int bottom;
}
sgui_rect;



#ifdef __cplusplus
extern "C" {
#endif



/**
 * \brief Set the coordinates of a rect using position and size
 *
 * Coordinates are given as signed intgers. The horizontal axis of the
 * coordinate system points to the left, the vertical axis points down, so
 * that the top edge coordinate of a rect is smaller than the bottom edge
 * coordinate and the left coordinate is smaller than the right.
 *
 * \param r      A pointer to the rect to set
 * \param left   The horizontal distance of the left edge from the origin
 * \param top    The vertical distance of the top edge from the origin
 * \param width  The width of the rectangle
 * \param height The height of the rectangle
 */
void sgui_rect_set_size( sgui_rect* r, int left, int top,
                         unsigned int width, unsigned int height );

/**
 * \brief Set the coordinates of rect
 *
 * Coordinates are given as signed intgers. The horizontal axis of the
 * coordinate system points to the left, the vertical axis points down, so
 * that the top edge coordinate of a rect is smaller than the bottom edge
 * coordinate and the left coordinate is smaller than the right.
 *
 * \param r      The rectangle to set
 * \param left   The horizontal distance of the left edge from the origin
 * \param top    The vertical distance of the top edge from the origin
 * \param right  The horizontal distance of the right edge from the origin
 * \param bottom The vertical distance of the bottom edge from the origin
 */
void sgui_rect_set( sgui_rect* r, int left, int top, int right, int bottom );

/**
 * \brief Copy the data of one rectangle into another
 *
 * \param dst The rectangle to copy to
 * \param src The rectangle to copy from
 */
void sgui_rect_copy( sgui_rect* dst, sgui_rect* src );

/**
 * \brief "Normalize" a rectangle, i.e. make sure the top coordinate is
 *        smaller than the bottom and the left smaller than the right
 */
void sgui_rect_normalize( sgui_rect* r );

/**
 * \brief Get the intersection between to rectangles
 *
 * This performs an intersection test between two rectangles and returns the
 * intersection. The pointer to the intersection rectangle can be safely set
 * to one of the testing rectangles as it is not touched until the
 * intersection test is done. If the intersection test fails, it is set to
 * an empty rectangle with all coordinates set to 0.
 *
 * \param r If non-NULL, returns the intersection area between the rectangles.
 * \param a The first rectangle to test. Must not be NULL.
 * \param b The second rectangle to test. Must not be NULL.
 *
 * \return Non-zero if the rectangles intersect, zero if they don't
 */
int sgui_rect_get_intersection( sgui_rect* r, sgui_rect* a, sgui_rect* b );

/**
 * \brief Clip a line segment using a rectangle
 *
 * The line segment, given as a point, direction and length in the same
 * coordinate system as the rectangle (see sgui_rect_set), is tested for
 * intersection with the rectangle. If the line segment and the rectangle
 * intersect, the given data of the line segment is set to the subsegment
 * that is inside the rectangle.
 *
 * \param r          The clipping rectangle
 * \param horizontal Non-zero if the line is horizontal, zero if it is
 *                   vertical.
 * \param x          The x coordinate of the starting point.
 * \param y          The y coordinate of the starting point.
 * \param length     The length of the line.
 *
 * \return Non-zero if the line segment intersects the rectangle, zero if not
 */
int sgui_rect_clip_line( sgui_rect* r, int horizontal, int* x, int* y,
                         unsigned int* length );



#ifdef __cplusplus
}
#endif



#endif /* SGUI_RECT_H */

