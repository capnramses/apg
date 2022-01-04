#include "apg_wav.h"
#include <stdio.h>
#include <stdint.h>
#include <math.h>

int main() {
  int n_chans         = 1;
  int sample_rate     = 11025;
  int n_samples       = 1024;
  int bits_per_sample = 16;
  // range of values: -32768 to 32767
  int16_t wav_data[1024];

  // store a sinewave pattern in the file 100 times
  for ( int i = 0; i < 1024; i++ ) {
    float t_fac = i / 1024.0f;
    float vf    = 32767.0f * sinf( t_fac * t_fac * 6.28f * 100.0f );
    wav_data[i] = (int16_t)vf;
  }

  bool result = apg_wav_write( "testout.wav", wav_data, 1024 * sizeof( int16_t ), n_chans, sample_rate, n_samples, bits_per_sample );
  if ( !result ) { return 1; }
  printf( "normal exit\n" );
  return 0;
}
