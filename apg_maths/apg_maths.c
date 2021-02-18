/* ===============================================================================================
Anton's 3D Maths Library (C99 version)
Version: 0.12
URL:     https://github.com/capnramses/apg
Licence: See apg_maths.h
Author:  Anton Gerdelan <antonofnote at gmail> @capnramses
=============================================================================================== */
#include "apg_maths.h"

#include <assert.h> // assert(). Note that release builds may remove checks for invalid data pointers.
#include <float.h>  // FLT_EPSILON, etc.
#include <math.h>   // sinf() etc.

void print_vec2( vec2 v ) { printf( "[%.2f, %.2f]\n", v.x, v.y ); }

void print_vec3( vec3 v ) { printf( "[%.2f, %.2f, %.2f]\n", v.x, v.y, v.z ); }

void print_vec4( vec4 v ) { printf( "[%.2f, %.2f, %.2f, %.2f]\n", v.x, v.y, v.z, v.w ); }

void print_mat4( mat4 m ) {
  printf( "\n" );
  printf( "[%.2f][%.2f][%.2f][%.2f]\n", m.m[0], m.m[4], m.m[8], m.m[12] );
  printf( "[%.2f][%.2f][%.2f][%.2f]\n", m.m[1], m.m[5], m.m[9], m.m[13] );
  printf( "[%.2f][%.2f][%.2f][%.2f]\n", m.m[2], m.m[6], m.m[10], m.m[14] );
  printf( "[%.2f][%.2f][%.2f][%.2f]\n", m.m[3], m.m[7], m.m[11], m.m[15] );
}

void print_quat( versor q ) { printf( "[%.2f ,%.2f, %.2f, %.2f]\n", q.w, q.x, q.y, q.z ); }

vec3 v3_v4( vec4 v ) { return ( vec3 ){ .x = v.x, .y = v.y, .z = v.z }; }

vec4 v4_v3f( vec3 v, float f ) { return ( vec4 ){ .x = v.x, .y = v.y, .z = v.z, .w = f }; }

vec3 add_vec3_f( vec3 a, float b ) { return ( vec3 ){ .x = a.x + b, .y = a.y + b, .z = a.z + b }; }

vec3 add_vec3_vec3( vec3 a, vec3 b ) { return ( vec3 ){ .x = a.x + b.x, .y = a.y + b.y, .z = a.z + b.z }; }

vec3 sub_vec3_f( vec3 a, float b ) { return ( vec3 ){ .x = a.x - b, .y = a.y - b, .z = a.z - b }; }

vec2 sub_vec2_vec2( vec2 a, vec2 b ) { return ( vec2 ){ .x = a.x - b.x, .y = a.y - b.y }; }

vec3 sub_vec3_vec3( vec3 a, vec3 b ) { return ( vec3 ){ .x = a.x - b.x, .y = a.y - b.y, .z = a.z - b.z }; }

vec3 mult_vec3_f( vec3 a, float b ) { return ( vec3 ){ .x = a.x * b, .y = a.y * b, .z = a.z * b }; }

vec3 mult_vec3_vec3( vec3 a, vec3 b ) { return ( vec3 ){ .x = a.x * b.x, .y = a.y * b.y, .z = a.z * b.z }; }

vec3 div_vec3_f( vec3 a, float b ) { return ( vec3 ){ .x = a.x / b, .y = a.y / b, .z = a.z / b }; }

vec3 div_vec3_vec3( vec3 a, vec3 b ) { return ( vec3 ){ .x = a.x / b.x, .y = a.y / b.y, .z = a.z / b.z }; }

vec4 div_vec4_f( vec4 v, float f ) { return ( vec4 ){ .x = v.x / f, .y = v.y / f, .z = v.z / f, .w = v.w / f }; }

/* magnitude or length of a vec2 */
float length_vec2( vec2 v ) { return sqrtf( v.x * v.x + v.y * v.y ); }

/* squared length */
float length2_vec2( vec2 v ) { return v.x * v.x + v.y * v.y; }

/* magnitude or length of a vec3 */
float length_vec3( vec3 v ) { return sqrtf( v.x * v.x + v.y * v.y + v.z * v.z ); }

/* squared length */
float length2_vec3( vec3 v ) { return v.x * v.x + v.y * v.y + v.z * v.z; }

vec3 normalise_vec3( vec3 v ) {
  vec3 vb;
  float l = length_vec3( v );
  if ( 0.0f == l ) { return ( vec3 ){ .x = 0.0f, .y = 0.0f, .z = 0.0f }; }
  vb.x = v.x / l;
  vb.y = v.y / l;
  vb.z = v.z / l;
  return vb;
}

vec4 normalise_plane( vec4 xyzd ) {
  vec4 out = xyzd;
  // "To normalize a plane we multiply _all four_ components by 1/||n|| (where n is the 3d part) but only n has unit length after normalization".
  float mag = length_vec3( v3_v4( xyzd ) );
  if ( fabsf( mag ) > 0.0f ) {
    float one_over_mag = 1.0f / mag;
    out.x *= one_over_mag;
    out.y *= one_over_mag;
    out.z *= one_over_mag;
    out.w *= one_over_mag;
  }
  return out;
}

float dot_vec3( vec3 a, vec3 b ) { return a.x * b.x + a.y * b.y + a.z * b.z; }

vec3 cross_vec3( vec3 a, vec3 b ) { return ( vec3 ){ .x = a.y * b.z - a.z * b.y, .y = a.z * b.x - a.x * b.z, .z = a.x * b.y - a.y * b.x }; }

float vec3_to_heading( vec3 d ) { return atan2f( -d.x, -d.z ) * APG_M_ONE_RAD_IN_DEG; }

vec3 heading_to_vec3( float degrees ) {
  float rad = degrees * APG_M_ONE_DEG_IN_RAD;
  return ( vec3 ){ .x = -sinf( rad ), .y = 0.0f, .z = -cosf( rad ) };
}

mat4 identity_mat4() {
  mat4 r  = { { 0 } };
  r.m[0]  = 1.0f;
  r.m[5]  = 1.0f;
  r.m[10] = 1.0f;
  r.m[15] = 1.0f;
  return r;
}

mat4 mult_mat4_mat4( mat4 a, mat4 b ) {
  mat4 r      = { { 0 } };
  int r_index = 0;
  for ( int col = 0; col < 4; col++ ) {
    for ( int row = 0; row < 4; row++ ) {
      float sum = 0.0f;
      for ( int i = 0; i < 4; i++ ) { sum += b.m[i + col * 4] * a.m[row + i * 4]; }
      r.m[r_index] = sum;
      r_index++;
    }
  }
  return r;
}

vec4 mult_mat4_vec4( mat4 m, vec4 v ) {
  float x = m.m[0] * v.x + m.m[4] * v.y + m.m[8] * v.z + m.m[12] * v.w;
  float y = m.m[1] * v.x + m.m[5] * v.y + m.m[9] * v.z + m.m[13] * v.w;
  float z = m.m[2] * v.x + m.m[6] * v.y + m.m[10] * v.z + m.m[14] * v.w;
  float w = m.m[3] * v.x + m.m[7] * v.y + m.m[11] * v.z + m.m[15] * v.w;
  return ( vec4 ){ .x = x, .y = y, .z = z, .w = w };
}

float det_mat4( mat4 mm ) {
  return mm.m[12] * mm.m[9] * mm.m[6] * mm.m[3] - mm.m[8] * mm.m[13] * mm.m[6] * mm.m[3] - mm.m[12] * mm.m[5] * mm.m[10] * mm.m[3] +
         mm.m[4] * mm.m[13] * mm.m[10] * mm.m[3] + mm.m[8] * mm.m[5] * mm.m[14] * mm.m[3] - mm.m[4] * mm.m[9] * mm.m[14] * mm.m[3] -
         mm.m[12] * mm.m[9] * mm.m[2] * mm.m[7] + mm.m[8] * mm.m[13] * mm.m[2] * mm.m[7] + mm.m[12] * mm.m[1] * mm.m[10] * mm.m[7] -
         mm.m[0] * mm.m[13] * mm.m[10] * mm.m[7] - mm.m[8] * mm.m[1] * mm.m[14] * mm.m[7] + mm.m[0] * mm.m[9] * mm.m[14] * mm.m[7] +
         mm.m[12] * mm.m[5] * mm.m[2] * mm.m[11] - mm.m[4] * mm.m[13] * mm.m[2] * mm.m[11] - mm.m[12] * mm.m[1] * mm.m[6] * mm.m[11] +
         mm.m[0] * mm.m[13] * mm.m[6] * mm.m[11] + mm.m[4] * mm.m[1] * mm.m[14] * mm.m[11] - mm.m[0] * mm.m[5] * mm.m[14] * mm.m[11] -
         mm.m[8] * mm.m[5] * mm.m[2] * mm.m[15] + mm.m[4] * mm.m[9] * mm.m[2] * mm.m[15] + mm.m[8] * mm.m[1] * mm.m[6] * mm.m[15] -
         mm.m[0] * mm.m[9] * mm.m[6] * mm.m[15] - mm.m[4] * mm.m[1] * mm.m[10] * mm.m[15] + mm.m[0] * mm.m[5] * mm.m[10] * mm.m[15];
}

mat4 inverse_mat4( mat4 mm ) {
  float det = det_mat4( mm );
  if ( 0.0f == det ) { return mm; }
  float inv_det = 1.0f / det;
  mat4 r;
  r.m[0]  = inv_det * ( mm.m[9] * mm.m[14] * mm.m[7] - mm.m[13] * mm.m[10] * mm.m[7] + mm.m[13] * mm.m[6] * mm.m[11] - mm.m[5] * mm.m[14] * mm.m[11] -
                       mm.m[9] * mm.m[6] * mm.m[15] + mm.m[5] * mm.m[10] * mm.m[15] );
  r.m[1]  = inv_det * ( mm.m[13] * mm.m[10] * mm.m[3] - mm.m[9] * mm.m[14] * mm.m[3] - mm.m[13] * mm.m[2] * mm.m[11] + mm.m[1] * mm.m[14] * mm.m[11] +
                       mm.m[9] * mm.m[2] * mm.m[15] - mm.m[1] * mm.m[10] * mm.m[15] );
  r.m[2]  = inv_det * ( mm.m[5] * mm.m[14] * mm.m[3] - mm.m[13] * mm.m[6] * mm.m[3] + mm.m[13] * mm.m[2] * mm.m[7] - mm.m[1] * mm.m[14] * mm.m[7] -
                       mm.m[5] * mm.m[2] * mm.m[15] + mm.m[1] * mm.m[6] * mm.m[15] );
  r.m[3]  = inv_det * ( mm.m[9] * mm.m[6] * mm.m[3] - mm.m[5] * mm.m[10] * mm.m[3] - mm.m[9] * mm.m[2] * mm.m[7] + mm.m[1] * mm.m[10] * mm.m[7] +
                       mm.m[5] * mm.m[2] * mm.m[11] - mm.m[1] * mm.m[6] * mm.m[11] );
  r.m[4]  = inv_det * ( mm.m[12] * mm.m[10] * mm.m[7] - mm.m[8] * mm.m[14] * mm.m[7] - mm.m[12] * mm.m[6] * mm.m[11] + mm.m[4] * mm.m[14] * mm.m[11] +
                       mm.m[8] * mm.m[6] * mm.m[15] - mm.m[4] * mm.m[10] * mm.m[15] );
  r.m[5]  = inv_det * ( mm.m[8] * mm.m[14] * mm.m[3] - mm.m[12] * mm.m[10] * mm.m[3] + mm.m[12] * mm.m[2] * mm.m[11] - mm.m[0] * mm.m[14] * mm.m[11] -
                       mm.m[8] * mm.m[2] * mm.m[15] + mm.m[0] * mm.m[10] * mm.m[15] );
  r.m[6]  = inv_det * ( mm.m[12] * mm.m[6] * mm.m[3] - mm.m[4] * mm.m[14] * mm.m[3] - mm.m[12] * mm.m[2] * mm.m[7] + mm.m[0] * mm.m[14] * mm.m[7] +
                       mm.m[4] * mm.m[2] * mm.m[15] - mm.m[0] * mm.m[6] * mm.m[15] );
  r.m[7]  = inv_det * ( mm.m[4] * mm.m[10] * mm.m[3] - mm.m[8] * mm.m[6] * mm.m[3] + mm.m[8] * mm.m[2] * mm.m[7] - mm.m[0] * mm.m[10] * mm.m[7] -
                       mm.m[4] * mm.m[2] * mm.m[11] + mm.m[0] * mm.m[6] * mm.m[11] );
  r.m[8]  = inv_det * ( mm.m[8] * mm.m[13] * mm.m[7] - mm.m[12] * mm.m[9] * mm.m[7] + mm.m[12] * mm.m[5] * mm.m[11] - mm.m[4] * mm.m[13] * mm.m[11] -
                       mm.m[8] * mm.m[5] * mm.m[15] + mm.m[4] * mm.m[9] * mm.m[15] );
  r.m[9]  = inv_det * ( mm.m[12] * mm.m[9] * mm.m[3] - mm.m[8] * mm.m[13] * mm.m[3] - mm.m[12] * mm.m[1] * mm.m[11] + mm.m[0] * mm.m[13] * mm.m[11] +
                       mm.m[8] * mm.m[1] * mm.m[15] - mm.m[0] * mm.m[9] * mm.m[15] );
  r.m[10] = inv_det * ( mm.m[4] * mm.m[13] * mm.m[3] - mm.m[12] * mm.m[5] * mm.m[3] + mm.m[12] * mm.m[1] * mm.m[7] - mm.m[0] * mm.m[13] * mm.m[7] -
                        mm.m[4] * mm.m[1] * mm.m[15] + mm.m[0] * mm.m[5] * mm.m[15] );
  r.m[11] = inv_det * ( mm.m[8] * mm.m[5] * mm.m[3] - mm.m[4] * mm.m[9] * mm.m[3] - mm.m[8] * mm.m[1] * mm.m[7] + mm.m[0] * mm.m[9] * mm.m[7] +
                        mm.m[4] * mm.m[1] * mm.m[11] - mm.m[0] * mm.m[5] * mm.m[11] );
  r.m[12] = inv_det * ( mm.m[12] * mm.m[9] * mm.m[6] - mm.m[8] * mm.m[13] * mm.m[6] - mm.m[12] * mm.m[5] * mm.m[10] + mm.m[4] * mm.m[13] * mm.m[10] +
                        mm.m[8] * mm.m[5] * mm.m[14] - mm.m[4] * mm.m[9] * mm.m[14] );
  r.m[13] = inv_det * ( mm.m[8] * mm.m[13] * mm.m[2] - mm.m[12] * mm.m[9] * mm.m[2] + mm.m[12] * mm.m[1] * mm.m[10] - mm.m[0] * mm.m[13] * mm.m[10] -
                        mm.m[8] * mm.m[1] * mm.m[14] + mm.m[0] * mm.m[9] * mm.m[14] );
  r.m[14] = inv_det * ( mm.m[12] * mm.m[5] * mm.m[2] - mm.m[4] * mm.m[13] * mm.m[2] - mm.m[12] * mm.m[1] * mm.m[6] + mm.m[0] * mm.m[13] * mm.m[6] +
                        mm.m[4] * mm.m[1] * mm.m[14] - mm.m[0] * mm.m[5] * mm.m[14] );
  r.m[15] = inv_det * ( mm.m[4] * mm.m[9] * mm.m[2] - mm.m[8] * mm.m[5] * mm.m[2] + mm.m[8] * mm.m[1] * mm.m[6] - mm.m[0] * mm.m[9] * mm.m[6] -
                        mm.m[4] * mm.m[1] * mm.m[10] + mm.m[0] * mm.m[5] * mm.m[10] );
  return r;
}

mat4 transpose_mat4( mat4 mm ) {
  mat4 r;
  r.m[0]  = mm.m[0];
  r.m[4]  = mm.m[1];
  r.m[8]  = mm.m[2];
  r.m[12] = mm.m[3];
  r.m[1]  = mm.m[4];
  r.m[5]  = mm.m[5];
  r.m[9]  = mm.m[6];
  r.m[13] = mm.m[7];
  r.m[2]  = mm.m[8];
  r.m[6]  = mm.m[9];
  r.m[10] = mm.m[10];
  r.m[14] = mm.m[11];
  r.m[3]  = mm.m[12];
  r.m[7]  = mm.m[13];
  r.m[11] = mm.m[14];
  r.m[15] = mm.m[15];
  return r;
}

mat4 translate_mat4( vec3 vv ) {
  mat4 r  = identity_mat4();
  r.m[12] = vv.x;
  r.m[13] = vv.y;
  r.m[14] = vv.z;
  return r;
}

mat4 rot_x_deg_mat4( float deg ) {
  float rad = deg * APG_M_ONE_DEG_IN_RAD;
  mat4 r    = identity_mat4();
  r.m[5] = r.m[10] = cosf( rad );
  r.m[9]           = -sinf( rad );
  r.m[6]           = sinf( rad );
  return r;
}

mat4 rot_y_deg_mat4( float deg ) {
  float rad = deg * APG_M_ONE_DEG_IN_RAD;
  mat4 r    = identity_mat4();
  r.m[0] = r.m[10] = cosf( rad );
  r.m[8]           = sinf( rad );
  r.m[2]           = -sinf( rad );
  return r;
}

mat4 rot_z_deg_mat4( float deg ) {
  float rad = deg * APG_M_ONE_DEG_IN_RAD;
  mat4 r    = identity_mat4();
  r.m[0] = r.m[5] = cosf( rad );
  r.m[4]          = -sinf( rad );
  r.m[1]          = sinf( rad );
  return r;
}

mat4 scale_mat4( vec3 v ) {
  mat4 r  = identity_mat4();
  r.m[0]  = v.x;
  r.m[5]  = v.y;
  r.m[10] = v.z;
  return r;
}

mat4 look_at( vec3 cam_pos, vec3 targ_pos, vec3 up ) {
  mat4 p    = translate_mat4( ( vec3 ){ .x = -cam_pos.x, .y = -cam_pos.y, .z = -cam_pos.z } );
  vec3 d    = sub_vec3_vec3( targ_pos, cam_pos );
  vec3 f    = normalise_vec3( d );
  vec3 r    = normalise_vec3( cross_vec3( f, up ) );
  vec3 u    = normalise_vec3( cross_vec3( r, f ) );
  mat4 ori  = identity_mat4();
  ori.m[0]  = r.x;
  ori.m[4]  = r.y;
  ori.m[8]  = r.z;
  ori.m[1]  = u.x;
  ori.m[5]  = u.y;
  ori.m[9]  = u.z;
  ori.m[2]  = -f.x;
  ori.m[6]  = -f.y;
  ori.m[10] = -f.z;
  return mult_mat4_mat4( ori, p );
}

mat4 orthographic( float l, float r, float b, float t, float n, float f ) {
  mat4 m    = { { 0 } };
  float rml = r - l;
  float tmb = t - b;
  float fmn = f - n;
  float nmf = n - f;
  if ( 0.0f == rml || 0.0f == tmb || 0.0f == fmn || 0.0f == nmf ) { return m; } // div by 0

  m.m[0]  = 2.0f / rml;
  m.m[5]  = 2.0f / tmb;
  m.m[10] = -2.0f / fmn;
  m.m[12] = -( r + l ) / rml;
  m.m[13] = -( t + b ) / tmb;
  m.m[14] = -( f + n ) / fmn;
  m.m[15] = 1.0f;

  return m;
}

mat4 perspective( float fovy, float aspect, float near_plane, float far_plane ) {
  float fov_rad = fovy * APG_M_ONE_DEG_IN_RAD;
  float range   = tanf( fov_rad / 2.0f ) * near_plane;
  float sx      = ( 2.0f * near_plane ) / ( range * aspect + range * aspect );
  float sy      = near_plane / range;
  float sz      = -( far_plane + near_plane ) / ( far_plane - near_plane );
  float pz      = -( 2.0f * far_plane * near_plane ) / ( far_plane - near_plane );
  mat4 m        = { { 0 } };
  m.m[0]        = sx;
  m.m[5]        = sy;
  m.m[10]       = sz;
  m.m[14]       = pz;
  m.m[11]       = -1.0f;
  return m;
}

mat4 perspective_offcentre_viewport( int vp_w, int vp_h, int subvp_x, int subvp_y, int subvp_w, int subvp_h, mat4 P_orig ) {
  float subvp_x_ndc = ( (float)subvp_x / (float)vp_w ) * 2.0f - 1.0f;
  float subvp_y_ndc = ( (float)subvp_y / (float)vp_h ) * 2.0f - 1.0f;
  float subvp_w_ndc = ( (float)subvp_w / (float)vp_w ) * 2.0f;
  float subvp_h_ndc = ( (float)subvp_h / (float)vp_h ) * 2.0f;
  // Create a scale and translation transform which maps the range [x_ndc, x_ndc+a_ndc] to [-1,1], and similar for y
  mat4 M  = { { 0 } };
  M.m[0]  = 2.0f / subvp_w_ndc;
  M.m[5]  = 2.0f / subvp_h_ndc;
  M.m[10] = 1.0f;
  M.m[12] = -2.0f * subvp_x_ndc / subvp_w_ndc - 1.0f;
  M.m[13] = -2.0f * subvp_y_ndc / subvp_h_ndc - 1.0f;
  M.m[15] = 1.0f;
  // Pre-Multiply M to the original projection matrix P
  mat4 P_asym = mult_mat4_mat4( M, P_orig );
  return P_asym;
}

void frustum_points_from_PV( mat4 PV, vec3* corners_wor ) {
  assert( corners_wor );

  mat4 clip_to_world = inverse_mat4( PV );

  // Start in clip space. First four have -1 in Z for OpenGL. should be 0 for D3D
  vec4 corners_clip[8] = {
    ( vec4 ){ -1, -1, -1, 1 }, // will be nbl 0
    ( vec4 ){ -1, 1, -1, 1 },  // will be ntl 1
    ( vec4 ){ 1, 1, -1, 1 },   // will be ntr 2
    ( vec4 ){ 1, -1, -1, 1 },  // will be nbr 3
    ( vec4 ){ -1, -1, 1, 1 },  // will be fbl 4
    ( vec4 ){ -1, 1, 1, 1 },   // will be ftl 5
    ( vec4 ){ 1, 1, 1, 1 },    // will be ftr 6
    ( vec4 ){ 1, -1, 1, 1 }    // will be fbr 7
  };
  for ( int i = 0; i < 8; i++ ) {
    corners_clip[i] = mult_mat4_vec4( clip_to_world, corners_clip[i] );
    corners_wor[i]  = v3_v4( div_vec4_f( corners_clip[i], corners_clip[i].w ) ); // perspective division
  }
}

void frustum_planes_from_PV( mat4 PV, vec4* planes_xyxd, bool normalise_planes ) {
  assert( planes_xyxd );

  planes_xyxd[0] = ( vec4 ){ .x = PV.m[3] + PV.m[0], .y = PV.m[7] + PV.m[4], .z = PV.m[11] + PV.m[8], .w = PV.m[15] + PV.m[12] };  // left clipping plane
  planes_xyxd[1] = ( vec4 ){ .x = PV.m[3] - PV.m[0], .y = PV.m[7] - PV.m[4], .z = PV.m[11] - PV.m[8], .w = PV.m[15] - PV.m[12] };  // right clipping plane
  planes_xyxd[2] = ( vec4 ){ .x = PV.m[3] + PV.m[1], .y = PV.m[7] + PV.m[5], .z = PV.m[11] + PV.m[9], .w = PV.m[15] + PV.m[13] };  // bottom clipping plane
  planes_xyxd[3] = ( vec4 ){ .x = PV.m[3] - PV.m[1], .y = PV.m[7] - PV.m[5], .z = PV.m[11] - PV.m[9], .w = PV.m[15] - PV.m[13] };  // top clipping plane
  planes_xyxd[4] = ( vec4 ){ .x = PV.m[3] + PV.m[2], .y = PV.m[7] + PV.m[6], .z = PV.m[11] + PV.m[10], .w = PV.m[15] + PV.m[14] }; // near clipping plane
  planes_xyxd[5] = ( vec4 ){ .x = PV.m[3] - PV.m[2], .y = PV.m[7] - PV.m[6], .z = PV.m[11] - PV.m[10], .w = PV.m[15] - PV.m[14] }; // far clipping plane
  if ( normalise_planes ) {
    for ( int i = 0; i < 6; i++ ) { planes_xyxd[i] = normalise_plane( planes_xyxd[i] ); }
  }
}

versor div_quat_f( versor qq, float s ) { return ( versor ){ .w = qq.w / s, .x = qq.x / s, .y = qq.y / s, .z = qq.z / s }; }

versor mult_quat_f( versor qq, float s ) { return ( versor ){ .w = qq.w * s, .x = qq.x * s, .y = qq.y * s, .z = qq.z * s }; }

vec3 mult_quat_vec3( versor q, vec3 v ) {
  vec3 b      = ( vec3 ){ .x = q.x, .y = q.y, .z = q.z };
  float b2    = b.x * b.x + b.y * b.y + b.z * b.z;
  vec3 part_a = mult_vec3_f( v, q.w * q.w - b2 );
  vec3 part_b = mult_vec3_f( b, dot_vec3( v, b ) * 2.0f );
  vec3 part_c = mult_vec3_f( cross_vec3( b, v ), q.w * 2.0f );
  vec3 out    = add_vec3_vec3( part_a, add_vec3_vec3( part_b, part_c ) );
  return out;
}

versor normalise_quat( versor q ) {
  float sum          = q.w * q.w + q.x * q.x + q.y * q.y + q.z * q.z;
  const float thresh = 0.0001f;
  if ( fabs( 1.0f - sum ) < thresh ) { return q; }
  float mag = sqrtf( sum );
  return div_quat_f( q, mag );
}

versor mult_quat_quat( versor a, versor b ) {
  versor result;
  result.w = b.w * a.w - b.x * a.x - b.y * a.y - b.z * a.z;
  result.x = b.w * a.x + b.x * a.w - b.y * a.z + b.z * a.y;
  result.y = b.w * a.y + b.x * a.z + b.y * a.w - b.z * a.x;
  result.z = b.w * a.z - b.x * a.y + b.y * a.x + b.z * a.w;
  return normalise_quat( result );
}

versor add_quat_quat( versor a, versor b ) {
  versor result;
  result.w = b.w + a.w;
  result.x = b.x + a.x;
  result.y = b.y + a.y;
  result.z = b.z + a.z;
  return normalise_quat( result );
}

versor quat_from_axis_rad( float radians, vec3 axis ) {
  versor result;
  result.w = cosf( radians / 2.0f );
  result.x = sinf( radians / 2.0f ) * axis.x;
  result.y = sinf( radians / 2.0f ) * axis.y;
  result.z = sinf( radians / 2.0f ) * axis.z;
  return result;
}

versor quat_from_axis_deg( float degrees, vec3 axis ) { return quat_from_axis_rad( APG_M_ONE_DEG_IN_RAD * degrees, axis ); }

mat4 quat_to_mat4( versor q ) {
  float w = q.w;
  float x = q.x;
  float y = q.y;
  float z = q.z;
  mat4 r;
  r.m[0]  = 1.0f - 2.0f * y * y - 2.0f * z * z;
  r.m[1]  = 2.0f * x * y + 2.0f * w * z;
  r.m[2]  = 2.0f * x * z - 2.0f * w * y;
  r.m[3]  = 0.0f;
  r.m[4]  = 2.0f * x * y - 2.0f * w * z;
  r.m[5]  = 1.0f - 2.0f * x * x - 2.0f * z * z;
  r.m[6]  = 2.0f * y * z + 2.0f * w * x;
  r.m[7]  = 0.0f;
  r.m[8]  = 2.0f * x * z + 2.0f * w * y;
  r.m[9]  = 2.0f * y * z - 2.0f * w * x;
  r.m[10] = 1.0f - 2.0f * x * x - 2.0f * y * y;
  r.m[11] = r.m[12] = r.m[13] = r.m[14] = 0.0f;
  r.m[15]                               = 1.0f;
  return r;
}

float dot_quat( versor q, versor r ) { return q.w * r.w + q.x * r.x + q.y * r.y + q.z * r.z; }

versor slerp_quat( versor q, versor r, float t ) {
  float cos_half_theta = dot_quat( q, r );
  if ( cos_half_theta < 0.0f ) {
    q              = mult_quat_f( q, -1.0f );
    cos_half_theta = dot_quat( q, r );
  }
  if ( fabsf( cos_half_theta ) >= 1.0f ) { return q; }
  float sin_half_theta = sqrtf( 1.0f - cos_half_theta * cos_half_theta );
  versor result;
  if ( fabsf( sin_half_theta ) < 0.001f ) {
    result.w = ( 1.0f - t ) * q.w + t * r.w;
    result.x = ( 1.0f - t ) * q.x + t * r.x;
    result.y = ( 1.0f - t ) * q.y + t * r.y;
    result.z = ( 1.0f - t ) * q.z + t * r.z;
    return result;
  }
  float half_theta = acosf( cos_half_theta );
  float a          = sinf( ( 1.0f - t ) * half_theta ) / sin_half_theta;
  float b          = sinf( t * half_theta ) / sin_half_theta;
  result.w         = q.w * a + r.w * b;
  result.x         = q.x * a + r.x * b;
  result.y         = q.y * a + r.y * b;
  result.z         = q.z * a + r.z * b;
  return result;
}

float wrap_degrees_360( float degrees ) {
  if ( degrees >= 0.0f && degrees < 360.0f ) { return degrees; }
  int multiples = (int)( degrees / 360.0f );
  if ( degrees > 0.0f ) {
    degrees = degrees - (float)multiples * 360.0f;
  } else {
    degrees = degrees + (float)multiples * 360.0f;
  }
  return degrees;
}

float abs_diff_btw_degrees( float first, float second ) {
  first  = wrap_degrees_360( first );
  second = wrap_degrees_360( second );

  float diff = fabsf( first - second );
  if ( diff >= 180.0f ) { diff = fabsf( diff - 360.0f ); }
  return diff;
}

float ray_plane( vec3 ray_origin, vec3 ray_direction, vec3 plane_normal, float plane_d ) {
  return -( dot_vec3( ray_origin, plane_normal ) + plane_d ) / dot_vec3( ray_direction, plane_normal );
}

bool ray_aabb( vec3 ray_origin, vec3 ray_direction, vec3 aabb_min, vec3 aabb_max, float tmin, float tmax ) {
  float* rd      = &ray_direction.x;
  float* ro      = &ray_origin.x;
  float* box_min = &aabb_min.x;
  float* box_max = &aabb_max.x;
  for ( int i = 0; i < 3; i++ ) {
    float invD = 1.0f / rd[i];
    float t0   = ( box_min[i] - ro[i] ) * invD;
    float t1   = ( box_max[i] - ro[i] ) * invD;
    if ( invD < 0.0 ) {
      float tmp = t0;
      t0        = t1;
      t1        = tmp;
    }
    tmin = t0 > tmin ? t0 : tmin;
    tmax = t1 < tmax ? t1 : tmax;
    if ( tmax <= tmin ) { return false; }
  }
  return true;
}

bool ray_obb( obb_t box, vec3 ray_o, vec3 ray_d, float* t, int* face_num ) {
  assert( t );
  *t             = 0.0f;
  float tmin     = -INFINITY;
  float tmax     = INFINITY;
  int slab_min_i = 0, slab_max_i = 0;
  vec3 p = sub_vec3_vec3( box.centre, ray_o );
  for ( int i = 0; i < 3; i++ ) { // 3 "slabs" (pair of front/back planes)
    float e = dot_vec3( box.norm_side_dir[i], p );
    float f = dot_vec3( box.norm_side_dir[i], ray_d );
    if ( fabs( f ) > FLT_EPSILON ) {
      float t1    = ( e + box.half_lengths[i] ) / f; // intersection on front
      float t2    = ( e - box.half_lengths[i] ) / f; // and back side of slab
      int t1_side = 1;                               // t1 is front
      if ( t1 > t2 ) {
        float tmp = t1;
        t1        = t2;
        t2        = tmp;
        t1_side   = -1; // t1 is back face of slab (opposing the slab normal)
      }
      if ( t1 > tmin ) {
        tmin       = t1;
        slab_min_i = ( i + 1 ) * t1_side;
      }
      if ( t2 < tmax ) {
        tmax       = t2;
        slab_max_i = ( i + 1 ) * -t1_side;
      }
      if ( tmin > tmax ) { return false; }
      if ( tmax < 0 ) { return false; }
    } else if ( -e - box.half_lengths[i] > 0 || -e + box.half_lengths[i] < 0 ) {
      return false;
    }
  }
  *t        = tmin > 0 ? tmin : tmax;
  *face_num = tmin > 0 ? slab_min_i : slab_max_i; // max is back side of slab (opposing face)
  return true;
}

aabb_t frustum_to_aabb( mat4 PV ) {
  aabb_t aabb = ( aabb_t ){ .min.x = 0.0f };
  vec3 corners_wor[8];
  frustum_points_from_PV( PV, corners_wor );
  for ( int i = 1; i < 8; i++ ) {
    aabb.min.x = APG_M_MIN( aabb.min.x, corners_wor[i].x );
    aabb.max.x = APG_M_MAX( aabb.max.x, corners_wor[i].x );
    aabb.min.y = APG_M_MIN( aabb.min.y, corners_wor[i].y );
    aabb.max.y = APG_M_MAX( aabb.max.y, corners_wor[i].y );
    aabb.min.z = APG_M_MIN( aabb.min.z, corners_wor[i].z );
    aabb.max.z = APG_M_MAX( aabb.max.z, corners_wor[i].z );
  }
  return aabb;
}

bool aabb_aabb( aabb_t a, aabb_t b ) {
  return ( a.min.x <= b.max.x && a.max.x >= b.min.x ) && ( a.min.y <= b.max.y && a.max.y >= b.min.y ) && ( a.min.z <= b.max.z && a.max.z >= b.min.z );
}

float distance_plane_point( vec4 plane_xyzd, vec3 point ) { return plane_xyzd.x * point.x + plane_xyzd.y * point.y + plane_xyzd.z * point.z + plane_xyzd.w; }

bool frustum_vs_aabb( const vec4* frustum_planes, aabb_t box ) {
  assert( frustum_planes );
  if ( !frustum_planes ) { return false; }

  for ( int i = 0; i < 6; i++ ) {
    int out = 0;
    if ( distance_plane_point( frustum_planes[i], ( vec3 ){ box.min.x, box.min.y, box.min.z } ) < 0.0f ) { out++; }
    if ( distance_plane_point( frustum_planes[i], ( vec3 ){ box.min.x, box.min.y, box.max.z } ) < 0.0f ) { out++; }
    if ( distance_plane_point( frustum_planes[i], ( vec3 ){ box.min.x, box.max.y, box.min.z } ) < 0.0f ) { out++; }
    if ( distance_plane_point( frustum_planes[i], ( vec3 ){ box.min.x, box.max.y, box.max.z } ) < 0.0f ) { out++; }
    if ( distance_plane_point( frustum_planes[i], ( vec3 ){ box.max.x, box.min.y, box.min.z } ) < 0.0f ) { out++; }
    if ( distance_plane_point( frustum_planes[i], ( vec3 ){ box.max.x, box.min.y, box.max.z } ) < 0.0f ) { out++; }
    if ( distance_plane_point( frustum_planes[i], ( vec3 ){ box.max.x, box.max.y, box.min.z } ) < 0.0f ) { out++; }
    if ( distance_plane_point( frustum_planes[i], ( vec3 ){ box.max.x, box.max.y, box.max.z } ) < 0.0f ) { out++; }
    if ( 8 == out ) { return false; }
  }
  return true;
}

bool frustum_vs_aabb_extra_check( const vec4* frustum_planes, const vec3* frustum_points, aabb_t box ) {
  assert( frustum_planes && frustum_points );
  if ( !frustum_planes || !frustum_points ) { return false; }

  if ( !frustum_vs_aabb( frustum_planes, box ) ) { return false; }

  // check frustum outside/inside box
  // NOTE: may be wrong
  int out = 0;
  for ( int i = 0; i < 8; i++ ) {
    if ( frustum_points[i].x > box.max.x ) { out++; }
  }
  if ( out == 8 ) { return false; }
  out = 0;
  for ( int i = 0; i < 8; i++ ) {
    if ( frustum_points[i].x < box.min.x ) { out++; }
  }
  if ( out == 8 ) { return false; }
  out = 0;
  for ( int i = 0; i < 8; i++ ) {
    if ( frustum_points[i].y > box.max.y ) { out++; }
  }
  if ( out == 8 ) { return false; }
  out = 0;
  for ( int i = 0; i < 8; i++ ) {
    if ( frustum_points[i].y < box.min.y ) { out++; }
  }
  if ( out == 8 ) { return false; }
  out = 0;
  for ( int i = 0; i < 8; i++ ) {
    if ( frustum_points[i].z > box.max.z ) { out++; }
  }
  if ( out == 8 ) { return false; }
  out = 0;
  for ( int i = 0; i < 8; i++ ) {
    if ( frustum_points[i].z < box.min.z ) { out++; }
  }
  if ( out == 8 ) { return false; }

  return true;
}
