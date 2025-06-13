// apg_mod - test/example - print .mod song score rows to stdout.
// By Anton Gerdelan

#include "apg_mod.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define APG_MIN( a, b ) ( ( a ) < ( b ) ? ( a ) : ( b ) )
#define APG_MAX( a, b ) ( ( a ) > ( b ) ? ( a ) : ( b ) )
#define APG_CLAMP( x, lo, hi ) ( APG_MIN( hi, APG_MAX( lo, x ) ) )

/* because string.h doesn't always have strnlen() */
static int apg_m_strnlen( const char* str, size_t maxlen ) {
  size_t i = 0;
  while ( i < maxlen && str[i] ) { i++; }
  return i;
}

static void apg_m_strncat( char* dst, const char* src, const size_t dst_max, const size_t src_max ) {
  assert( dst && src );

  size_t dst_len      = apg_m_strnlen( dst, dst_max );
  size_t src_len      = apg_m_strnlen( src, src_max );
  size_t space_in_dst = dst_max - dst_len;

  assert( src_len <= space_in_dst && "ERROR: Not enough space in destination string." );

  dst[dst_len] = '\0'; /* Just in case it wasn't already terminated. */

  if ( 0 == space_in_dst ) { return; }

  size_t n = APG_MIN( space_in_dst, src_len ); /* Use src_max if smaller. */
  memmove( &dst[dst_len], src, n );
  size_t last_i = dst_len + n < dst_max ? dst_len + n : dst_max - 1;
  dst[last_i]   = '\0';
}

static void apg_m_strncpy( char* dst, const size_t dst_max, const char* src ) {
  size_t src_max = apg_m_strnlen( src, dst_max );
  dst[0]         = '\0';
  apg_m_strncat( dst, src, dst_max, src_max );
}

int main( int argc, char** argv ) {
  if ( argc < 2 ) {
    printf( "Usage: %s MYFILE.mod\n", argv[0] );
    return 0;
  }
  const char* filename_ptr = argv[1];

  apg_mod_t mod = (apg_mod_t){ .sample_data_ptrs[0] = NULL };
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
        apg_mod_note_t note = (apg_mod_note_t){ .sample_idx = 0 };
        if ( apg_mod_fetch_note( &mod, p_idx, r_idx, c_idx, &note ) ) {
          int idx = apg_mod_find_period_table_idx( note.period_value_12b );
          char tmp[4];
          apg_m_strncpy( tmp, 4, "..." );
          if ( idx > -1 ) { apg_m_strncpy( tmp, 4, _note_names[idx] ); }
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
