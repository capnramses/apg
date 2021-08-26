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
 * Licence   | See bottom of this file.
 *
 * The goal is something as simple as possible plus some telemetry hooks.
 * The original use case is for processing path-finding AI in a voxel game.
 *
 * For a good explanation of simple thread pools in C, see John Schember's work at:
 * https://nachtimwald.com/2019/04/12/thread-pool-in-c/
 * Which uses detached threads and signals, which I'm also adopting here because it is
 * simpler than my previous library. apg_jobs uses an up-front allocated queue instead of the original
 * linked list, and a wait condition when pushing to a full array.
 * For a more sophisticated task scheduler library example, see Doug Binks' enkiTS:
 * https://github.com/dougbinks/enkiTSExamples
 *
 * TODO
 * ----
 * - Test on OS X & with .dylibs.
 * - Real time? thread usage timeline chart output hooks similar to or using Remotery.
 */

#pragma once

#ifdef _WIN32
#define APG_JOBS_EXPORT __declspec( dllexport )
#else
#define APG_JOBS_EXPORT
#endif

#include <stdbool.h>

/** Forward-declaration of internal-use context struct (used internally). */
APG_JOBS_EXPORT typedef struct apg_jobs_pool_internal_t apg_jobs_pool_internal_t;

/** The main context struct for this library. Instantiate one of these on you main thread. */
APG_JOBS_EXPORT typedef struct apg_jobs_pool_t { apg_jobs_pool_internal_t* context_ptr; } apg_jobs_pool_t;

/** All jobs for workers are defined as a function of this format. */
typedef void ( *apg_jobs_work )( void* args_ptr );

/** @return The number of logical processors on the system. */
APG_JOBS_EXPORT unsigned int apg_jobs_n_logical_procs();

/** Start the jobs system and its threads.
 * @param pool_ptr       The pool pointed to will be initialised by this function. Must not be NULL.
 * @param n_workers      The number of worker threads to create. In the test program my optimal processing time is with ~4x the number of logical cores.
 * @param queue_max_jobs Size reserved in the queue. Allocates about 16 bytes per job. Must not be 0.
 * @return               False on any error or invalid argument value.
 * @note                 To query the number of cores use `apg_jobs_count_logical_procs()`.
 * @note                 This function allocates heap memory internally, which is freed with a call to apg_jobs_free().
 */
APG_JOBS_EXPORT bool apg_jobs_init( apg_jobs_pool_t* pool_ptr, int n_workers, int queue_max_jobs );

/** Stop the jobs system and stop its threads, and free memory allocated by apg_jobs_init().
 * @param pool_ptr  Pointer to the thread pool to shut down. Must not be NULL.
 * @return          False on any error.
 */
APG_JOBS_EXPORT bool apg_jobs_free( apg_jobs_pool_t* pool_ptr );

/** Add a job to the work queue. A worker thread will pick this up eventually and call your function with your argument.
 * @param pool_ptr     Pointer to the thread pool to use. Must not be NULL.
 * @param job_func_ptr A pointer to your function to execute as the 'job'.
 * @param args_ptr     Any arguments you want to pass on as the argument of job_func_ptr.
 * @returns            False on any error.
 * @note               If there is no space left in the queue then this function will block and
 *                     wait for a space to be freed as a job is popped by a worker thread.
 */
APG_JOBS_EXPORT bool apg_jobs_push_job( apg_jobs_pool_t* pool_ptr, apg_jobs_work job_func_ptr, void* args_ptr );

/** Block the calling thread until all the work in the queue is completed.
 * @param pool_ptr     Pointer to the thread pool to use. Must not be NULL.
 */
APG_JOBS_EXPORT void apg_jobs_wait( apg_jobs_pool_t* pool_ptr );

/*
-------------------------------------------------------------------------------------
This software is available under two licences - you may use it under either licence.
-------------------------------------------------------------------------------------
FIRST LICENCE OPTION

>                                  Apache License
>                            Version 2.0, January 2004
>                         http://www.apache.org/licenses/
>    Copyright 2019 Anton Gerdelan.
>    Licensed under the Apache License, Version 2.0 (the "License");
>    you may not use this file except in compliance with the License.
>    You may obtain a copy of the License at
>        http://www.apache.org/licenses/LICENSE-2.0
>    Unless required by applicable law or agreed to in writing, software
>    distributed under the License is distributed on an "AS IS" BASIS,
>    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
>    See the License for the specific language governing permissions and
>    limitations under the License.
-------------------------------------------------------------------------------------
SECOND LICENCE OPTION

> This is free and unencumbered software released into the public domain.
>
> Anyone is free to copy, modify, publish, use, compile, sell, or
> distribute this software, either in source code form or as a compiled
> binary, for any purpose, commercial or non-commercial, and by any
> means.
>
> In jurisdictions that recognize copyright laws, the author or authors
> of this software dedicate any and all copyright interest in the
> software to the public domain. We make this dedication for the benefit
> of the public at large and to the detriment of our heirs and
> successors. We intend this dedication to be an overt act of
> relinquishment in perpetuity of all present and future rights to this
> software under copyright law.
>
> THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
> EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
> MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
> IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
> OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
> ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
> OTHER DEALINGS IN THE SOFTWARE.
>
> For more information, please refer to <http://unlicense.org>
-------------------------------------------------------------------------------------

Some of the work here is based on or draws inspiration from John Schember's article
https://nachtimwald.com/2019/04/12/thread-pool-in-c/
The website states any code not otherwise-specified is licenced under MIT:

> Copyright John Schember <john@nachtimwald.com>
>
> Permission is hereby granted, free of charge, to any person obtaining a copy of
> this software and associated documentation files (the "Software"), to deal in
> the Software without restriction, including without limitation the rights to
> use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
> of the Software, and to permit persons to whom the Software is furnished to do
> so, subject to the following conditions:
>
> The above copyright notice and this permission notice shall be included in all
> copies or substantial portions of the Software.
>
> THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
> IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
> FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
> AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
> LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
> OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
> SOFTWARE.
*/
