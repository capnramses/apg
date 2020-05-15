#include "two.h"
#define APG_IMPLEMENTATION
#include "apg.h"
#include <stdio.h>
#include <stdint.h>

int main( int argc, char** argv ) {
  g_apg_argc = argc;
  g_apg_argv = argv;

  apg_time_init();

  double T    = 0.0;
  double prev = apg_time_s();
  while ( T < 1.0 ) {
    double curr    = apg_time_s();
    double elapsed = curr - prev;
    prev           = curr;
    T += elapsed;
    printf( "T= %lf\n", T );
  }

  apg_start_log();

  int pn = apg_check_param( "hello" );
  printf( "the user param 'hello' was found at arg num %i (-1 indicates not found)\n", pn );
  //if ( pn > 0 ) { return 0; }

  uint32_t uval = APG_MAX( 110, 120 );
  printf( "uval = %u\n", uval );
  apg_log( "the value was %u\n", uval );

  uval = APG_CLAMP( uval, 11, 12 );
  printf( "uval = %u\n", uval );

  apg_log_err( "ERROR: this is a fake error log\n" );

  two();

  printf( "test exited normally\n" );

  return 0;
}
