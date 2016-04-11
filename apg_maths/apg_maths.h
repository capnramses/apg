/*****************************************************************************\
Anton's Maths Library - C99 version
Anton Gerdelan <antonofnote at gmail>
First v. branched from C++ original 5 May 2015
Compacted 11 April 2016
\*****************************************************************************/
#pragma once
#include <stdio.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <string.h>
#ifndef M_PI // C99 removed M_PI
#define M_PI 3.14159265358979323846
#endif
#define ONE_DEG_IN_RAD (2.0 * M_PI) / 360.0 // 0.017444444
#define ONE_RAD_IN_DEG 360.0 / (2.0 * M_PI) //57.2957795

typedef struct vec2 { float v[2]; } vec2;
typedef struct vec3 { float v[3]; } vec3;
typedef struct vec4 { float v[4]; } vec4;
typedef struct mat4 { float m[16]; } mat4;
typedef struct versor { float q[4]; } versor;

/*-----------------------------PRINT FUNCTIONS-------------------------------*/
static inline void print_vec2 (vec2 v) {
	printf ("[%.2f, %.2f]\n", v.v[0], v.v[1]);
}

static inline void print_vec3 (vec3 v) {
	printf ("[%.2f, %.2f, %.2f]\n", v.v[0], v.v[1], v.v[2]);
}

static inline void print_vec4 (vec4 v) {
	printf ("[%.2f, %.2f, %.2f, %.2f]\n", v.v[0], v.v[1], v.v[2], v.v[3]);
}

static inline void print_mat4 (mat4 m) {
	printf("\n");
	printf ("[%.2f][%.2f][%.2f][%.2f]\n", m.m[0], m.m[4], m.m[8], m.m[12]);
	printf ("[%.2f][%.2f][%.2f][%.2f]\n", m.m[1], m.m[5], m.m[9], m.m[13]);
	printf ("[%.2f][%.2f][%.2f][%.2f]\n", m.m[2], m.m[6], m.m[10], m.m[14]);
	printf ("[%.2f][%.2f][%.2f][%.2f]\n", m.m[3], m.m[7], m.m[11], m.m[15]);
}

static inline void print_quat (versor q) {
	printf ("[%.2f ,%.2f, %.2f, %.2f]\n", q.q[0], q.q[1], q.q[2], q.q[3]);
}

/*------------------------------VECTOR FUNCTIONS-----------------------------*/
static inline vec3 add_vec3_f (vec3 a, float b) {
	return (vec3){ .v = { a.v[0] + b, a.v[1] + b, a.v[2] + b } };
}

static inline vec3 sub_vec3_f (vec3 a, float b) {
	return (vec3){ .v = { a.v[0] - b, a.v[1] - b, a.v[2] - b } };
}

static inline vec3 mult_vec3_f (vec3 a, float b) {
	return (vec3){ .v = { a.v[0] * b, a.v[1] * b, a.v[2] * b } };
}

static inline vec3 div_vec3_f (vec3 a, float b) {
	return (vec3){ .v = { a.v[0] / b, a.v[1] / b, a.v[2] / b } };
}

static inline vec3 add_vec3_vec3 (vec3 a, vec3 b) {
	return (vec3){ .v = { a.v[0] + b.v[0], a.v[1] + b.v[1], a.v[2] + b.v[2] } };
}

static inline vec3 sub_vec3_vec3 (vec3 a, vec3 b) {
	return (vec3){ .v = { a.v[0] - b.v[0], a.v[1] - b.v[1], a.v[2] - b.v[2] } };
}

static inline vec3 mult_vec3_vec3 (vec3 a, vec3 b) {
	return (vec3){ .v = { a.v[0] * b.v[0], a.v[1] * b.v[1], a.v[2] * b.v[2] } };
}

static inline vec3 div_vec3_vec3 (vec3 a, vec3 b) {
	return (vec3){ .v = { a.v[0] / b.v[0], a.v[1] / b.v[1], a.v[2] / b.v[2] } };
}

static inline float length_vec3 (vec3 v) {
	return sqrt (v.v[0] * v.v[0] + v.v[1] * v.v[1] + v.v[2] * v.v[2]);
}

static inline float length2_vec3 (vec3 v) {
	return v.v[0] * v.v[0] + v.v[1] * v.v[1] + v.v[2] * v.v[2];
}

static inline vec3 normalise_vec3 (vec3 v) {
	vec3 vb;
	float l = length_vec3 (v);
	if (0.0f == l) { return (vec3){ .v = {0.0f, 0.0f, 0.0f} }; }
	vb.v[0] = v.v[0] / l;
	vb.v[1] = v.v[1] / l;
	vb.v[2] = v.v[2] / l;
	return vb;
}

static inline float dot_vec3 (vec3 a, vec3 b) {
	return a.v[0] * b.v[0] + a.v[1] * b.v[1] + a.v[2] * b.v[2];
}

static inline vec3 cross_vec3 (vec3 a, vec3 b) {
	return (vec3) { .v = {
		a.v[1] * b.v[2] - a.v[2] * b.v[1],
		a.v[2] * b.v[0] - a.v[0] * b.v[2],
		a.v[0] * b.v[1] - a.v[1] * b.v[0]
	} };
}

// converts an un-normalised direction vector's X,Z components into a heading
// in degrees
// NB i suspect that the z is backwards here but i've used in in
// several places like this. d'oh!
static inline float vec3_to_heading (vec3 d) {
	return atan2 (-d.v[0], -d.v[2]) * ONE_RAD_IN_DEG;
}

// very informal function to convert a heading (e.g. y-axis orientation) into
// a 3d vector with components in x and z axes
static inline vec3 heading_to_vec3 (float degrees) {
	float rad = degrees * ONE_DEG_IN_RAD;
	return (vec3) { .v = {-sinf (rad), 0.0f, -cosf (rad) } };
}

/*-----------------------------MATRIX FUNCTIONS------------------------------*/
static inline mat4 zero_mat4 () {
	mat4 r;
	memset (r.m, 0, 16 * sizeof (float));
	return r;
}

static inline mat4 identity_mat4 () {
	mat4 r = zero_mat4 ();
	r.m[0] = 1.0f;
	r.m[5] = 1.0f;
	r.m[10] = 1.0f;
	r.m[15] = 1.0f;
	return r;
}

static inline mat4 mult_mat4_mat4 (mat4 a, mat4 b) {
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

static inline vec4 mult_mat4_vec4 (mat4 m, vec4 v) {
	float x = m.m[0] * v.v[0] + m.m[4] * v.v[1] + m.m[8] * v.v[2] +
		m.m[12] * v.v[3];
	float y = m.m[1] * v.v[0] + m.m[5] * v.v[1] + m.m[9] * v.v[2] +
		m.m[13] * v.v[3];
	float z = m.m[2] * v.v[0] + m.m[6] * v.v[1] + m.m[10] * v.v[2] +
		m.m[14] * v.v[3];
	float w = m.m[3] * v.v[0] + m.m[7] * v.v[1] + m.m[11] * v.v[2] +
		m.m[15] * v.v[3];
	return (vec4){ .v = { x, y, z, w } };
}

static inline float det_mat4 (mat4 mm) {
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

static inline mat4 inverse_mat4 (mat4 mm) {
	float det = det_mat4 (mm);
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
static inline mat4 transpose_mat4 (mat4 mm) {
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
static inline mat4 translate_mat4 (vec3 vv) {
	mat4 r = identity_mat4 ();
	r.m[12] = vv.v[0];
	r.m[13] = vv.v[1];
	r.m[14] = vv.v[2];
	return r;
}

static inline mat4 rot_x_deg_mat4 (float deg) {
	float rad = deg * ONE_DEG_IN_RAD;
	mat4 r = identity_mat4 ();
	r.m[5] = cos (rad);
	r.m[9] = -sin (rad);
	r.m[6] = sin (rad);
	r.m[10] = cos (rad);
	return r;
}

static inline mat4 rot_y_deg_mat4 (float deg) {
	float rad = deg * ONE_DEG_IN_RAD;
	mat4 r = identity_mat4 ();
	r.m[0] = cos (rad);
	r.m[8] = sin (rad);
	r.m[2] = -sin (rad);
	r.m[10] = cos (rad);
	return r;
}

static inline mat4 rot_z_deg_mat4 (float deg) {
	float rad = deg * ONE_DEG_IN_RAD;
	mat4 r = identity_mat4 ();
	r.m[0] = cos (rad);
	r.m[4] = -sin (rad);
	r.m[1] = sin (rad);
	r.m[5] = cos (rad);
	return r;
}

static inline mat4 scale_mat4 (vec3 v) {
	mat4 r = identity_mat4 ();
	r.m[0] = v.v[0];
	r.m[5] = v.v[1];
	r.m[10] = v.v[2];
	return r;
}

/*-----------------------VIRTUAL CAMERA MATRIX FUNCTIONS---------------------*/
static inline mat4 look_at (vec3 cam_pos, vec3 targ_pos, vec3 up) {
	mat4 p = translate_mat4 ((vec3){ .v = {-cam_pos.v[0], -cam_pos.v[1],
		-cam_pos.v[2]}});
	vec3 d = sub_vec3_vec3 (targ_pos, cam_pos);
	vec3 f = normalise_vec3 (d);
	vec3 r = normalise_vec3 (cross_vec3 (f, up));
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

static inline mat4 perspective (float fovy, float aspect, float near, float far) {
	float fov_rad = fovy * ONE_DEG_IN_RAD;
	float range = tan (fov_rad / 2.0f) * near;
	float sx = (2.0f * near) / (range * aspect + range * aspect);
	float sy = near / range;
	float sz = -(far + near) / (far - near);
	float pz = -(2.0f * far * near) / (far - near);
	mat4 m = zero_mat4 ();
	m.m[0] = sx;
	m.m[5] = sy;
	m.m[10] = sz;
	m.m[14] = pz;
	m.m[11] = -1.0f;
	return m;
}

/*----------------------------HAMILTON IN DA HOUSE!--------------------------*/
static inline versor div_quat_f (versor qq, float s) {
	return (versor) {
		.q = { qq.q[0] / s, qq.q[1] / s, qq.q[2] / s, qq.q[3] / s } };
}

static inline versor mult_quat_f (versor qq, float s) {
	return (versor) {
		.q = { qq.q[0] * s, qq.q[1] * s, qq.q[2] * s, qq.q[3] * s } };
}

static inline versor normalise_quat (versor q) {
	float sum = q.q[0] * q.q[0] + q.q[1] * q.q[1] +
		q.q[2] * q.q[2] + q.q[3] * q.q[3];
	const float thresh = 0.0001f;
	if (fabs (1.0f - sum) < thresh) { return q; }
	float mag = sqrt (sum);
	return div_quat_f (q, mag);
}

static inline versor mult_quat_quat (versor a, versor b) {
	versor result;
	result.q[0] = b.q[0] * a.q[0] - b.q[1] * a.q[1] -
		b.q[2] * a.q[2] - b.q[3] * a.q[3];
	result.q[1] = b.q[0] * a.q[1] + b.q[1] * a.q[0] -
		b.q[2] * a.q[3] + b.q[3] * a.q[2];
	result.q[2] = b.q[0] * a.q[2] + b.q[1] * a.q[3] +
		b.q[2] * a.q[0] - b.q[3] * a.q[1];
	result.q[3] = b.q[0] * a.q[3] - b.q[1] * a.q[2] +
		b.q[2] * a.q[1] + b.q[3] * a.q[0];
	return normalise_quat (result);
}

static inline versor add_quat_quat (versor a, versor b) {
	versor result;
	result.q[0] = b.q[0] + a.q[0];
	result.q[1] = b.q[1] + a.q[1];
	result.q[2] = b.q[2] + a.q[2];
	result.q[3] = b.q[3] + a.q[3];
	return normalise_quat (result);
}

static inline versor quat_from_axis_rad (float radians, vec3 axis) {
	versor result;
	result.q[0] = cos (radians / 2.0);
	result.q[1] = sin (radians / 2.0) * axis.v[0];
	result.q[2] = sin (radians / 2.0) * axis.v[1];
	result.q[3] = sin (radians / 2.0) * axis.v[2];
	return result;
}

static inline versor quat_from_axis_deg (float degrees, vec3 axis) {
	return quat_from_axis_rad (ONE_DEG_IN_RAD * degrees, axis);
}

static inline mat4 quat_to_mat4 (versor q) {
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

static inline float dot_quat (versor q, versor r) {
	return q.q[0] * r.q[0] + q.q[1] * r.q[1] + q.q[2] * r.q[2] + q.q[3] * r.q[3];
}

static inline versor slerp_quat (versor q, versor r, float t) {
	float cos_half_theta = dot_quat (q, r);
	if (cos_half_theta < 0.0f) {
		for (int i = 0; i < 4; i++) { q.q[i] *= -1.0f; }
		cos_half_theta = dot_quat (q, r);
	}
	if (fabs (cos_half_theta) >= 1.0f) { return q; }
	float sin_half_theta = sqrt (1.0f - cos_half_theta * cos_half_theta);
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
	for (int i = 0; i < 4; i++) { result.q[i] = q.q[i] * a + r.q[i] * b; }
	return result;
}
