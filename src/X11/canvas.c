/*
 * canvas.c
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
#include "internal.h"

#include <ft2build.h>
#include FT_FREETYPE_H



struct sgui_font
{
    FT_Library freetype;
    FT_Face face;
    void* buffer;
};



#define IS_OUTSIDE_SCISSOR_RECT( x, y, w, h, canvas ) \
           ( (x)>(canvas)->sex || (y)>(canvas)->sey ||\
            ((x)+(int)(w))<(canvas)->sx || ((y)+(int)(h))<(canvas)->sy )

#define NEED_CHANNEL_SWAP( f ) ((f)==SCF_RGBA8 || (f)==SCF_RGB8)

#define HAS_ALPHA_CHANNEL( f ) ((f)==SCF_RGBA8 || (f)==SCF_BGRA8)

#define COLOR_COPY( a, b ) (a)[0]=(b)[0]; (a)[1]=(b)[1]; (a)[2]=(b)[2]
#define COLOR_COPY_INV( a, b ) (a)[0]=(b)[2]; (a)[1]=(b)[1]; (a)[2]=(b)[0]

#define COLOR_BLEND( a, b, A, iA )\
        (a)[0] = ((a)[0]*iA + (b)[0]*A)>>8;\
        (a)[1] = ((a)[1]*iA + (b)[1]*A)>>8;\
        (a)[2] = ((a)[2]*iA + (b)[2]*A)>>8;

#define COLOR_BLEND_INV( a, b, A, iA )\
        (a)[0] = ((a)[0]*iA + (b)[2]*A)>>8;\
        (a)[1] = ((a)[1]*iA + (b)[1]*A)>>8;\
        (a)[2] = ((a)[2]*iA + (b)[0]*A)>>8;

/***************************** Helper functions *****************************/
int utf8_char_length( unsigned char c )
{
    c >>= 3;

    if( c == 0x1E )
        return 4;

    c >>= 1;

    if( c == 0x0E )
        return 3;

    c >>= 1;

    return (c==0x06) ? 2 : 1;
}

unsigned long to_utf32( const unsigned char* utf8, int* length )
{
    unsigned long ch;
    int i;

    *length = utf8_char_length( *utf8 );

    switch( *length )
    {
    case 4: ch = (*utf8 ^ 0xf0); break;
    case 3: ch = (*utf8 ^ 0xe0); break;
    case 2: ch = (*utf8 ^ 0xc0); break;
    case 1: ch =  *utf8;         break;
    }

    ++utf8;

    for( i=*length; i>1; --i, ++utf8 )
    {
        ch <<= 6;
        ch |= (*utf8 ^ 0x80);
    }

    return ch;
}

int blend_glyph_on_canvas( sgui_canvas* canvas, unsigned char* glyph,
                           int x, int y, unsigned int w, unsigned int h,
                           unsigned char color[3] )
{
    unsigned char A, iA, *src, *dst, *row;
    unsigned int i, j, ds, dt, delta;

    /* adjust parameters to only blend visible portion */
    dst = ((unsigned char*)canvas->data) + (y*canvas->width + x)*4;

    if( y<canvas->sy )
    {
        delta = canvas->sy-y;

        dst += delta*canvas->width*4;
        glyph += delta*w;
        h -= delta;
        y = canvas->sy;
    }

    if( (y+(int)h) > canvas->sey )
        h = canvas->sey - y;

    ds = w;
    dt = canvas->width*4;

    if( x < canvas->sx )
    {
        delta = canvas->sx - x;

        w -= delta;
        glyph += delta;
        x = canvas->sx;
    }

    if( (x+(int)w) >= canvas->sex )
        w = canvas->sex - x;

    /* do the blend */
    for( j=0; j<h; ++j, glyph+=ds, dst+=dt )
    {
        for( src=glyph, row=dst, i=0; i<w; ++i, row+=4, ++src )
        {
            A = *src;
            iA = 255-A;

            COLOR_BLEND( row, color, A, iA );
        }
    }

    return 0;
}

/****************************** Font functions ******************************/
sgui_font* sgui_font_load_from_file( const char* filename )
{
    sgui_font* font;

    /* allocate font structure */
    font = malloc( sizeof(sgui_font) );

    if( !font )
        return NULL;

    /* load font */
    if( FT_Init_FreeType( &font->freetype ) )
    {
        free( font );
        return NULL;
    }

    if( FT_New_Face( font->freetype, filename, 0, &font->face ) )
    {
        FT_Done_FreeType( font->freetype );
        free( font );
        return NULL;
    }

    font->buffer = NULL;

    return font;
}

sgui_font* sgui_font_load_from_mem( void* buffer, unsigned int buffersize )
{
    sgui_font* font;

    /* sanity check */
    if( !buffer || !buffersize )
        return NULL;

    /* allocate font structure */
    font = malloc( sizeof(sgui_font) );

    if( !font )
        return NULL;

    /* load font */
    if( FT_Init_FreeType( &font->freetype ) )
    {
        free( font );
        return NULL;
    }

    if(FT_New_Memory_Face(font->freetype, buffer, buffersize, 0, &font->face))
    {
        FT_Done_FreeType( font->freetype );
        free( font );
        free( buffer );
        return NULL;
    }

    font->buffer = buffer;

    return font;
}

void sgui_font_destroy( sgui_font* font )
{
    if( font )
    {
        FT_Done_Face( font->face );
        FT_Done_FreeType( font->freetype );

        free( font->buffer );
        free( font );
    }
}

unsigned int sgui_font_get_text_extents_plain( sgui_font* font_face,
                                               unsigned int font_height,
                                               const char* text,
                                               unsigned int length )
{
    unsigned int x = 0;
    unsigned long character;
    int len = 0;
    FT_UInt glyph_index = 0;
    FT_UInt previous = 0;
    FT_Bool useKerning;
    unsigned int i;

    /* sanity check */
    if( !text || !font_face || !font_height || !length )
        return 0;

    /* set rendering pixel size */
    FT_Set_Pixel_Sizes( font_face->face, 0, font_height );

    useKerning = FT_HAS_KERNING( font_face->face );

    for( i=0; i<length && (*text) && (*text!='\n'); text+=len, i+=len )
    {
        if( *text == ' ' )
        {
            x += (font_height/3);
            len = 1;
            continue;
        }

        /* UTF8 -> UTF32 -> glyph index */
        character = to_utf32( (const unsigned char*)text, &len );
        glyph_index = FT_Get_Char_Index( font_face->face, character );

        /* load and render */
        FT_Load_Glyph( font_face->face, glyph_index, FT_LOAD_DEFAULT );
        FT_Render_Glyph( font_face->face->glyph, FT_RENDER_MODE_NORMAL );

        /* apply kerning */
        if( useKerning && previous && glyph_index )
        {
            FT_Vector delta;
            FT_Get_Kerning( font_face->face, previous, glyph_index,
                            FT_KERNING_DEFAULT, &delta );
            x -= (abs( delta.x ) >> 6);
        }

        /* advance */
        x += font_face->face->glyph->bitmap.width + 1;

        previous = glyph_index;
    }

    return x;
}

/***************************** Canvas functions *****************************/
sgui_canvas* sgui_canvas_create( unsigned int width, unsigned int height,
                                 Display* dpy )
{
    sgui_canvas* cv = malloc( sizeof(sgui_canvas) );

    if( !cv )
        return NULL;

    memset( cv, 0, sizeof(sgui_canvas) );

    cv->data = malloc( width * height * 4 );

    if( !cv->data )
    {
        free( cv );
        return NULL;
    }

    cv->img = XCreateImage( dpy, CopyFromParent, 24, ZPixmap, 0,
                            cv->data, width, height, 32, 0 );

    if( !cv->img )
    {
        free( cv->data );
        free( cv );
        return NULL;
    }

    cv->width  = width;
    cv->height = height;
    cv->clear  = 1;

    cv->sex    = width  ? (width -1) : 0;
    cv->sey    = height ? (height-1) : 0;

    return cv;
}

void sgui_canvas_destroy( sgui_canvas* canvas )
{
    if( canvas )
    {
        if( canvas->img )
        {
            canvas->img->data = NULL;
            XDestroyImage( canvas->img );
        }

        free( canvas->data );
        free( canvas );
    }
}

void sgui_canvas_resize( sgui_canvas* canvas, unsigned int width,
                         unsigned int height, Display* dpy )
{
    unsigned int new_mem, old_mem;

    if( !canvas || (canvas->width==width && height==canvas->height) )
        return;

    new_mem = width*height;
    old_mem = canvas->width*canvas->height;

    if( new_mem != old_mem )
    {
        free( canvas->data );
        canvas->data = malloc( new_mem * 4 );
    }

    canvas->img->data = NULL;
    XDestroyImage( canvas->img );

    canvas->img = XCreateImage( dpy, CopyFromParent, 24, ZPixmap, 0,
                                (char*)canvas->data, width, height, 32, 0 );

    canvas->width  = width;
    canvas->height = height;

    canvas->sx     = 0;
    canvas->sy     = 0;
    canvas->sex    = width  ? (width -1) : 0;
    canvas->sey    = height ? (height-1) : 0;
}




void sgui_canvas_get_size( sgui_canvas* canvas, unsigned int* width,
                           unsigned int* height )
{
    if( width  ) *width  = canvas ? canvas->width  : 0;
    if( height ) *height = canvas ? canvas->height : 0;
}




void sgui_canvas_set_background_color( sgui_canvas* canvas,
                                       unsigned char* color,
                                       SGUI_COLOR_FORMAT format )
{
    if( canvas && color )
    {
        if( NEED_CHANNEL_SWAP( format ) )
        {
            COLOR_COPY_INV( canvas->bg_color, color );
        }
        else
        {
            COLOR_COPY( canvas->bg_color, color );
        }
    }
}

void sgui_canvas_clear( sgui_canvas* canvas, int x, int y,
                        unsigned int width, unsigned int height )
{
    unsigned int i, j;
    unsigned char *dst, *row;

    if( !canvas || !canvas->clear )
        return;

    x += canvas->ox;
    y += canvas->oy;

    if( IS_OUTSIDE_SCISSOR_RECT( x, y, width, height, canvas ) )
        return;

    /* adjust parameters to only draw visible portion */
    if( y<canvas->sy ) { height -= canvas->sy-y; y = canvas->sy; }
    if( x<canvas->sx ) { width  -= canvas->sx-x; x = canvas->sx; }

    if( (y+((int)height-1)) > canvas->sey )
        height = canvas->sey - y;

    if( (x+((int)width-1)) > canvas->sex )
        width = canvas->sex - x;

    dst = (unsigned char*)canvas->data + (y*canvas->width+x)*4;

    /* clear */
    for( j=0; j<height; ++j, dst+=canvas->width*4 )
    {
        for( row=dst, i=0; i<width; ++i, row+=4 )
        {
            COLOR_COPY( row, canvas->bg_color );
        }
    }
}



void sgui_canvas_set_scissor_rect( sgui_canvas* canvas, int x, int y,
                                   unsigned int width, unsigned int height )
{
    int sx, sy, sex, sey;

    if( canvas )
    {
        x += canvas->ox;
        y += canvas->oy;

        if( width && height )
        {
            if( canvas->scissor_stack_pointer == SGUI_CANVAS_STACK_DEPTH )
                return;

            /* push current scissor rect */
            canvas->sc_stack_x [canvas->scissor_stack_pointer] = canvas->sx;
            canvas->sc_stack_y [canvas->scissor_stack_pointer] = canvas->sy;
            canvas->sc_stack_ex[canvas->scissor_stack_pointer] = canvas->sex;
            canvas->sc_stack_ey[canvas->scissor_stack_pointer] = canvas->sey;

            ++(canvas->scissor_stack_pointer);

            /* merge rectangles */
            sx = x;
            sy = y;
            sex = x + width - 1;
            sey = y + height - 1;

            if( sx<canvas->sx     ) x   = canvas->sx;
            if( sy<canvas->sy     ) y   = canvas->sy;
            if( sex > canvas->sex ) sex = canvas->sex;
            if( sey > canvas->sey ) sey = canvas->sey;

            /* set scissor rect */
            canvas->sx  = sx;
            canvas->sy  = sy;
            canvas->sex = sex;
            canvas->sey = sey;

            /* sanity adjustments */
            if( canvas->sx < 0 ) canvas->sx = 0;
            if( canvas->sy < 0 ) canvas->sy = 0;

            if( canvas->sex>=(int)canvas->width )
                canvas->sex = canvas->width ? (canvas->width -1) : 0;

            if( canvas->sey>=(int)canvas->height )
                canvas->sey = canvas->height ? (canvas->height-1) : 0;

            if( canvas->sx > canvas->sex ) canvas->sex = canvas->sx;
            if( canvas->sy > canvas->sey ) canvas->sey = canvas->sy;
        }
        else
        {
            if( canvas->scissor_stack_pointer )
            {
                /* pop old scissor rect from stack */
                --(canvas->scissor_stack_pointer);

                sx  = canvas->sc_stack_x [ canvas->scissor_stack_pointer ];
                sy  = canvas->sc_stack_y [ canvas->scissor_stack_pointer ];
                sex = canvas->sc_stack_ex[ canvas->scissor_stack_pointer ];
                sey = canvas->sc_stack_ey[ canvas->scissor_stack_pointer ];

                canvas->sx  = sx;
                canvas->sy  = sy;
                canvas->sex = sex;
                canvas->sey = sey;
            }
            else
            {
                canvas->sx  = 0;
                canvas->sy  = 0;
                canvas->sex = canvas->width  ? (canvas->width -1) : 0;
                canvas->sey = canvas->height ? (canvas->height-1) : 0;
            }
        }
    }
}

void sgui_canvas_allow_clear( sgui_canvas* canvas, int clear )
{
    if( canvas )
        canvas->clear = clear;
}

int sgui_canvas_is_clear_allowed( sgui_canvas* canvas )
{
    return canvas ? canvas->clear : 0;
}

void sgui_canvas_set_offset( sgui_canvas* canvas, int x, int y )
{
    if( canvas )
    {
        if( canvas->offset_stack_pointer == SGUI_CANVAS_STACK_DEPTH )
            return;

        canvas->offset_stack_x[ canvas->offset_stack_pointer ] = canvas->ox;
        canvas->offset_stack_y[ canvas->offset_stack_pointer ] = canvas->oy;

        canvas->ox += x;
        canvas->oy += y;

        ++(canvas->offset_stack_pointer);
    }
}

void sgui_canvas_restore_offset( sgui_canvas* canvas )
{
    if( canvas )
    {
        if( canvas->offset_stack_pointer )
        {
            --(canvas->offset_stack_pointer);

            canvas->ox = canvas->offset_stack_x[canvas->offset_stack_pointer];
            canvas->oy = canvas->offset_stack_y[canvas->offset_stack_pointer];
        }
        else
        {
            canvas->ox = 0;
            canvas->oy = 0;
        }
    }
}

void sgui_canvas_blit( sgui_canvas* canvas, int x, int y, unsigned int width,
                       unsigned int height, SGUI_COLOR_FORMAT format,
                       const void* data )
{
    unsigned char *dst, *src, *drow, *srow;
    unsigned int i, j, src_bpp = 3;
    unsigned int ds, dt;

    if( !canvas || !width || !height || !data )
        return;

    x += canvas->ox;
    y += canvas->oy;

    /* don't blit if outside the drawing area */
    if( IS_OUTSIDE_SCISSOR_RECT( x, y, width, height, canvas ) )
        return;

    /* color format checks */
    if( HAS_ALPHA_CHANNEL( format ) )
        src_bpp = 4;

    /* compute source and destination pointers */
    dst = ((unsigned char*)canvas->data) + (y*canvas->width + x)*4;
    src = (unsigned char*)data;

    /* adjust parameters to only blit visible portion */
    if( y<canvas->sy )
    {
        dst += (canvas->sy-y)*canvas->width*4;
        src += (canvas->sy-y)*width*src_bpp;
        y = canvas->sy;
    }

    if( (y+((int)height-1)) > canvas->sey )
        height = canvas->sey - y;

    ds = width*src_bpp;
    dt = canvas->width*4;

    if( x < canvas->sx )
    {
        width -= canvas->sx - x;
        src += (canvas->sx - x)*src_bpp;
        x = canvas->sx;
    }

    if( (x+((int)width-1)) > canvas->sex )
        width = canvas->sex - x;

    /* do the blit */
    if( NEED_CHANNEL_SWAP( format ) )
    {
        for( j=0; j<height; ++j, src+=ds, dst+=dt )
        {
            for( drow=dst, srow=src, i=0; i<width; ++i, drow+=4,
                                                        srow+=src_bpp )
            {
                COLOR_COPY_INV( drow, srow );
            }
        }
    }
    else
    {
        for( j=0; j<height; ++j, src+=ds, dst+=dt )
        {
            for( drow=dst, srow=src, i=0; i<width; ++i, drow+=4,
                                                        srow+=src_bpp )
            {
                COLOR_COPY( drow, srow );
            }
        }
    }
}

void sgui_canvas_blend( sgui_canvas* canvas, int x, int y, unsigned int width,
                        unsigned int height, SGUI_COLOR_FORMAT format,
                        const void* data )
{
    unsigned char *dst, *src, *drow, *srow, A, iA;
    unsigned int i, j, ds, dt;

    if( !canvas || !width || !height || !data )
        return;

    if( !HAS_ALPHA_CHANNEL( format ) )
        return;

    x += canvas->ox;
    y += canvas->oy;

    /* don't blend outside the drawing area */
    if( IS_OUTSIDE_SCISSOR_RECT( x, y, width, height, canvas ) )
        return;

    /* compute source and destination pointers */
    dst = ((unsigned char*)canvas->data) + (y*canvas->width + x)*4;
    src = (unsigned char*)data;

    /* adjust parameters to only blit visible portion */
    if( y<canvas->sy )
    {
        dst += (canvas->sy-y)*canvas->width*4;
        src += (canvas->sy-y)*width*4;
        y = canvas->sy;
    }

    if( (y+((int)height-1)) > canvas->sey )
        height = canvas->sey - y;

    ds = width*4;
    dt = canvas->width*4;

    if( x < canvas->sx )
    {
        width -= canvas->sx - x;
        src += (canvas->sx - x)*4;
        x = canvas->sx;
    }

    if( (x+((int)width-1)) > canvas->sex )
        width = canvas->sex - x;

    /* do the blend */
    if( NEED_CHANNEL_SWAP( format ) )
    {
        for( j=0; j<height; ++j, src+=ds, dst+=dt )
        {
            for( drow=dst, srow=src, i=0; i<width; ++i, drow+=4, srow+=4 )
            {
                A = srow[3], iA = 255-A;

                COLOR_BLEND_INV( drow, srow, A, iA );
            }
        }
    }
    else
    {
        for( j=0; j<height; ++j, src+=ds, dst+=dt )
        {
            for( drow=dst, srow=src, i=0; i<width; ++i, drow+=4, srow+=4 )
            {
                A = srow[3], iA = 255-A;

                COLOR_BLEND( drow, srow, A, iA );
            }
        }
    }
}

void sgui_canvas_draw_box( sgui_canvas* canvas, int x, int y,
                           unsigned int width, unsigned int height,
                           unsigned char* color, SGUI_COLOR_FORMAT format )
{
    unsigned char c[3], A, iA;
    unsigned int i, j;
    unsigned char *dst, *row;

    if( !canvas || !color )
        return;

    x += canvas->ox;
    y += canvas->oy;

    /* don't draw outside the drawing area */
    if( IS_OUTSIDE_SCISSOR_RECT( x, y, width, height, canvas ) )
        return;

    /* color format checks */
    if( NEED_CHANNEL_SWAP( format ) )
    {
        COLOR_COPY_INV( c, color );
    }
    else
    {
        COLOR_COPY( c, color );
    }

    /* adjust parameters to only draw visible portion */
    if( y<canvas->sy ) { height -= canvas->sy-y; y = canvas->sy; }
    if( x<canvas->sx ) { width  -= canvas->sx-x; x = canvas->sx; }

    if( (y+((int)height-1)) > canvas->sey )
        height = canvas->sey - y;

    if( (x+((int)width-1)) > canvas->sex )
        width = canvas->sex - x;

    dst = (unsigned char*)canvas->data + (y*canvas->width+x)*4;

    if( HAS_ALPHA_CHANNEL( format ) )
    {
        A = color[3];
        iA = 255 - A;

        for( j=0; j<height; ++j, dst+=canvas->width*4 )
        {
            for( row=dst, i=0; i<width; ++i, row+=4 )
            {
                COLOR_BLEND( row, c, A, iA );
            }
        }
    }
    else
    {
        for( j=0; j<height; ++j, dst+=canvas->width*4 )
        {
            for( row=dst, i=0; i<width; ++i, row+=4 )
            {
                COLOR_COPY( row, c );
            }
        }
    }
}

void sgui_canvas_draw_line( sgui_canvas* canvas, int x, int y,
                            unsigned int length, int horizontal,
                            unsigned char* color, SGUI_COLOR_FORMAT format )
{
    unsigned char* dst;
    unsigned char c[3], A, iA;
    unsigned int i, delta;

    if( canvas )
    {
        x += canvas->ox;
        y += canvas->oy;

        if( NEED_CHANNEL_SWAP( format ) )
        {
            COLOR_COPY_INV( c, color );
        }
        else
        {
            COLOR_COPY( c, color );
        }

        if( horizontal )
        {
            if( y<canvas->sy || y>canvas->sey )
                return;

            if( (x+(int)length)<canvas->sx || x>canvas->sex )
                return;

            if( x<canvas->sx )
            {
                length -= canvas->sx - x;
                x = canvas->sx;
            }

            if( (x+(int)length) > canvas->sex )
                length = canvas->sex - x;

            delta = 4;
        }
        else
        {
            if( x<canvas->sx || x>canvas->sex )
                return;

            if( (y+(int)length)<canvas->sy || y>canvas->sey )
                return;

            if( y<canvas->sy )
            {
                length -= canvas->sy - y;
                y = canvas->sy;
            }

            if( (y+(int)length) > canvas->sey )
                length = canvas->sey - y;

            delta = canvas->width*4;
        }

        dst = (unsigned char*)canvas->data +
              (y*canvas->width+x)*4;

        if( HAS_ALPHA_CHANNEL( format ) )
        {
            A = color[3];
            iA = 255 - A;

            for( i=0; i<length; ++i, dst+=delta )
            {
                COLOR_BLEND( dst, c, A, iA );
            }
        }
        else
        {
            for( i=0; i<length; ++i, dst+=delta )
            {
                COLOR_COPY( dst, c );
            }
        }
    }
}

void sgui_canvas_draw_text_plain( sgui_canvas* canvas, int x, int y,
                                  sgui_font* font_face,
                                  unsigned int font_height,
                                  unsigned char* color,
                                  SGUI_COLOR_FORMAT format,
                                  const char* text, unsigned int length )
{
    FT_UInt glyph_index = 0;
    FT_UInt previous = 0;
    FT_GlyphSlot glyph;
    FT_Bool useKerning;
    int len = 0, bearing;
    unsigned int i;
    unsigned long character;
    unsigned char c[3];

    if( !canvas || !font_face || !font_height )
        return;

    x += canvas->ox;
    y += canvas->oy;

    if( x>canvas->sex || y>canvas->sey )
        return;

    if( NEED_CHANNEL_SWAP( format ) )
    {
        COLOR_COPY_INV( c, color );
    }
    else
    {
        COLOR_COPY( c, color );
    }

    FT_Set_Pixel_Sizes( font_face->face, 0, font_height );

    useKerning = FT_HAS_KERNING( font_face->face );

    for( i=0; i<length && (*text) && (*text!='\n'); text+=len, i+=len )
    {
        if( *text == ' ' )
        {
            x += ((int)font_height/3);
            len = 1;
            continue;
        }

        /* UTF8 -> UTF32 -> glyph index */
        character = to_utf32( (const unsigned char*)text, &len );
        glyph_index = FT_Get_Char_Index( font_face->face, character );

        /* load and render glyph */
        FT_Load_Glyph( font_face->face, glyph_index, FT_LOAD_DEFAULT );
        FT_Render_Glyph( font_face->face->glyph, FT_RENDER_MODE_NORMAL );

        /* apply kerning */
        if( useKerning && previous && glyph_index )
        {
            FT_Vector delta;
            FT_Get_Kerning( font_face->face, previous, glyph_index,
                            FT_KERNING_DEFAULT, &delta );
            x -= abs( delta.x ) >> 6;
        } 

        /* blend onto destination buffer */
        glyph = font_face->face->glyph;

        bearing = font_height - glyph->bitmap_top;

        if( !IS_OUTSIDE_SCISSOR_RECT( x, y+bearing, glyph->bitmap.width,
                                      glyph->bitmap.rows, canvas ) )
        {
            blend_glyph_on_canvas( canvas, glyph->bitmap.buffer, x, y+bearing,
                                   glyph->bitmap.width, glyph->bitmap.rows,
                                   c );
        }

        x += font_face->face->glyph->bitmap.width + 1;
        previous = glyph_index;
    }
}
