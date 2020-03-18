/* Test Read Program for apg_bmp
Uses stb_image_write to write out a PNG to 'out.png' as proof.
Anton Gerdelan
*/

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h" // used for writing out file to check it worked
#include "apg_bmp.h"
#include <stdio.h>
#include <stdlib.h>

int main( int argc, const char** argv ) {
  if ( argc < 2 ) {
    printf( "usage: ./test_readwrite_bmp INPUT.BMP\n" );
    return 0;
  }
  printf( "Reading `%s`. Writing out.png\n", argv[1] );
  int w = 0, h = 0;
  unsigned int n_chans   = 0;
  unsigned char* img_mem = apg_bmp_read( argv[1], &w, &h, &n_chans );
  if ( !img_mem ) {
    fprintf( stderr, "ERROR: failed to read image `%s`\n", argv[1] );
    return 1;
  }

  if ( !stbi_write_png( "out.png", w, h, n_chans, img_mem, w * n_chans ) ) {
    fprintf( stderr, "ERROR: writing out PNG\n" );
    return 1;
  }
  printf( "wrote image of w %u h %u n_chans %u\n", w, h, n_chans );

  free( img_mem );
  printf( "done\n" );
  return 0;
}
