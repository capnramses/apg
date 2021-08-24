/** @file apg_jobs.h
 * apg_jobs is a simple jobs/worker system using a thread pool.
 *
 * apg_jobs  | Threaded jobs/worker library.
 * --------- | ----------
 * Version   | 0.1  (2021/08/24)
 * Authors   | Anton Gerdelan https://github.com/capnramses
 * Copyright | 2021, Anton Gerdelan
 * Language  | C99
 * Files     | 2
 * Licence   | TBD
 *
 * The goal is something as simple as possible plus some telemetry hooks.
 * The original use case is for processing path-finding AI in a voxel game.
 *
 * TODO:
 * - windows MSVC batch file build.
 * - test programs
 * - real time? chart output similar to Remotery
 */

#pragma once

#ifdef _WIN32
#define APG_JOBS_EXPORT __declspec( dllexport )
#else
#define APG_JOBS_EXPORT
#endif

#include <stdbool.h>

/// The main context struct for this library. Create one of these on you main thread.
APG_JOBS_EXPORT typedef struct apg_jobs_pool_t apg_jobs_pool_t;

/// All jobs for workers are defined as a function of this format.
APG_JOBS_EXPORT typedef void ( *apg_jobs_work )( void* args_ptr );

/** Start the jobs system and its threads.
 * @param pool_ptr  The pool pointed to will be initialised by this function. Must not be NULL.
 * @param n_workers The number of worker threads to create. A good number is 1 per logical core on the machine. Must not be NULL.
 * @return          False on error.
 * @note            To query the number of cores available see https://stackoverflow.com/questions/150355/programmatically-find-the-number-of-cores-on-a-machine.
 */
APG_JOBS_EXPORT bool apg_jobs_init( apg_jobs_pool_t* pool_ptr, int n_workers );

/** Stop the jobs system and stop its threads.
 * @param pool_ptr  Pointer to the thread pool to shut down. Must not be NULL.
 * @return          False on error.
 */
APG_JOBS_EXPORT bool apg_jobs_free( apg_jobs_pool_t* pool_ptr );

/** Add a job to the work queue.
 * @param pool_ptr     Pointer to the thread pool to use. Must not be NULL.
 * @param job_func_ptr Function comprising the job to execute.
 * @param args_ptr     ... TODO
 * @returns            ... TODO -- maybe block if the queue is full?
 */
APG_JOBS_EXPORT bool apg_jobs_push_job( apg_jobs_pool_t* pool_ptr, apg_jobs_work job_func_ptr, void* args_ptr );

/** Block until all the work in the queue is completed.
 * @param pool_ptr     Pointer to the thread pool to use. Must not be NULL.
 */
APG_JOBS_EXPORT void apg_jobs_wait( apg_jobs_pool_t* pool_ptr );
