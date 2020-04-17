// Utility for updating apg_pixfont.c with a new hard-coded font image
// Example use case: "Ah! I forgot to add ê to the typeface image...let's paint that into the reference
// image and then update the hard-coded version in the library."
// Input: some font image .png
// Output: A C array in text is written to stdout that can be copy-pasted into the code.
// Author: Anton Gerdelan, 17 April 2020
// Build:
// gcc -o bake_font bake_font_array.c -lm
// Usage:
// ./bake_font MY_IMAGE.png > some_code.c

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <stdio.h>

int main( int argc, char** argv ) {
  if ( argc < 2 ) {
    printf( "Usage: ./bake_font MY_IMAGE.png > some_code.c\n" );
    return 0;
  }
  int w = 0, h = 0, n = 0;
  unsigned char* img_ptr = stbi_load( argv[1], &w, &h, &n, 1 );
  if ( !img_ptr ) {
    fprintf( stderr, "ERROR loading font from image file `%s`\n", argv[1] );
    return 1;
  }
  printf( "// clang-format off\n" );
  printf( "static const int _font_img_w = %i;\n", w );
  printf( "static const int _font_img_h = %i;\n", h );
  printf( "static const unsigned char _font_img[] = { " );
  int max = w * h;
  for ( int idx = 0; idx < max - 1; idx++ ) { printf( "%i,", img_ptr[idx] ); } // 1 byte per pixel so ++
  printf( "%i", img_ptr[max - 1] );                                            // no comma on last element
  printf( " };\n" );
  printf( "// clang-format on\n" );

  free( img_ptr );
  return 0;
}
