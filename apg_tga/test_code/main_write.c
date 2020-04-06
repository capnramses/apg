/*
Test Program for apg_tga.h
Author:  Anton Gerdelan -- @capnramses
*/

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define APG_TGA_IMPLEMENTATION
#include "apg_tga.h"
#include <stdio.h>
#include <stdint.h>

int main( int argc, char** argv ) {
  if ( argc < 2 ) {
    printf( "usage: %s my_file.tga\n", argv[0] );
    return 0;
  }
  printf( "loading %s\n", argv[1] );
  int w, h, n;
  uint8_t* img = stbi_load( argv[1], &w, &h, &n, 0 );
  if ( !img ) { return 1; }

  printf( "loaded %s - %ix%i n=%i!\n", argv[1], w, h, n );
  if(!apg_tga_bgr_to_rgb( img, (unsigned int)w,(unsigned int)h,(unsigned int)n)){return 1;}

  if ( !apg_tga_write_file( "out.tga", img, (unsigned int)w, (unsigned int)h, (unsigned int)n ) ) {
    fprintf( stderr, "ERROR writing file\n" );
    return 1;
  }
  free( img );
  return 0;
}
