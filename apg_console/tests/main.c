// test program for apg_console
// Anton Gerdelan 24 Mar 2020

#include "apg_console.h"
#include <stdio.h>
#include <stdlib.h>

bool anton_func( const char* arg_str ) {
  printf( "the value given to anton_func was `%s`\n", arg_str );
  return true;
}

int main() {
  int anton_var = 666;
  apg_c_register_var( "anton_var", &anton_var, APG_C_INT32 );
  apg_c_var_t* v_ptr = apg_c_get_var( "anton_var" );
  printf( "var: `%s`. val: %i\n", v_ptr->str, *( (int*)v_ptr->var_ptr ) );
  apg_c_register_func( "anton_func", anton_func );

  apg_c_append_user_entered_text( "help\n" );
  apg_c_dump_to_stdout();
  apg_c_append_user_entered_text( "clear\n" );
  apg_c_dump_to_stdout();
  apg_c_append_user_entered_text( "list_vars\n" );
  apg_c_output_clear();
  apg_c_append_user_entered_text( "list_funcs\n" );
  apg_c_printf( "a line I added" );
  apg_c_printf( "another line I added" );
  apg_c_dump_to_stdout();
  apg_c_append_user_entered_text( "anton_func\n" );
  apg_c_append_user_entered_text( "anton_func 123.456\n" );
  if ( apg_c_image_redraw_required() ) {
    printf( "apg_c_image_redraw_required()=TRUE\n" );
  } else {
    printf( "apg_c_image_redraw_required()=FALSE\n" );
  }
  int w                       = 512;
  int h                       = 512;
  int n_channels              = 3;
  uint8_t* img_ptr            = malloc( w * h * n_channels );
  uint8_t background_colour[] = {0xFF, 0xFF, 0x00, 0xFF};
  apg_c_draw_to_image_mem( img_ptr, w, h, n_channels, background_colour ); // NOTE(Anton) img output would be nice but pulls another dependency into test app
  if ( apg_c_image_redraw_required() ) {
    printf( "apg_c_image_redraw_required()=TRUE\n" );
  } else {
    printf( "apg_c_image_redraw_required()=FALSE\n" );
  }
  apg_c_append_user_entered_text( "anton_f" );
  apg_c_backspace();
  apg_c_append_user_entered_text( "\n" ); // should output "ERROR" for "anton_" here in text (just testing backspace works)
  apg_c_dump_to_stdout();
  apg_c_append_user_entered_text( "anton_f" );
  apg_c_autocomplete();
  apg_c_append_user_entered_text( "\n" );
  apg_c_dump_to_stdout();
  apg_c_reuse_hist( 1 );
  free( img_ptr );
  return 0;
}
