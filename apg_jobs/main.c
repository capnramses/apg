#include "apg_jobs.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> // usleep

void work_cb( void* arg_ptr ) {
  int* val = arg_ptr;
  int old  = *val;
  *val += 1000;
  unsigned long pid = pthread_self();
  fprintf( stderr, "starting job tid=%p, old=%d, val=%d\n", (void*)pid, old, *val );
  usleep( rand() % 10000000 );
  fprintf( stderr, "ending job tid=%p, old=%d, val=%d\n", (void*)pid, old, *val );
  // if ( *val % 2 ) { usleep( 100000 ); }
}

int main() {
  // similar to `lscpu` command, where my machine has 1 socket with 4 cores per socket
  // and 2 threads per core (8 logical, 4 physical).
  int n_procs = apg_jobs_n_logical_procs();
  printf( "n_procs = %i\n", n_procs );

  apg_jobs_pool_t thread_pool;
  int queue_max = 256;
  bool ret      = apg_jobs_init( &thread_pool, n_procs + 1, queue_max );
  if ( !ret ) {
    fprintf( stderr, "ERROR: failed to init pool\n" );
    return 1;
  }

  int num_items = 100;
  int* vals     = malloc( sizeof( int ) * num_items );
  for ( int i = 0; i < num_items; i++ ) {
    vals[i] = i;
    apg_jobs_push_job( &thread_pool, work_cb, &vals[i] );
  }

  apg_jobs_wait( &thread_pool );

  for ( int i = 0; i < num_items; i++ ) { printf( "%d\n", vals[i] ); }

  ret = apg_jobs_free( &thread_pool );
  if ( !ret ) {
    fprintf( stderr, "ERROR: failed to free pool\n" );
    return 1;
  }

  free( vals );
  printf( "normal halt\n" );
  return 0;
}
