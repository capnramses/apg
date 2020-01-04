// unit tests for apg_console
// C99

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include "apg_console.h"
#include <assert.h>
#include <float.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TEST_VARS

int main() {
  int n_lines = apg_c_count_lines();
  assert( 0 == n_lines );

  float* ret_a = apg_c_get_var( "anton" ); // fetch non-existent var
  assert( NULL == ret_a );
  bool ret_b = apg_c_set_var( "anton", 0.0f ); // set non-existent var
  assert( false == ret_b );
  n_lines = apg_c_count_lines();
  assert( 0 == n_lines );
  float* ret_c = apg_c_create_var( "anton", 123.0f ); // create var
  assert( ret_c );
  float* ret_d = apg_c_create_var( "anton", 456.0f ); // double-create extant var
  assert( NULL == ret_d );
  float* ret_e = apg_c_get_var( "anton" ); // fetch var
  assert( ret_e && fabsf( *ret_e ) >= 123.0f - FLT_EPSILON );
  bool ret_f = apg_c_set_var( "anton", 789.0f ); // change var
  assert( ret_f );
  float* ret_g = apg_c_get_var( "anton" ); // fetch var
  assert( ret_g && fabsf( *ret_g ) >= 789.0f - FLT_EPSILON );

  n_lines = apg_c_count_lines();
  printf( "n_lines=%i\n", n_lines );
  assert( 1 == n_lines );

  apg_c_dump_to_stdout();
  printf( "---\n" );

  apg_c_dump_to_stdout();
  printf( "---\n" );
  float* ret_j = apg_c_create_var( "antonio", 101112.0f ); // set another far that would match substring "ant"
  assert( ret_j );

  apg_c_dump_to_stdout();
  printf( "---\n" );

  n_lines = apg_c_count_lines();
  printf( "n_lines=%i\n", n_lines );
  assert( 3 == n_lines );

  apg_c_print( "first item" );
  apg_c_print( "second item" );
  apg_c_print( "3rd item" );
  apg_c_print( "4th item" );
  apg_c_print( "5th item" );
  apg_c_dump_to_stdout();
  printf( "---\n" );
  for ( int i = 0; i < 30; i++ ) {
    char tmp[256];
    sprintf( tmp, "item +%i", i );
    apg_c_print( tmp );
  }
  apg_c_dump_to_stdout();
  printf( "---\n" );

  n_lines = apg_c_count_lines();
  assert( n_lines == APG_C_OUTPUT_LINES_MAX );

  int w = 1204, h = 768, n_chans = 3;
  uint8_t console_background[4] = { 0, 0, 0, 255 };
  uint8_t* img_ptr              = calloc( w * h * n_chans, 1 );
  bool retimg                   = apg_c_draw_to_image_mem( img_ptr, w, h, n_chans, console_background );
  assert( retimg );
  stbi_write_png( "testoutput.png", w, h, n_chans, img_ptr, w * n_chans );

  apg_c_output_clear();

  free( img_ptr );

  { // test parsing
    bool resa = apg_c_append_user_entered_text( "ant" );
    assert( resa );
    bool resb = apg_c_append_user_entered_text( "on" );
    assert( resb );
    bool resc = apg_c_append_user_entered_text( "io\n" );
    assert( resc );

    uint8_t* img_ptr = calloc( w * h * n_chans, 1 );

    bool resimg = apg_c_draw_to_image_mem( img_ptr, w, h, n_chans, console_background );
    assert( resimg ); // empty image
    stbi_write_png( "testuserenteredtext.png", w, h, n_chans, img_ptr, w * n_chans );

    free( img_ptr );
  }

  printf( "tests DONE\n" );
  return 0;
}
