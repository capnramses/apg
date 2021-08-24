#include "apg_jobs.h"
#ifndef _WIN32
#include <pthread.h>
#endif

/// Description of a job in the job queue.
typedef struct _job_t {
  apg_jobs_work job_func_ptr;
  void* args_ptr;
} _job_t;

/// Thread pool context. Includes queue of work.
struct apg_jobs_pool_t {
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
