/** Program to compare two BMP images' memory after loading them in.
 * Useful to check if apg_bmp_write() is working without modifying the image.
 */
#include "apg_bmp.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main( int argc, char** argv ) {
  if ( argc < 3 ) {
    printf( "Usage: test_bmp_cmp.bin first.bmp second.bmp\n" );
    return 0;
  }

  const char* img_a = argv[1];
  const char* img_b = argv[2];
  printf( "loading and comparing a=%s b=%s.\n", img_a, img_b );

  int w1 = 0, w2 = 0, h1 = 0, h2 = 0;
  unsigned int n_chans1 = 0, n_chans2 = 0;

  unsigned char *a_ptr = NULL, *b_ptr = NULL;
  a_ptr = apg_bmp_read( img_a, &w1, &h1, &n_chans1 );
  if ( !a_ptr ) {
    fprintf( stderr, "ERROR: Failed to load first image; `%s`\n", img_a );
    goto exit_fail;
  }
  b_ptr = apg_bmp_read( img_b, &w2, &h2, &n_chans2 );
  if ( !b_ptr ) {
    fprintf( stderr, "ERROR: Failed to load second image; `%s`\n", img_b );
    goto exit_fail;
  }
  {
    int x = 0, y = 0, n = 0;
    unsigned char* data1 = stbi_load( img_a, &x, &y, &n, 0 );
    if ( !data1 ) {
      fprintf( stderr, "ERROR: stb Failed to load first image; `%s`\n", img_a );
      goto exit_fail;
    }
    if ( w1 != x || h1 != y || (int)n_chans1 != n ) {
      fprintf( stderr, "ERROR: Image 1 dims/chans (%ix%i @ %i) do not match stbi (%ix%i @ %i) \n!", w1, h1, n_chans1, x, y, n );
      goto exit_fail;
    }
    int r = memcmp( a_ptr, data1, w1 * h1 * n_chans1 );
    if ( r != 0 ) {
      fprintf( stderr, "ERROR: First image memcmp vs stbi was %i\n", r );
      goto exit_fail;
    }
    free( data1 );
  }
  {
    int x = 0, y = 0, n = 0;
    unsigned char* data2 = stbi_load( img_b, &x, &y, &n, 0 );
    if ( !data2 ) {
      fprintf( stderr, "ERROR: stb Failed to load second image; `%s`\n", img_b );
      goto exit_fail;
    }
    if ( w2 != x || h2 != y || (int)n_chans2 != n ) {
      fprintf( stderr, "ERROR: Image 2 dims/chans (%ix%i @ %i) do not match stbi (%ix%i @ %i) \n!", w2, h2, n_chans2, x, y, n );
      goto exit_fail;
    }
    int r = memcmp( b_ptr, data2, w2 * h2 * n_chans2 );
    if ( r != 0 ) {
      fprintf( stderr, "ERROR: Second image memcmp vs stbi was %i\n", r );
      goto exit_fail;
    }
    free( data2 );
  }

  {
    if ( w1 != w2 || h1 != h2 || n_chans1 != n_chans2 ) {
      fprintf( stderr, "ERROR: Image dims/chans do not match!\n" );
      goto exit_fail;
    }

    int r = memcmp( a_ptr, b_ptr, w1 * h1 * n_chans1 );

    if ( 0 != r ) {
      printf( "FAILED COMPARISON! " );
    } else {
      printf( "PASSED COMPARISON " );
    }
    printf( "memcmp result=%i\n", r );
  }

  free( a_ptr );
  free( b_ptr );
  return 0;

exit_fail:
  if ( a_ptr ) { free( a_ptr ); }
  if ( b_ptr ) { free( b_ptr ); }
  return 1;
}
