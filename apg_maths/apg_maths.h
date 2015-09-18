/*****************************************************************************\
| Anton's Maths Library C99 version with new standard                         |
| Email: anton at antongerdelan dot net                                       |
| Branched from C++ version 5 May 2015                                        |
| Copyright Dr Anton Gerdelan                                                 |
|*****************************************************************************|
| Commonly-used maths structures and functions, resembling GLSL.              |
| Simple-as-possible.                                                         |
| Function names have suffix to denote type as C has no function overloading. |
| Structs vec3, mat4, versor just hold arrays of floats called "v","m","q"    |
| For example, to get values from a mat4 do: my_mat.m                         |
| A versor is the proper name for a unit quaternion.                          |
| For the C++ version with operator overloading see the .hpp file             |
\*****************************************************************************/
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
#define TAU 2.0 * M_PI
#define ONE_DEG_IN_RAD (2.0 * M_PI) / 360.0 // 0.017444444
#define ONE_RAD_IN_DEG 360.0 / (2.0 * M_PI) //57.2957795

// data structures
typedef struct vec2 vec2;
typedef struct vec3 vec3;
typedef struct vec4 vec4;
typedef struct mat4 mat4;
typedef struct versor versor;

// xy
struct vec2 {
	float v[2];
};

// xyz
struct vec3 {
	float v[3];
};

// xyzw
struct vec4 {
	float v[4];
};

// stored like this:
// 00 04 08 12
// 01 05 09 13
// 02 06 10 14
// 03 07 11 15
struct mat4 {
	float m[16];
};

// a unit quaternion used for rotation (xyzw)
struct versor {
	float q[4];
};

// print functions
void print_vec2 (vec2 v);
void print_vec3 (vec3 v);
void print_vec4 (vec4 v);
void print_mat4 (mat4 m);
void print_quat (versor q);

// vector functions -- construction and assignment
// create from 2 scalars
vec2 vec2_from_2f (float x, float y);
//
vec2 vec2_from_vec2 (vec2 vv);
// create from 3 scalars
vec3 vec3_from_3f (float x, float y, float z);
// create from a vec2 (x,y) and a scalar (z)
vec3 vec3_from_vec2_f (vec2 vv, float z);
// equals
vec3 vec3_from_vec3 (vec3 vv);
// create from a vec4, discard w
vec3 vec3_from_vec4 (vec4 vv);
// create from 3 scalars
vec4 vec4_from_4f (float x, float y, float z, float w);
// create from a vec3 (x,y,z) and a scalar (w)
vec4 vec4_from_vec3_f (vec3 vv, float w);
// equals
vec4 vec4_from_vec4 (vec4 vv);

// vector functions -- component-wise operations
// add vec3
vec3 add_vec3_vec3 (vec3 a, vec3 b);
// sub vec3
vec3 sub_vec3_vec3 (vec3 a, vec3 b);
// add scalar
vec3 add_vec3_f (vec3 a, float b);
// sub scalar
vec3 sub_vec3_f (vec3 a, float b);
// mult by scalar
vec3 mult_vec3_f (vec3 a, float b);
// div by scalar
vec3 div_vec3_f (vec3 a, float b);
// component-wise mult with vec3
vec3 mult_vec3_vec3 (vec3 a, vec3 b);
// component-wise div with vec3
vec3 div_vec3_vec3 (vec3 a, vec3 b);

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
// NOTE(anton) i removed the component-wise maths for vec4 because we don't do
// that for w in graphics

// matrix functions -- construction and assignment
mat4 zero_mat4 ();
mat4 identity_mat4 ();
mat4 mat4_from_mat4 (mat4 mm);

// matrix functions -- linear algebra
mat4 mult_mat4_mat4 (mat4 a, mat4 b);
// determinant
float det_mat4 (mat4 mm);
mat4 inverse_mat4 (mat4 mm);
mat4 transpose_mat4 (mat4 mm);

// matrix functions -- affine functions
// i got rid of the GLU-style first-argument-is-another matrix convention
// because it's seldom used and .: a waste of computation
mat4 translate_mat4 (vec3 v);
mat4 rot_x_deg_mat4 (float deg);
mat4 rot_y_deg_mat4 (float deg);
mat4 rot_z_deg_mat4 (float deg);
mat4 scale_mat4 (vec3 v);

// matrix functions -- camera functions
mat4 look_at (vec3 cam_pos, vec3 targ_pos, vec3 up);
mat4 perspective (float fovy, float aspect, float near, float far);

// quaternion functions -- construction and assignment
versor versor_from_4f (float x, float y, float z, float w);
versor versor_from_versor (versor qq);

// quaternion functions -- component-wise operations
versor div_quat_f (versor qq, float s);
versor mult_quat_f (versor qq, float s);
versor mult_quat_quat (versor a, versor b);
versor add_quat_quat (versor a, versor b);

// quaternion functions -- geometric
versor quat_from_axis_rad (float radians, float x, float y, float z);
versor quat_from_axis_deg (float degrees, float x, float y, float z);

// quaternion functions -- interoperability
mat4 quat_to_mat4 (versor q);
float dot_quat (versor q, versor r);
versor normalise_quat (versor q);

// quaternion functions -- interpolation
versor slerp (versor q, versor r);
versor slerp_quat (versor q, versor r, float t);

/*-----------------------------PRINT FUNCTIONS-------------------------------*/
//
inline void print_vec2 (vec2 v) {
	printf ("[%.2f, %.2f]\n", v.v[0], v.v[1]);
}

//
inline void print_vec3 (vec3 v) {
	printf ("[%.2f, %.2f, %.2f]\n", v.v[0], v.v[1], v.v[2]);
}

//
inline void print_vec4 (vec4 v) {
	printf ("[%.2f, %.2f, %.2f, %.2f]\n", v.v[0], v.v[1], v.v[2], v.v[3]);
}

//
inline void print_mat4 (mat4 m) {
	printf("\n");
	printf ("[%.2f][%.2f][%.2f][%.2f]\n", m.m[0], m.m[4], m.m[8], m.m[12]);
	printf ("[%.2f][%.2f][%.2f][%.2f]\n", m.m[1], m.m[5], m.m[9], m.m[13]);
	printf ("[%.2f][%.2f][%.2f][%.2f]\n", m.m[2], m.m[6], m.m[10], m.m[14]);
	printf ("[%.2f][%.2f][%.2f][%.2f]\n", m.m[3], m.m[7], m.m[11], m.m[15]);
}

//
inline void print_quat (versor q) {
	printf ("[%.2f ,%.2f, %.2f, %.2f]\n", q.q[0], q.q[1], q.q[2], q.q[3]);
}

/*------------------------------VECTOR FUNCTIONS-----------------------------*/
inline vec2 vec2_from_2f (float x, float y) {
	vec2 r;
	r.v[0] = x;
	r.v[1] = y;
	return r;
}

inline vec2 vec2_from_vec2 (vec2 vv) {
	vec2 r;
	r.v[0] = vv.v[0];
	r.v[1] = vv.v[1];
	return r;
}

inline vec3 vec3_from_3f (float x, float y, float z) {
	vec3 r;
	r.v[0] = x;
	r.v[1] = y;
	r.v[2] = z;
	return r;
}

inline vec3 vec3_from_vec2_f (vec2 vv, float z) {
	vec3 r;
	r.v[0] = vv.v[0];
	r.v[1] = vv.v[1];
	r.v[2] = z;
	return r;
}

inline vec3 vec3_from_vec3 (vec3 vv) {
	vec3 r;
	r.v[0] = vv.v[0];
	r.v[1] = vv.v[1];
	r.v[2] = vv.v[2];
	return r;
}

// create vec3 by truncating vec4
inline vec3 vec3_from_vec4 (vec4 vv) {
	vec3 r;
	r.v[0] = vv.v[0];
	r.v[1] = vv.v[1];
	r.v[2] = vv.v[2];
	return r;
}

inline vec3 add_vec3_vec3 (vec3 a, vec3 b) {
	vec3 r;
	r.v[0] = a.v[0] + b.v[0];
	r.v[1] = a.v[1] + b.v[1];
	r.v[2] = a.v[2] + b.v[2];
	return r;
}

inline vec3 sub_vec3_vec3 (vec3 a, vec3 b) {
	vec3 r;
	r.v[0] = a.v[0] - b.v[0];
	r.v[1] = a.v[1] - b.v[1];
	r.v[2] = a.v[2] - b.v[2];
	return r;
}

inline vec3 add_vec3_f (vec3 a, float b) {
	vec3 r;
	r.v[0] = a.v[0] + b;
	r.v[1] = a.v[1] + b;
	r.v[2] = a.v[2] + b;
	return r;
}

inline vec3 sub_vec3_f (vec3 a, float b) {
	vec3 r;
	r.v[0] = a.v[0] - b;
	r.v[1] = a.v[1] - b;
	r.v[2] = a.v[2] - b;
	return r;
}

inline vec3 mult_vec3_f (vec3 a, float b) {
	vec3 r;
	r.v[0] = a.v[0] * b;
	r.v[1] = a.v[1] * b;
	r.v[2] = a.v[2] * b;
	return r;
}

inline vec3 div_vec3_f (vec3 a, float b) {
	vec3 r;
	r.v[0] = a.v[0] / b;
	r.v[1] = a.v[1] / b;
	r.v[2] = a.v[2] / b;
	return r;
}

inline vec3 mult_vec3_vec3 (vec3 a, vec3 b) {
	vec3 r;
	r.v[0] = a.v[0] * b.v[0];
	r.v[1] = a.v[1] * b.v[1];
	r.v[2] = a.v[2] * b.v[2];
	return r;
}

inline vec3 div_vec3_vec3 (vec3 a, vec3 b) {
	vec3 r;
	r.v[0] = a.v[0] / b.v[0];
	r.v[1] = a.v[1] / b.v[1];
	r.v[2] = a.v[2] / b.v[2];
	return r;
}

// create from 3 scalars
inline vec4 vec4_from_4f (float x, float y, float z, float w) {
	vec4 r;
	r.v[0] = x;
	r.v[1] = y;
	r.v[2] = z;
	r.v[3] = w;
	return r;
}

// create from a vec3 (x,y,z) and a scalar (w)
inline vec4 vec4_from_vec3_f (vec3 vv, float w) {
	vec4 r;
	r.v[0] = vv.v[0];
	r.v[1] = vv.v[1];
	r.v[2] = vv.v[2];
	r.v[3] = w;
	return r;
}

// equals
inline vec4 vec4_from_vec4 (vec4 vv) {
	vec4 r;
	r.v[0] = vv.v[0];
	r.v[1] = vv.v[1];
	r.v[2] = vv.v[2];
	r.v[3] = vv.v[3];
	return r;
}

//
inline float length_vec3 (vec3 v) {
	return sqrt (v.v[0] * v.v[0] + v.v[1] * v.v[1] + v.v[2] * v.v[2]);
}

// squared length
inline float length2_vec3 (vec3 v) {
	return v.v[0] * v.v[0] + v.v[1] * v.v[1] + v.v[2] * v.v[2];
}

// note: proper spelling (hehe)
inline vec3 normalise_vec3 (vec3 v) {
	vec3 vb;
	float l = length_vec3 (v);
	if (0.0f == l) {
		return vec3_from_3f (0.0f, 0.0f, 0.0f);
	}
	vb.v[0] = v.v[0] / l;
	vb.v[1] = v.v[1] / l;
	vb.v[2] = v.v[2] / l;
	return vb;
}

//
inline float dot_vec3 (vec3 a, vec3 b) {
	return a.v[0] * b.v[0] + a.v[1] * b.v[1] + a.v[2] * b.v[2];
}

//
inline vec3 cross_vec3 (vec3 a, vec3 b) {
	float x = a.v[1] * b.v[2] - a.v[2] * b.v[1];
	float y = a.v[2] * b.v[0] - a.v[0] * b.v[2];
	float z = a.v[0] * b.v[1] - a.v[1] * b.v[0];
	return vec3_from_3f (x, y, z);
}

// converts an un-normalised direction vector's X,Z components into a heading
// in degrees
// NB i suspect that the z is backwards here but i've used in in
// several places like this. d'oh!
inline float vec3_to_heading (vec3 d) {
	return atan2 (-d.v[0], -d.v[2]) * ONE_RAD_IN_DEG;
}

// very informal function to convert a heading (e.g. y-axis orientation) into
// a 3d vector with components in x and z axes
inline vec3 heading_to_vec3 (float degrees) {
	float rad = degrees * ONE_DEG_IN_RAD;
	return vec3_from_3f (-sinf (rad), 0.0f, -cosf (rad));
}

/*-----------------------------MATRIX FUNCTIONS------------------------------*/
//
inline mat4 zero_mat4 () {
	mat4 r;
	memset (r.m, 0, 16 * sizeof (float));
	return r;
}

//
inline mat4 identity_mat4 () {
	mat4 r = zero_mat4 ();
	r.m[0] = 1.0f;
	r.m[5] = 1.0f;
	r.m[10] = 1.0f;
	r.m[15] = 1.0f;
	return r;
}

// equals
inline mat4 mat4_from_mat4 (mat4 mm) {
	mat4 r;
	memcpy (r.m, mm.m, 16 * sizeof (float)); 
	return r;
}

//
inline mat4 mult_mat4_mat4 (mat4 a, mat4 b) {
	mat4 r = zero_mat4 ();
	int r_index = 0;
	for (int col = 0; col < 4; col++) {
		for (int row = 0; row < 4; row++) {
			float sum = 0.0f;
			for (int i = 0; i < 4; i++) {
				sum += b.m[i + col * 4] * a.m[row + i * 4];
			}
			r.m[r_index] = sum;
			r_index++;
		}
	}
	return r;
}

// returns a scalar value with the determinant for a 4x4 matrix
// see http://www.euclideanspace.com/maths/algebra/matrix/functions/determinant/fourD/index.htm
inline float det_mat4 (mat4 mm) {
	return
		mm.m[12] * mm.m[9] * mm.m[6] * mm.m[3] -
		mm.m[8] * mm.m[13] * mm.m[6] * mm.m[3] -
		mm.m[12] * mm.m[5] * mm.m[10] * mm.m[3] +
		mm.m[4] * mm.m[13] * mm.m[10] * mm.m[3] +
		mm.m[8] * mm.m[5] * mm.m[14] * mm.m[3] -
		mm.m[4] * mm.m[9] * mm.m[14] * mm.m[3] -
		mm.m[12] * mm.m[9] * mm.m[2] * mm.m[7] +
		mm.m[8] * mm.m[13] * mm.m[2] * mm.m[7] +
		mm.m[12] * mm.m[1] * mm.m[10] * mm.m[7] -
		mm.m[0] * mm.m[13] * mm.m[10] * mm.m[7] -
		mm.m[8] * mm.m[1] * mm.m[14] * mm.m[7] +
		mm.m[0] * mm.m[9] * mm.m[14] * mm.m[7] +
		mm.m[12] * mm.m[5] * mm.m[2] * mm.m[11] -
		mm.m[4] * mm.m[13] * mm.m[2] * mm.m[11] -
		mm.m[12] * mm.m[1] * mm.m[6] * mm.m[11] +
		mm.m[0] * mm.m[13] * mm.m[6] * mm.m[11] +
		mm.m[4] * mm.m[1] * mm.m[14] * mm.m[11] -
		mm.m[0] * mm.m[5] * mm.m[14] * mm.m[11] -
		mm.m[8] * mm.m[5] * mm.m[2] * mm.m[15] +
		mm.m[4] * mm.m[9] * mm.m[2] * mm.m[15] +
		mm.m[8] * mm.m[1] * mm.m[6] * mm.m[15] -
		mm.m[0] * mm.m[9] * mm.m[6] * mm.m[15] -
		mm.m[4] * mm.m[1] * mm.m[10] * mm.m[15] +
		mm.m[0] * mm.m[5] * mm.m[10] * mm.m[15];
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
	r.m[0] = inv_det * (
		mm.m[9] * mm.m[14] * mm.m[7] - mm.m[13] * mm.m[10] * mm.m[7] +
		mm.m[13] * mm.m[6] * mm.m[11] - mm.m[5] * mm.m[14] * mm.m[11] -
		mm.m[9] * mm.m[6] * mm.m[15] + mm.m[5] * mm.m[10] * mm.m[15]
	);
	r.m[1] = inv_det * (
		mm.m[13] * mm.m[10] * mm.m[3] - mm.m[9] * mm.m[14] * mm.m[3] -
		mm.m[13] * mm.m[2] * mm.m[11] + mm.m[1] * mm.m[14] * mm.m[11] +
		mm.m[9] * mm.m[2] * mm.m[15] - mm.m[1] * mm.m[10] * mm.m[15]
	);
	r.m[2] = inv_det * (
		mm.m[5] * mm.m[14] * mm.m[3] - mm.m[13] * mm.m[6] * mm.m[3] +
		mm.m[13] * mm.m[2] * mm.m[7] - mm.m[1] * mm.m[14] * mm.m[7] -
		mm.m[5] * mm.m[2] * mm.m[15] + mm.m[1] * mm.m[6] * mm.m[15]
	);
	r.m[3] = inv_det * (
		mm.m[9] * mm.m[6] * mm.m[3] - mm.m[5] * mm.m[10] * mm.m[3] -
		mm.m[9] * mm.m[2] * mm.m[7] + mm.m[1] * mm.m[10] * mm.m[7] +
		mm.m[5] * mm.m[2] * mm.m[11] - mm.m[1] * mm.m[6] * mm.m[11]
	);
	r.m[4] = inv_det * (
		mm.m[12] * mm.m[10] * mm.m[7] - mm.m[8] * mm.m[14] * mm.m[7] -
		mm.m[12] * mm.m[6] * mm.m[11] + mm.m[4] * mm.m[14] * mm.m[11] +
		mm.m[8] * mm.m[6] * mm.m[15] - mm.m[4] * mm.m[10] * mm.m[15]
	);
	r.m[5] = inv_det * (
		mm.m[8] * mm.m[14] * mm.m[3] - mm.m[12] * mm.m[10] * mm.m[3] +
		mm.m[12] * mm.m[2] * mm.m[11] - mm.m[0] * mm.m[14] * mm.m[11] -
		mm.m[8] * mm.m[2] * mm.m[15] + mm.m[0] * mm.m[10] * mm.m[15]
	);
	r.m[6] = inv_det * (
		mm.m[12] * mm.m[6] * mm.m[3] - mm.m[4] * mm.m[14] * mm.m[3] -
		mm.m[12] * mm.m[2] * mm.m[7] + mm.m[0] * mm.m[14] * mm.m[7] +
		mm.m[4] * mm.m[2] * mm.m[15] - mm.m[0] * mm.m[6] * mm.m[15]
	);
	r.m[7] = inv_det * (
		mm.m[4] * mm.m[10] * mm.m[3] - mm.m[8] * mm.m[6] * mm.m[3] +
		mm.m[8] * mm.m[2] * mm.m[7] - mm.m[0] * mm.m[10] * mm.m[7] -
		mm.m[4] * mm.m[2] * mm.m[11] + mm.m[0] * mm.m[6] * mm.m[11]
	);
	r.m[8] = inv_det * (
		mm.m[8] * mm.m[13] * mm.m[7] - mm.m[12] * mm.m[9] * mm.m[7] +
		mm.m[12] * mm.m[5] * mm.m[11] - mm.m[4] * mm.m[13] * mm.m[11] -
		mm.m[8] * mm.m[5] * mm.m[15] + mm.m[4] * mm.m[9] * mm.m[15]
	);
	r.m[9] = inv_det * (
		mm.m[12] * mm.m[9] * mm.m[3] - mm.m[8] * mm.m[13] * mm.m[3] -
		mm.m[12] * mm.m[1] * mm.m[11] + mm.m[0] * mm.m[13] * mm.m[11] +
		mm.m[8] * mm.m[1] * mm.m[15] - mm.m[0] * mm.m[9] * mm.m[15]
	);
	r.m[10] = inv_det * (
		mm.m[4] * mm.m[13] * mm.m[3] - mm.m[12] * mm.m[5] * mm.m[3] +
		mm.m[12] * mm.m[1] * mm.m[7] - mm.m[0] * mm.m[13] * mm.m[7] -
		mm.m[4] * mm.m[1] * mm.m[15] + mm.m[0] * mm.m[5] * mm.m[15]
	);
	r.m[11] = inv_det * (
		mm.m[8] * mm.m[5] * mm.m[3] - mm.m[4] * mm.m[9] * mm.m[3] -
		mm.m[8] * mm.m[1] * mm.m[7] + mm.m[0] * mm.m[9] * mm.m[7] +
		mm.m[4] * mm.m[1] * mm.m[11] - mm.m[0] * mm.m[5] * mm.m[11]
	);
	r.m[12] = inv_det * (
		mm.m[12] * mm.m[9] * mm.m[6] - mm.m[8] * mm.m[13] * mm.m[6] -
		mm.m[12] * mm.m[5] * mm.m[10] + mm.m[4] * mm.m[13] * mm.m[10] +
		mm.m[8] * mm.m[5] * mm.m[14] - mm.m[4] * mm.m[9] * mm.m[14]
	);
	r.m[13] = inv_det * (
		mm.m[8] * mm.m[13] * mm.m[2] - mm.m[12] * mm.m[9] * mm.m[2] +
		mm.m[12] * mm.m[1] * mm.m[10] - mm.m[0] * mm.m[13] * mm.m[10] -
		mm.m[8] * mm.m[1] * mm.m[14] + mm.m[0] * mm.m[9] * mm.m[14]
	);
	r.m[14] = inv_det * (
		mm.m[12] * mm.m[5] * mm.m[2] - mm.m[4] * mm.m[13] * mm.m[2] -
		mm.m[12] * mm.m[1] * mm.m[6] + mm.m[0] * mm.m[13] * mm.m[6] +
		mm.m[4] * mm.m[1] * mm.m[14] - mm.m[0] * mm.m[5] * mm.m[14]
	);
	r.m[15] = inv_det * (
		mm.m[4] * mm.m[9] * mm.m[2] - mm.m[8] * mm.m[5] * mm.m[2] +
		mm.m[8] * mm.m[1] * mm.m[6] - mm.m[0] * mm.m[9] * mm.m[6] -
		mm.m[4] * mm.m[1] * mm.m[10] + mm.m[0] * mm.m[5] * mm.m[10]
	);
	return r;
}

// returns a 16-element array flipped on the main diagonal
inline mat4 transpose_mat4 (mat4 mm) {
	mat4 r;
	r.m[0] = mm.m[0];
	r.m[1] = mm.m[4];
	r.m[2] = mm.m[8];
	r.m[3] = mm.m[12];
	r.m[4] = mm.m[1];
	r.m[5] = mm.m[5];
	r.m[6] = mm.m[9];
	r.m[7] = mm.m[13];
	r.m[8] = mm.m[2];
	r.m[9] = mm.m[6];
	r.m[10] = mm.m[10];
	r.m[11] = mm.m[14];
	r.m[12] = mm.m[3];
	r.m[13] = mm.m[7];
	r.m[14] = mm.m[11];
	r.m[15] = mm.m[15];
	return r;
}

/*--------------------------AFFINE MATRIX FUNCTIONS--------------------------*/
// translate a 4d matrix with xyz array
inline mat4 translate_mat4 (vec3 vv) {
	mat4 r = identity_mat4 ();
	r.m[12] = vv.v[0];
	r.m[13] = vv.v[1];
	r.m[14] = vv.v[2];
	return r;
}

// rotate around x axis by an angle in degrees
inline mat4 rot_x_deg_mat4 (float deg) {
	// convert to radians
	float rad = deg * ONE_DEG_IN_RAD;
	mat4 r = identity_mat4 ();
	r.m[5] = cos (rad);
	r.m[9] = -sin (rad);
	r.m[6] = sin (rad);
	r.m[10] = cos (rad);
	return r;
}

// rotate around y axis by an angle in degrees
inline mat4 rot_y_deg_mat4 (float deg) {
	// convert to radians
	float rad = deg * ONE_DEG_IN_RAD;
	mat4 r = identity_mat4 ();
	r.m[0] = cos (rad);
	r.m[8] = sin (rad);
	r.m[2] = -sin (rad);
	r.m[10] = cos (rad);
	return r;
}

// rotate around z axis by an angle in degrees
inline mat4 rot_z_deg_mat4 (float deg) {
	// convert to radians
	float rad = deg * ONE_DEG_IN_RAD;
	mat4 r = identity_mat4 ();
	r.m[0] = cos (rad);
	r.m[4] = -sin (rad);
	r.m[1] = sin (rad);
	r.m[5] = cos (rad);
	return r;
}

// scale a matrix by [x, y, z]
inline mat4 scale_mat4 (vec3 v) {
	mat4 r = identity_mat4 ();
	r.m[0] = v.v[0];
	r.m[5] = v.v[1];
	r.m[10] = v.v[2];
	return r;
}

/*-----------------------VIRTUAL CAMERA MATRIX FUNCTIONS---------------------*/
// returns a view matrix using the GLU lookAt style.
inline mat4 look_at (vec3 cam_pos, vec3 targ_pos, vec3 up) {
	// inverse translation
	mat4 p = translate_mat4 (vec3_from_3f (-cam_pos.v[0], -cam_pos.v[1],
		-cam_pos.v[2]));
	// distance vector
	vec3 d = sub_vec3_vec3 (targ_pos, cam_pos);
	// forward vector
	vec3 f = normalise_vec3 (d);
	// right vector
	vec3 r = normalise_vec3 (cross_vec3 (f, up));
	// real up vector
	vec3 u = normalise_vec3 (cross_vec3 (r, f));
	mat4 ori = identity_mat4 ();
	ori.m[0] = r.v[0];
	ori.m[4] = r.v[1];
	ori.m[8] = r.v[2];
	ori.m[1] = u.v[0];
	ori.m[5] = u.v[1];
	ori.m[9] = u.v[2];
	ori.m[2] = -f.v[0];
	ori.m[6] = -f.v[1];
	ori.m[10] = -f.v[2];
	
	return mult_mat4_mat4 (ori, p);
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
	m.m[0] = sx;
	m.m[5] = sy;
	m.m[10] = sz;
	m.m[14] = pz;
	m.m[11] = -1.0f;
	return m;
}

/*----------------------------HAMILTON IN DA HOUSE!--------------------------*/
// manual cons
inline versor versor_from_4f (float x, float y, float z, float w) {
	versor r;
	r.q[0] = x;
	r.q[1] = y;
	r.q[2] = z;
	r.q[3] = w;
	return r;
}

// assignment
inline versor versor_from_versor (versor qq) {
	versor r;
	memcpy (r.q, qq.q, 4 * sizeof (float));
	return r;
}

// divide versor by a scalar
inline versor div_quat_f (versor qq, float s) {
	versor result;
	result.q[0] = qq.q[0] / s;
	result.q[1] = qq.q[1] / s;
	result.q[2] = qq.q[2] / s;
	result.q[3] = qq.q[3] / s;
	return result;
}

// mult versor by a scalar
inline versor mult_quat_f (versor qq, float s) {
	versor result;
	result.q[0] = qq.q[0] * s;
	result.q[1] = qq.q[1] * s;
	result.q[2] = qq.q[2] * s;
	result.q[3] = qq.q[3] * s;
	return result;
}

// component-wise mult versor by a versor
inline versor mult_quat_quat (versor a, versor b) {
	versor result;
	result.q[0] = b.q[0] * a.q[0] - b.q[1] * a.q[1] -
		b.q[2] * a.q[2] - b.q[3] * a.q[3];
	result.q[1] = b.q[0] * a.q[1] + b.q[1] * a.q[0] -
		b.q[2] * a.q[3] + b.q[3] * a.q[2];
	result.q[2] = b.q[0] * a.q[2] + b.q[1] * a.q[3] +
		b.q[2] * a.q[0] - b.q[3] * a.q[1];
	result.q[3] = b.q[0] * a.q[3] - b.q[1] * a.q[2] +
		b.q[2] * a.q[1] + b.q[3] * a.q[0];
	// re-normalise in case of mangling
	return normalise_quat (result);
}

// add versor to a versor
inline versor add_quat_quat (versor a, versor b) {
	versor result;
	result.q[0] = b.q[0] + a.q[0];
	result.q[1] = b.q[1] + a.q[1];
	result.q[2] = b.q[2] + a.q[2];
	result.q[3] = b.q[3] + a.q[3];
	// re-normalise in case of mangling
	return normalise_quat (result);
}

// create quaternion from normalised axis and angle in radians around axis
inline versor quat_from_axis_rad (float radians, float x, float y, float z) {
	versor result;
	result.q[0] = cos (radians / 2.0);
	result.q[1] = sin (radians / 2.0) * x;
	result.q[2] = sin (radians / 2.0) * y;
	result.q[3] = sin (radians / 2.0) * z;
	return result;
}

// create quaternion from normalised axis and angle in degrees around axis
inline versor quat_from_axis_deg (float degrees, float x, float y, float z) {
	return quat_from_axis_rad (ONE_DEG_IN_RAD * degrees, x, y, z);
}

// convert versor to rotation matrix
inline mat4 quat_to_mat4 (versor q) {
	float w = q.q[0];
	float x = q.q[1];
	float y = q.q[2];
	float z = q.q[3];
	mat4 r;
	r.m[0] = 1.0f - 2.0f * y * y - 2.0f * z * z;
	r.m[1] = 2.0f * x * y + 2.0f * w * z;
	r.m[2] = 2.0f * x * z - 2.0f * w * y;
	r.m[3] = 0.0f;
	r.m[4] = 2.0f * x * y - 2.0f * w * z;
	r.m[5] = 1.0f - 2.0f * x * x - 2.0f * z * z;
	r.m[6] = 2.0f * y * z + 2.0f * w * x;
	r.m[7] = 0.0f;
	r.m[8] = 2.0f * x * z + 2.0f * w * y;
	r.m[9] = 2.0f * y * z - 2.0f * w * x;
	r.m[10] = 1.0f - 2.0f * x * x - 2.0f * y * y;
	r.m[11] = 0.0f;
	r.m[12] = 0.0f;
	r.m[13] = 0.0f;
	r.m[14] = 0.0f;
	r.m[15] = 1.0f;
	return r;
}

// normalise a quaternion into a unit quaternion (versor) for use in rotation
inline versor normalise_quat (versor q) {
	// norm(q) = q / magnitude (q)
	// magnitude (q) = sqrt (w*w + x*x...)
	// only compute sqrt if interior sum != 1.0
	float sum =
		q.q[0] * q.q[0] + q.q[1] * q.q[1] +
		q.q[2] * q.q[2] + q.q[3] * q.q[3];
	// NB: floats have min 6 digits of precision
	const float thresh = 0.0001f;
	if (fabs (1.0f - sum) < thresh) {
		return q;
	}
	float mag = sqrt (sum);
	return div_quat_f (q, mag);
}

// dot product of two quaternions
inline float dot_quat (versor q, versor r) {
	return q.q[0] * r.q[0] + q.q[1] * r.q[1] + q.q[2] * r.q[2] + q.q[3] * r.q[3];
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
			q.q[i] *= -1.0f;
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
			result.q[i] = (1.0f - t) * q.q[i] + t * r.q[i];
		}
		return result;
	}
	float half_theta = acos (cos_half_theta);
	float a = sin ((1.0f - t) * half_theta) / sin_half_theta;
	float b = sin (t * half_theta) / sin_half_theta;
	for (int i = 0; i < 4; i++) {
		result.q[i] = q.q[i] * a + r.q[i] * b;
	}
	return result;
}
