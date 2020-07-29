#define APG_IMPLEMENTATION
#include "apg.h"
#include <stdio.h>
#include <string.h>

int main() {
  const char* input_buffer  = "ABBCCCDDDDEEEEEFFFFFFGGGGGGGHHHHHHHHIIIIIIIII";
  char* decompressed_buffer = NULL;

  printf( "-------------compression-----------\n" );
  size_t input_sz      = strlen( input_buffer ) + 1;
  size_t compressed_sz = 0;
  apg_rle_compress( input_buffer, input_sz, NULL, &compressed_sz ); // +1 to include the \0
  char* compressed_buffer = malloc( compressed_sz );
  apg_rle_compress( input_buffer, input_sz, compressed_buffer, &compressed_sz ); // +1 to include the \0
  printf( "input str: %s\noutput str: %s\n", input_buffer, compressed_buffer );
  float pc = ( (float)compressed_sz / (float)input_sz ) * 100.0f;
  printf( "input len=%u\noutput len=%u\n%.2f%% of original size\n", input_sz, compressed_sz, pc );

  printf( "-------------and decompression-----------\n" );
  {
    size_t decompressed_sz = 0;
    apg_rle_decompress( compressed_buffer, compressed_sz, NULL, &decompressed_sz );
    printf( "decompressed_sz=%u\n", decompressed_sz );
    decompressed_buffer = malloc( decompressed_sz );
    apg_rle_decompress( compressed_buffer, compressed_sz, decompressed_buffer, &decompressed_sz );
    printf( "decompressed buffer: %s\n", decompressed_buffer );

    assert( strcmp( decompressed_buffer, input_buffer ) == 0 );
    assert( decompressed_sz == input_sz );

    free( decompressed_buffer );
  }

  free( compressed_buffer );
  return 0;
}