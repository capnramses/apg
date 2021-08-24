#include "apg_jobs.h"
#include <stdio.h>

int main() {
  // similar to `lscpu` command, where my machine has 1 socket with 4 cores per socket
  // and 2 threads per core (8 logical, 4 physical).
  int n_procs = apg_jobs_n_logical_procs();
  printf( "n_procs = %i\n", n_procs );

  apg_jobs_pool_t thread_pool;
  bool ret = apg_jobs_init( &thread_pool, n_procs + 1 );
  if ( !ret ) {
    fprintf( stderr, "ERROR: failed to init pool\n" );
    return 1;
  }

  ret = apg_jobs_free( &thread_pool );
  if ( !ret ) {
    fprintf( stderr, "ERROR: failed to free pool\n" );
    return 1;
  }

  printf( "normal halt\n" );
  return 0;
}
