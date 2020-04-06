
#define APG_TGA_IMPLEMENTATION
#include "apg_tga.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include <stdio.h>

int main( int argc, char** argv ) {
  if ( argc < 2 ) {
    printf( "usage: test_read MY_IMAGE.tga\n" );
    return 0;
  }
  printf( "opening `%s`\n", argv[1] );

  unsigned int w = 0, h = 0;
  unsigned int n_chans   = 0;
  int vert_flip          = 0;
  unsigned char* img_mem = apg_tga_read_file( argv[1], &w, &h, &n_chans, vert_flip );
  if ( !img_mem ) {
    fprintf( stderr, "ERROR: failed to read image `%s`\n", argv[1] );
    return 1;
  }

  // convert BGR to RGB or BGRA to RGBA
  if (!apg_tga_bgr_to_rgb(img_mem, w,h,n_chans)) { return 1; }

  if ( !stbi_write_png( "out.png", w, h, n_chans, img_mem, w * n_chans ) ) {
    fprintf( stderr, "ERROR: writing out PNG\n" );
    return 1;
  }
  printf( "wrote image of w %u h %u n_chans %u\n", w, h, n_chans );

  free( img_mem );
  printf( "done\n" );
  return 0;
}
