#include "apg_maths.h"
#include <stdio.h>
#include <string.h> // memset

int main() {
  printf( "testing C99 version of maths library\n" );

  // vec2 v2, v2b;
  vec3 v3, v3b;
  // vec4 v4, v4b;
  mat4 m4; // m4b;
  // versor q, qb;

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

  { // tests for distance_plane_point(). distances should be negative if point is behind the plane.
    const int n        = 8;
    vec4 planes_xyzd[] = {
      ( vec4 ){1, 0, 0, 0},                             // 0
      ( vec4 ){1, 0, 0, -1},                            // 1
      ( vec4 ){1, 0, 0, 0},                             // 2
      ( vec4 ){1, 0, 0, -1},                            // 3
      ( vec4 ){0, 1, 0, 0},                             // 4
      ( vec4 ){1, 0, 0, 1},                             // 5
      v4_v3f( normalise_vec3( ( vec3 ){1, 0, 1} ), 1 ), // 6
      ( vec4 ){0, 1, 0, -1}                             // 7
    };
    vec3 points[] = {( vec3 ){2, 0, 0}, ( vec3 ){2, 0, 0}, ( vec3 ){-2, 0, 0}, ( vec3 ){-2, 0, 0}, ( vec3 ){-2, 0, 0}, ( vec3 ){-1, 0, 0}, ( vec3 ){1, 0, 0},
      ( vec3 ){1, 0, 0}};
    for ( int i = 0; i < n; i++ ) {
      float dist = distance_plane_point( planes_xyzd[i], points[i] );
      printf( "test %i) distance from plane (%.2f,%.2f,%.2f,%.2f) to point (%.2f,%.2f,%.2f) = %f\n", i, planes_xyzd[i].x, planes_xyzd[i].y, planes_xyzd[i].z,
        planes_xyzd[i].w, points[i].x, points[i].y, points[i].z, dist );
    }
  }
  { // tests for frustum_planes_from_PV()
  printf("\ntests for frustum_planes_from_PV()\n");
    mat4 P  = perspective( 45, 1, 10, 100 );
    mat4 V  = look_at( ( vec3 ){.x = 0, .y = 0, .z = 0}, ( vec3 ){.z = -100}, ( vec3 ){.y = 1} );
    mat4 PV = mult_mat4_mat4( P, V );
    vec4 planes_xyzd[6];
    bool normalise = true;
    frustum_planes_from_PV( PV, planes_xyzd, normalise );
    for ( int i = 0; i < 6; i++ ) {
      printf( "plane %i:\n", i );
      print_vec4( planes_xyzd[i] );
    }
  }
  return 0;
}
