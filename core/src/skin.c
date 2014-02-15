/*
 * skin.c
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
#define SGUI_BUILDING_DLL
#include "sgui_filesystem.h"
#include "sgui_internal.h"
#include "sgui_pixmap.h"
#include "sgui_skin.h"
#include "sgui_font.h"
#include "sgui_utf8.h"

#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>



static sgui_skin skin;
static unsigned char* image;
static int color_format;



static void read_image_file( const char* filename, sgui_filesystem* fs )
{
    unsigned char header[ 18 ], temp;
    unsigned int x, y, bpp;
    unsigned char* ptr;
    void* file;

    file = fs->file_open_read( fs, filename );

    /* read the TGA header */
    fs->file_read( file, header, 1, 18 );

    /* Skip the image ID field */
    while( (header[0])-- )
        fs->file_read( file, &temp, 1, 1 );

    /* read some information we care about */
    skin.pixmap_width  = header[ 12 ] | (header[ 13 ]<<8);
    skin.pixmap_height = header[ 14 ] | (header[ 15 ]<<8);
    bpp                = header[ 16 ] >> 3;

    /* sanity checks */
    if( header[ 2 ]!=2 || header[ 1 ] || (bpp!=3 && bpp!=4) )
    {
        fs->file_close( file );
        return;
    }

    if( !skin.pixmap_width || !skin.pixmap_height )
    {
        fs->file_close( file );
        return;
    }

    /* Read the image data */
    image = malloc( skin.pixmap_width*skin.pixmap_height*bpp );
    fs->file_read( file, image, bpp, skin.pixmap_width*skin.pixmap_height );
    color_format = bpp==3 ? SGUI_RGB8 : SGUI_RGBA8;

    /* swap red and blue channels */
    for( ptr=image, y=0; y<skin.pixmap_height; ++y )
    {
        for( x=0; x<skin.pixmap_width; ++x, ptr+=bpp )
        {
            temp = ptr[0];
            ptr[0] = ptr[2];
            ptr[2] = temp;
        }
    }

    fs->file_close( file );
}

static void image_to_pixmap( sgui_skin* s, sgui_pixmap* pixmap )
{
    sgui_pixmap_load( pixmap, 0, 0, image, 0, 0,
                      s->pixmap_width, s->pixmap_height,
                      s->pixmap_width, color_format );
}

/****************************************************************************/

void sgui_skin_load( const char* configfile, sgui_filesystem* fs )
{
    int i, j, x, y, w, h;
    char line[ 128 ];
    void* cfgfile;

    if( !fs )
        fs = sgui_filesystem_get_default( );

    cfgfile = fs->file_open_read( fs, configfile );

#define ELEMENT( name, index )\
        if( !strcmp(line+i,name) )\
            sgui_rect_set_size(skin.elements+index,x,y,w,h)

    while( !fs->file_eof( cfgfile ) )
    {
        sgui_filesystem_read_line( fs, cfgfile, line, sizeof(line) );

        /* isolate attribute name */
        for( i=0;  isspace( line[i] );                            ++i );
        for( j=i; !isspace( line[j] ) && line[j]!='=' && line[j]; ++j );

        if( line[j] == '\0' )
            continue;

        /* terminate name and find assignment */
        if( line[j]=='=' )
        {
            line[ j++ ] = '\0';
        }
        else
        {
            line[ j++ ] = '\0';
            while( line[j]!='=' && line[j] ) ++j;
            ++j;
        }

        if( line[j] == '\0' )
            continue;

        /* find value */
        while( isspace( line[j] ) ) ++j;

        if( line[j]!='(' && line[j]!='"' )
            continue;

        /* read skin file */
        if( !strcmp( line+i, "skin.file" ) )
        {
            for( i=j+1; line[i]!='"' && line[i]; ++i );

            line[i] = '\0';

            read_image_file( line+j+1, fs );
            skin.load_to_pixmap = image_to_pixmap;
        }
        else if( !strcmp( line+i, "window.color" ) )
        {
            if( sscanf( line+j, "(%d,%d,%d,%d)", &x, &y, &w, &h ) == 4 )
            {
                skin.window_color[0] = x;
                skin.window_color[1] = y;
                skin.window_color[2] = w;
                skin.window_color[3] = h;
            }
        }
        else if( !strcmp( line+i, "font.color" ) )
        {
            if( sscanf( line+j, "(%d,%d,%d,%d)", &x, &y, &w, &h ) == 4 )
            {
                skin.font_color[0] = x;
                skin.font_color[1] = y;
                skin.font_color[2] = w;
                skin.font_color[3] = h;
            }
        }
        else if( !strcmp( line+i, "font.height" ) )
        {
            sscanf( line+j, "(%u)", &skin.font_height );
        }

        /* read skin elements */
        if( sscanf( line+j, "(%d,%d,%d,%d)", &x, &y, &w, &h ) != 4 )
            continue;

        ELEMENT( "sbar.h.pane.left", SGUI_SCROLL_BAR_H_PANE_LEFT );
        ELEMENT( "sbar.h.pane.center", SGUI_SCROLL_BAR_H_PANE_CENTER );
        ELEMENT( "sbar.h.pane.right", SGUI_SCROLL_BAR_H_PANE_RIGHT );
        ELEMENT( "sbar.h.bar.background", SGUI_SCROLL_BAR_H_BACKGROUND );
        ELEMENT( "sbar.v.pane.top", SGUI_SCROLL_BAR_V_PANE_TOP );
        ELEMENT( "sbar.v.pane.center", SGUI_SCROLL_BAR_V_PANE_CENTER );
        ELEMENT( "sbar.v.pane.bottom", SGUI_SCROLL_BAR_V_PANE_BOTTOM );
        ELEMENT( "sbar.v.bar.background", SGUI_SCROLL_BAR_V_BACKGROUND );
        ELEMENT( "sbar.up", SGUI_SCROLL_BAR_BUTTON_UP );
        ELEMENT( "sbar.down", SGUI_SCROLL_BAR_BUTTON_DOWN );
        ELEMENT( "sbar.left", SGUI_SCROLL_BAR_BUTTON_LEFT );
        ELEMENT( "sbar.right", SGUI_SCROLL_BAR_BUTTON_RIGHT );
        ELEMENT( "sbar.in.up", SGUI_SCROLL_BAR_BUTTON_UP_IN );
        ELEMENT( "sbar.in.down", SGUI_SCROLL_BAR_BUTTON_DOWN_IN );
        ELEMENT( "sbar.in.left", SGUI_SCROLL_BAR_BUTTON_LEFT_IN );
        ELEMENT( "sbar.in.right", SGUI_SCROLL_BAR_BUTTON_RIGHT_IN );
        ELEMENT( "tab.cap.left", SGUI_TAB_CAP_LEFT );
        ELEMENT( "tab.cap.center", SGUI_TAB_CAP_CENTER );
        ELEMENT( "tab.cap.right", SGUI_TAB_CAP_RIGHT );
        ELEMENT( "tab.left", SGUI_TAB_LEFT );
        ELEMENT( "tab.left.top", SGUI_TAB_LEFT_TOP );
        ELEMENT( "tab.left.bottom", SGUI_TAB_LEFT_BOTTOM );
        ELEMENT( "tab.right", SGUI_TAB_RIGHT );
        ELEMENT( "tab.right.top", SGUI_TAB_RIGHT_TOP );
        ELEMENT( "tab.right.bottom", SGUI_TAB_RIGHT_BOTTOM );
        ELEMENT( "tab.top", SGUI_TAB_TOP );
        ELEMENT( "tab.bottom", SGUI_TAB_BOTTOM );
        ELEMENT( "tab.gap.left", SGUI_TAB_GAP_LEFT );
        ELEMENT( "tab.gap.right", SGUI_TAB_GAP_RIGHT );
    }

#undef ELEMENT

    fs->file_close( cfgfile );
}

void sgui_skin_unload( void )
{
    free( image );
    image = NULL;

    memset( &skin, 0, sizeof(sgui_skin) );
}

void sgui_skin_set( sgui_skin* ui_skin )
{
    if( ui_skin )
    {
        memcpy( &skin, ui_skin, sizeof(sgui_skin) );
    }
    else
    {
        sgui_interal_skin_init_default( &skin );
    }
}

void sgui_skin_get_pixmap_size( unsigned int* width, unsigned int* height )
{
    if( width  ) *width  = skin.pixmap_width;
    if( height ) *height = skin.pixmap_height;
}

void sgui_skin_to_pixmap( sgui_pixmap* pixmap )
{
    if( skin.load_to_pixmap && pixmap )
    {
        skin.load_to_pixmap( &skin, pixmap );
    }
}

void sgui_skin_get_element( unsigned int element, sgui_rect* r )
{
    if( r && (element < SGUI_SKIN_ELEMENTS) )
    {
        sgui_rect_copy( r, skin.elements + element );
    }
}

void sgui_skin_set_default_font( sgui_font* normal, sgui_font* bold,
                                 sgui_font* italic, sgui_font* bold_italic )
{
    skin.font_norm = normal;
    skin.font_bold = bold;
    skin.font_ital = italic;
    skin.font_boit = bold_italic;
}

void sgui_skin_get_window_background_color( unsigned char* color )
{
    if( color )
    {
        color[0] = skin.window_color[0];
        color[1] = skin.window_color[1];
        color[2] = skin.window_color[2];
        color[3] = skin.window_color[3];
    }
}

void sgui_skin_get_default_font_color( unsigned char* color )
{
    if( color )
    {
        color[0] = skin.font_color[0];
        color[1] = skin.font_color[1];
        color[2] = skin.font_color[2];
        color[3] = skin.font_color[3];
    }
}

unsigned int sgui_skin_get_default_font_height( void )
{
    return skin.font_height;
}

sgui_font* sgui_skin_get_default_font( int bold, int italic )
{
    if( bold && italic ) return skin.font_boit;
    if( bold           ) return skin.font_bold;
    if( italic         ) return skin.font_ital;

    return skin.font_norm;
}

unsigned int sgui_skin_default_font_extents( const char* text,
                                             unsigned int length,
                                             int bold, int italic )
{
    sgui_font* font_face = sgui_skin_get_default_font( bold, italic );
    unsigned int x = 0, w, len = 0, i;
    unsigned long character, previous = 0;

    /* sanity check */
    if( !text || !font_face || !length )
        return 0;

    /* for each character */
    for( i=0; i<length && (*text) && (*text!='\n'); text+=len, i+=len )
    {
        /* load the next glyph */
        character = sgui_utf8_decode( text, &len );
        sgui_font_load_glyph( font_face, character );

        /* advance cursor */
        x += sgui_font_get_kerning_distance( font_face, previous, character );
        sgui_font_get_glyph_metrics( font_face, &w, NULL, NULL );

        x += w + 1;

        /* store previous glyph character for kerning */
        previous = character;
    }

    return x;
}

void sgui_skin_get_text_extents( const char* text, sgui_rect* r )
{
    unsigned int X = 0, longest = 0, lines = 1, i = 0, font_stack_index = 0;
    char font_stack[10], f = 0;

    /* sanity check */
    if( !text || !r )
        return;

    for( ; text && text[ i ]; ++i )
    {
        if( text[ i ] == '<' )  /* we found a tag */
        {
            /* get extends for what we found so far */
            X += sgui_skin_default_font_extents( text, i, f&0x02, f&0x01 );

            /* change fonts accordingly */
            if( text[ i+1 ] == 'b' )
            {
                font_stack[ font_stack_index++ ] = f;
                f |= 0x02;
            }
            else if( text[ i+1 ] == 'i' )
            {
                font_stack[ font_stack_index++ ] = f;
                f |= 0x01;
            }
            else if( text[ i+1 ] == '/' && font_stack_index )
            {
                f = font_stack[ --font_stack_index ];
            }

            /* skip to tag end */
            text = strchr( text+i, '>' );

            if( text )
                ++text;

            /* reset i to -1, so it starts with 0 in the next iteration */
            i = -1;
        }
        else if( text[ i ] == '\n' )
        {
            /* get extends for what we found so far */
            X += sgui_skin_default_font_extents( text, i, f&0x02, f&0x01 );

            /* store the length of the longest line */
            if( X > longest )
                longest = X;

            ++lines;        /* increment line counter */
            text += i + 1;  /* skip to next line */
            i = -1;         /* restart with 0 at next iteration */
            X = 0;          /* move cursor back to the left */
        }
    }

    /* get the extents of what we didn't get so far */
    X += sgui_skin_default_font_extents( text, i, f&0x02, f&0x01 );

    if( X > longest )
        longest = X;

    /* Add font height/2 as fudge factor to the height, because our crude
       computation here does not take into account that characters can peek
       out below the line */
    r->left   = 0;
    r->top    = 0;
    r->right  = longest - 1;
    r->bottom = lines * skin.font_height + skin.font_height/2 - 1;
}

void sgui_skin_get_checkbox_extents( sgui_rect* r )
{
    if( r )
        skin.get_checkbox_extents( &skin, r );
}

void sgui_skin_get_radio_button_extents( sgui_rect* r )
{
    if( r )
        skin.get_radio_button_extents( &skin, r );
}

unsigned int sgui_skin_get_edit_box_height( void )
{
    return skin.get_edit_box_height( &skin );
}

unsigned int sgui_skin_get_edit_box_border_width( void )
{
    return skin.get_edit_box_border_width( &skin );
}

unsigned int sgui_skin_get_frame_border_width( void )
{
    return skin.get_frame_border_width( &skin );
}

unsigned int sgui_skin_get_progess_bar_width( void )
{
    return skin.get_progess_bar_width( &skin );
}

void sgui_skin_draw_checkbox( sgui_canvas* canvas, int x, int y, int checked )
{
    if( canvas )
        skin.draw_checkbox( &skin, canvas, x, y, checked );
}

void sgui_skin_draw_radio_button( sgui_canvas* canvas, int x, int y,
                                  int checked )
{
    if( canvas )
        skin.draw_radio_button( &skin, canvas, x, y, checked );
}

void sgui_skin_draw_button( sgui_canvas* canvas, sgui_rect* r, int pressed )
{
    if( canvas && r )
        skin.draw_button( &skin, canvas, r, pressed );
}

void sgui_skin_draw_editbox( sgui_canvas* canvas, sgui_rect* r,
                             const char* text, int offset, int cursor )
{
    offset = offset<0 ? 0 : offset;

    if( canvas && r )
        skin.draw_editbox( &skin, canvas, r, text, offset, cursor );
}

void sgui_skin_draw_frame( sgui_canvas* canvas, sgui_rect* r )
{
    if( canvas && r )
        skin.draw_frame( &skin, canvas, r );
}

void sgui_skin_draw_group_box( sgui_canvas* canvas, sgui_rect* r,
                               const char* caption )
{
    if( canvas && r )
        skin.draw_group_box( &skin, canvas, r, caption );
}

void sgui_skin_draw_progress_bar( sgui_canvas* canvas, int x, int y,
                                  unsigned int length, int vertical,
                                  int percentage )
{
    percentage = percentage<0 ? 0 : (percentage>100 ? 100 : percentage);

    if( canvas )
        skin.draw_progress_bar( &skin, canvas, x, y, length,
                                vertical, percentage );
}

void sgui_skin_draw_progress_stippled( sgui_canvas* canvas, int x, int y,
                                       unsigned int length, int vertical,
                                       int percentage )
{
    if( canvas )
        skin.draw_progress_stippled( &skin, canvas, x, y, length, vertical,
                                     percentage );
}

