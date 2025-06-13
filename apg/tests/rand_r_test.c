#define APG_IMPLEMENTATION
#define APG_NO_BACKTRACES
#include "apg.h"
#include <stdio.h>
#include <time.h>

int main( void ) {
  apg_rand_t initial_seed = time( NULL ); // Equivalent to `srand( time( NULL ) );`.
  apg_rand_t working_seed = initial_seed; // In case we want to remember the original sequence start.
  printf( "Initial seed = %lu\n", initial_seed );
  for ( int i = 0; i < 10; i++ ) { printf( "apg_rand_r returned %i\n", apg_rand_r( &working_seed ) ); }
  for ( int i = 0; i < 10; i++ ) { printf( "apg_randf_r returned %f\n", apg_randf_r( &working_seed ) ); }
  printf( "Working seed at exit = %lu\n", working_seed );

  // These results should match the ones given by the above.
  apg_srand( initial_seed );
  for ( int i = 0; i < 10; i++ ) { printf( "apg_rand returned %i\n", apg_rand() ); }
  for ( int i = 0; i < 10; i++ ) { printf( "apg_randf returned %f\n", apg_randf() ); }

  return 0;
}
