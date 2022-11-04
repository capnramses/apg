/* apg_mod - test/example - play a .mod song using PortAudio.
By Anton Gerdelan

WORK IN PROGRESS

Ubuntu: `sudo apt install portaudio19-dev`

Compile e.g.:
gcc examples/play_song.c apg_mod.c -o play_song.bin -I ./ -lm -lportaudio


TODO

beside every row print:
* ?preload samples into audio buffers e.g. line 29 in tests/dump_wavs.c but into raw buffers?
* play the sample indicated without any extra effect
* work out correct frequency
* remove buffered print statements to improve timing
* do proper channel mixing
* and mix up to 4 channels together
* add advanced effects (maybe loops first)
*/

#include "portaudio.h"
#include "apg_mod.h"
#include <stdint.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

const uint32_t sample_rate_hz = 11025;

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

	// TODO WIP Create buffers for sound samples.
	//
  for ( int i = 0; i < APG_MOD_N_SAMPLES; i++ ) {
    if ( 0 == mod.sample_sz_bytes[i] ) { continue; }
    uint32_t n_samples = mod.sample_sz_bytes[i] / sizeof( int16_t ); // not sure why this exists as a param if it can be derived from other params.
    
		// TODO put this stuff in a buffer: (I guess 'samples' here is the actual data-points on the sound waveform encoding.

		//bool ret           = apg_wav_write( tmp, mod.sample_data_ptrs[i], mod.sample_sz_bytes[i], 1, sample_rate_hz, n_samples, 16 );
  }

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

  if ( !apg_mod_free( &mod ) ) {
    fprintf( stderr, "ERROR: could not free mod\n" );
    return 1;
  }

  printf( "Normal exit\n" );
  return 0;
}
