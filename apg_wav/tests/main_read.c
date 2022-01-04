#include "apg_wav.h"
#include <stdio.h>
#include <stdint.h>
#include <math.h>

int main( int argc, char** argv ) {
  if ( argc < 2 ) {
    printf( "usage: ./read FILE.wav\n" );
    return 1;
  }
  apg_wav_t wav_data = ( apg_wav_t ){ .file_data_ptr = NULL };
  bool res           = apg_wav_read( argv[1], &wav_data );
  if ( !res ) { return 1; }

  printf(
    "wav_data:\n  file_sz = \t\t%i\n  fmt_sz = \t\t%i\n  fmt_type = \t\t%i\n  n_chans = \t\t%i\n  sample_rate_hz = \t%i\n  byte_rate = \t\t%i\n  block_align = "
    "\t%i\n  bits_per_sample = \t%i\n  data_sz =\t\t%i\n",
    wav_data.header_ptr->file_sz, wav_data.header_ptr->fmt_sz, wav_data.header_ptr->fmt_type, wav_data.header_ptr->n_chans, wav_data.header_ptr->sample_rate_hz,
    wav_data.header_ptr->byte_rate, wav_data.header_ptr->block_align, wav_data.header_ptr->bits_per_sample, wav_data.header_ptr->data_sz );

  res = apg_wav_free( &wav_data );
  if ( !res ) { return 1; }

  printf( "normal exit\n" );
  return 0;
}
