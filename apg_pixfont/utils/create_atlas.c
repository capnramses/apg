/**
 * @file create_atlas.c
 *
 * @author Anton Gerdelan - @capnramses - <antongdl@protonmail.com>.
 *
 * @brief Stand-alone utility program to create a typical 16x16 cell atlas image (sheet of characters) using apg_pixelfont.
 *
 * Example use cases:
 *   - Real-time graphical applications where you sample a pre-made atlas texture when rendering text.
 *   - "I want to have pre-made atlas images with my font in various styles; outlined, bold, etc."
 *
 * Atlas contains ASCII codepoints 0x20-0x7F (skips control codes before space),
 * and then includes the Latin-1 supplement, excluding its control codes section also.
 * This leaves ~64 blanks at the end that could be repurposed.
 *
 *  Advantage to this?
 *    - Images of pre-baked outlines or styles without requiring run-time generation.
 *    - Does it need to be in a square image? No. Power of two dimensions are fine, but it might help with
 *      mipmaps on some setups, or work with existing software better than a strip.
 *    - Could the main library just use these atlases? Yes, but I find it easier to hand-edit an atlas.
 *    - There are some use-cases where baking that into code is handy for quick blit-to-screen one-off debugging.
 *    - Could you just load 256 invidual glyphs of each style to memory? Yes, that would fit into texture arrays
 *      and avoid bleeding issues with mipmaps. There's plenty of padding for the glyphs here though. For thickness 2
 *      use 32x32px cells. This also looks a bit nicer for italics.
 *
 * Build
 *   cc -o create_atlas ./create_atlas.c ../apg_pixfont.c -I ../ -I ../../third_party/stb/
 *
 * Usage
 *   ./create_atlas [OPTIONS]
 *
 * Options include:
 *   --prefix STRING
 *     File name prefix.
 *     Utility generates several style files called [prefix]_bold.png, [prefix]_underline.png, etc.
 *     Default "atlas".
 *
 *   --typeface INT
 *     0 - normal, 1 - short.
 *
 * Licence: See bottom of this file.
 *
 * History:
 *   - 2025 Jun 14 - Updated for short typeface support.
 *   - 2023 Feb 26 - First version.
 */

#include "apg_pixfont.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define GLYPHS_ACROSS 16 // Number of glyphs in each atlas row. Note: not the same as pixel width of each cell!
#define GLYPHS_DOWN 16   // Number of glyphs in each atlas column. Note: not the same as pixel width of each cell!

int cell_dims[2] = { 16, 16 }; // ~1MB per 256-char sheet. Prefer power-of-two sizes, but not essential.

#define MASK_FIRST_ONE 128   // 128 or 10000000
#define MASK_FIRST_TWO 192   // 192 or 11000000
#define MASK_FIRST_THREE 224 // 224 or 11100000
#define MASK_FIRST_FOUR 240  // 240 or 11110000
#define MASK_FIRST_FIVE 248  // 248 or 11111000

int apg_cp_to_utf8( uint32_t codepoint, char* mbs ) {
  assert( mbs );
  if ( !mbs ) {
    mbs[0] = (char)0xEF;
    mbs[1] = (char)0xBF;
    mbs[2] = (char)0xBD;
    mbs[3] = '\0';
    return 4;
  }

  if ( codepoint <= 0x7F ) { // ASCII
    mbs[0] = (char)codepoint;
    mbs[1] = '\0';
    return 1;
  }
  // 2 bytes: max 0b11111111111 -> 110xxxxx 10xxxxxx
  if ( codepoint <= 0x07FF ) { // (U+0080 to U+07FF - Latin, Greek, Hebrew, Arabic, ... )
    // ( shift off lower byte(s), cut off anything in front from earlier byte(s), append 110xxxxx to front )
    mbs[0] = (char)( ( ( codepoint >> 6 ) & 0x1F ) | MASK_FIRST_TWO );
    mbs[1] = (char)( ( codepoint & 0x3F ) | MASK_FIRST_ONE );
    mbs[2] = '\0';
    return 2;
  }
  // 3-bytes
  if ( codepoint <= 0xFFFF ) {
    mbs[0] = (char)( ( ( codepoint >> 12 ) & 0x0F ) | MASK_FIRST_THREE );
    mbs[1] = (char)( ( ( codepoint >> 6 ) & 0x3F ) | MASK_FIRST_ONE );
    mbs[2] = (char)( ( codepoint & 0x3F ) | MASK_FIRST_ONE );
    mbs[3] = '\0';
    return 3;
  }
  // 4-bytes
  if ( codepoint <= 0x10FFFF ) {
    mbs[0] = (char)( ( ( codepoint >> 18 ) & 0x07 ) | MASK_FIRST_FOUR );
    mbs[1] = (char)( ( ( codepoint >> 12 ) & 0x3F ) | MASK_FIRST_ONE );
    mbs[2] = (char)( ( ( codepoint >> 6 ) & 0x3F ) | MASK_FIRST_ONE );
    mbs[3] = (char)( ( codepoint & 0x3F ) | MASK_FIRST_ONE );
    mbs[4] = '\0';
    return 4;
  }
  // unknown unicode - use diamond question mark
  mbs[0] = (char)0xEF;
  mbs[1] = (char)0xBF;
  mbs[2] = (char)0xBD;
  mbs[3] = '\0';
  return 4;
}

uint32_t get_codepoint( int index ) {
  uint32_t codepoint = index + ' '; // Space is the glyph in the top-left (skip ASCII control codes).

  if ( codepoint > 127 ) { codepoint += ( 0xA0 - 0x80 ); } // A0 includes Latin-1 Punctuation ans Symbols section.

  // up to position 175 ( 11 rows of 16 )
  // Extras
  if ( 192 == index ) { codepoint = 0x0153; }
  if ( 193 == index ) { codepoint = 0x0154; }
  if ( 194 == index ) { codepoint = 0x01EA; }
  if ( 195 == index ) { codepoint = 0x01EB; }
  return codepoint;
}

bool draw_atlas( const char* filename, int thickness, int add_outline, apgpf_style_t style, apgpf_typeface_t typeface ) {
  int max_w = 0, max_h = 0;
  for ( int i = 0; i < 256; i++ ) {
    uint32_t codepoint = get_codepoint( i );
    char tmp[5]        = { i - ' ', '\0' };
    if ( !apg_cp_to_utf8( codepoint, tmp ) ) { return false; }
    int w = 0, h = 0;
    if ( !apg_pixfont_image_size_for_str( tmp, &w, &h, thickness, add_outline, style, 0, typeface ) ) { continue; }
    max_w = w < max_w ? max_w : w;
    max_h = h < max_h ? max_h : h;
  }
  printf( "atlas style %i: glyph max_w,h = %ix%i cell pixel dims = %ix%i\n", (int)style, max_w, max_h, cell_dims[0], cell_dims[1] );
  // -2 below, is to account for +1 for outline and +1 for uneven number adjustment.
  // Default y size is 16 pixels, if outline is drawn then 17. That will get rounded up to 18.
  // In the worst case a fully-spanning top-to-bottom glyph (unlikely) would lose the outline pixel on the bottom.
  // Biggest width for 1 glyph in a cell, at thickness 1, should be 6 (widest glyph) + 7 (max italics shift) + 1 (outline) = 14.
  if ( max_w > cell_dims[0] || max_h - 2 > cell_dims[1] ) {
    fprintf( stderr, "ERROR: cells are not big enough for glyphs. Resize.\n" );
    // return 1;
  }
  int n_chans        = 4; // Could be 2, or even 1, if there are no outlines.
  uint8_t* atlas_ptr = calloc( cell_dims[0] * cell_dims[1] * n_chans, 256 );
  if ( !atlas_ptr ) {
    fprintf( stderr, "ERROR: malloc OOM\n" );
    return false;
  }
  size_t subimg_sz    = cell_dims[0] * cell_dims[1] * n_chans;
  uint8_t* subimg_ptr = calloc( subimg_sz, 1 );
  if ( !subimg_ptr ) {
    fprintf( stderr, "ERROR: malloc OOM\n" );
    return false;
  }
  for ( int i = 0; i < ( GLYPHS_ACROSS * GLYPHS_DOWN ); i++ ) {
    memset( subimg_ptr, 0, subimg_sz );
    uint32_t codepoint = get_codepoint( i );
    char tmp[5]        = { i - ' ', '\0' };
    if ( !apg_cp_to_utf8( codepoint, tmp ) ) { return false; }
    uint32_t cell_col = i % GLYPHS_ACROSS;
    uint32_t cell_row = i / GLYPHS_ACROSS;
    if ( !apg_pixfont_str_into_image( tmp, subimg_ptr, cell_dims[0], cell_dims[1], n_chans, 0xFF, 0xFF, 0xFF, 0xFF, thickness, add_outline, style, 0, typeface ) ) {
      fprintf( stderr, "Bad result writing image for char %i '%c'\n", i, tmp[0] );
      continue;
    }
    /* uncomment this bit to get a separate image for each glyph:
    char subimg_str[64];
    snprintf( subimg_str, 64, "images/%i.png", i );
    stbi_write_png( subimg_str, cell_dims[0], cell_dims[1], n_chans, subimg_ptr, cell_dims[0] * n_chans );
    */
    size_t cell_row_offset = cell_row * GLYPHS_ACROSS * cell_dims[0] * cell_dims[1] * n_chans;
    size_t cell_col_offset = cell_col * cell_dims[0] * n_chans;
    for ( int y = 0; y < cell_dims[1]; y++ ) { // Copy one sub-image line at a time.
      uint8_t* dst_ptr = atlas_ptr + cell_row_offset + cell_col_offset + y * GLYPHS_ACROSS * cell_dims[0] * n_chans;
      uint8_t* src_ptr = subimg_ptr + cell_dims[0] * n_chans * y;
      memcpy( dst_ptr, src_ptr, cell_dims[0] * n_chans );
    }
  }
  int ret = stbi_write_png( filename, cell_dims[0] * GLYPHS_ACROSS, cell_dims[1] * GLYPHS_DOWN, n_chans, atlas_ptr, cell_dims[0] * GLYPHS_ACROSS * n_chans );
  free( subimg_ptr );
  free( atlas_ptr );
  if ( !ret ) { return false; }
  return true;
}

typedef enum arg_opt_name_t {
  ARG_OPT_PREFIX,   //
  ARG_OPT_TYPEFACE, //
  ARG_OPT_MAX       //
} arg_opt_name_t;

static const char* arg_options[] = {
  "--prefix",  //
  "--typeface" //
};

int main( int argc, char** argv ) {
  char prefix[256] = { 0 };
  strncat( prefix, "atlas", 255 );
  int typeface = 0;

  for ( int i = 1; i < argc; i++ ) {
    for ( int j = 0; j < ARG_OPT_MAX; j++ ) {
      if ( 0 == strncmp( argv[i], arg_options[j], 32 ) ) {
        switch ( (arg_opt_name_t)j ) {
        case ARG_OPT_PREFIX:
          if ( i + 1 < argc ) {
            prefix[0] = '\0';
            strncat( prefix, argv[i + 1], 255 );
            i++;
          }
          break;

        case ARG_OPT_TYPEFACE:
          if ( i + 1 < argc ) {
            typeface = atoi( argv[i + 1] );
            assert( typeface == 0 || typeface == 1 );
            i++;
          }
          break;

        default: break;
        }
      }
    }
  }

  printf( "Using atlas filename prefix `%s`\n", prefix );
  printf( "Using typeface `%i`\n", typeface );

  const char* atlas_normal_str        = "_normal.png";
  const char* atlas_bold_str          = "_bold.png";
  const char* atlas_italic_str        = "_italic.png";
  const char* atlas_underline_str     = "_underline.png";
  const char* atlas_strikethrough_str = "_strikethrough.png";
  // Find max dimensions per character and check that font will fit in atlas.
  int thickness   = 1;
  int add_outline = 1;

  char op_filename[2048];
  snprintf( op_filename, 2048, "%s%s", prefix, atlas_normal_str );
  if ( !draw_atlas( op_filename, thickness, add_outline, APGPF_STYLE_REGULAR, (apgpf_typeface_t)typeface ) ) {
    fprintf( stderr, "ERROR: Drawing regular atlas.\n" );
    return 1;
  }
  snprintf( op_filename, 2048, "%s%s", prefix, atlas_bold_str );
  if ( !draw_atlas( op_filename, thickness, add_outline, APGPF_STYLE_BOLD, (apgpf_typeface_t)typeface ) ) {
    fprintf( stderr, "ERROR: Drawing bold atlas.\n" );
    return 1;
  }
  snprintf( op_filename, 2048, "%s%s", prefix, atlas_italic_str );
  if ( !draw_atlas( op_filename, thickness, add_outline, APGPF_STYLE_ITALIC, (apgpf_typeface_t)typeface ) ) {
    fprintf( stderr, "ERROR: Drawing italic atlas.\n" );
    return 1;
  }
  snprintf( op_filename, 2048, "%s%s", prefix, atlas_underline_str );
  if ( !draw_atlas( op_filename, thickness, add_outline, APGPF_STYLE_UNDERLINE, (apgpf_typeface_t)typeface ) ) {
    fprintf( stderr, "ERROR: Drawing underline atlas.\n" );
    return 1;
  }
  snprintf( op_filename, 2048, "%s%s", prefix, atlas_strikethrough_str );
  if ( !draw_atlas( op_filename, thickness, add_outline, APGPF_STYLE_STRIKETHROUGH, (apgpf_typeface_t)typeface ) ) {
    fprintf( stderr, "ERROR: Drawing strikthrough atlas.\n" );
    return 1;
  }
  printf( "done\n" );
  return 0;
}

/*
-------------------------------------------------------------------------------------
This software is available under two licences - you may use it under either licence.
-------------------------------------------------------------------------------------
FIRST LICENCE OPTION

>                                  Apache License
>                            Version 2.0, January 2004
>                         http://www.apache.org/licenses/
>    Copyright 2019 Anton Gerdelan.
>    Licensed under the Apache License, Version 2.0 (the "License");
>    you may not use this file except in compliance with the License.
>    You may obtain a copy of the License at
>        http://www.apache.org/licenses/LICENSE-2.0
>    Unless required by applicable law or agreed to in writing, software
>    distributed under the License is distributed on an "AS IS" BASIS,
>    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
>    See the License for the specific language governing permissions and
>    limitations under the License.
-------------------------------------------------------------------------------------
SECOND LICENCE OPTION

> This is free and unencumbered software released into the public domain.
>
> Anyone is free to copy, modify, publish, use, compile, sell, or
> distribute this software, either in source code form or as a compiled
> binary, for any purpose, commercial or non-commercial, and by any
> means.
>
> In jurisdictions that recognize copyright laws, the author or authors
> of this software dedicate any and all copyright interest in the
> software to the public domain. We make this dedication for the benefit
> of the public at large and to the detriment of our heirs and
> successors. We intend this dedication to be an overt act of
> relinquishment in perpetuity of all present and future rights to this
> software under copyright law.
>
> THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
> EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
> MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
> IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
> OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
> ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
> OTHER DEALINGS IN THE SOFTWARE.
>
> For more information, please refer to <http://unlicense.org>
-------------------------------------------------------------------------------------
*/
