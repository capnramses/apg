/** @file main.c
 * Test program for apg_jobs.
 * Only runs on *nix machines since it uses pthread and usleep().
 */

#include "apg_jobs.h"
#ifdef _WIN32
#include <windows.h>
#else
#include <pthread.h>
#include <unistd.h> // usleep
#endif
#include <stdio.h>
#include <stdlib.h>

void apg_sleep_ms( int ms ) {
#ifdef WIN32
  Sleep( ms ); /* NOTE(Anton) may not need this since using gcc on Windows and usleep() works */
#elif _POSIX_C_SOURCE >= 199309L
  struct timespec ts;
  ts.tv_sec  = ms / 1000;
  ts.tv_nsec = ( ms % 1000 ) * 1000000;
  nanosleep( &ts, NULL );
#else
  usleep( ms * 1000 );
#endif
}

void work_cb( void* arg_ptr ) {
  int* val = arg_ptr;
  int old  = *val;
  *val += 1000;

#ifndef _WIN32
  unsigned long pid = pthread_self();
#else
  unsigned long pid = 0;
#endif
  fprintf( stderr, "starting job tid=0x%x, old=%d, val=%d\n", pid, old, *val );
  apg_sleep_ms( rand() % 1000 );
  fprintf( stderr, "ending job tid=0x%x, old=%d, val=%d\n", pid, old, *val );
}

int main() {
  // similar to `lscpu` command, where my machine has 1 socket with 4 cores per socket
  // and 2 threads per core (8 logical, 4 physical).
  int n_procs = apg_jobs_n_logical_procs();
  printf( "n_procs = %i\n", n_procs );

  apg_jobs_pool_t thread_pool;
  int queue_max = 256; // make this really short ( like 2 ) to test queue block/wait behaviour
  bool ret      = apg_jobs_init( &thread_pool, n_procs * 4, queue_max );
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
