/** apg_pixfont example using pixfont characters in an OpenGL texture array.
 * A simpler example would just generate a texture from a whole string of text,
 * but for performance we will use individual characters here.
 * Texutre arrays should support >= 256 layers in OpenGL 3.0, and 2048+ in 4.5.
 * This can be checked with GL_MAX_ARRAY_TEXTURE_LAYERS.
 */

#include "apg_pixfont.h"
#include <stdio.h>

int main() {
  int ws[256], hs[256];
  for ( int i = 0; i < 256; i++ ) {
    char str[2] = { i, 0 };
    int ret     = apg_pixfont_image_size_for_str( str, &ws[i], &hs[i], 2, 1, 0 );
    if ( APG_PIXFONT_FAILURE == ret ) {
      printf( "index %i failed\n", i );
      continue;
    }
    printf( "%i) %ix%i\n", i, ws[i], hs[i] );
  }

  return 0;
}
