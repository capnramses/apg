#include "apg_maths.h"
#include <stdio.h>
#include <string.h> // memset

int main() {
  printf( "testing C99 version of maths library\n" );

  // vec2 v2, v2b;
  vec3 v3, v3b;
  //vec4 v4, v4b;
  mat4 m4;//m4b;
  //versor q, qb;

  // constructors and assignment
  printf( "* zero_mat4\n" );
  memset( &m4, 0, sizeof( mat4 ) );
  print_mat4( m4 );
  printf( "* identity_mat4\n" );
  m4 = identity_mat4();
  print_mat4( m4 );

  v3  = ( vec3 ){.x = 0, .y = 0, .z = 1};
  v3b = ( vec3 ){.x = 0, .y = 0, .z = 2};

  printf( "* add_vec3_vec3\n" );
  v3 = add_vec3_vec3( v3, v3b );
  print_vec3( v3 );
  printf( "* sub_vec3_vec3\n" );
  v3 = sub_vec3_vec3( v3, v3b );
  print_vec3( v3 );

  mat4 la = look_at( ( vec3 ){.x = 0.0, .y = 7.0f, .z = 0.0}, ( vec3 ){.x = 0.0f, .y = 0.0f, .z = 0.0f}, ( vec3 ){.x = 0, .y = 0, .z = -1} );
  print_mat4( la );

  // add_vec3_f
  // sub_vec3_f
  // mult_vec3_f
  // div_vec3_f
  // mult_vec3_vec3
  // div_vec3_vec3

  // length_vec3
  // length2_vec3

  return 0;
}
