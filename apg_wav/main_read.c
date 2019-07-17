#include "apg_wav.h"
#include <stdio.h>
#include <stdint.h>
#include <math.h>

int main( int argc, char** argv ) {
  if ( argc < 2 ) {
    printf( "usage: ./read FILE.wav\n" );
    return 1;
  }
  int n_chans         = 0;
  int sample_rate     = 0;
  int n_samples       = 0;
  int bits_per_sample = 0;

  unsigned char* wav_data = apg_read_wav( argv[1], &n_chans, &sample_rate, &n_samples, &bits_per_sample );
  if ( !wav_data ) { return 1; }
  return 0;
}
