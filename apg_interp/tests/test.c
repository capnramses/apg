#include "apg_interp.h"
#include <stdio.h>
#include <time.h>
#include <unistd.h> // unix sleep
#include <stdbool.h>

int main() {
  printf( "hello world\n" );
  time_t prev_time, curr_time;
  time( &prev_time );
  double x = 0.0;
  while ( true ) {
    time( &curr_time );
    double delta_s = difftime( curr_time, prev_time );
    prev_time      = curr_time;
    x += delta_s;
    if ( x > 20.0 ) { x = 0.0; }
    printf( "x = %f\n", x );

    float y;
    y = apg_interp_lerp( x, 0.0f, 20.0f, 0.0f, 1.0f );
    printf( "lerp = %f\n", y );
    y = apg_interp_accel_sine( x, 0.0f, 20.0f, 0.0f, 1.0f );
    printf( "accel_sine = %f\n", y );
    y = apg_interp_accel_exp( x, 0.0f, 20.0f, 0.0f, 1.0f );
    printf( "accel_exp = %f\n", y );
    y = apg_interp_bow_string( x, 0.0f, 20.0f, 0.0f, 1.0f );
    printf( "bow_string = %f\n", y );
    y = apg_interp_decel_sine( x, 0.0f, 20.0f, 0.0f, 1.0f );
    printf( "decel_sine = %f\n", y );
    y = apg_interp_decel_elastic( x, 0.0f, 20.0f, 0.0f, 1.0f );
    printf( "decel_elastic = %f\n", y );
    y = apg_interp_decel_bounce( x, 0.0f, 20.0f, 0.0f, 1.0f );
    printf( "decel_bounce = %f\n", y );

    sleep( 1 ); // 1 second
  }
  return 0;
}
