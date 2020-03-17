/* Test Read Program for apg_bmp -- for fuzzing tests
Anton Gerdelan
*/

#include "apg_bmp.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

int main( int argc, const char** argv ) {
  if ( argc < 2 ) {
    printf( "usage: ./test_readwrite_bmp INPUT.BMP\n" );
    return 0;
  }
	assert( argc > 1 );
  int w = 0, h = 0, n_chans = 0;
  unsigned char* img_mem = apg_bmp_read( argv[1], &w, &h, &n_chans );
  if ( !img_mem ) {
    fprintf( stderr, "ERROR: failed to read image `%s`\n", argv[1] );
    return 1;
  }
  free( img_mem );
  return 0;
}
