// Example utility to create a typical 16x16 cell atlas image using apg_pixelfont
// by Anton Gerdelan 2023 Feb 26.
//
// Atlas contains ASCII codepoints 0x20-0x7F (skips control codes before space),
// and then includes the Latin-1 supplement, excluding its control codes section also.
// This leaves ~64 blanks at the end that could be repurposed.
//
// * Advantage to this? Images of pre-baked outlines or styles without requiring run-time generation.
// * Does it need to be in a square image? No. Power of two dimensions are fine, but it might help with
//   mipmaps on some setups, or work with existing software better than a strip.
// * Could the main library just use these atlases? Yes, but I find it easier to hand-edit an atlas.
//   There are some use-cases where baking that into code is handy for quick blit-to-screen one-off debugging.
// * Could you just load 256 invidual glyphs of each style to memory? Yes, that would fit into texture arrays
//   and avoid bleeding issues with mipmaps. There's plenty of padding for the glyphs here though. For thickness 2
//   use 32x32px cells. This also looks a bit nicer for italics.

// gcc .\create_atlas.c ..\apg_pixfont.c -I ..\ -I ..\..\third_party\stb\

#include "apg_pixfont.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
  uint32_t codepoint = index + ' ';                        // Space is the glyph in the top-left (skip ASCII control codes).
  
  if ( codepoint > 127 ) { codepoint += ( 0xA0 - 0x80 ); } // A0 includes Latin-1 Punctuation ans Symbols section.

  // up to position 175 ( 11 rows of 16 )
  // Extras
  if ( 192 == index ) { codepoint = 0x0153; printf("got cp!\n");}
  if ( 193 == index ) { codepoint = 0x0154; }
  if ( 194 == index ) { codepoint = 0x01EA; }
  if ( 195 == index ) { codepoint = 0x01EB; }
  return codepoint;
}

bool draw_atlas( const char* filename, int thickness, int add_outline, apg_pixfont_style_t style ) {
  int max_w = 0, max_h = 0;
  for ( int i = 0; i < 256; i++ ) {
    uint32_t codepoint = get_codepoint( i );
    char tmp[5]        = { i - ' ', '\0' };
    if ( !apg_cp_to_utf8( codepoint, tmp ) ) { return false; }
    int w = 0, h = 0;
    if ( !apg_pixfont_image_size_for_str( tmp, &w, &h, thickness, add_outline, style, 0 ) ) { continue; }
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
  for ( int i = 0; i < 256; i++ ) {
    memset( subimg_ptr, 0, subimg_sz );
    uint32_t codepoint = get_codepoint( i );
    char tmp[5]        = { i - ' ', '\0' };
    if ( !apg_cp_to_utf8( codepoint, tmp ) ) { return false; }
    uint32_t cell_col = i % 16;
    uint32_t cell_row = i / 16;
    if ( !apg_pixfont_str_into_image( tmp, subimg_ptr, cell_dims[0], cell_dims[1], n_chans, 0xFF, 0xFF, 0xFF, 0xFF, thickness, add_outline, style, 0 ) ) {
      fprintf( stderr, "Bad result writing image for char %i '%c'\n", i, tmp[0] );
      continue;
    }
    /* uncomment this bit to get a separate image for each glyph:
    char subimg_str[64];
    sprintf( subimg_str, "images/%i.png", i );
    stbi_write_png( subimg_str, cell_dims[0], cell_dims[1], n_chans, subimg_ptr, cell_dims[0] * n_chans );
    */
    size_t cell_row_offset = cell_row * 16 * cell_dims[0] * cell_dims[1] * n_chans;
    size_t cell_col_offset = cell_col * cell_dims[0] * n_chans;
    for ( int y = 0; y < cell_dims[1]; y++ ) { // Copy one sub-image line at a time.
      uint8_t* dst_ptr = atlas_ptr + cell_row_offset + cell_col_offset + y * 16 * cell_dims[0] * n_chans;
      uint8_t* src_ptr = subimg_ptr + cell_dims[0] * n_chans * y;
      memcpy( dst_ptr, src_ptr, cell_dims[0] * n_chans );
    }
  }
  int ret = stbi_write_png( filename, cell_dims[0] * 16, cell_dims[1] * 16, n_chans, atlas_ptr, cell_dims[0] * 16 * n_chans );
  free( subimg_ptr );
  free( atlas_ptr );
  if ( !ret ) { return false; }
  return true;
}

int main() {
  const char* atlas_str               = "atlas.png";
  const char* atlas_bold_str          = "atlas_bold.png";
  const char* atlas_italic_str        = "atlas_italic.png";
  const char* atlas_underline_str     = "atlas_underline.png";
  const char* atlas_strikethrough_str = "atlas_strikethrough.png";
  // Find max dimensions per character and check that font will fit in atlas.
  int thickness   = 1;
  int add_outline = 1;

  if ( !draw_atlas( atlas_str, thickness, add_outline, APG_PIXFONT_STYLE_REGULAR ) ) { fprintf( stderr, "ERROR drawing regular atlas.\n" ); }
  if ( !draw_atlas( atlas_bold_str, thickness, add_outline, APG_PIXFONT_STYLE_BOLD ) ) { fprintf( stderr, "ERROR drawing bold atlas.\n" ); }
  if ( !draw_atlas( atlas_italic_str, thickness, add_outline, APG_PIXFONT_STYLE_ITALIC ) ) { fprintf( stderr, "ERROR drawing italic atlas.\n" ); }
  if ( !draw_atlas( atlas_underline_str, thickness, add_outline, APG_PIXFONT_STYLE_UNDERLINE ) ) { fprintf( stderr, "ERROR drawing underline atlas.\n" ); }
  if ( !draw_atlas( atlas_strikethrough_str, thickness, add_outline, APG_PIXFONT_STYLE_STRIKETHROUGH ) ) {
    fprintf( stderr, "ERROR drawing strikthrough atlas.\n" );
  }
  printf( "done\n" );
  return 0;
}
