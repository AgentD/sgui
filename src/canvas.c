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
#include "sgui_canvas.h"

#include <stdlib.h>
#include <string.h>

#include <ft2build.h>
#include FT_FREETYPE_H



struct sgui_font
{
    FT_Library freetype;
    FT_Face face;
    void* buffer;
};

struct sgui_canvas
{
    unsigned int width, height;
    void* data;
    SGUI_COLOR_FORMAT format;
    unsigned int bpp;
    int own_buffer;

    int sx, sy, sex, sey;
};

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
                           unsigned char R, unsigned char G, unsigned char B )
{
    unsigned char A, iA, *src, *dst, *row;
    unsigned int i, j, ds, dt;

    /* don't blend if outside the drawing area */
    if( (x+(int)w)<canvas->sx || (y+(int)h)<canvas->sy )
        return -1;

    if( x>canvas->sex || y>canvas->sey )
        return 1;

    /* adjust parameters to only blend visible portion */
    dst = ((unsigned char*)canvas->data) + (y*canvas->width + x)*canvas->bpp;

    if( y<canvas->sy )
    {
        dst += (canvas->sy-y)*canvas->width*canvas->bpp;
        glyph += (canvas->sy-y)*w;
        y = 0;
    }

    if( (y+(int)h) > canvas->sey )
        h = canvas->sey - y;

    ds = w;
    dt = canvas->width*canvas->bpp;

    if( x < canvas->sx )
    {
        w -= canvas->sx - x;
        glyph += canvas->sx - x;
        x = canvas->sx;
    }

    if( (x+(int)w) >= canvas->sex )
        w = canvas->sex - x;

    /* do the blend */
    for( j=0; j<h; ++j, glyph+=ds, dst+=dt )
    {
        for( src=glyph, row=dst, i=0; i<w; ++i, row+=canvas->bpp, ++src )
        {
            A = *src;
            iA = 255-A;

            row[0] = (R*A + row[0]*iA) >> 8;
            row[1] = (G*A + row[1]*iA) >> 8;
            row[2] = (B*A + row[2]*iA) >> 8;
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

void sgui_font_get_text_extents( sgui_font* font_norm, sgui_font* font_bold,
                                 sgui_font* font_ital, sgui_font* font_boit,
                                 unsigned int font_height, const char* text,
                                 unsigned int* width, unsigned int* height )
{
    int i = 0, font_stack_index = 0;
    unsigned int X = 0, longest = 0, lines = 1;
    sgui_font* f = font_norm;
    sgui_font* font_stack[10];

    if( !font_norm || !font_height || !text || (!width && !height) )
        return;

    for( ; text && text[ i ]; ++i )
    {
        if( text[ i ] == '<' )
        {
            X += sgui_font_get_text_extents_plain( f, font_height, text, i );

            if( text[ i+1 ] == 'b' )
            {
                font_stack[ font_stack_index++ ] = f;
                f = f==font_ital ? font_boit : font_bold;
            }
            else if( text[ i+1 ] == 'i' )
            {
                font_stack[ font_stack_index++ ] = f;
                f = f==font_bold ? font_boit : font_ital;
            }
            else if( text[ i+1 ] == '/' && font_stack_index )
            {
                f = font_stack[ --font_stack_index ];
            }

            text = strchr( text+i, '>' );

            if( text )
                ++text;

            i = -1;
        }
        else if( text[ i ] == '\n' )
        {
            X += sgui_font_get_text_extents_plain( f, font_height, text, i );

            if( X > longest )
                longest = X;

            ++lines;
            text += i + 1;
            i = -1;
            X = 0;
        }
    }

    X += sgui_font_get_text_extents_plain( f, font_height, text, i );

    if( X > longest )
        longest = X;

    if( width  ) *width  = longest;
    if( height ) *height = lines * (font_height+font_height/4);
}




/***************************** Canvas functions *****************************/
sgui_canvas* sgui_canvas_create( unsigned int width, unsigned int height,
                                 SGUI_COLOR_FORMAT format )
{
    sgui_canvas* cv = malloc( sizeof(sgui_canvas) );
    unsigned int bpp = 3;

    if( !cv )
        return NULL;

    if( format==SCF_RGBA8 || format==SCF_BGRA8 )
        bpp = 4;

    cv->width      = width;
    cv->height     = height;
    cv->data       = malloc( width * height * bpp );
    cv->format     = format;
    cv->bpp        = bpp;
    cv->own_buffer = 1;

    cv->sx         = 0;
    cv->sy         = 0;
    cv->sex        = width  ? (width -1) : 0;
    cv->sey        = height ? (height-1) : 0;

    if( !cv->data )
    {
        free( cv );
        return NULL;
    }

    return cv;
}

sgui_canvas* sgui_canvas_create_use_buffer( void* buffer,
                                            unsigned int width,
                                            unsigned int height,
                                            SGUI_COLOR_FORMAT format )
{
    sgui_canvas* cv = malloc( sizeof(sgui_canvas) );
    unsigned int bpp = 3;

    if( !cv )
        return NULL;

    if( format==SCF_RGBA8 || format==SCF_BGRA8 )
        bpp = 4;

    cv->width      = width;
    cv->height     = height;
    cv->data       = buffer;
    cv->format     = format;
    cv->bpp        = bpp;
    cv->own_buffer = 0;

    cv->sx         = 0;
    cv->sy         = 0;
    cv->sex        = width  ? (width -1) : 0;
    cv->sey        = height ? (height-1) : 0;

    return cv;
}

void sgui_canvas_destroy( sgui_canvas* canvas )
{
    if( canvas )
    {
        if( canvas->own_buffer )
            free( canvas->data );

        free( canvas );
    }
}

void sgui_canvas_get_size( sgui_canvas* canvas, unsigned int* width,
                           unsigned int* height )
{
    if( canvas )
    {
        if( width  ) *width  = canvas->width;
        if( height ) *height = canvas->height;
    }
    else
    {
        if( width  ) *width  = 0;
        if( height ) *height = 0;
    }
}

void sgui_canvas_resize( sgui_canvas* canvas, unsigned int width,
                         unsigned int height )
{
    unsigned int new_mem, old_mem;

    if( !canvas || (canvas->width==width && height==canvas->height) )
        return;

    if( !canvas->own_buffer )
        return;

    new_mem = width*height;
    old_mem = canvas->width*canvas->height;

    if( new_mem != old_mem )
    {
        free( canvas->data );
        canvas->data = malloc( new_mem * canvas->bpp );
    }

    canvas->width  = width;
    canvas->height = height;

    canvas->sx     = 0;
    canvas->sy     = 0;
    canvas->sex    = width  ? (width -1) : 0;
    canvas->sey    = height ? (height-1) : 0;
}




void sgui_canvas_set_scissor_rect( sgui_canvas* canvas, int x, int y,
                                   unsigned int width, unsigned int height )
{
    if( canvas )
    {
        if( width && height )
        {
            canvas->sx  = x;
            canvas->sy  = y;
            canvas->sex = x + width;
            canvas->sey = y + height;

            if( canvas->sx < 0 )
                canvas->sx = 0;

            if( canvas->sy < 0 )
                canvas->sy = 0;

            if( canvas->sex>=(int)canvas->width )
                canvas->sex = canvas->width ? (canvas->width -1) : 0;

            if( canvas->sey>=(int)canvas->height )
                canvas->sey = canvas->height ? (canvas->height-1) : 0;
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

void sgui_canvas_blit( sgui_canvas* canvas, int x, int y, unsigned int width,
                       unsigned int height, SGUI_COLOR_FORMAT format,
                       const void* data )
{
    unsigned char *dst, *src, *drow, *srow;
    unsigned int i, j, src_bpp = 3, R = 0, G = 1, B = 2;
    unsigned int ds, dt;

    if( !canvas || !width || !height || !data )
        return;

    /* don't blit if outside the drawing area */
    if( (x+(int)width)<canvas->sx || (y+(int)height)<canvas->sy )
        return;

    if( x>canvas->sex || y>canvas->sey )
        return;

    /* color format checks */
    if( format==SCF_RGBA8 || format==SCF_BGRA8 )
        src_bpp = 4;

    if( ((format==SCF_RGB8 || format==SCF_RGBA8) &&
         (canvas->format==SCF_BGR8 || canvas->format==SCF_BGRA8)) ||
        ((format==SCF_BGR8 || format==SCF_BGRA8) &&
         (canvas->format==SCF_RGB8 || canvas->format==SCF_RGBA8)) )
    {
        R = 2;
        G = 1;
        B = 0;
    }

    /* compute source and destination pointers */
    dst = ((unsigned char*)canvas->data) + (y*canvas->width + x)*canvas->bpp;
    src = (unsigned char*)data;

    /* adjust parameters to only blit visible portion */
    if( y<canvas->sy )
    {
        dst += (canvas->sy-y)*canvas->width*canvas->bpp;
        src += (canvas->sy-y)*width*src_bpp;
        y = canvas->sy;
    }

    if( (y+((int)height-1)) > canvas->sey )
        height = canvas->sey - y;

    ds = width*src_bpp;
    dt = canvas->width*canvas->bpp;

    if( x < canvas->sx )
    {
        width -= canvas->sx - x;
        src += (canvas->sx - x)*src_bpp;
        x = canvas->sx;
    }

    if( (x+((int)width-1)) > canvas->sex )
        width = canvas->sex - x;

    /* do the blit */
    for( j=0; j<height; ++j, src+=ds, dst+=dt )
    {
        for( drow=dst, srow=src, i=0; i<width; ++i, drow+=canvas->bpp,
                                                    srow+=src_bpp )
        {
            drow[ 0 ] = srow[ R ];
            drow[ 1 ] = srow[ G ];
            drow[ 2 ] = srow[ B ];
        }
    }
}

void sgui_canvas_blend( sgui_canvas* canvas, int x, int y, unsigned int width,
                        unsigned int height, SGUI_COLOR_FORMAT format,
                        const void* data )
{
    unsigned char *dst, *src, *drow, *srow, A, iA;
    unsigned int i, j, R = 0, G = 1, B = 2, ds, dt;

    if( !canvas || !width || !height || !data )
        return;

    if( format!=SCF_RGBA8 && format!=SCF_BGRA8 )
        return;

    /* don't blend outside the drawing area */
    if( (x+(int)width)<canvas->sx || (y+(int)height)<canvas->sy )
        return;

    if( x>canvas->sex || y>canvas->sey )
        return;

    /* color format checks */
    if( (format==SCF_RGBA8 &&
         (canvas->format==SCF_BGR8 || canvas->format==SCF_BGRA8)) ||
        (format==SCF_BGRA8 &&
         (canvas->format==SCF_RGB8 || canvas->format==SCF_RGBA8)) )
    {
        R = 2;
        G = 1;
        B = 0;
    }

    /* compute source and destination pointers */
    dst = ((unsigned char*)canvas->data) + (y*canvas->width + x)*canvas->bpp;
    src = (unsigned char*)data;

    /* adjust parameters to only blit visible portion */
    if( y<canvas->sy )
    {
        dst += (canvas->sy-y)*canvas->width*canvas->bpp;
        src += (canvas->sy-y)*width*4;
        y = canvas->sy;
    }

    if( (y+((int)height-1)) > canvas->sey )
        height = canvas->sey - y;

    ds = width*4;
    dt = canvas->width*canvas->bpp;

    if( x < canvas->sx )
    {
        width -= canvas->sx - x;
        src += (canvas->sx - x)*4;
        x = canvas->sx;
    }

    if( (x+((int)width-1)) > canvas->sex )
        width = canvas->sex - x;

    /* do the blend */
    for( j=0; j<height; ++j, src+=ds, dst+=dt )
    {
        for( drow=dst, srow=src, i=0; i<width; ++i, drow+=canvas->bpp,
                                                    srow+=4 )
        {
            A = srow[3], iA = 255-A;

            drow[0] = (srow[R]*A + drow[0]*iA) >> 8;
            drow[1] = (srow[G]*A + drow[1]*iA) >> 8;
            drow[2] = (srow[B]*A + drow[2]*iA) >> 8;
        }
    }
}

void sgui_canvas_draw_box( sgui_canvas* canvas, int x, int y,
                           unsigned int width, unsigned int height,
                           unsigned char* color, SGUI_COLOR_FORMAT format )
{
    unsigned char R=0, G=1, B=2, A, iA;
    unsigned int i, j;
    unsigned char *dst, *row;

    if( !canvas || !color )
        return;

    /* don't draw outside the drawing area */
    if( (x+(int)width)<canvas->sx || (y+(int)height)<canvas->sy )
        return;

    if( x>canvas->sex || y>canvas->sey )
        return;

    /* color format checks */
    if( ((format==SCF_RGBA8 || format==SCF_RGB8) &&
         (canvas->format==SCF_BGR8 || canvas->format==SCF_BGRA8)) ||
        ((format==SCF_BGRA8 && format==SCF_BGR8) &&
         (canvas->format==SCF_RGB8 || canvas->format==SCF_RGBA8)) )
    {
        R = color[2];
        G = color[1];
        B = color[0];
    }
    else
    {
        R = color[0];
        G = color[1];
        B = color[2];
    }

    /* adjust parameters to only draw visible portion */
    if( y<canvas->sy ) { height -= canvas->sy-y; y = canvas->sy; }
    if( x<canvas->sx ) { width  -= canvas->sx-x; x = canvas->sx; }

    if( (y+((int)height-1)) > canvas->sey )
        height = canvas->sey - y;

    if( (x+((int)width-1)) > canvas->sex )
        width = canvas->sex - x;

    dst = (unsigned char*)canvas->data + (y*canvas->width+x)*canvas->bpp;

    if( format==SCF_RGBA8 || format==SCF_BGRA8 )
    {
        A = color[3];
        iA = 255 - A;

        for( j=0; j<height; ++j, dst+=canvas->width*canvas->bpp )
        {
            for( row=dst, i=0; i<width; ++i, row+=canvas->bpp )
            {
                row[0] = (row[0]*iA + R*A) >> 8;
                row[1] = (row[1]*iA + G*A) >> 8;
                row[2] = (row[2]*iA + B*A) >> 8;
            }
        }
    }
    else
    {
        for( j=0; j<height; ++j, dst+=canvas->width*canvas->bpp )
        {
            for( row=dst, i=0; i<width; ++i, row+=canvas->bpp )
            {
                row[0] = R;
                row[1] = G;
                row[2] = B;
            }
        }
    }
}

void sgui_canvas_draw_line( sgui_canvas* canvas, int x, int y,
                            unsigned int length, int horizontal,
                            unsigned char* color, SGUI_COLOR_FORMAT format )
{
    unsigned char* dst;
    unsigned char R, G, B, A, iA;
    unsigned int i;

    if( canvas )
    {
        if( ((format==SCF_RGBA8 || format==SCF_RGB8) &&
             (canvas->format==SCF_BGR8 || canvas->format==SCF_BGRA8)) ||
            ((format==SCF_BGRA8 && format==SCF_BGR8) &&
             (canvas->format==SCF_RGB8 || canvas->format==SCF_RGBA8)) )
        {
            R = color[2];
            G = color[1];
            B = color[0];
        }
        else
        {
            R = color[0];
            G = color[1];
            B = color[2];
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

            dst = (unsigned char*)canvas->data +
                  (y*canvas->width+x)*canvas->bpp;

            if( format==SCF_RGBA8 || format==SCF_BGRA8 )
            {
                A = color[3];
                iA = 255 - A;

                for( i=0; i<length; ++i, dst+=canvas->bpp )
                {
                    dst[0] = (R*A + iA*dst[0]) >> 8;
                    dst[1] = (G*A + iA*dst[1]) >> 8;
                    dst[2] = (B*A + iA*dst[2]) >> 8;
                }
            }
            else
            {
                for( i=0; i<length; ++i, dst+=canvas->bpp )
                {
                    dst[0] = R;
                    dst[1] = G;
                    dst[2] = B;
                }
            }
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

            dst = (unsigned char*)canvas->data +
                  (y*canvas->width+x)*canvas->bpp;

            if( (y+(int)length) > canvas->sey )
                length = canvas->sey - y;

            if( format==SCF_RGBA8 || format==SCF_BGRA8 )
            {
                A = color[3];
                iA = 255 - A;

                for( i=0; i<length; ++i, dst+=canvas->width*canvas->bpp )
                {
                    dst[0] = (R*A + iA*dst[0]) >> 8;
                    dst[1] = (G*A + iA*dst[1]) >> 8;
                    dst[2] = (B*A + iA*dst[2]) >> 8;
                }
            }
            else
            {
                for( i=0; i<length; ++i, dst+=canvas->width*canvas->bpp )
                {
                    dst[0] = R;
                    dst[1] = G;
                    dst[2] = B;
                }
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
    unsigned char R = color[0], G = color[1], B = color[2];

    if( !canvas || !font_face || !font_height )
        return;

    if( x>canvas->sex || y>canvas->sey )
        return;

    if( ((format==SCF_RGBA8 || format==SCF_RGB8) &&
         (canvas->format==SCF_BGR8 || canvas->format==SCF_BGRA8)) ||
        ((format==SCF_BGRA8 && format==SCF_BGR8) &&
         (canvas->format==SCF_RGB8 || canvas->format==SCF_RGBA8)) )
    {
        R = color[2];
        G = color[1];
        B = color[0];
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

        if( blend_glyph_on_canvas( canvas, glyph->bitmap.buffer, x, y+bearing,
                                   glyph->bitmap.width, glyph->bitmap.rows,
                                   R, G, B ) > 0 )
        {
            break;
        }

        x += font_face->face->glyph->bitmap.width + 1;
        previous = glyph_index;
    }
}

void sgui_canvas_draw_text( sgui_canvas* canvas, int x, int y,
                            sgui_font* font_norm, sgui_font* font_bold,
                            sgui_font* font_ital, sgui_font* font_boit,
                            unsigned int font_height, unsigned char* color,
                            SGUI_COLOR_FORMAT format, const char* text )
{
    int i = 0, X = 0, font_stack_index = 0;
    sgui_font* f = font_norm;
    sgui_font* font_stack[10];
    unsigned char col[3];
    long c;

    if( !canvas || !font_norm || !font_height || !color || !text )
        return;

    col[0] = color[0];
    col[1] = color[1];
    col[2] = color[2];

    for( ; text && text[ i ]; ++i )
    {
        if( text[ i ] == '<' )
        {
            sgui_canvas_draw_text_plain( canvas, x+X, y, f, font_height, col,
                                         format, text, i );

            X += sgui_font_get_text_extents_plain( f, font_height, text, i );

            if( !strncmp( text+i+1, "color=", 6 ) )
            {
                if( !strncmp( text+i+9, "default", 7 ) )
                {
                    col[0] = color[0];
                    col[1] = color[1];
                    col[2] = color[2];
                }
                else
                {
                    c = strtol( text+i+9, NULL, 16 );

                    if( format==SCF_RGBA8 || format==SCF_RGB8 )
                    {
                        col[0] = (c>>16) & 0xFF;
                        col[1] = (c>>8 ) & 0xFF;
                        col[2] =  c      & 0xFF;
                    }
                    else
                    {
                        col[2] = (c>>16) & 0xFF;
                        col[1] = (c>>8 ) & 0xFF;
                        col[0] =  c      & 0xFF;
                    }
                }
            }
            else if( text[ i+1 ] == 'b' )
            {
                font_stack[ font_stack_index++ ] = f;
                f = f==font_ital ? font_boit : font_bold;
            }
            else if( text[ i+1 ] == 'i' )
            {
                font_stack[ font_stack_index++ ] = f;
                f = f==font_bold ? font_boit : font_ital;
            }
            else if( text[ i+1 ] == '/' && font_stack_index )
            {
                f = font_stack[ --font_stack_index ];
            }

            text = strchr( text+i, '>' );

            if( text )
                ++text;

            i = -1;
        }
        else if( text[ i ] == '\n' )
        {
            sgui_canvas_draw_text_plain( canvas, x+X, y, f, font_height, col,
                                         format, text, i );

            text += i + 1;
            i = -1;
            X = 0;
            y += font_height+font_height/4;
        }
    }

    sgui_canvas_draw_text_plain( canvas, x+X, y, f, font_height, col,
                                 format, text, i );
}

void* sgui_canvas_get_raw_data( sgui_canvas* canvas )
{
    return canvas ? canvas->data : NULL;
}

void sgui_canvas_set_raw_data( sgui_canvas* canvas, SGUI_COLOR_FORMAT format,
                               unsigned int width, unsigned int height,
                               void* data )
{
    if( canvas )
    {
        canvas->width  = width;
        canvas->height = height;
        canvas->format = format;
        canvas->bpp    = (format==SCF_RGBA8 || format==SCF_BGRA8) ? 4 : 3;

        canvas->sx     = 0;
        canvas->sy     = 0;
        canvas->sex    = width  ? (width-1)  : 0;
        canvas->sey    = height ? (height-1) : 0;

        if( canvas->own_buffer && canvas->data )
            free( canvas->data );

        if( data )
        {
            canvas->data       = data;
            canvas->own_buffer = 0;
        }
        else
        {
            canvas->data       = malloc( width * height * canvas->bpp );
            canvas->own_buffer = 1;
        }
    }
}

