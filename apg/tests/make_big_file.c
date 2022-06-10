// Simple program to generate a multi-GB file to use for testing.

#include <stdio.h>
#include <stdlib.h>

int main() {
  FILE* f_ptr = fopen( "bigfile.dat", "wb" );
  if ( !f_ptr ) {
    fprintf( stderr, "ERROR opening file for writing\n" );
    return 1;
  }
  size_t sz = 1024ULL * 1024ULL * 1024ULL * 6ULL;
  void* ptr = malloc( sz );
  int out   = fwrite( ptr, sz, 1, f_ptr );
  fclose( f_ptr );
  if ( 1 != out ) {
    fprintf( stderr, "ERROR writing file\n" );
    return 1;
  }

  return 0;
}
