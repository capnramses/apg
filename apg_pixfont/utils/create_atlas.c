// Example utility to create a typical 16x16 cell atlas image using apg_pixelfont
// by Anton Gerdelan 2023 Feb 26.

// gcc .\create_atlas.c ..\apg_pixfont.c -I ..\ -I ..\..\third_party\stb\

// TODO(Anton) - remove control code sections from atlas output.

#include "apg_pixfont.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int cell_dims[2] = { 16, 16 }; // ~1MB per 256-char sheet. Prefer power-of-two sizes, but not essential.

bool draw_atlas( const char* filename, int padding_px, int thickness, int add_outline, apg_pixfont_style_t style ) {
  int max_w = 0, max_h = 0;
  for ( int i = 0; i < 256; i++ ) {
    char tmp[2] = { i, '\0' };
    int w = 0, h = 0;
    if ( !apg_pixfont_image_size_for_str( tmp, &w, &h, thickness, add_outline, style, 0 ) ) { continue; }
    max_w = w < max_w ? max_w : w;
    max_h = h < max_h ? max_h : h;
  }
  printf( "max_w,h = {%i,%i}\n", max_w, max_h );
  if ( max_w + padding_px > cell_dims[0] || max_h + padding_px > cell_dims[1] ) {
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
    char tmp[5]  = { i, '\0' };
    int cell_col = i % 16;
    int cell_row = i / 16;
    // Latin-1 Supplement.
    if ( i >= 160 ) {
      tmp[0] = 0xC3;
      tmp[1] = i - 160 + 0x81 - 33;
      tmp[2] = 0x00;
    }
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
  printf( "Writing atlas.png\n" );
  int ret = stbi_write_png( filename, cell_dims[0] * 16, cell_dims[1] * 16, n_chans, atlas_ptr, cell_dims[0] * 16 * n_chans );
  fprintf( stderr, "%i writing atlas\n", ret );

  free( subimg_ptr );
  free( atlas_ptr );

  return true;
}

int main() {
  const char* atlas_str        = "atlas.png";
  const char* atlas_italic_str = "atlas_italic.png";
  const char* atlas_bold_str   = "atlas_bold.png";
  // Find max dimensions per character and check that font will fit in atlas.
  int padding_px            = 4;
  int thickness             = 1;
  int add_outline           = 1;
  apg_pixfont_style_t style = APG_PIXFONT_STYLE_REGULAR;

  if ( !draw_atlas( atlas_str, padding_px, thickness, add_outline, APG_PIXFONT_STYLE_REGULAR ) ) { fprintf( stderr, "ERROR drawing regular atlas.\n" ); }
  if ( !draw_atlas( atlas_italic_str, padding_px, thickness, add_outline, APG_PIXFONT_STYLE_ITALIC ) ) { fprintf( stderr, "ERROR drawing italic atlas.\n" ); }
  if ( !draw_atlas( atlas_bold_str, padding_px, thickness, add_outline, APG_PIXFONT_STYLE_BOLD ) ) { fprintf( stderr, "ERROR drawing bold atlas.\n" ); }

  printf( "done\n" );
  return 0;
}
