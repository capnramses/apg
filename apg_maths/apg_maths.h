/* ===============================================================================================
Anton's 3D Maths Library (C99 version)
URL:     https://github.com/capnramses/apg
Licence: See bottom of file.
Author:  Anton Gerdelan <antonofnote at gmail> @capnramses
==================================================================================================
History:
v0.8 - 21 May 2020 - AABB intersection and frustum bounds and plane-extraction functions.
v0.7 - 19 Apr 2020 - Removed M_PI, replaced with APG_PI. Added prefix to macros to avoid redefinition vulnerability.
v0.6 -  9 Apr 2020 - Frustum extraction functions.
v0.5 - 11 Apr 2016 - Compacted.
v0.4 - 12 Apr 2016 - Switched to .x .y .z notation for vectors and quaternions.
v0.3 - 17 Jul 2019 - Updated with maths code from voxel game project.
v0.2 - 20 Nov 2019 - Added a cpp clause.
v0.1 -  5 May 2015 - Branched from C++ original.
==================================================================================================
TODO:
Vectors
  project and reject vectors
Matrix
  arbitrary axis rot
Virtual Camera
  orthographic
Quaternions
  conjugates
Geometry
  distance point to line
  distance line to line
  distance point to plane
  ~plane reflection matrix
  line-plane intersect
  line-sphere intersect
  line-OBB intersect
  line-AABB intersect
=============================================================================================== */
#pragma once

#include <assert.h>  // assert(). Note that release builds may remove checks for invalid data pointers.
#include <float.h>   // FLT_EPSILON, etc.
#include <math.h>    // sinf() etc.
#include <stdbool.h> // `bool` in C99
#include <stdio.h>   // Used for printing vectors to stdout.

#ifdef __cplusplus
extern "C" {
#endif

#define APG_M_PI 3.14159265358979323846 // Stable pi. C99 removed M_PI and it is .: not stable between builds.
#define APG_M_TWO_PI APG_M_PI * 2.0
#define APG_M_HALF_PI APG_M_PI / 2.0
#define APG_M_ONE_DEG_IN_RAD ( 2.0 * APG_M_PI ) / 360.0 // 0.017444444
#define APG_M_ONE_RAD_IN_DEG 360.0 / ( 2.0 * APG_M_PI ) // 57.2957795

#define APG_M_MIN( a, b ) ( ( a ) < ( b ) ? ( a ) : ( b ) )
#define APG_M_MAX( a, b ) ( ( a ) > ( b ) ? ( a ) : ( b ) )
#define APG_M_CLAMP( x, lo, hi ) ( APG_M_MIN( hi, APG_M_MAX( lo, x ) ) )

typedef struct vec2 {
  float x, y;
} vec2;

typedef struct vec3 {
  float x, y, z;
} vec3;

typedef struct vec4 {
  float x, y, z, w;
} vec4;

typedef struct ivec3 {
  int x, y, z;
} ivec3;

typedef struct mat4 {
  float m[16];
} mat4;

/* 'Versor' is the proper name for a _unit quaternion_ (the kind used for geometric rotations) */
typedef struct versor {
  float w, x, y, z;
} versor;

/* Oriented Bounding Box (OBB) volume and orientation definition. */
typedef struct obb_t {   // A in RTR notation
  vec3 centre;           // a^c in RTR notation
  vec3 norm_side_dir[3]; // a^u, a^v, a^w in RTR notation
  float half_lengths[3]; // centre to face. must be positive. h_u, h_v, h_w in RTR notation
} obb_t;

/* Axis-Aligned Bounding Box (AABB) volume given box two diagonally-opposing corners. */
typedef struct aabb_t {
  vec3 min, max;
} aabb_t;

static inline void print_vec2( vec2 v ) { printf( "[%.2f, %.2f]\n", v.x, v.y ); }
static inline void print_vec3( vec3 v ) { printf( "[%.2f, %.2f, %.2f]\n", v.x, v.y, v.z ); }
static inline void print_vec4( vec4 v ) { printf( "[%.2f, %.2f, %.2f, %.2f]\n", v.x, v.y, v.z, v.w ); }
static inline void print_mat4( mat4 m ) {
  printf( "\n" );
  printf( "[%.2f][%.2f][%.2f][%.2f]\n", m.m[0], m.m[4], m.m[8], m.m[12] );
  printf( "[%.2f][%.2f][%.2f][%.2f]\n", m.m[1], m.m[5], m.m[9], m.m[13] );
  printf( "[%.2f][%.2f][%.2f][%.2f]\n", m.m[2], m.m[6], m.m[10], m.m[14] );
  printf( "[%.2f][%.2f][%.2f][%.2f]\n", m.m[3], m.m[7], m.m[11], m.m[15] );
}
static inline void print_quat( versor q ) { printf( "[%.2f ,%.2f, %.2f, %.2f]\n", q.w, q.x, q.y, q.z ); }

static inline vec3 v3_v4( vec4 v ) { return ( vec3 ){.x = v.x, .y = v.y, .z = v.z}; }
static inline vec4 v4_v3f( vec3 v, float f ) { return ( vec4 ){.x = v.x, .y = v.y, .z = v.z, .w = f}; }

static inline vec3 add_vec3_f( vec3 a, float b ) { return ( vec3 ){.x = a.x + b, .y = a.y + b, .z = a.z + b}; }
static inline vec3 add_vec3_vec3( vec3 a, vec3 b ) { return ( vec3 ){.x = a.x + b.x, .y = a.y + b.y, .z = a.z + b.z}; }

static inline vec3 sub_vec3_f( vec3 a, float b ) { return ( vec3 ){.x = a.x - b, .y = a.y - b, .z = a.z - b}; }
static inline vec2 sub_vec2_vec2( vec2 a, vec2 b ) { return ( vec2 ){.x = a.x - b.x, .y = a.y - b.y}; }
static inline vec3 sub_vec3_vec3( vec3 a, vec3 b ) { return ( vec3 ){.x = a.x - b.x, .y = a.y - b.y, .z = a.z - b.z}; }

static inline vec3 mult_vec3_f( vec3 a, float b ) { return ( vec3 ){.x = a.x * b, .y = a.y * b, .z = a.z * b}; }
static inline vec3 mult_vec3_vec3( vec3 a, vec3 b ) { return ( vec3 ){.x = a.x * b.x, .y = a.y * b.y, .z = a.z * b.z}; }

static inline vec3 div_vec3_f( vec3 a, float b ) { return ( vec3 ){.x = a.x / b, .y = a.y / b, .z = a.z / b}; }
static inline vec3 div_vec3_vec3( vec3 a, vec3 b ) { return ( vec3 ){.x = a.x / b.x, .y = a.y / b.y, .z = a.z / b.z}; }
static inline vec4 div_vec4_f( vec4 v, float f ) { return ( vec4 ){.x = v.x / f, .y = v.y / f, .z = v.z / f, .w = v.w / f}; }

/* magnitude or length of a vec2 */
static inline float length_vec2( vec2 v ) { return sqrt( v.x * v.x + v.y * v.y ); }
/* squared length */
static inline float length2_vec2( vec2 v ) { return v.x * v.x + v.y * v.y; }
/* magnitude or length of a vec3 */
static inline float length_vec3( vec3 v ) { return sqrtf( v.x * v.x + v.y * v.y + v.z * v.z ); }
/* squared length */
static inline float length2_vec3( vec3 v ) { return v.x * v.x + v.y * v.y + v.z * v.z; }

static inline vec3 normalise_vec3( vec3 v ) {
  vec3 vb;
  float l = length_vec3( v );
  if ( 0.0f == l ) { return ( vec3 ){.x = 0.0f, .y = 0.0f, .z = 0.0f}; }
  vb.x = v.x / l;
  vb.y = v.y / l;
  vb.z = v.z / l;
  return vb;
}

/* Note that plane normalisation also affects the d component of the plane, but only the xyz normal component is unit-length afterwards.
Based on FoGED Vol 1 Ch 3.4.2 pg 115 by Eric Lengyel */
static inline vec4 normalise_plane( vec4 xyzd ) {
  vec4 out = xyzd;
  // "To normalize a plane we multiply _all four_ components by 1/||n|| (where n is the 3d part) but only n has unit length after normalization"
  float mag = length_vec3( v3_v4( xyzd ) );
  if ( fabsf( mag ) > 0.0f ) {
    float one_over_mag = 1.0 / mag;
    out.x *= one_over_mag;
    out.y *= one_over_mag;
    out.z *= one_over_mag;
    out.w *= one_over_mag;
  }
  return out;
}

static inline float dot_vec3( vec3 a, vec3 b ) { return a.x * b.x + a.y * b.y + a.z * b.z; }

static inline vec3 cross_vec3( vec3 a, vec3 b ) { return ( vec3 ){.x = a.y * b.z - a.z * b.y, .y = a.z * b.x - a.x * b.z, .z = a.x * b.y - a.y * b.x}; }

/* Converts an un-normalised direction vector's X,Z components into a heading in degrees. */
static inline float vec3_to_heading( vec3 d ) { return atan2f( -d.x, -d.z ) * APG_M_ONE_RAD_IN_DEG; }
/* Very informal function to convert a heading (e.g. y-axis orientation) into a 3d vector with components in x and z axes. */
static inline vec3 heading_to_vec3( float degrees ) {
  float rad = degrees * APG_M_ONE_DEG_IN_RAD;
  return ( vec3 ){.x = -sinf( rad ), .y = 0.0f, .z = -cosf( rad )};
}

static inline mat4 identity_mat4() {
  mat4 r  = {{0}};
  r.m[0]  = 1.0f;
  r.m[5]  = 1.0f;
  r.m[10] = 1.0f;
  r.m[15] = 1.0f;
  return r;
}

static inline mat4 mult_mat4_mat4( mat4 a, mat4 b ) {
  mat4 r      = {{0}};
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

static inline vec4 mult_mat4_vec4( mat4 m, vec4 v ) {
  float x = m.m[0] * v.x + m.m[4] * v.y + m.m[8] * v.z + m.m[12] * v.w;
  float y = m.m[1] * v.x + m.m[5] * v.y + m.m[9] * v.z + m.m[13] * v.w;
  float z = m.m[2] * v.x + m.m[6] * v.y + m.m[10] * v.z + m.m[14] * v.w;
  float w = m.m[3] * v.x + m.m[7] * v.y + m.m[11] * v.z + m.m[15] * v.w;
  return ( vec4 ){.x = x, .y = y, .z = z, .w = w};
}

static inline float det_mat4( mat4 mm ) {
  return mm.m[12] * mm.m[9] * mm.m[6] * mm.m[3] - mm.m[8] * mm.m[13] * mm.m[6] * mm.m[3] - mm.m[12] * mm.m[5] * mm.m[10] * mm.m[3] +
         mm.m[4] * mm.m[13] * mm.m[10] * mm.m[3] + mm.m[8] * mm.m[5] * mm.m[14] * mm.m[3] - mm.m[4] * mm.m[9] * mm.m[14] * mm.m[3] -
         mm.m[12] * mm.m[9] * mm.m[2] * mm.m[7] + mm.m[8] * mm.m[13] * mm.m[2] * mm.m[7] + mm.m[12] * mm.m[1] * mm.m[10] * mm.m[7] -
         mm.m[0] * mm.m[13] * mm.m[10] * mm.m[7] - mm.m[8] * mm.m[1] * mm.m[14] * mm.m[7] + mm.m[0] * mm.m[9] * mm.m[14] * mm.m[7] +
         mm.m[12] * mm.m[5] * mm.m[2] * mm.m[11] - mm.m[4] * mm.m[13] * mm.m[2] * mm.m[11] - mm.m[12] * mm.m[1] * mm.m[6] * mm.m[11] +
         mm.m[0] * mm.m[13] * mm.m[6] * mm.m[11] + mm.m[4] * mm.m[1] * mm.m[14] * mm.m[11] - mm.m[0] * mm.m[5] * mm.m[14] * mm.m[11] -
         mm.m[8] * mm.m[5] * mm.m[2] * mm.m[15] + mm.m[4] * mm.m[9] * mm.m[2] * mm.m[15] + mm.m[8] * mm.m[1] * mm.m[6] * mm.m[15] -
         mm.m[0] * mm.m[9] * mm.m[6] * mm.m[15] - mm.m[4] * mm.m[1] * mm.m[10] * mm.m[15] + mm.m[0] * mm.m[5] * mm.m[10] * mm.m[15];
}

// TODO(Anton) look up fast inverse video tutorial
static inline mat4 inverse_mat4( mat4 mm ) {
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

static inline mat4 transpose_mat4( mat4 mm ) {
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

static inline mat4 translate_mat4( vec3 vv ) {
  mat4 r  = identity_mat4();
  r.m[12] = vv.x;
  r.m[13] = vv.y;
  r.m[14] = vv.z;
  return r;
}

static inline mat4 rot_x_deg_mat4( float deg ) {
  float rad = deg * APG_M_ONE_DEG_IN_RAD;
  mat4 r    = identity_mat4();
  r.m[5] = r.m[10] = cosf( rad );
  r.m[9]           = -sinf( rad );
  r.m[6]           = sinf( rad );
  return r;
}

static inline mat4 rot_y_deg_mat4( float deg ) {
  float rad = deg * APG_M_ONE_DEG_IN_RAD;
  mat4 r    = identity_mat4();
  r.m[0] = r.m[10] = cosf( rad );
  r.m[8]           = sinf( rad );
  r.m[2]           = -sinf( rad );
  return r;
}

static inline mat4 rot_z_deg_mat4( float deg ) {
  float rad = deg * APG_M_ONE_DEG_IN_RAD;
  mat4 r    = identity_mat4();
  r.m[0] = r.m[5] = cosf( rad );
  r.m[4]          = -sinf( rad );
  r.m[1]          = sinf( rad );
  return r;
}

static inline mat4 scale_mat4( vec3 v ) {
  mat4 r  = identity_mat4();
  r.m[0]  = v.x;
  r.m[5]  = v.y;
  r.m[10] = v.z;
  return r;
}

/* Creates a view matrix, using typical "look at" parameters. Most graphics mathematics libraries have a similar function. */
static inline mat4 look_at( vec3 cam_pos, vec3 targ_pos, vec3 up ) {
  mat4 p    = translate_mat4( ( vec3 ){.x = -cam_pos.x, .y = -cam_pos.y, .z = -cam_pos.z} );
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

static inline mat4 perspective( float fovy, float aspect, float near_plane, float far_plane ) {
  float fov_rad = fovy * APG_M_ONE_DEG_IN_RAD;
  float range   = tanf( fov_rad / 2.0f ) * near_plane;
  float sx      = ( 2.0f * near_plane ) / ( range * aspect + range * aspect );
  float sy      = near_plane / range;
  float sz      = -( far_plane + near_plane ) / ( far_plane - near_plane );
  float pz      = -( 2.0f * far_plane * near_plane ) / ( far_plane - near_plane );
  mat4 m        = {{0}};
  m.m[0]        = sx;
  m.m[5]        = sy;
  m.m[10]       = sz;
  m.m[14]       = pz;
  m.m[11]       = -1.0f;
  return m;
}

/* Create a standard *asymmetric* perspective projection matrix for special case of a subwindow viewport
- original viewport from (0,0) with size (vp_w, vp_h)
- subwindow viewport from (subvp_x,subvp_y) with size (subvp_w,subvp_h)
- Note: mouse coords, if used, may required a y direction flip.
- Note: this function does not modify near or far plane. It could do my adding z scaling to M.
- Note: this function uses an axis-parallel subwindow but it could be modified to a parallelogram shape.
Code based on excellent problem description here: https://stackoverflow.com/questions/50110934/display-recursively-rendered-scene-into-a-plane
*/
static inline mat4 perspective_offcentre_viewport( int vp_w, int vp_h, int subvp_x, int subvp_y, int subvp_w, int subvp_h, mat4 P_orig ) {
  float subvp_x_ndc = ( (float)subvp_x / (float)vp_w ) * 2.0f - 1.0f;
  float subvp_y_ndc = ( (float)subvp_y / (float)vp_h ) * 2.0f - 1.0f;
  float subvp_w_ndc = ( (float)subvp_w / (float)vp_w ) * 2.0f;
  float subvp_h_ndc = ( (float)subvp_h / (float)vp_h ) * 2.0f;
  // Create a scale and translation transform which maps the range [x_ndc, x_ndc+a_ndc] to [-1,1], and similar for y
  mat4 M  = {{0}};
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

/* Takes any world-to-clip space matrix and inverts to retrieve the 8 corner points in world space
based on http://donw.io/post/frustum-point-extraction/

PARAMS
  PV      - Any world-to-clip space matrix.
  corners - A buffer of 8x vec3. Must not be NULL. World space corner points will be set here.

REMARKS
Expect some floating small point error compared to original world space points.
*/
static inline void frustum_points_from_PV( mat4 PV, vec3* corners_wor ) {
  assert( corners_wor );

  mat4 clip_to_world = inverse_mat4( PV );

  // Start in clip space. First four have -1 in Z for OpenGL. should be 0 for D3D
  vec4 corners_clip[8] = {
    ( vec4 ){-1, -1, -1, 1}, // will be nbl 0
    ( vec4 ){-1, 1, -1, 1},  // will be ntl 1
    ( vec4 ){1, 1, -1, 1},   // will be ntr 2
    ( vec4 ){1, -1, -1, 1},  // will be nbr 3
    ( vec4 ){-1, -1, 1, 1},  // will be fbl 4
    ( vec4 ){-1, 1, 1, 1},   // will be ftl 5
    ( vec4 ){1, 1, 1, 1},    // will be ftr 6
    ( vec4 ){1, -1, 1, 1}    // will be fbr 7
  };
  for ( int i = 0; i < 8; i++ ) {
    corners_clip[i] = mult_mat4_vec4( clip_to_world, corners_clip[i] );
    corners_wor[i]  = v3_v4( div_vec4_f( corners_clip[i], corners_clip[i].w ) ); // perspective division
  }
}

/* Fast extraction of 6 clip planes .
Based on http://www8.cs.umu.se/kurser/5DV051/HT12/lab/plane_extraction.pdf

PARAMS
  PV               - Any world-to-clip space matrix.
  corners          - A buffer of 6x vec4 to set world space planes. Must not be NULL. The plane coefficients are in the form: [n|d].
  normalise_planes - Set to false if you only need the planes for in-front/behind tests, and not distance of point from plane.
REMARKS Note that plane normalisation also affects the d component of the plane, but only the xyz normal component is unit-length afterwards.
*/
static inline void frustum_planes_from_PV( mat4 PV, vec4* planes_xyxd, bool normalise_planes ) {
  assert( planes_xyxd );

  planes_xyxd[0] = ( vec4 ){.x = PV.m[3] + PV.m[0], .y = PV.m[7] + PV.m[4], .z = PV.m[11] + PV.m[8], .w = PV.m[15] + PV.m[12]};  // left clipping plane
  planes_xyxd[1] = ( vec4 ){.x = PV.m[3] - PV.m[0], .y = PV.m[7] - PV.m[4], .z = PV.m[11] - PV.m[8], .w = PV.m[15] - PV.m[12]};  // right clipping plane
  planes_xyxd[2] = ( vec4 ){.x = PV.m[3] + PV.m[1], .y = PV.m[7] + PV.m[5], .z = PV.m[11] + PV.m[9], .w = PV.m[15] + PV.m[13]};  // bottom clipping plane
  planes_xyxd[3] = ( vec4 ){.x = PV.m[3] - PV.m[1], .y = PV.m[7] - PV.m[5], .z = PV.m[11] - PV.m[9], .w = PV.m[15] - PV.m[13]};  // top clipping plane
  planes_xyxd[4] = ( vec4 ){.x = PV.m[3] + PV.m[2], .y = PV.m[7] + PV.m[6], .z = PV.m[11] + PV.m[10], .w = PV.m[15] + PV.m[14]}; // near clipping plane
  planes_xyxd[5] = ( vec4 ){.x = PV.m[3] - PV.m[2], .y = PV.m[7] - PV.m[6], .z = PV.m[11] - PV.m[10], .w = PV.m[15] - PV.m[14]}; // far clipping plane
  if ( normalise_planes ) {
    for ( int i = 0; i < 6; i++ ) { planes_xyxd[i] = normalise_plane( planes_xyxd[i] ); }
  }
}

static inline versor div_quat_f( versor qq, float s ) { return ( versor ){.w = qq.w / s, .x = qq.x / s, .y = qq.y / s, .z = qq.z / s}; }

static inline versor mult_quat_f( versor qq, float s ) { return ( versor ){.w = qq.w * s, .x = qq.x * s, .y = qq.y * s, .z = qq.z * s}; }

/* Rotates vector v using quaternion q by calculating the sandwich product: v' = qvq^-1
From pg 89 in E.Lengyel's "FOGED: Mathematics".
Another version (may be faster?):
  t = 2 * cross(q.xyz, v)
  v' = v + q.w * t + cross(q.xyz, t)
found https://blog.molecular-matters.com/2013/05/24/a-faster-quaternion-vector-multiplication/
attributed to a post by Fabian Giesen (no longer online).
TODO(Anton) WARNING not tested. */
static inline vec3 mult_quat_vec3( versor q, vec3 v ) {
  vec3 b      = ( vec3 ){.x = q.x, .y = q.y, .z = q.z};
  float b2    = b.x * b.x + b.y * b.y + b.z * b.z;
  vec3 part_a = mult_vec3_f( v, q.w * q.w - b2 );
  vec3 part_b = mult_vec3_f( b, dot_vec3( v, b ) * 2.0f );
  vec3 part_c = mult_vec3_f( cross_vec3( b, v ), q.w * 2.0f );
  vec3 out    = add_vec3_vec3( part_a, add_vec3_vec3( part_b, part_c ) );
  return out;
}

/* This function is useful for maintaining a versor's unit quaternion state. */
static inline versor normalise_quat( versor q ) {
  float sum          = q.w * q.w + q.x * q.x + q.y * q.y + q.z * q.z;
  const float thresh = 0.0001f;
  if ( fabs( 1.0f - sum ) < thresh ) { return q; }
  float mag = sqrtf( sum );
  return div_quat_f( q, mag );
}

static inline versor mult_quat_quat( versor a, versor b ) {
  versor result;
  result.w = b.w * a.w - b.x * a.x - b.y * a.y - b.z * a.z;
  result.x = b.w * a.x + b.x * a.w - b.y * a.z + b.z * a.y;
  result.y = b.w * a.y + b.x * a.z + b.y * a.w - b.z * a.x;
  result.z = b.w * a.z - b.x * a.y + b.y * a.x + b.z * a.w;
  return normalise_quat( result );
}

static inline versor add_quat_quat( versor a, versor b ) {
  versor result;
  result.w = b.w + a.w;
  result.x = b.x + a.x;
  result.y = b.y + a.y;
  result.z = b.z + a.z;
  return normalise_quat( result );
}

/* Create a versor representing from an arbitrary axis and an angle to rotate about that axis. */
static inline versor quat_from_axis_rad( float radians, vec3 axis ) {
  versor result;
  result.w = cosf( radians / 2.0 );
  result.x = sinf( radians / 2.0 ) * axis.x;
  result.y = sinf( radians / 2.0 ) * axis.y;
  result.z = sinf( radians / 2.0 ) * axis.z;
  return result;
}

static inline versor quat_from_axis_deg( float degrees, vec3 axis ) { return quat_from_axis_rad( APG_M_ONE_DEG_IN_RAD * degrees, axis ); }

/* Creates a 4x4 rotation matrix from a quaternion. See also: mult_quat_vec3().
Note: it is also possible to create a quaternion from a matrix. See pg 93 in E.Lengyel's "FOGED: Mathematics" */
static inline mat4 quat_to_mat4( versor q ) {
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

/* Dot product of two quaternions. */
static inline float dot_quat( versor q, versor r ) { return q.w * r.w + q.x * r.x + q.y * r.y + q.z * r.z; }

/* Spherical interpolation between two quaternions.
RETURNS q when t = 0, r when t = 1, and an intermediate quaternion at values between 0 and 1. */
static inline versor slerp_quat( versor q, versor r, float t ) {
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

/* Reduce or normalise an angle in degrees into the repeating range of [0, 360] */
static inline float wrap_degrees_360( float degrees ) {
  if ( degrees >= 0.0f && degrees < 360.0f ) { return degrees; }
  int multiples = (int)( degrees / 360.0f );
  if ( degrees > 0.0f ) {
    degrees = degrees - (float)multiples * 360.0f;
  } else {
    degrees = degrees + (float)multiples * 360.0f;
  }
  return degrees;
}

/* Returns the absolute difference, in wrapped [0, 360] range, between two angles in degrees.
The input angles do not need to be expressed in the [0, 360] range. */
static inline float abs_diff_btw_degrees( float first, float second ) {
  first  = wrap_degrees_360( first );
  second = wrap_degrees_360( second );

  float diff = fabsf( first - second );
  if ( diff >= 180.0f ) { diff = fabsf( diff - 360.0f ); }
  return diff;
}

/* RETURNS t, The distance along the infinite line of the ray from ray origin to intersection.
If t is negative then intersection is a 'miss' (intersection behind ray origin).
Intersection 3D co-ordinates xyz are then `ray_origin + ray_direction * t`. */
static inline float ray_plane( vec3 ray_origin, vec3 ray_direction, vec3 plane_normal, float plane_d ) {
  return -( dot_vec3( ray_origin, plane_normal ) + plane_d ) / dot_vec3( ray_direction, plane_normal );
}

/* Ray - axis-aligned bounding box (AABB) geometric intersection test.
Adapted from https://psgraphics.blogspot.com/2016/02/new-simple-ray-box-test-from-andrew.html */
static inline bool ray_aabb( vec3 ray_origin, vec3 ray_direction, vec3 aabb_min, vec3 aabb_max, float tmin, float tmax ) {
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

/* Ray - oriented bounding box (OBB) geometric intersection test.
PARAMS
  box      - Definition of a cuboid volume's bounds and orientation.
  ray_o    - XYZ co-ordinates of the ray's origin.
  ray_d    - Unit vector representing the ray's direction.
  t        - Intersection distance along ray.
  face_num - The "slab" index (1,2,3) corresponding to box side direction intersected. face_num will be negative for the opposing side.
             Note that it's not (0,1,2) because negative zero for the opposing face would be problematic.
RETURNS true if the ray intersects with the box. */
static inline bool ray_obb( obb_t box, vec3 ray_o, vec3 ray_d, float* t, int* face_num ) {
  assert( t );
  *t         = 0.0f;
  float tmin = -INFINITY;
  float tmax = INFINITY;
  int imin = 0, imax = 0;
  vec3 p = sub_vec3_vec3( box.centre, ray_o );
  for ( int i = 0; i < 3; i++ ) { // 3 "slabs" (pair of front/back planes)
    float e = dot_vec3( box.norm_side_dir[i], p );
    float f = dot_vec3( box.norm_side_dir[i], ray_d );
    if ( fabs( f ) > FLT_EPSILON ) {
      float t1 = ( e + box.half_lengths[i] ) / f; // intersection on front
      float t2 = ( e - box.half_lengths[i] ) / f; // and back side of slab
      if ( t1 > t2 ) {
        float tmp = t1;
        t1        = t2;
        t2        = tmp;
      }
      if ( t1 > tmin ) {
        tmin = t1;
        imin = i;
      }
      if ( t2 < tmax ) {
        tmax = t2;
        imax = -i;
      }
      if ( tmin > tmax ) { return false; }
      if ( tmax < 0 ) { return false; }
    } else if ( -e - box.half_lengths[i] > 0 || -e + box.half_lengths[i] < 0 ) {
      return false;
    }
  }
  *t        = tmin > 0 ? tmin : tmax;
  *face_num = tmin > 0 ? imin + 1 : imax + 1;
  return true;
}

/* Derive the Axis-Aligned Bounding Box for a camera frustum.
PARAMS
  PV         - Perspective * View virtual camera matrix.
RETURNS Axis-aligned box surrounding the frustum. Note that this is usually very large. */
static inline aabb_t frustum_to_aabb( mat4 PV ) {
  aabb_t aabb = ( aabb_t ){.min.x = 0.0f};
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

/* Intersection test for Axis-Oriented Bounding Box with Axis-Oriented Bounding Box.
PARAMS
  a, b - The two axis-aligned bounding boxes.
RETURNS true if box A intersects box B
*/
static inline bool aabb_aabb( aabb_t a, aabb_t b ) {
  return ( a.min.x <= b.max.x && a.max.x >= b.min.x ) && ( a.min.y <= b.max.y && a.max.y >= b.min.y ) && ( a.min.z <= b.max.z && a.max.z >= b.min.z );
}

/* Distance between a point and a plane. This is a specialised dot product.
PARAMS
  planes_xyzd - A plane with coefficients in the form: ax + by + cz + d = 0 (normal xyz,d). Where -d (note the negative) is the distance from the origin to the
plane surface in the direction of the normal (xyz).
  point       - The 3D point.
RETURNS A positive value if the point is in front of the plane, or a negative value if the point is behind the plane
REMARKS The function does not normalise the input plane, and will operate with non-unit vector normals.
See also https://mathworld.wolfram.com/Point-PlaneDistance.html.
*/
static inline float distance_plane_point( vec4 plane_xyzd, vec3 point ) {
  return plane_xyzd.x * point.x + plane_xyzd.y * point.y + plane_xyzd.z * point.z + plane_xyzd.w;
}

/* Intersection test for frustum with Axis-Oriented Bounding Box.
Using the 'check if each point is on the visible side of each frustum plane' test as described at:
https://iquilezles.org/www/articles/frustumcorrect/frustumcorrect.htm
Modified with the " 3.4.2 Distance Between a Point and a Plane" method from FoGED Vol. 1 by Eric Lengyel.
PARAMS
  frustum_planes     - A buffer of 6x vec4 to set world space planes. Must not be NULL. The plane coefficients are in the form: ax + by + cz + d = 0 (normal
xyz,d). Where -d (note the negative) is the distance from the origin to the plane surface in the direction of the normal (xyz).
  aabb               - Diagonally-opposed corner points of the box.
RETURNS true if the AABB is in the frustum.
*/
static inline bool frustum_vs_aabb( const vec4* frustum_planes, const vec3* frustum_points, aabb_t box ) {
  for ( int i = 0; i < 6; i++ ) {
    int out = 0;
    if ( distance_plane_point( frustum_planes[i], ( vec3 ){box.min.x, box.min.y, box.min.z} ) < 0.0 ) { out++; }
    if ( distance_plane_point( frustum_planes[i], ( vec3 ){box.min.x, box.min.y, box.max.z} ) < 0.0 ) { out++; }
    if ( distance_plane_point( frustum_planes[i], ( vec3 ){box.min.x, box.max.y, box.min.z} ) < 0.0 ) { out++; }
    if ( distance_plane_point( frustum_planes[i], ( vec3 ){box.min.x, box.max.y, box.max.z} ) < 0.0 ) { out++; }
    if ( distance_plane_point( frustum_planes[i], ( vec3 ){box.max.x, box.min.y, box.min.z} ) < 0.0 ) { out++; }
    if ( distance_plane_point( frustum_planes[i], ( vec3 ){box.max.x, box.min.y, box.max.z} ) < 0.0 ) { out++; }
    if ( distance_plane_point( frustum_planes[i], ( vec3 ){box.max.x, box.max.y, box.min.z} ) < 0.0 ) { out++; }
    if ( distance_plane_point( frustum_planes[i], ( vec3 ){box.max.x, box.max.y, box.max.z} ) < 0.0 ) { out++; }
    if ( 8 == out ) { return false; }
  }
  // check frustum outside/inside box
  // NOTE: may be wrong
  int out = 0;
  for ( int i = 0, out = 0; i < 8; i++ ) {
    if ( frustum_points[i].x > box.max.x ) { out++; }
  }
  if ( out == 8 ) { return false; }

  for ( int i = 0, out = 0; i < 8; i++ ) {
    if ( frustum_points[i].x < box.min.x ) { out++; }
  }
  if ( out == 8 ) { return false; }

  for ( int i = 0, out = 0; i < 8; i++ ) {
    if ( frustum_points[i].y > box.max.y ) { out++; }
  }
  if ( out == 8 ) { return false; }

  for ( int i = 0, out = 0; i < 8; i++ ) {
    if ( frustum_points[i].y < box.min.y ) { out++; }
  }
  if ( out == 8 ) { return false; }

  for ( int i = 0, out = 0; i < 8; i++ ) {
    if ( frustum_points[i].z > box.max.z ) { out++; }
  }
  if ( out == 8 ) { return false; }

  for ( int i = 0, out = 0; i < 8; i++ ) {
    if ( frustum_points[i].z < box.min.z ) { out++; }
  }
  if ( out == 8 ) { return false; }

  return true;
}

#ifdef __cplusplus
}
#endif

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
*/
