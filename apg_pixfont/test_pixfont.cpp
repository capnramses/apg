#include "apg_pixfont.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include <stdio.h>

#define N_TEST_STRINGS 7

int main() {
  const char* test_strings[N_TEST_STRINGS] = {
    "Hello, World!",                                 //
    "This is a line\nbreak - did it work?",          //
    "über spaß!",                                    //
    "An bhfuil cead agam dul\ngo dtí an leithreas?", //
    "This is My Lovely, Big Heading",                //
    "Absolute Unit of a Text",                       //
    "Nobody Needs\nText This\nHUGE!"                 //
  };
  const int n_chans[N_TEST_STRINGS]            = {1, 2, 3, 4, 4, 4, 4};
  int thickness[N_TEST_STRINGS]                = {1, 1, 1, 1, 2, 3, 10};
  bool outlines[N_TEST_STRINGS]                = {false, true, false, true, true, true, true};
  const char* output_filenames[N_TEST_STRINGS] = {"0.png", "1.png", "2.png", "4.png", "5.png", "6.png", "7.png"};

  for ( int i = 0; i < N_TEST_STRINGS; i++ ) {
    // ALLOCATE MEMORY FOR TEXT IMAGE
    int w = 0, h = 0;

    bool result = apg_pixfont_image_size_for_str( test_strings[i], &w, &h, thickness[i], outlines[i] );
    if ( !result ) {
      fprintf( stderr, "ERROR: sizing string image %i\n", i );
      return 1;
    }

    // CREATE TEXT IMAGE
    unsigned char* text_img = (unsigned char*)calloc( 1, w * h * n_chans[i] );
    result                  = apg_pixfont_str_into_image( test_strings[i], text_img, w, h, n_chans[i], 0xFF, 0x7F, 0x00, 0xFF, thickness[i], outlines[i] );
    if ( !result ) {
      fprintf( stderr, "ERROR: creating string image %i\n", i );
      return 1;
    }

    // WRITE OUTPUT TO FILE
    result = stbi_write_png( output_filenames[i], w, h, n_chans[i], text_img, w * n_chans[i] );
    if ( !result ) {
      fprintf( stderr, "ERROR: writing string image %i\n", i );
      return 1;
    }

    // FREE IMAGE MEMORY
    free( text_img );
  }

  // convert a font image file to C array for pasting into apg_pixfont.c
#ifdef GENERATE_C_ARRAY_FONT
  {
    int w = 0, h = 0, n = 0;
    unsigned char* img = stbi_load( "pixfont.png", &w, &h, &n, 0 );
    if ( !font.img ) {
      fprintf( stderr, "ERROR: loading font image\n" );
      return 1;
    }

    FILE* fptr = fopen( "font.c", "w" );
    fprintf( fptr, "static const int _font_img_w = %i;\n", font.w );
    fprintf( fptr, "static const int _font_img_h = %i;\n", font.h );
    fprintf( fptr, "static const int _font_img_n_chans = %i;\n", font.n_colour_channels );
    fprintf( fptr, "static const unsigned char _font_img[] = { " );
    for ( int y = 0; y < font.h; y++ ) {
      for ( int x = 0; x < font.w; x++ ) {
        if ( y != font.h - 1 || x != font.w - 1 ) {
          fprintf( fptr, "0X%02X,", font.img[y * font.w + x] );
        } else {
          fprintf( fptr, "0X%02X", font.img[y * font.w + x] );
        }
      }
    }
    fprintf( fptr, " };\n" );
    fclose( fptr );
  }
#endif

  return 0;
}
