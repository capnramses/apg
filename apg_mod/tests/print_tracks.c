// apg_mod - test/example - print .mod song score rows to stdout.
// By Anton Gerdelan

#include "apg_mod.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

int main( int argc, char** argv ) {
  if ( argc < 2 ) {
    printf( "Usage: %s MYFILE.mod\n", argv[0] );
    return 0;
  }
  const char* filename_ptr = argv[1];

  apg_mod_t mod = ( apg_mod_t ){ .sample_data_ptrs[0] = NULL };
  if ( !apg_mod_read_file( filename_ptr, &mod ) ) {
    fprintf( stderr, "ERROR: could not open file %s\n", filename_ptr );
    return 1;
  }

  printf( "Loaded song: %s\n", mod.song_name );

  for ( int o_idx = 0; o_idx < mod.n_orders; o_idx++ ) {
    uint8_t p_idx = mod.orders_ptr[o_idx];
    printf( "o%i/%i --p%i--\n", o_idx, mod.n_orders, p_idx );
    for ( int r_idx = 0; r_idx < APG_MOD_N_PATTERN_ROWS; r_idx++ ) {
      printf( "r%2i] ", r_idx );
      for ( int c_idx = 0; c_idx < mod.n_chans; c_idx++ ) {
        apg_mod_note_t note = ( apg_mod_note_t ){ .sample_idx = 0 };
        if ( apg_mod_fetch_note( &mod, p_idx, r_idx, c_idx, &note ) ) {
          int idx = apg_mod_find_period_table_idx( note.period_value_12b );
          char tmp[4];
          strcpy( tmp, "..." );
          if ( idx > -1 ) { strcpy( tmp, _note_names[idx] ); }
          printf( "%s %2i %2i %2i, ", tmp, note.sample_idx, note.effect_type_4b, note.effect_params );
        }
      }
      printf( "\n" );
    }
  }

  // Write samples to .raw files.
 // for ( int i = 0; i < APG_MOD_N_SAMPLES; i++ ) {
  //  bool ret = apg_mod_dump_raw_sample_file( &mod, i );
  //  if ( !ret ) { fprintf( stderr, "ERROR: could not write sample %i\n", i ); }
  //}

  if ( !apg_mod_free( &mod ) ) {
    fprintf( stderr, "ERROR: could not free mod\n" );
    return 1;
  }

  printf( "Normal exit\n" );
  return 0;
}
