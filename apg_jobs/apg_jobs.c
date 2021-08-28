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
#include <time.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <pthread.h>
#endif

/* The follow ifdef section is from a wrapper of pthread for Windows by John Schember:
https://nachtimwald.com/2019/04/05/cross-platform-thread-wrapper/
NOTE(Anton) It's tidier than my earlier multi-platform code that used containers/abstractions -
that added a layer of data structure stuff and custom calls - it's nicer for the brain to just work
with pthread directly.
*/
#ifdef _WIN32

typedef CRITICAL_SECTION pthread_mutex_t;
typedef void pthread_mutexattr_t;
typedef void pthread_attr_t;
typedef void pthread_condattr_t;
typedef void pthread_rwlockattr_t;
typedef HANDLE pthread_t;
typedef CONDITION_VARIABLE pthread_cond_t;

/** Timing used for timed conditionals.
struct timespec {
  long tv_sec;
  long tv_nsec;
}; */

/** pthread_cond_timedwait takes a struct timespec but SleepConditionVariableCS takes a DWORD of ms. */
static DWORD timespec_to_ms( const struct timespec* abstime ) {
  if ( abstime == NULL ) { return INFINITE; }
  DWORD t = (DWORD)( ( ( abstime->tv_sec - time( NULL ) ) * 1000 ) + ( abstime->tv_nsec / 1000000 ) );
  return t;
}

/** Timing can be used for conditionals. */
void ms_to_timespec( struct timespec* ts, unsigned int ms ) {
  if ( ts == NULL ) { return; }
  ts->tv_sec  = ( ms / 1000 ) + time( NULL );
  ts->tv_nsec = ( ms % 1000 ) * 1000000;
}

/** NOTE(Anton) I did the same thing last time but with more ugly casting. */
int pthread_create( pthread_t* thread, pthread_attr_t* attr, void* ( *start_routine )(void*), void* arg ) {
  (void)attr;
  if ( thread == NULL || start_routine == NULL ) { return 1; }
  // NOTE(Anton) this ugly cast is so i can use the same routine for both platforms
  long unsigned int ( *routine_wrapper )( void* ) = (long unsigned int ( * )( void* ))start_routine;
  *thread                                         = CreateThread( NULL, 0, routine_wrapper, arg, 0, NULL );
  if ( *thread == NULL ) return 1;
  return 0;
}

/** "For join we will wait for the thread to stop (blocking) and destroy it."
 * NOTE(Anton) I did the same thing last time.
 */
int pthread_join( pthread_t thread, void** value_ptr ) {
  (void)value_ptr;
  WaitForSingleObject( thread, INFINITE );
  CloseHandle( thread );
  return 0;
}

/** "Detach is a bit odd because it looks like we’re destroying the thread.
 * This is partly true this will cause the thread to be cleaned up but CloseHandle does not stop the thread.
 * It will keep running without interruption (and without having this function block)
 * and once finished be cleaned up."
 */
int pthread_detach( pthread_t thread ) {
  CloseHandle( thread );
  return 0;
}

/** "Mutexes on Windows are known as Critical Sections and we can directly wrap them."
 * NOTE(Anton) in an earlier wrapper I used CreateMutex() etc.
 */
int pthread_mutex_init( pthread_mutex_t* mutex, pthread_mutexattr_t* attr ) {
  (void)attr;
  if ( mutex == NULL ) { return 1; }
  InitializeCriticalSection( mutex );
  return 0;
}

/** NOTE(Anton) last time I used CloseHandle() on my mutex created with CreateMutex(). */
int pthread_mutex_destroy( pthread_mutex_t* mutex ) {
  if ( mutex == NULL ) { return 1; }
  DeleteCriticalSection( mutex );
  return 0;
}

/** NOTE(Anton) last time I used WaitForSingleObject() on my mutex created with CreateMutex(). */
int pthread_mutex_lock( pthread_mutex_t* mutex ) {
  if ( mutex == NULL ) { return 1; }
  EnterCriticalSection( mutex );
  return 0;
}

/** NOTE(Anton) last time I used ReleaseMutex() on my mutex created with CreateMutex(). */
int pthread_mutex_unlock( pthread_mutex_t* mutex ) {
  if ( mutex == NULL ) { return 1; }
  LeaveCriticalSection( mutex );
  return 0;
}

int pthread_cond_init( pthread_cond_t* cond, pthread_condattr_t* attr ) {
  (void)attr;
  if ( cond == NULL ) { return 1; }
  InitializeConditionVariable( cond );
  return 0;
}

/** "Windows does not have a destroy for conditionals." */
int pthread_cond_destroy( pthread_cond_t* cond ) {
  (void)cond;
  return 0;
}

int pthread_cond_timedwait( pthread_cond_t* cond, pthread_mutex_t* mutex, const struct timespec* abstime ) {
  if ( cond == NULL || mutex == NULL ) { return 1; }
  if ( !SleepConditionVariableCS( cond, mutex, timespec_to_ms( abstime ) ) ) { return 1; }
  return 0;
}

int pthread_cond_wait( pthread_cond_t* cond, pthread_mutex_t* mutex ) {
  if ( cond == NULL || mutex == NULL ) { return 1; }
  return pthread_cond_timedwait( cond, mutex, NULL );
}

int pthread_cond_signal( pthread_cond_t* cond ) {
  if ( cond == NULL ) { return 1; }
  WakeConditionVariable( cond );
  return 0;
}

int pthread_cond_broadcast( pthread_cond_t* cond ) {
  if ( cond == NULL ) { return 1; }
  WakeAllConditionVariable( cond );
  return 0;
}

// NOTE(Anton) I left out the pthread_rwlock...() stuff because the impl relied on a C++ Windows class.
// https://docs.microsoft.com/en-us/cpp/cppcx/wrl/srwlock-class?view=msvc-160
#endif

/// Description of a job in the job queue.
typedef struct _job_t {
  /// Function representing the job that is called by the worker thread.
  apg_jobs_work job_func_ptr;
  /// Arguments to the function. Note that this is not mutex-protected between threads.
  void* args_ptr;
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

  /// Single mutex used for all locking.
  pthread_mutex_t queue_mutex;
  /// When a space is cleared in the queue fire this off to clear a blocked main thread.
  pthread_cond_t space_in_queue_signal;
  /// Signals the threads that there is work to be processed.
  pthread_cond_t job_queued_signal;
  /// Signals when there are no threads processing.
  pthread_cond_t workers_finished_cond;

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
    // wake all threads waiting to push a job (usually it's just one - the main thread)
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

  pthread_mutex_init( &pool_ptr->context_ptr->queue_mutex, NULL );
  pthread_cond_init( &pool_ptr->context_ptr->job_queued_signal, NULL );
  pthread_cond_init( &pool_ptr->context_ptr->workers_finished_cond, NULL );

  // NB - can use pthread_self() to identify a thread's id integer.
  for ( int i = 0; i < n_workers; i++ ) {
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
  }

  return true;
}

//
//
bool apg_jobs_free( apg_jobs_pool_t* pool_ptr ) {
  if ( !pool_ptr || !pool_ptr->context_ptr || !pool_ptr->context_ptr->queue_ptr ) { return false; }

  // delete work backlog and signal all threads to stop
  pthread_mutex_lock( &pool_ptr->context_ptr->queue_mutex );
  {
    free( pool_ptr->context_ptr->queue_ptr );
    pool_ptr->context_ptr->queue_ptr = NULL;
    pool_ptr->context_ptr->n_queued  = 0;
    pool_ptr->context_ptr->stop      = true;
    // wake up all threads waiting for a job to be queued so they can see the stop flag is raised.
    pthread_cond_broadcast( &pool_ptr->context_ptr->job_queued_signal );
  }
  pthread_mutex_unlock( &pool_ptr->context_ptr->queue_mutex );

  // wait for any threads that were already processing
  apg_jobs_wait( pool_ptr );

  pthread_mutex_destroy( &pool_ptr->context_ptr->queue_mutex );
  pthread_cond_destroy( &pool_ptr->context_ptr->job_queued_signal );
  pthread_cond_destroy( &pool_ptr->context_ptr->workers_finished_cond );

  free( pool_ptr->context_ptr );
  pool_ptr->context_ptr = NULL;

  return true;
}

//
//
bool apg_jobs_push_job( apg_jobs_pool_t* pool_ptr, apg_jobs_work job_func_ptr, void* args_ptr ) {
  if ( !pool_ptr || !job_func_ptr ) { return false; }

  bool pushed = false;

  pthread_mutex_lock( &pool_ptr->context_ptr->queue_mutex );
  {
    // queue full
    // block and wait here if there is no space in the queue
    while ( pool_ptr->context_ptr->n_queued >= pool_ptr->context_ptr->queue_max_items ) {
      // The cond unlocks the mutex when first called, and re-locks the mutex when signalled and awoken.
      pthread_cond_wait( &pool_ptr->context_ptr->space_in_queue_signal, &pool_ptr->context_ptr->queue_mutex );
    } // loop just in case a thread was awoken but the queue is empty because e.g. another thread emptied it first or some bad queue state.

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

/** Further OS examples:
 * https://stackoverflow.com/questions/150355/programmatically-find-the-number-of-cores-on-a-machine
 */
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
}
