/* ===============================================================================================
Anton's 3D Maths Library (C99 version)
URL:     https://github.com/capnramses/apg
Licence: See bottom of file.
Author:  Anton Gerdelan <antonofnote at gmail> @capnramses
==================================================================================================
History:
v0.12  18 Feb 2021 - Small fixes to reduce warnings with MSVC.
v0.11  17 Feb 2021 - Bug fix: ray-obb negative face indices reporting correctly. Switched from single-header to 2 files.
v0.10   7 Feb 2021 - Tidied docs.
v0.9 -  2 Feb 2021 - Orthographic.
v0.8 - 21 May 2020 - AABB intersection and frustum bounds and plane-extraction functions.
v0.7 - 19 Apr 2020 - Removed M_PI, replaced with APG_PI. Added prefix to macros to avoid redefinition vulnerability.
v0.6 -  9 Apr 2020 - Frustum extraction functions.
v0.5 - 20 Nov 2019 - Added a cpp clause.
v0.4 - 17 Jul 2019 - Updated with maths code from voxel game project.
v0.3 - 11 Apr 2016 - Compacted.
v0.2 - 12 Apr 2016 - Switched to .x .y .z notation for vectors and quaternions.
v0.1 -  5 May 2015 - Branched from C++ original.
==================================================================================================
TODO:
Vectors
  project and reject vectors
Matrix
  arbitrary axis rot
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

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h> // `bool` in C99
#include <stdio.h>   // Used for printing vectors to stdout.

#define APG_M_PI 3.14159265358979323846f                  // Stable pi. C99 removed M_PI and it is .: not stable between builds.
#define APG_M_ONE_DEG_IN_RAD ( 2.0f * APG_M_PI ) / 360.0f // 0.017444444
#define APG_M_ONE_RAD_IN_DEG 360.0f / ( 2.0f * APG_M_PI ) // 57.2957795

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

/** 'Versor' is the proper name for a unit quaternion (the kind used for geometric rotations). */
typedef struct versor {
  float w, x, y, z;
} versor;

/** Oriented Bounding Box (OBB) volume and orientation definition. */
typedef struct obb_t {   /// 'A' in Real-Time Rendering book notation.
  vec3 centre;           /// a^c in RTR notation.
  vec3 norm_side_dir[3]; /// a^u, a^v, a^w in RTR notation.
  float half_lengths[3]; /// Distance from the centre to each face in basis vector directions. Must be positive. h_u, h_v, h_w in RTR notation.
} obb_t;

/** Axis-Aligned Bounding Box (AABB) volume given by two diagonally-opposing corners. */
typedef struct aabb_t {
  vec3 min, max;
} aabb_t;

void print_vec2( vec2 v );
void print_vec3( vec3 v );
void print_vec4( vec4 v );
void print_mat4( mat4 m );
void print_quat( versor q );

vec3 v3_v4( vec4 v );
vec4 v4_v3f( vec3 v, float f );

vec3 add_vec3_f( vec3 a, float b );
vec3 add_vec3_vec3( vec3 a, vec3 b );

vec3 sub_vec3_f( vec3 a, float b );
vec2 sub_vec2_vec2( vec2 a, vec2 b );
vec3 sub_vec3_vec3( vec3 a, vec3 b );

vec3 mult_vec3_f( vec3 a, float b );
vec3 mult_vec3_vec3( vec3 a, vec3 b );

vec3 div_vec3_f( vec3 a, float b );
vec3 div_vec3_vec3( vec3 a, vec3 b );
vec4 div_vec4_f( vec4 v, float f );

/** Magnitude or length of a vec2. */
float length_vec2( vec2 v );

/** Squared length. */
float length2_vec2( vec2 v );

/** Magnitude or length of a vec3. */
float length_vec3( vec3 v );

/** Squared length. */
float length2_vec3( vec3 v );

vec3 normalise_vec3( vec3 v );

/** Note that plane normalisation also affects the d component of the plane, but only the xyz normal component is unit-length afterwards.
Based on FoGED Vol 1 Ch 3.4.2 pg 115 by Eric Lengyel. */
vec4 normalise_plane( vec4 xyzd );

float dot_vec3( vec3 a, vec3 b );

vec3 cross_vec3( vec3 a, vec3 b );

/** Converts an un-normalised direction vector's X,Z components into a heading in degrees. */
float vec3_to_heading( vec3 d );

/** Very informal function to convert a heading (e.g. y-axis orientation) into a 3D vector with components in x and z axes. */
vec3 heading_to_vec3( float degrees );

mat4 identity_mat4();

mat4 mult_mat4_mat4( mat4 a, mat4 b );

vec4 mult_mat4_vec4( mat4 m, vec4 v );

float det_mat4( mat4 mm );

// TODO(Anton) look up fast inverse video tutorial
mat4 inverse_mat4( mat4 mm );

mat4 transpose_mat4( mat4 mm );

mat4 translate_mat4( vec3 vv );

mat4 rot_x_deg_mat4( float deg );

mat4 rot_y_deg_mat4( float deg );

mat4 rot_z_deg_mat4( float deg );

mat4 scale_mat4( vec3 v );

/** Creates a view matrix, using typical "look at" parameters. Most graphics mathematics libraries have a similar function. */
mat4 look_at( vec3 cam_pos, vec3 targ_pos, vec3 up );

/** Creates an orthographic projection matrix. */
mat4 orthographic( float l, float r, float b, float t, float n, float f );

/** Creates a perspective projection matrix. */
mat4 perspective( float fovy, float aspect, float near_plane, float far_plane );

/** Create a standard *asymmetric* perspective projection matrix for special case of a subwindow viewport
- original viewport from (0,0) with size (vp_w, vp_h)
- subwindow viewport from (subvp_x,subvp_y) with size (subvp_w,subvp_h)
- Note: mouse coords, if used, may required a y direction flip.
- Note: this function does not modify near or far plane. It could do my adding z scaling to M.
- Note: this function uses an axis-parallel subwindow but it could be modified to a parallelogram shape.
Code based on excellent problem description here: https://stackoverflow.com/questions/50110934/display-recursively-rendered-scene-into-a-plane
*/
mat4 perspective_offcentre_viewport( int vp_w, int vp_h, int subvp_x, int subvp_y, int subvp_w, int subvp_h, mat4 P_orig );

/** Takes any world-to-clip space matrix and inverts to retrieve the 8 corner points in world space.
 * Based on http://donw.io/post/frustum-point-extraction/
 *
 * @param PV      Any world-to-clip space matrix.
 * @param corners A buffer of 8x vec3. Must not be NULL. World space corner points will be set here.
 *
 * @note Expect some floating small point error compared to original world space points.
 */
void frustum_points_from_PV( mat4 PV, vec3* corners_wor );

/** Fast extraction of 6 clip planes.
 * Based on http://www8.cs.umu.se/kurser/5DV051/HT12/lab/plane_extraction.pdf
 * @param PV               Any world-to-clip space matrix.
 * @param corners          A buffer of 6x vec4 to set world space planes. Must not be NULL. The plane coefficients are in the form: [n|d].
 * @param normalise_planes Set to false if you only need the planes for in-front/behind tests, and not distance of point from plane.
 *
 * @note That plane normalisation also affects the d component of the plane, but only the xyz normal component is unit-length afterwards.
 */
void frustum_planes_from_PV( mat4 PV, vec4* planes_xyxd, bool normalise_planes );

versor div_quat_f( versor qq, float s );

versor mult_quat_f( versor qq, float s );

/** Rotates vector v using quaternion q by calculating the sandwich product: v' = qvq^-1
 * From pg 89 in E.Lengyel's "FOGED: Mathematics".
 *
 * Another version (may be faster?):
 *  t = 2 * cross(q.xyz, v)
 *  v' = v + q.w * t + cross(q.xyz, t)
 * Found https://blog.molecular-matters.com/2013/05/24/a-faster-quaternion-vector-multiplication/
 * Attributed to a post by Fabian Giesen (no longer online).
 *
 * @warning This function has not been unit-tested. TODO(Anton)
 */
vec3 mult_quat_vec3( versor q, vec3 v );

/** This function is useful for maintaining a versor's unit quaternion state. */
versor normalise_quat( versor q );

versor mult_quat_quat( versor a, versor b );

versor add_quat_quat( versor a, versor b );

/** Create a versor representing from an arbitrary axis and an angle to rotate about that axis. */
versor quat_from_axis_rad( float radians, vec3 axis );

versor quat_from_axis_deg( float degrees, vec3 axis );

/** Creates a 4x4 rotation matrix from a quaternion. See also: mult_quat_vec3().
 * @note It is also possible to create a quaternion from a matrix.
 * See pg 93 in E.Lengyel's "FOGED: Mathematics".
 */
mat4 quat_to_mat4( versor q );

/** Dot product of two quaternions. */
float dot_quat( versor q, versor r );

/** Spherical interpolation between two quaternions.
 * @return q when t = 0, r when t = 1, and an intermediate quaternion at values between 0 and 1.
 */
versor slerp_quat( versor q, versor r, float t );

/** Reduce or normalise an angle in degrees into the repeating range of [0, 360]. */
float wrap_degrees_360( float degrees );

/** Returns the absolute difference, in wrapped [0, 360] range, between two angles in degrees.
 * The input angles do not need to be expressed in the [0, 360] range.
 */
float abs_diff_btw_degrees( float first, float second );

/** RETURNS t, The distance along the infinite line of the ray from ray origin to intersection.
 * If t is negative then intersection is a 'miss' (intersection behind ray origin).
 * Intersection 3D co-ordinates xyz are then `ray_origin + ray_direction * t`.
 */
float ray_plane( vec3 ray_origin, vec3 ray_direction, vec3 plane_normal, float plane_d );

/** Ray - axis-aligned bounding box (AABB) geometric intersection test.
 * Adapted from https://psgraphics.blogspot.com/2016/02/new-simple-ray-box-test-from-andrew.html
 */
bool ray_aabb( vec3 ray_origin, vec3 ray_direction, vec3 aabb_min, vec3 aabb_max, float tmin, float tmax );

/** Ray - oriented bounding box (OBB) geometric intersection test.
 * @param  box      Definition of a cuboid volume's bounds and orientation.
 * @param ray_o     XYZ co-ordinates of the ray's origin.
 * @param  ray_d    Unit vector representing the ray's direction.
 * @param  t        Intersection distance along ray.
 * @param  face_num The "slab" index (1,2,3) corresponding to box side direction intersected. face_num will be negative for the opposing side.
 *                  Note that it's not (0,1,2) because negative zero for the opposing face would be problematic.
 * @return          True if the ray intersects with the box.
 */
bool ray_obb( obb_t box, vec3 ray_o, vec3 ray_d, float* t, int* face_num );

/** Derive the Axis-Aligned Bounding Box for a camera frustum.
 * @param  PV Perspective * View virtual camera matrix.
 * @return    Axis-aligned box surrounding the frustum. Note that this is usually very large.
 */
aabb_t frustum_to_aabb( mat4 PV );

/** Intersection test for Axis-Oriented Bounding Box with Axis-Oriented Bounding Box.
 * @param a The first axis-aligned bounding box.
 * @param b The second axis-aligned bounding box.
 * @return  True if box A intersects box B.
 */
bool aabb_aabb( aabb_t a, aabb_t b );

/** Distance between a point and a plane. This is a specialised dot product.
 * @param planes_xyzd A plane with coefficients in the form: ax + by + cz + d = 0 (normal xyz,d).
 *                    Where -d (note the negative) is the distance from the origin to the plane surface in the direction of the normal (xyz).
 * @param point       The 3D point.
 * @return A positive value if the point is in front of the plane, or a negative value if the point is behind the plane
 * @note The function does not normalise the input plane, and will operate with non-unit vector normals.
 * See also https://mathworld.wolfram.com/Point-PlaneDistance.html.
 */
float distance_plane_point( vec4 plane_xyzd, vec3 point );

/** Intersection test for frustum with Axis-Oriented Bounding Box.
 * Using the 'check if each point is on the visible side of each frustum plane' test as described at:
 * https://iquilezles.org/www/articles/frustumcorrect/frustumcorrect.htm
 * Modified with the " 3.4.2 Distance Between a Point and a Plane" method from FoGED Vol. 1 by Eric Lengyel.
 * @param frustum_planes A buffer of 6x vec4 to set world space planes. Must not be NULL. The plane coefficients are in the form: ax + by + cz + d = 0 (normal
 * xyz,d). Where -d (note the negative) is the distance from the origin to the plane surface in the direction of the normal (xyz).
 * @param aabb           Diagonally-opposed corner points of the box.
 * @return True if the AABB is in the frustum.
 */
bool frustum_vs_aabb( const vec4* frustum_planes, aabb_t box );

bool frustum_vs_aabb_extra_check( const vec4* frustum_planes, const vec3* frustum_points, aabb_t box );

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
