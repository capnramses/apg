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
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
  /// Array of jobs of length queue_max_items. @warning Memory in array must be accessed inside locked queue_mutex.
  _job_t* queue_ptr;
  /// Number of elements of space allocated in queue_ptr
  int queue_max_items;
  /// Index of the 'front' element in queue_ptr. This will move around as the front of the queue is popped. @warning Must be accessed inside locked queue_mutex.
  int queue_front_idx;
  /// Number of elements in queue_ptr where a job is stored. These can wrap around back past index zero. @warning Must be accessed inside locked queue_mutex.
  int n_queued;

#ifndef _WIN32
  /// Single mutex used for all locking.
  pthread_mutex_t queue_mutex;
  /// When a space is cleared in the queue fire this off to clear a blocked main thread.
  pthread_cond_t space_in_queue_signal;
  /// Signals the threads that there is work to be processed.
  pthread_cond_t job_queued_signal;
  /// Signals when there are no threads processing.
  pthread_cond_t workers_finished_cond;
#endif
  /// Number of threads that are currently working on a job.
  int n_working;
  /// Number of live threads, counting those working and not working.
  int n_threads;
  /// Flag to stop threads.
  bool stop;
};

/** Get the job at the front of the queue and adjust the queue.
 * @warning This function must be called within a locked queue mutex.
 */
static bool _apg_jobs_pop_job( apg_jobs_pool_t* pool_ptr, _job_t* job_ptr ) {
  if ( !pool_ptr || !job_ptr ) { return false; }
  if ( pool_ptr->context_ptr->queue_max_items < 1 ) { return false; }

  bool popped = false;

  if ( pool_ptr->context_ptr->n_queued > 0 ) {
    *job_ptr                               = pool_ptr->context_ptr->queue_ptr[pool_ptr->context_ptr->queue_front_idx];
    pool_ptr->context_ptr->queue_front_idx = ( pool_ptr->context_ptr->queue_front_idx + 1 ) % pool_ptr->context_ptr->queue_max_items;
    pool_ptr->context_ptr->n_queued--;
    popped = true;
    pthread_cond_broadcast( &pool_ptr->context_ptr->space_in_queue_signal );
  }

  return popped;
}

//
//
static void* _worker_thread_func( void* args_ptr ) {
  apg_jobs_pool_t* pool_ptr = args_ptr;
  assert( pool_ptr );

  _job_t job = ( _job_t ){ .args_ptr = NULL };

  while ( true ) {
    {
      pthread_mutex_lock( &pool_ptr->context_ptr->queue_mutex );

      // if we're still running but the queue is empty then wait this thread in a conditional
      while ( pool_ptr->context_ptr->n_queued == 0 && !pool_ptr->context_ptr->stop ) {
        // The cond unlocks the mutex when first called, and re-locks the mutex when signalled and awoken.
        pthread_cond_wait( &pool_ptr->context_ptr->job_queued_signal, &pool_ptr->context_ptr->queue_mutex );
      } // loop just in case a thread was awoken but the queue is empty because e.g. another thread emptied it first or some bad queue state.

      // stop thread if stop flag is raised, and before getting any more work.
      if ( pool_ptr->context_ptr->stop ) {
        pool_ptr->context_ptr->n_threads--;
        // TODO(Anton) this feels like it could cause a problem if a wait() and a stop are combined since it's fired when the _first_ thread has finished.
        // TODO(Anton) use the same if(){} as below to test from n_working == 0 first so only the final thread to stop fires this signal?
        pthread_cond_signal( &pool_ptr->context_ptr->workers_finished_cond ); // signal that no threads are processing
        pthread_mutex_unlock( &pool_ptr->context_ptr->queue_mutex );          // remember to unlock mutex
        break;
      }

      // retrieve next job from queue
      if ( !_apg_jobs_pop_job( pool_ptr, &job ) ) {
        fprintf( stderr, "ERROR: popping job from queue -- we have an unexpected exception/regression\n" );
      } else {
        pool_ptr->context_ptr->n_working++;
      }
      pthread_mutex_unlock( &pool_ptr->context_ptr->queue_mutex );
    }

    // process the job (not mutex locked)
    if ( job.job_func_ptr != NULL ) { job.job_func_ptr( job.args_ptr ); }

    { // end job
      pthread_mutex_lock( &pool_ptr->context_ptr->queue_mutex );

      pool_ptr->context_ptr->n_working--;
      // if no threads are processing anything and there are no more jobs to be done then signal that
      if ( !pool_ptr->context_ptr->stop && pool_ptr->context_ptr->n_working == 0 && pool_ptr->context_ptr->n_queued == 0 ) {
        pthread_cond_signal( &pool_ptr->context_ptr->workers_finished_cond ); // signal that no threads are processing
      }

      pthread_mutex_unlock( &pool_ptr->context_ptr->queue_mutex );
    }
  } // endwhile thread worker continuous loop

  return NULL;
}

//
//
bool apg_jobs_init( apg_jobs_pool_t* pool_ptr, int n_workers, int queue_max_jobs ) {
  if ( !pool_ptr || n_workers < 1 || queue_max_jobs < 1 ) { return false; }

  pool_ptr->context_ptr = calloc( 1, sizeof( apg_jobs_pool_internal_t ) );
  if ( !pool_ptr->context_ptr ) { return false; }

  pool_ptr->context_ptr->queue_max_items = queue_max_jobs;
  pool_ptr->context_ptr->queue_ptr       = calloc( pool_ptr->context_ptr->queue_max_items, sizeof( _job_t ) );
  if ( !pool_ptr->context_ptr->queue_ptr ) {
    free( pool_ptr->context_ptr );
    return false;
  }
  printf( "created queue of length %i elements, size %lu bytes\n", pool_ptr->context_ptr->queue_max_items,
    (unsigned long)pool_ptr->context_ptr->queue_max_items * sizeof( _job_t ) );

  // NB - can use pthread_self() to identify a thread's id integer.
  for ( int i = 0; i < n_workers; i++ ) {
#ifndef _WIN32
    pthread_t thread;
    int ret = pthread_create( &thread, NULL, _worker_thread_func, pool_ptr );
    if ( 0 != ret ) {
      // TODO handle this thread not starting e.g. delete threads up to i.
      return false;
    }
    pool_ptr->context_ptr->n_threads++;
    ret = pthread_detach( thread ); // these clean up on exit
    if ( 0 != ret ) {
      // TODO handle this thread not detaching e.g. close threads up to i.
      return false;
    }
#endif
  }

  return true;
}

//
//
bool apg_jobs_free( apg_jobs_pool_t* pool_ptr ) {
  if ( !pool_ptr || !pool_ptr->context_ptr || !pool_ptr->context_ptr->queue_ptr ) { return false; }

#ifndef _WIN32
  // delete work backlog and signal all threads to stop
  pthread_mutex_lock( &pool_ptr->context_ptr->queue_mutex );
  {
    free( pool_ptr->context_ptr->queue_ptr );
    pool_ptr->context_ptr->queue_ptr = NULL;
    pool_ptr->context_ptr->n_queued  = 0;
    pool_ptr->context_ptr->stop      = true;
    fprintf( stderr, "STOP! flag raised\n" );
    // wake up all threads waiting for a job to be queued so they can see the stop flag is raised.
    pthread_cond_broadcast( &pool_ptr->context_ptr->job_queued_signal );
  }
  pthread_mutex_unlock( &pool_ptr->context_ptr->queue_mutex );
#endif

  // wait for any threads that were already processing
  apg_jobs_wait( pool_ptr );

#ifndef _WIN32
  pthread_mutex_destroy( &pool_ptr->context_ptr->queue_mutex );
  pthread_cond_destroy( &pool_ptr->context_ptr->job_queued_signal );
  pthread_cond_destroy( &pool_ptr->context_ptr->workers_finished_cond );
#endif

  free( pool_ptr->context_ptr );
  pool_ptr->context_ptr = NULL;

  return true;
}

//
//
bool apg_jobs_push_job( apg_jobs_pool_t* pool_ptr, apg_jobs_work job_func_ptr, void* args_ptr ) {
  if ( !pool_ptr || !job_func_ptr ) { return false; }

  bool pushed = false;

  // space_in_queue_signal

  pthread_mutex_lock( &pool_ptr->context_ptr->queue_mutex );
  {
    // queue full
    // block and wait here if there is no space in the queue
    if ( pool_ptr->context_ptr->n_queued >= pool_ptr->context_ptr->queue_max_items ) {
      // The cond unlocks the mutex when first called, and re-locks the mutex when signalled and awoken.
      pthread_cond_wait( &pool_ptr->context_ptr->space_in_queue_signal, &pool_ptr->context_ptr->queue_mutex );
    }
    { // push to end of queue
      int end_idx = ( pool_ptr->context_ptr->queue_front_idx + pool_ptr->context_ptr->n_queued ) % pool_ptr->context_ptr->queue_max_items;
      assert( end_idx >= 0 && end_idx < pool_ptr->context_ptr->queue_max_items );
      pool_ptr->context_ptr->queue_ptr[end_idx].args_ptr     = args_ptr;
      pool_ptr->context_ptr->queue_ptr[end_idx].job_func_ptr = job_func_ptr;
      pool_ptr->context_ptr->n_queued++;
      // wake up all threads waiting for a job to be queued.
      pthread_cond_broadcast( &pool_ptr->context_ptr->job_queued_signal );
      pushed = true;
    }
  }
  pthread_mutex_unlock( &pool_ptr->context_ptr->queue_mutex );

  return pushed;
}

//
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

//
// TODO(Anton) revise
void apg_jobs_wait( apg_jobs_pool_t* pool_ptr ) {
  if ( !pool_ptr ) { return; }

#ifndef _WIN32
  pthread_mutex_lock( &pool_ptr->context_ptr->queue_mutex );
  while ( true ) { // this loops in case any thread woke up after the wait call.
    if ( ( !pool_ptr->context_ptr->stop && pool_ptr->context_ptr->n_working != 0 ) || ( pool_ptr->context_ptr->stop && pool_ptr->context_ptr->n_threads != 0 ) ) {
      // NOTE(Anton) this signal can be fired during a 'stop' event when /the first/ thread has finished - the others may still be processing work.
      pthread_cond_wait( &pool_ptr->context_ptr->workers_finished_cond, &pool_ptr->context_ptr->queue_mutex ); // wait for signal that no threads are processing
    } else {
      break;
    }
  }
  pthread_mutex_unlock( &pool_ptr->context_ptr->queue_mutex );
#endif
}
