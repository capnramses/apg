#include "apg_mod.h"
#include "apg_wav.h"
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

  uint32_t sample_rate_hz = 11025;
  // data_sz == n_samples * n_chans * bytes_per_sample

  // Write samples to .wav files.
  for ( int i = 0; i < APG_MOD_N_SAMPLES; i++ ) {
    if ( 0 == mod.sample_sz_bytes[i] ) { continue; }
    char tmp[64];
    sprintf( tmp, "sample%02i.wav", i );
    uint32_t n_samples = mod.sample_sz_bytes[i] / sizeof( int16_t ); // not sure why this exists as a param if it can be derived from other params.
    bool ret           = apg_wav_write( tmp, mod.sample_data_ptrs[i], mod.sample_sz_bytes[i], 1, sample_rate_hz, n_samples, 16 );
    if ( !ret ) {
      printf( "couldnt write a sample for %i\n", i );
      continue;
    }
    printf( "Wrote %s, size %u.\n", tmp );
  }

  if ( !apg_mod_free( &mod ) ) {
    fprintf( stderr, "ERROR: could not free mod\n" );
    return 1;
  }

  printf( "Normal exit\n" );
  return 0;
}
