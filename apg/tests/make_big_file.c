// Simple program to generate a multi-GB file to use for testing.

#include <stdio.h>
#include <stdlib.h>

int main() {
  FILE* f_ptr = fopen( "bigfile.dat", "wb" );
  if ( !f_ptr ) {
    fprintf( stderr, "ERROR opening file for writing\n" );
    return 1;
  }
  int n_gbs = 6;
  // Allocate 1GB
  size_t sz = 1024ULL * 1024ULL * 1024ULL;
  void* ptr = malloc( sz );
  if ( !ptr ) {
    fprintf( stderr, "ERROR OOM\n" );
    return 1;
  }
  // Write in smaller stages so Windows doesn't choke on an internal 2GB limit.
  for ( int i = 0; i < n_gbs; i++ ) {
    fprintf( stdout, "Writing %zu bytes... %i/%i\n", sz, i + 1, n_gbs );
    size_t out = fwrite( ptr, sz, 1, f_ptr );
    if ( 1 != out ) {
      fprintf( stderr, "ERROR writing file\n" );
      return 1;
    }
  }
  fclose( f_ptr );

  return 0;
}
