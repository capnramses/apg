// Test program to check if multi-GB files are read and addressed correctly on a particular platform.

#define APG_NO_BACKTRACES
#define APG_IMPLEMENTATION
#include "../apg.h"

int main() {
  apg_time_init();

  // Test file size independently first.
  int64_t sz = apg_file_size( "bigfile.dat" );
  printf( "  apg_file_size() reports %lli bytes.\n", sz );

  double start = apg_time_s();

  apg_file_t record = ( apg_file_t ){ .sz = 0 };
  if ( !apg_read_entire_file( "bigfile.dat", &record ) ) {
    fprintf( stderr, "ERROR: reading file \n" );
    return 1;
  }

  double end = apg_time_s() - start;

  printf( "File size as long int is % li\n     size as size_t is\t  %zu\nSize of long is\t\t  %zu\nSize of off_t is\t  %zu\n", (long int)record.sz, record.sz,
    sizeof( long ), sizeof( off_t ) );
  printf( "File read of size         %zu was successful.\n", record.sz );
  
  printf("File read took %lf seconds.\n", end );
  return 0;
}
