/*
                         Anton's 3d Maths Library
                             Clang C99 branch

                            Dr Anton Gerdelan
                         Trinity College Dublin
                                Ireland
                          <gerdela@scss.tcd.ie>
                        this version: 12 Feb 2016

This branch only works with the Clang C compiler as it uses Clang's OpenCL
vector data type extensions.
For the C++ version see the .hpp file.

  * Commonly-used maths structures and functions, resembling GLSL.
  * Simple-as-possible.
  * Function names have suffix to denote type as C has no function overloading.
  * vec2, vec3, vec4, versor components can be accessed with GLSL style
    "swizzling" e.g. V.xyzw or V.zy, or by array-like access i.e. V[2]
  * a versor is the proper name for a unit quaternion.

#include this header file.
In C we need to link math.h's libm: "-lm"
*/
#pragma once

#include <stdio.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <string.h> // memset, memcpy

// C99 removed M_PI
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
// const used to convert degrees into radians
#define ONE_DEG_IN_RAD (2.0 * M_PI) / 360.0 // 0.017444444
#define ONE_RAD_IN_DEG 360.0 / (2.0 * M_PI) //57.2957795

typedef float vec4 __attribute__((ext_vector_type(4))); // OpenCL style
typedef float vec3 __attribute__((ext_vector_type(3)));
typedef float vec2 __attribute__((ext_vector_type(2)));
typedef float versor __attribute__((ext_vector_type(4)));
typedef float mat4 __attribute__((ext_vector_type(16)));
typedef float mat3 __attribute__((ext_vector_type(9)));

void print_vec2 (vec2 v);
void print_vec3 (vec3 v);
void print_vec4 (vec4 v);
void print_mat4 (mat4 m);
void print_quat (versor q);

// vector functions -- geometric
// vector magnitude
float length_vec3 (vec3 v);
// squared vector length
float length2_vec3 (vec3 v);
//
vec3 normalise_vec3 (vec3 v);
// dot product
float dot_vec3 (vec3 a, vec3 b);
// cross product
vec3 cross_vec3 (vec3 a, vec3 b);
// an arbitrary -Z == 0 degrees, -X = 90 degrees etc. thing
float vec3_to_heading (vec3 d);
// reverse of the above
vec3 heading_to_vec3 (float degrees);

mat4 zero_mat4 ();
mat4 identity_mat4 ();
mat4 mat4_mul_mat4 (mat4 a, mat4 b);
vec4 mat4_mul_vec4 (mat4 m, vec4 v);
// determinant
float det (mat4 mm);
mat4 inverse (mat4 mm);
mat4 transpose (mat4 mm);

// matrix functions -- affine functions
mat4 translate_mat4 (vec3 v);
mat4 rot_x_deg_mat4 (float deg);
mat4 rot_y_deg_mat4 (float deg);
mat4 rot_z_deg_mat4 (float deg);
mat4 scale_mat4 (vec3 v);

// matrix functions -- camera functions
mat4 look_at (vec3 cam_pos, vec3 targ_pos, vec3 up);
mat4 perspective (float fovy, float aspect, float near, float far);

// quaternion functions -- geometric
versor quat_from_axis_rad (float rad, vec3 axis);
versor quat_from_axis_deg (float deg, vec3 axis);
// note: NOTE the same as the built-in q * q component-wise vector function
versor quat_mul_quat (versor a, versor b);
// same as q + q but does normalisation check
versor add_quat_quat (versor a, versor b);
mat4 quat_to_mat4 (versor q);
float dot_quat (versor q, versor r);
versor normalise_quat (versor q);

// quaternion functions -- interpolation
versor slerp_quat (versor q, versor r, float t);

/*-----------------------------PRINT FUNCTIONS-------------------------------*/
inline void print_vec2 (vec2 v) {
	printf ("[%.2f, %.2f]\n", v.x, v.y);
}

inline void print_vec3 (vec3 v) {
	printf ("[%.2f, %.2f, %.2f]\n", v.x, v.y, v.z);
}

inline void print_vec4 (vec4 v) {
	printf ("[%.2f, %.2f, %.2f, %.2f]\n", v.x, v.y, v.z, v.w);
}

inline void print_mat4 (mat4 m) {
	printf("\n");
	printf ("[%.2f][%.2f][%.2f][%.2f]\n", m[0], m[4], m[8], m[12]);
	printf ("[%.2f][%.2f][%.2f][%.2f]\n", m[1], m[5], m[9], m[13]);
	printf ("[%.2f][%.2f][%.2f][%.2f]\n", m[2], m[6], m[10], m[14]);
	printf ("[%.2f][%.2f][%.2f][%.2f]\n", m[3], m[7], m[11], m[15]);
}

inline void print_quat (versor q) {
	printf ("[%.2f ,%.2f, %.2f, %.2f]\n", q[0], q[1], q[2], q[3]);
}
/*------------------------------VECTOR FUNCTIONS-----------------------------*/
inline float length_vec3 (vec3 v) {
	return sqrt (v.x * v.x + v.y * v.y + v.z * v.z);
}

// squared length
inline float length2_vec3 (vec3 v) {
	return v.x * v.x + v.y * v.y + v.z * v.z;
}

// note: proper spelling (hehe)
inline vec3 normalise_vec3 (vec3 v) {
	float l = length_vec3 (v);
	if (0.0f == l) {
		return (vec3){0.0f, 0.0f, 0.0f};
	}
	return v / l;
}

inline float dot_vec3 (vec3 a, vec3 b) {
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

inline vec3 cross_vec3 (vec3 a, vec3 b) {
	float x = a.y * b.z - a.z * b.y;
	float y = a.z * b.x - a.x * b.z;
	float z = a.x * b.y - a.y * b.x;
	return (vec3){x, y, z};
}

// converts an un-normalised direction vector's X,Z components into a heading
// in degrees
// NB i suspect that the z is backwards here but i've used in in
// several places like this. d'oh!
inline float vec3_to_heading (vec3 d) {
	return atan2 (-d.x, -d.z) * ONE_RAD_IN_DEG;
}

// very informal function to convert a heading (e.g. y-axis orientation) into
// a 3d vector with components in x and z axes
inline vec3 heading_to_vec3 (float degrees) {
	float rad = degrees * ONE_DEG_IN_RAD;
	return (vec3){-sinf (rad), 0.0f, -cosf (rad)};
}

/*-----------------------------MATRIX FUNCTIONS------------------------------*/
inline mat4 zero_mat4 () {
	mat4 r;
	memset (&r, 0, sizeof (mat4));
	return r;
}

inline mat4 identity_mat4 () {
	mat4 r = zero_mat4 ();
	r[0] = 1.0f;
	r[5] = 1.0f;
	r[10] = 1.0f;
	r[15] = 1.0f;
	return r;
}

inline mat4 mat4_mul_mat4 (mat4 a, mat4 b) {
	mat4 r = zero_mat4 ();
	int r_index = 0;
	for (int col = 0; col < 4; col++) {
		for (int row = 0; row < 4; row++) {
			float sum = 0.0f;
			for (int i = 0; i < 4; i++) {
				sum += b[i + col * 4] * a[row + i * 4];
			}
			r[r_index] = sum;
			r_index++;
		}
	}
	return r;
}

inline vec4 mat4_mul_vec4 (mat4 m, vec4 v) {
	float x = m[0] * v.x + m[4] * v.y + m[8] * v.z + m[12] * v.w;
	float y = m[1] * v.x + m[5] * v.y + m[9] * v.z + m[13] * v.w;
	float z = m[2] * v.x + m[6] * v.y + m[10] * v.z + m[14] * v.w;
	float w = m[3] * v.x + m[7] * v.y + m[11] * v.z + m[15] * v.w;
	return (vec4){x, y, z, w};
}

// returns a scalar value with the determinant for a 4x4 matrix
// see http://www.euclideanspace.com/maths/algebra/matrix/functions/determinant/fourD/index.htm
inline float det_mat4 (mat4 mm) {
	return
		mm[12] * mm[9] * mm[6] * mm[3] -
		mm[8] * mm[13] * mm[6] * mm[3] -
		mm[12] * mm[5] * mm[10] * mm[3] +
		mm[4] * mm[13] * mm[10] * mm[3] +
		mm[8] * mm[5] * mm[14] * mm[3] -
		mm[4] * mm[9] * mm[14] * mm[3] -
		mm[12] * mm[9] * mm[2] * mm[7] +
		mm[8] * mm[13] * mm[2] * mm[7] +
		mm[12] * mm[1] * mm[10] * mm[7] -
		mm[0] * mm[13] * mm[10] * mm[7] -
		mm[8] * mm[1] * mm[14] * mm[7] +
		mm[0] * mm[9] * mm[14] * mm[7] +
		mm[12] * mm[5] * mm[2] * mm[11] -
		mm[4] * mm[13] * mm[2] * mm[11] -
		mm[12] * mm[1] * mm[6] * mm[11] +
		mm[0] * mm[13] * mm[6] * mm[11] +
		mm[4] * mm[1] * mm[14] * mm[11] -
		mm[0] * mm[5] * mm[14] * mm[11] -
		mm[8] * mm[5] * mm[2] * mm[15] +
		mm[4] * mm[9] * mm[2] * mm[15] +
		mm[8] * mm[1] * mm[6] * mm[15] -
		mm[0] * mm[9] * mm[6] * mm[15] -
		mm[4] * mm[1] * mm[10] * mm[15] +
		mm[0] * mm[5] * mm[10] * mm[15];
}

// TODO(anton) pretty sure there's a nicer method in Lengyel's book
// returns a 16-element array that is the inverse of a 16-element array (4x4
// matrix).
// see http://www.euclideanspace.com/maths/algebra/matrix/functions/inverse/fourD/index.htm
inline mat4 inverse_mat4 (mat4 mm) {
	float det = det_mat4 (mm);
	/* there is no inverse if determinant is zero (not likely unless scale is
	broken) */
	if (0.0f == det) {
		fprintf (stderr, "WARNING. matrix has no determinant. can not invert\n");
		return mm;
	}
	float inv_det = 1.0f / det;
	
	mat4 r;
	r[0] = inv_det * (
		mm[9] * mm[14] * mm[7] - mm[13] * mm[10] * mm[7] +
		mm[13] * mm[6] * mm[11] - mm[5] * mm[14] * mm[11] -
		mm[9] * mm[6] * mm[15] + mm[5] * mm[10] * mm[15]
	);
	r[1] = inv_det * (
		mm[13] * mm[10] * mm[3] - mm[9] * mm[14] * mm[3] -
		mm[13] * mm[2] * mm[11] + mm[1] * mm[14] * mm[11] +
		mm[9] * mm[2] * mm[15] - mm[1] * mm[10] * mm[15]
	);
	r[2] = inv_det * (
		mm[5] * mm[14] * mm[3] - mm[13] * mm[6] * mm[3] +
		mm[13] * mm[2] * mm[7] - mm[1] * mm[14] * mm[7] -
		mm[5] * mm[2] * mm[15] + mm[1] * mm[6] * mm[15]
	);
	r[3] = inv_det * (
		mm[9] * mm[6] * mm[3] - mm[5] * mm[10] * mm[3] -
		mm[9] * mm[2] * mm[7] + mm[1] * mm[10] * mm[7] +
		mm[5] * mm[2] * mm[11] - mm[1] * mm[6] * mm[11]
	);
	r[4] = inv_det * (
		mm[12] * mm[10] * mm[7] - mm[8] * mm[14] * mm[7] -
		mm[12] * mm[6] * mm[11] + mm[4] * mm[14] * mm[11] +
		mm[8] * mm[6] * mm[15] - mm[4] * mm[10] * mm[15]
	);
	r[5] = inv_det * (
		mm[8] * mm[14] * mm[3] - mm[12] * mm[10] * mm[3] +
		mm[12] * mm[2] * mm[11] - mm[0] * mm[14] * mm[11] -
		mm[8] * mm[2] * mm[15] + mm[0] * mm[10] * mm[15]
	);
	r[6] = inv_det * (
		mm[12] * mm[6] * mm[3] - mm[4] * mm[14] * mm[3] -
		mm[12] * mm[2] * mm[7] + mm[0] * mm[14] * mm[7] +
		mm[4] * mm[2] * mm[15] - mm[0] * mm[6] * mm[15]
	);
	r[7] = inv_det * (
		mm[4] * mm[10] * mm[3] - mm[8] * mm[6] * mm[3] +
		mm[8] * mm[2] * mm[7] - mm[0] * mm[10] * mm[7] -
		mm[4] * mm[2] * mm[11] + mm[0] * mm[6] * mm[11]
	);
	r[8] = inv_det * (
		mm[8] * mm[13] * mm[7] - mm[12] * mm[9] * mm[7] +
		mm[12] * mm[5] * mm[11] - mm[4] * mm[13] * mm[11] -
		mm[8] * mm[5] * mm[15] + mm[4] * mm[9] * mm[15]
	);
	r[9] = inv_det * (
		mm[12] * mm[9] * mm[3] - mm[8] * mm[13] * mm[3] -
		mm[12] * mm[1] * mm[11] + mm[0] * mm[13] * mm[11] +
		mm[8] * mm[1] * mm[15] - mm[0] * mm[9] * mm[15]
	);
	r[10] = inv_det * (
		mm[4] * mm[13] * mm[3] - mm[12] * mm[5] * mm[3] +
		mm[12] * mm[1] * mm[7] - mm[0] * mm[13] * mm[7] -
		mm[4] * mm[1] * mm[15] + mm[0] * mm[5] * mm[15]
	);
	r[11] = inv_det * (
		mm[8] * mm[5] * mm[3] - mm[4] * mm[9] * mm[3] -
		mm[8] * mm[1] * mm[7] + mm[0] * mm[9] * mm[7] +
		mm[4] * mm[1] * mm[11] - mm[0] * mm[5] * mm[11]
	);
	r[12] = inv_det * (
		mm[12] * mm[9] * mm[6] - mm[8] * mm[13] * mm[6] -
		mm[12] * mm[5] * mm[10] + mm[4] * mm[13] * mm[10] +
		mm[8] * mm[5] * mm[14] - mm[4] * mm[9] * mm[14]
	);
	r[13] = inv_det * (
		mm[8] * mm[13] * mm[2] - mm[12] * mm[9] * mm[2] +
		mm[12] * mm[1] * mm[10] - mm[0] * mm[13] * mm[10] -
		mm[8] * mm[1] * mm[14] + mm[0] * mm[9] * mm[14]
	);
	r[14] = inv_det * (
		mm[12] * mm[5] * mm[2] - mm[4] * mm[13] * mm[2] -
		mm[12] * mm[1] * mm[6] + mm[0] * mm[13] * mm[6] +
		mm[4] * mm[1] * mm[14] - mm[0] * mm[5] * mm[14]
	);
	r[15] = inv_det * (
		mm[4] * mm[9] * mm[2] - mm[8] * mm[5] * mm[2] +
		mm[8] * mm[1] * mm[6] - mm[0] * mm[9] * mm[6] -
		mm[4] * mm[1] * mm[10] + mm[0] * mm[5] * mm[10]
	);
	return r;
}

// returns a 16-element array flipped on the main diagonal
inline mat4 transpose_mat4 (mat4 mm) {
	mat4 r;
	r[0] = mm[0];
	r[1] = mm[4];
	r[2] = mm[8];
	r[3] = mm[12];
	r[4] = mm[1];
	r[5] = mm[5];
	r[6] = mm[9];
	r[7] = mm[13];
	r[8] = mm[2];
	r[9] = mm[6];
	r[10] = mm[10];
	r[11] = mm[14];
	r[12] = mm[3];
	r[13] = mm[7];
	r[14] = mm[11];
	r[15] = mm[15];
	return r;
}

/*--------------------------AFFINE MATRIX FUNCTIONS--------------------------*/
// translate a 4d matrix with xyz array
inline mat4 translate_mat4 (vec3 vv) {
	mat4 r = identity_mat4 ();
	r[12] = vv.x;
	r[13] = vv.y;
	r[14] = vv.z;
	return r;
}

// rotate around x axis by an angle in degrees
inline mat4 rot_x_deg_mat4 (float deg) {
	// convert to radians
	float rad = deg * ONE_DEG_IN_RAD;
	mat4 r = identity_mat4 ();
	r[5] = cos (rad);
	r[9] = -sin (rad);
	r[6] = sin (rad);
	r[10] = cos (rad);
	return r;
}

// rotate around y axis by an angle in degrees
inline mat4 rot_y_deg_mat4 (float deg) {
	// convert to radians
	float rad = deg * ONE_DEG_IN_RAD;
	mat4 r = identity_mat4 ();
	r[0] = cos (rad);
	r[8] = sin (rad);
	r[2] = -sin (rad);
	r[10] = cos (rad);
	return r;
}

// rotate around z axis by an angle in degrees
inline mat4 rot_z_deg_mat4 (float deg) {
	// convert to radians
	float rad = deg * ONE_DEG_IN_RAD;
	mat4 r = identity_mat4 ();
	r[0] = cos (rad);
	r[4] = -sin (rad);
	r[1] = sin (rad);
	r[5] = cos (rad);
	return r;
}

// scale a matrix by [x, y, z]
inline mat4 scale_mat4 (vec3 v) {
	mat4 r = identity_mat4 ();
	r[0] = v.x;
	r[5] = v.y;
	r[10] = v.z;
	return r;
}

/*-----------------------VIRTUAL CAMERA MATRIX FUNCTIONS---------------------*/
// returns a view matrix using the GLU lookAt style.
inline mat4 look_at (vec3 cam_pos, vec3 targ_pos, vec3 up) {
	// inverse translation
	mat4 p = translate_mat4 (-cam_pos);
	// distance vector
	vec3 d = targ_pos - cam_pos;
	// forward vector
	vec3 f = normalise_vec3 (d);
	// right vector
	vec3 r = normalise_vec3 (cross_vec3 (f, up));
	// real up vector
	vec3 u = normalise_vec3 (cross_vec3 (r, f));
	mat4 ori = identity_mat4 ();
	ori[0] = r.x;
	ori[4] = r.y;
	ori[8] = r.z;
	ori[1] = u.x;
	ori[5] = u.y;
	ori[9] = u.z;
	ori[2] = -f.x;
	ori[6] = -f.y;
	ori[10] = -f.z;
	return mat4_mul_mat4 (ori, p);
}

// returns a perspective matrix mimicking the opengl projection style
// remeber if calculating aspect to do floating point division, not integer
inline mat4 perspective (float fovy, float aspect, float near, float far) {
	float fov_rad = fovy * ONE_DEG_IN_RAD;
	float range = tan (fov_rad / 2.0f) * near;
	float sx = (2.0f * near) / (range * aspect + range * aspect);
	float sy = near / range;
	float sz = -(far + near) / (far - near);
	float pz = -(2.0f * far * near) / (far - near);
	mat4 m = zero_mat4 (); // make sure bottom-right corner is zero
	m[0] = sx;
	m[5] = sy;
	m[10] = sz;
	m[14] = pz;
	m[11] = -1.0f;
	return m;
}

/*----------------------------HAMILTON IN DA HOUSE!--------------------------*/
// component-wise mult versor by a versor
inline versor quat_mul_quat (versor a, versor b) {
	versor result;
	result[0] = b[0] * a[0] - b[1] * a[1] - b[2] * a[2] - b[3] * a[3];
	result[1] = b[0] * a[1] + b[1] * a[0] - b[2] * a[3] + b[3] * a[2];
	result[2] = b[0] * a[2] + b[1] * a[3] + b[2] * a[0] - b[3] * a[1];
	result[3] = b[0] * a[3] - b[1] * a[2] + b[2] * a[1] + b[3] * a[0];
	// re-normalise in case of mangling
	return normalise_quat (result);
}

// add versor to a versor
inline versor add_quat_quat (versor a, versor b) {
	versor result = a + b;
	// re-normalise in case of mangling
	return normalise_quat (result);
}

// create quaternion from normalised axis and angle in radians around axis
inline versor quat_from_axis_rad (float radians, vec3 axis) {
	versor result;
	result[0] = cos (radians / 2.0);
	result[1] = sin (radians / 2.0) * axis.x;
	result[2] = sin (radians / 2.0) * axis.y;
	result[3] = sin (radians / 2.0) * axis.z;
	return result;
}

// create quaternion from normalised axis and angle in degrees around axis
inline versor quat_from_axis_deg (float degrees, vec3 axis) {
	return quat_from_axis_rad (ONE_DEG_IN_RAD * degrees, axis);
}

// convert versor to rotation matrix
inline mat4 quat_to_mat4 (versor q) {
	float w = q[0];
	float x = q[1];
	float y = q[2];
	float z = q[3];
	mat4 r;
	r[0] = 1.0f - 2.0f * y * y - 2.0f * z * z;
	r[1] = 2.0f * x * y + 2.0f * w * z;
	r[2] = 2.0f * x * z - 2.0f * w * y;
	r[3] = 0.0f;
	r[4] = 2.0f * x * y - 2.0f * w * z;
	r[5] = 1.0f - 2.0f * x * x - 2.0f * z * z;
	r[6] = 2.0f * y * z + 2.0f * w * x;
	r[7] = 0.0f;
	r[8] = 2.0f * x * z + 2.0f * w * y;
	r[9] = 2.0f * y * z - 2.0f * w * x;
	r[10] = 1.0f - 2.0f * x * x - 2.0f * y * y;
	r[11] = 0.0f;
	r[12] = 0.0f;
	r[13] = 0.0f;
	r[14] = 0.0f;
	r[15] = 1.0f;
	return r;
}

// normalise a quaternion into a unit quaternion (versor) for use in rotation
inline versor normalise_quat (versor q) {
	// norm(q) = q / magnitude (q)
	// magnitude (q) = sqrt (w*w + x*x...)
	// only compute sqrt if interior sum != 1.0
	float sum =
		q[0] * q[0] + q[1] * q[1] +
		q[2] * q[2] + q[3] * q[3];
	// NB: floats have min 6 digits of precision
	const float thresh = 0.0001f;
	if (fabs (1.0f - sum) < thresh) {
		return q;
	}
	float mag = sqrt (sum);
	return q / mag;
}

// dot product of two quaternions
inline float dot_quat (versor q, versor r) {
	return q[0] * r[0] + q[1] * r[1] + q[2] * r[2] + q[3] * r[3];
}

// spherical linear interpolation between two quaternions
// factor t between 0.0 and 1.0
// returns interpolated versor
inline versor slerp_quat (versor q, versor r, float t) {
	// angle between q0-q1
	float cos_half_theta = dot_quat (q, r);
	// as found here http://stackoverflow.com/questions/2886606/flipping-issue-when-interpolating-rotations-using-quaternions
	// if dot product is negative then one quaternion should be negated, to make
	// it take the short way around, rather than the long way
	// yeah! and furthermore Susan, I had to recalculate the d.p. after this
	if (cos_half_theta < 0.0f) {
		for (int i = 0; i < 4; i++) {
			q[i] *= -1.0f;
		}
		cos_half_theta = dot_quat (q, r);
	}
	// if qa=qb or qa=-qb then theta = 0 and we can return qa
	if (fabs (cos_half_theta) >= 1.0f) {
		return q;
	}
	// Calculate temporary values
	float sin_half_theta = sqrt (1.0f - cos_half_theta * cos_half_theta);
	// if theta = 180 degrees then result is not fully defined
	// we could rotate around any axis normal to qa or qb
	versor result;
	if (fabs (sin_half_theta) < 0.001f) {
		for (int i = 0; i < 4; i++) {
			result[i] = (1.0f - t) * q[i] + t * r[i];
		}
		return result;
	}
	float half_theta = acos (cos_half_theta);
	float a = sin ((1.0f - t) * half_theta) / sin_half_theta;
	float b = sin (t * half_theta) / sin_half_theta;
	for (int i = 0; i < 4; i++) {
		result[i] = q[i] * a + r[i] * b;
	}
	return result;
}
