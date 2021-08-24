#include "apg_jobs.h"
#include <stdio.h>
#ifndef _WIN32
#include <unistd.h>
#endif

int main() {
	// similar to `lscpu` command, where my machine has 1 socket with 4 cores per socket
	// and 2 threads per core (8 logical, 4 physical).
  int n_procs = sysconf( _SC_NPROCESSORS_ONLN );
  printf( "n_procs = %i\n", n_procs );

  return 0;
}
