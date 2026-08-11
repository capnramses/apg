// test program for apg_console
// Anton Gerdelan 24 Mar 2020

#include "apg_console.h"
#include <stdio.h>
#include <stdlib.h>

bool anton_func( const char* arg_str ) {
  printf( "the value given to anton_func was `%s`\n", arg_str );
  return true;
}

int main( void ) {

  // resolving a prior bug where text wouldn't print until a \n was entered
  apg_c_append_user_entered_text( "does this print without a slash n?" );
  apg_c_dump_to_stdout();
  apg_c_append_user_entered_text("\n");

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
  apg_c_append_user_entered_text( "anton_f" );
  apg_c_backspace();
  apg_c_append_user_entered_text( "\n" ); // should output "ERROR" for "anton_" here in text (just testing backspace works)
  apg_c_dump_to_stdout();
  apg_c_append_user_entered_text( "anton_f" );
  apg_c_autocomplete();
  apg_c_append_user_entered_text( "\n" );
  apg_c_dump_to_stdout();
  apg_c_reuse_hist( 1 );
  return 0;
}
