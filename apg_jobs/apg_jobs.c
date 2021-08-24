/** @file apg_jobs.c
 * apg_jobs is a simple jobs/worker system using a thread pool.
 *
 * apg_jobs  | Threaded jobs/worker library.
 * --------- | ----------
 * Version   | 0.1  (2021/08/24)
 * Authors   | Anton Gerdelan https://github.com/capnramses
 * Language  | C99
 * Files     | 2
 * Licence   | See header file.
 */
#include "apg_jobs.h"
#include <stdlib.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <pthread.h>
#endif

/// Description of a job in the job queue.
typedef struct _job_t {
  /// Function representing the job that is called by the worker thread.
  apg_jobs_work job_func_ptr;
  /// Arguments to the function.
  void* args_ptr; // TODO(Anton) -- is a pointer really actually for realsies safe here to be accessed from a thread.
} _job_t;

/// Thread pool context. Includes queue of work.
struct apg_jobs_pool_internal_t {
  _job_t* queue_ptr;
  int queue_max_items;
  int queue_front_idx;
  int n_queued;

#ifndef _WIN32
  /// Single mutex used for all locking.
  pthread_mutex_t work_mutex;
  /// Signals the threads that there is work to be processed.
  pthread_cond_t work_cond;
  /// Signals when there are no threads processing.
  pthread_cond_t working_cond;
#endif
  /// Combines signals above to determine how many threads are processing work.
  int n_working;
  /// Number of live threads, counting those working and not working.
  int n_threads;
  /// Flag to stop threads.
  bool stop;
};

static void* _worker_thread_func( void* args_ptr ) { return NULL; }

// Further examples: https://stackoverflow.com/questions/150355/programmatically-find-the-number-of-cores-on-a-machine
unsigned int apg_jobs_n_logical_procs() {
#ifdef _WIN32
  SYSTEM_INFO sys_info;
  GetSystemInfo( &sys_info );
  return sys_info.dwNumberOfProcessors;
#else
  return (unsigned int)sysconf( _SC_NPROCESSORS_ONLN );
#endif
}

bool apg_jobs_init( apg_jobs_pool_t* pool_ptr, int n_workers ) {
  if ( !pool_ptr || n_workers < 1 ) { return false; }

  pool_ptr->context_ptr = calloc( 1, sizeof( apg_jobs_pool_internal_t ) );
  if ( !pool_ptr->context_ptr ) { return false; }

  for ( int i = 0; i < n_workers; i++ ) {
#ifndef _WIN32
    pthread_t thread;
    int ret = pthread_create( &thread, NULL, _worker_thread_func, pool_ptr );
    if ( 0 != ret ) {
      // TODO handle this thread not starting e.g. close threads up to i.
      return false;
    }
    ret = pthread_detach( thread ); // these clean up on exit
    if ( 0 != ret ) {
      // TODO handle this thread not detaching e.g. close threads up to i.
      return false;
    }
#endif
  }

  return true;
}

bool apg_jobs_free( apg_jobs_pool_t* pool_ptr ) {
  if ( !pool_ptr || !pool_ptr->context_ptr ) { return false; }

  //

  free( pool_ptr->context_ptr );
  pool_ptr->context_ptr = NULL;

  return true;
}
