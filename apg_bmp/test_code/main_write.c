#include "apg_bmp.h"
#include <stdio.h>
#include <stdlib.h>

int main( int argc, char** argv ) {
  if ( argc < 3 ) {
    printf( "usage: ./readwrite input.bmp output.bmp\n" );
    return 0;
  }

  printf( "reading `%s`\n", argv[1] );

  int w = 0, h = 0;
  unsigned int n_chans   = 0;
  unsigned char* img_ptr = apg_bmp_read( argv[1], &w, &h, &n_chans );
  if ( !img_ptr ) {
    fprintf( stderr, "ERROR reading BMP\n" );
    return 1;
  }

  printf( "read image\n|-w = %i\n|-h = %i\n|-n_chans = %u\n", w, h, n_chans );

  printf( "writing `%s`\n", argv[2] );
  if ( !apg_bmp_write( argv[2], img_ptr, w, h, n_chans ) ) {
    fprintf( stderr, "ERROR writing BMP\n" );
    return 1;
  }

  apg_bmp_free( img_ptr );

  printf( "wrote image file. program done\n" );

  return 1;
}
