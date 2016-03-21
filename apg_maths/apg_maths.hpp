/*****************************************************************************\
| Anton's Maths Library - https://github.com/capnramses/apg                   |
| This is the C++ version with operator overloading.                          |
| e-mail: anton at antongerdelan dot net                                      |
| Revised and inlined into a header file: 16 Jun 2014                         |
| Replaced internal arrays with .x .y .z etc: 29 January 2016                 |
| Copyright Dr Anton Gerdelan, Trinity College Dublin, Ireland                |
|*****************************************************************************|
| Commonly-used maths structures and functions for 3d graphics                |
|                                                                             |
| #Use#                                                                       |
| Just include this header file, you don't need to put any macro junk.        |
| Declare variables of type: vec2, vec3, vec4, mat3, mat4, versor.            |
| A versor is the proper name for a unit quaternion.                          |
| vec3 a = vec3 (0.0f, 1.0f, 2.0f); // constructors similar to GLSL           |
| vec3 b = a; // copy constructor                                             |
| vec3 c = a - b; // overloaded operators                                     |
| print (c); // helper functions                                              |
| mat4 R = rotate_y_deg (10.0f); // affine and virtual camera builders        |
| vec4 d = R * vec4 (c, 1.0); // works like you'd expect                      |
| glUniformMatrix4fv (location, 1, GL_FALSE, R.m); // access data in matrix   |
| glUniform3f (location, c.x, c.y, c.z); // access data in vector             |
|                                                                             |
| #Design#                                                                    |
| ##(1) Functionality / Familiarity / Error Protection##                      |
| The functions and data types resemble GLSL and also the old glu interface.  |
| e.g. normalise (my_vec3) rather than my_vec3.normalise().                   |
| I did insist on Imperial Standard English though.                           |
| I only add functions when I actually use them more than once.               |
| Some are deliberately left out to avoid common mistakes e.g. component-wise |
| operations on vec4s. If you really want to vec4*vec4 then I would code it   |
| manually each time, just to be on the safe side.                            |
| I considered just using arrays of floats instead of vec3 etc. to satisfy (3)|
| but the type safety and similarity to GLSL were more important.             |
| I changed internal memory from an array to .x .y etc. for vectors, but not  |
| matrices. Experience shows this is less confusing to read than i.e.         |
| my_vector.v[1]. I didn't go the way of GLM to have both because it violates |
| (2) and (3).                                                                |
| We can assume struct contents are always ordered as expected in memory in   |
| C++ so we can get away with it here, but this is not so in C before C99!    |
| ##(2) Readability##                                                         |
| I want students to be able to follow most of it, so I don't use any         |
| short-hands that might make sense to more experienced graphics people and   |
| try to add lots of appropriate comments                                     |
| ##(3) Simplicity##                                                          |
| Just one plain header file, no library linkage required, no build system.   |
| No external docs needed. No templates. No scary C++ OOP bits.               |
| ##(4) Efficiency##                                                          |
| Mostly inlined, simple, cache-friendly code. I might add SIMD intrinsics    |
| if it doesn't impact (2) and (3) much, and/or is educational.               |
\*****************************************************************************/
#pragma once
#include <stdio.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <string.h>
#include <assert.h>
// some implementations of math.h removed M_PI
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
// const used to convert degrees into radians
#define ONE_DEG_IN_RAD (2.0 * M_PI) / 360.0 // 0.017444444
#define ONE_RAD_IN_DEG 360.0 / (2.0 * M_PI) //57.2957795
// ----------------------------------------------------------------------------
// -------------------------------- declaration -------------------------------
// ----------------------------------------------------------------------------
// ------------------------------ data structures -----------------------------
struct vec2;
struct vec3;
struct vec4;
struct versor;
struct mat3;
struct mat4;
// ------------------------------ print functions -----------------------------
void print (const vec2& v);
void print (const vec3& v);
void print (const vec4& v);
void print (const mat3& m);
void print (const mat4& m);
void print (const versor& q);
// ----------------------------- vector functions -----------------------------
float length (const vec2& v);
float length2 (const vec2& v);
vec2 normalise (const vec2& v);
float length (const vec3& v);
float length2 (const vec3& v);
vec3 normalise (const vec3& v);
float dot (const vec3& a, const vec3& b);
inline vec3 cross (const vec3& a, const vec3& b);
float direction_to_heading (const vec3& d);
vec3 heading_to_direction (float degrees);
vec3 lerp (const vec3& a, const vec3& b, float t);
// ----------------------------- matrix functions -----------------------------
mat3 zero_mat3 ();
mat3 identity_mat3 ();
mat4 zero_mat4 ();
mat4 identity_mat4 ();
float determinant (const mat4& mm);
mat4 inverse (const mat4& mm);
mat4 transpose (const mat4& mm);
// ----------------------------- affine functions -----------------------------
mat4 translate (const vec3& v);
mat4 rot_x_deg (float deg);
mat4 rot_y_deg (float deg);
mat4 rot_z_deg (float deg);
mat4 scale (const vec3& v);
// ---------------------------- camera functions ------------------------------
mat4 look_at (const vec3& cam_pos, vec3 targ_pos, const vec3& up);
mat4 perspective (float fovy, float aspect, float near, float far);
// --------------------------- quaternion functions ---------------------------
versor quat_from_axis_rad (float radians, const vec3& axis);
versor quat_from_axis_deg (float degrees, const vec3& axis);
mat4 quat_to_mat4 (const versor& q);
float dot (const versor& q, const versor& r);
// stupid overloading wouldn't let me use const
versor normalise (versor q);
versor slerp (versor q, versor r, float t);
// ----------------------------------------------------------------------------
// ----------------------------- implementation ------------------------------
// ----------------------------------------------------------------------------
struct vec2 {
	float x, y;
	vec2 () {}
	vec2 (float _x, float _y) {
		x = _x;
		y = _y;
	}
	vec2 (const vec2& v) {
		x = v.x;
		y = v.y;
	}
	vec2 (const vec3& v);
	vec2 (const vec4& v);
};

struct vec3 {
	float x, y, z;
	vec3 () {}
	vec3 (float _x, float _y, float _z) {
		x = _x;
		y = _y;
		z = _z;
	}
	vec3 (const vec2& v, float _z) {
		x = v.x;
		y = v.y;
		z = _z;
	}
	vec3 (const vec3& v) {
		x = v.x;
		y = v.y;
		z = v.z;
	}
	vec3 (const vec4& v);
	vec3 operator+ (const vec3& rhs) {
		vec3 vc;
		vc.x = x + rhs.x;
		vc.y = y + rhs.y;
		vc.z = z + rhs.z;
		return vc;
	}
	vec3& operator+= (const vec3& rhs) {
		x += rhs.x;
		y += rhs.y;
		z += rhs.z;
		return *this; // return self
	}
	vec3 operator- (const vec3& rhs) {
		vec3 vc;
		vc.x = x - rhs.x;
		vc.y = y - rhs.y;
		vc.z = z - rhs.z;
		return vc;
	}
	vec3& operator-= (const vec3& rhs) {
		x -= rhs.x;
		y -= rhs.y;
		z -= rhs.z;
		return *this;
	}
	vec3 operator* (const vec3& rhs) {
		vec3 vc;
		vc.x = x * rhs.x;
		vc.y = y * rhs.y;
		vc.z = z * rhs.z;
		return vc;
	}
	vec3& operator*= (const vec3& rhs) {
		x *= rhs.x;
		y *= rhs.y;
		z *= rhs.z;
		return *this;
	}
	vec3 operator/ (const vec3& rhs) {
		vec3 vc;
		vc.x = x / rhs.x;
		vc.y = y / rhs.y;
		vc.z = z / rhs.z;
		return vc;
	}
	vec3& operator/= (const vec3& rhs) {
		x /= rhs.x;
		y /= rhs.y;
		z /= rhs.z;
		return *this;
	}
	vec3 operator+ (float rhs) {
		vec3 vc;
		vc.x = x + rhs;
		vc.y = y + rhs;
		vc.z = z + rhs;
		return vc;
	}
	vec3& operator+= (float rhs) {
		x += rhs;
		y += rhs;
		z += rhs;
		return *this;
	}
	vec3 operator- (float rhs) {
		vec3 vc;
		vc.x = x - rhs;
		vc.y = y - rhs;
		vc.z = z - rhs;
		return vc;
	}
	vec3& operator-= (float rhs) {
		x -= rhs;
		y -= rhs;
		z -= rhs;
		return *this;
	}
	vec3 operator* (float rhs) {
		vec3 vc;
		vc.x = x * rhs;
		vc.y = y * rhs;
		vc.z = z * rhs;
		return vc;
	}
	vec3& operator*= (float rhs) {
		x *= rhs;
		y *= rhs;
		z *= rhs;
		return *this;
	}
	vec3 operator/ (float rhs) {
		vec3 vc;
		vc.x = x / rhs;
		vc.y = y / rhs;
		vc.z = z / rhs;
		return vc;
	}
	vec3& operator/= (float rhs) {
		x /= rhs;
		y /= rhs;
		z /= rhs;
		return *this;
	}
	vec3& operator= (const vec3& rhs) {
		x = rhs.x;
		y = rhs.y;
		z = rhs.z;
		return *this;
	}
};

struct vec4 {
	float x, y, z, w;
	vec4 () {}
	vec4 (float _x, float _y, float _z, float _w) {
		x = _x;
		y = _y;
		z = _z;
		w = _w;
	}
	vec4 (const vec2& v, float _z, float _w) {
		x = v.x;
		y = v.y;
		z = _z;
		w = _w;
	}
	vec4 (const vec3& v, float _w) {
		x = v.x;
		y = v.y;
		z = v.z;
		w = _w;
	}
	vec4 (const vec4& v) {
		x = v.x;
		y = v.y;
		z = v.z;
		w = v.w;
	}
	// NOTE: i deliberately didn't overload arithmetic operators for the vec4,
	// because it's usually a mistake when you do piece-wise arithmetic with
	// vec4s in graphics as the 4th channel is not part of the vector proper
};

// stored like this:
// 00 03 06
// 01 04 07
// 02 05 08
struct mat3 {
	float m[9];
	// NOTE: i deliberately didn't put contructors in because this is a source of
	// confusion and error
	float get_el (int col, int row) {
		assert (row >= 0 && row <= 2 && col >= 0 && col <= 2);
		int index = col * 3 + row;
		return m[index];
	}
	float set_el (int col, int row, float val) {
		assert (row >= 0 && row <= 2 && col >= 0 && col <= 2);
		int index = col * 3 + row;
		m[index] = val;
	}
};

// stored like this:
// 00 04 08 12
// 01 05 09 13
// 02 06 10 14
// 03 07 11 15
struct mat4 {
	float m[16];
	// NOTE: i deliberately didn't put contructors in because this is a source of
	// confusion and error
	float get_el (int col, int row) {
		assert (row >= 0 && row <= 3 && col >= 0 && col <= 3);
		int index = col * 4 + row;
		return m[index];
	}
	float set_el (int col, int row, float val) {
		assert (row >= 0 && row <= 3 && col >= 0 && col <= 3);
		int index = col * 4 + row;
		m[index] = val;
	}
	vec4 operator* (const vec4& rhs) {
		// 0x + 4y + 8z + 12w
		float x = m[0] * rhs.x + m[4] * rhs.y + m[8] * rhs.z + m[12] * rhs.w;
		// 1x + 5y + 9z + 13w
		float y = m[1] * rhs.x + m[5] * rhs.y + m[9] * rhs.z + m[13] * rhs.w;
		// 2x + 6y + 10z + 14w
		float z = m[2] * rhs.x + m[6] * rhs.y + m[10] * rhs.z + m[14] * rhs.w;
		// 3x + 7y + 11z + 15w
		float w = m[3] * rhs.x + m[7] * rhs.y + m[11] * rhs.z + m[15] * rhs.w;
		return vec4 (x, y, z, w);
	}
	mat4 operator* (const mat4& rhs) {
		mat4 r = zero_mat4 ();
		int r_index = 0;
		for (int col = 0; col < 4; col++) {
			for (int row = 0; row < 4; row++) {
				float sum = 0.0f;
				for (int i = 0; i < 4; i++) {
					sum += rhs.m[i + col * 4] * m[row + i * 4];
				}
				r.m[r_index] = sum;
				r_index++;
			}
		}
		return r;
	}
	mat4& operator= (const mat4& rhs) {
		for (int i = 0; i < 16; i++) {
			m[i] = rhs.m[i];
		}
		return *this;
	}
};

// a unit quaternion used for rotation
struct versor {
	float w, x, y, z; // NOTE: 'w' is q[0] here, not position [3] as in vec4
	versor operator/ (float rhs) {
		versor result;
		result.w = w / rhs;
		result.x = x / rhs;
		result.y = y / rhs;
		result.z = z / rhs;
		return result;
	}
	versor operator* (float rhs) {
		versor result;
		result.w = w * rhs;
		result.x = x * rhs;
		result.y = y * rhs;
		result.z = z * rhs;
		return result;
	}
	versor operator* (const versor& rhs) {
		versor result;
		result.w = rhs.w * w - rhs.x * x -
			rhs.y * y - rhs.z * z;
		result.x = rhs.w * x + rhs.x * w -
			rhs.y * z + rhs.z * y;
		result.y = rhs.w * y + rhs.x * z +
			rhs.y * w - rhs.z * x;
		result.z = rhs.w * z - rhs.x * y +
			rhs.y * x + rhs.z * w;
		// re-normalise in case of mangling
		return normalise (result);
	}
	versor operator+ (const versor& rhs) {
		versor result;
		result.w = rhs.w + w;
		result.x = rhs.x + x;
		result.y = rhs.y + y;
		result.z = rhs.z + z;
		// re-normalise in case of mangling
		return normalise (result);
	}
};

/*-----------------------------PRINT FUNCTIONS-------------------------------*/
inline void print (const vec2& v) {
	printf ("[%.2f, %.2f]\n", v.x, v.y);
}
inline void print (const vec3& v) {
	printf ("[%.2f, %.2f, %.2f]\n", v.x, v.y, v.z);
}
inline void print (const vec4& v) {
	printf ("[%.2f, %.2f, %.2f, %.2f]\n", v.x, v.y, v.z, v.w);
}
inline void print (const mat3& m) {
	printf("\n");
	printf ("[%.2f][%.2f][%.2f]\n", m.m[0], m.m[3], m.m[6]);
	printf ("[%.2f][%.2f][%.2f]\n", m.m[1], m.m[4], m.m[7]);
	printf ("[%.2f][%.2f][%.2f]\n", m.m[2], m.m[5], m.m[8]);
}
inline void print (const mat4& m) {
	printf("\n");
	printf ("[%.2f][%.2f][%.2f][%.2f]\n", m.m[0], m.m[4], m.m[8], m.m[12]);
	printf ("[%.2f][%.2f][%.2f][%.2f]\n", m.m[1], m.m[5], m.m[9], m.m[13]);
	printf ("[%.2f][%.2f][%.2f][%.2f]\n", m.m[2], m.m[6], m.m[10], m.m[14]);
	printf ("[%.2f][%.2f][%.2f][%.2f]\n", m.m[3], m.m[7], m.m[11], m.m[15]);
}
inline void print (const versor& q) {
	printf ("[%.2f ,%.2f, %.2f, %.2f]\n", q.w, q.x, q.y, q.z);
}
/*------------------------------VECTOR FUNCTIONS-----------------------------*/
// these are out here because compiler complains about incomplete type usage
inline vec2::vec2 (const vec3& v) {
	x = v.x;
	y = v.y;
}

inline vec2::vec2 (const vec4& v) {
	x = v.x;
	y = v.y;
}

inline vec3::vec3 (const vec4& v) {
	x = v.x;
	y = v.y;
	z = v.z;
}

inline float length (const vec2& v) {
	return sqrt (v.x * v.x + v.y * v.y);
}

// squared length
inline float length2 (const vec2& v) {
	return v.x * v.x + v.y * v.y;
}

// NOTE: proper spelling (hehe)
inline vec2 normalise (const vec2& v) {
	vec2 vb;
	float l = length (v);
	if (0.0f == l) {
		return vec2 (0.0f, 0.0f);
	}
	vb.x = v.x / l;
	vb.y = v.y / l;
	return vb;
}

inline float length (const vec3& v) {
	return sqrt (v.x * v.x + v.y * v.y + v.z * v.z);
}

// squared length
inline float length2 (const vec3& v) {
	return v.x * v.x + v.y * v.y + v.z * v.z;
}

// NOTE: proper spelling (hehe)
inline vec3 normalise (const vec3& v) {
	vec3 vb;
	float l = length (v);
	if (0.0f == l) {
		return vec3 (0.0f, 0.0f, 0.0f);
	}
	vb.x = v.x / l;
	vb.y = v.y / l;
	vb.z = v.z / l;
	return vb;
}

inline float dot (const vec3& a, const vec3& b) {
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

inline vec3 cross (const vec3& a, const vec3& b) {
	float x = a.y * b.z - a.z * b.y;
	float y = a.z * b.x - a.x * b.z;
	float z = a.x * b.y - a.y * b.x;
	return vec3 (x, y, z);
}

// converts an un-normalised direction vector's X,Z components into a heading
// in degrees
// NB i suspect that the z is backwards here but i've used in in
// several places like this. d'oh!
inline float direction_to_heading (vec3 d) {
	return (float)(atan2 (-d.x, -d.z) * ONE_RAD_IN_DEG);
}

// very informal function to convert a heading (e.g. y-axis orientation) into
// a 3d vector with components in x and z axes
inline vec3 heading_to_direction (float degrees) {
	float rad = degrees * ONE_DEG_IN_RAD;
	return vec3 (-sinf (rad), 0.0f, -cosf (rad));
}

inline vec3 lerp (vec3 a, vec3 b, float t) {
	return a * t + b * (1.0f - t);
}
/*-----------------------------MATRIX FUNCTIONS------------------------------*/
inline mat3 zero_mat3 () {
	mat3 M;
	memset (M.m, 0, 9 * sizeof (float));
	return M;
}

inline mat3 identity_mat3 () {
	mat3 M = zero_mat3 ();
	M.m[0] = M.m[4] = M.m[8] = 1.0f;
	return M;
}

inline mat4 zero_mat4 () {
	mat4 M;
	memset (M.m, 0, 16 * sizeof(float));
	return M;
}

inline mat4 identity_mat4 () {
	mat4 M = zero_mat4 ();
	M.m[0] = M.m[5] = M.m[10] = M.m[15] = 1.0f;
	return M;
}

// returns a scalar value with the determinant for a 4x4 matrix
// see http://www.euclideanspace.com/maths/algebra/matrix/functions/determinant/fourD/index.htm
inline float determinant (const mat4& mm) {
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

// see http://www.euclideanspace.com/maths/algebra/matrix/functions/inverse/fourD/index.htm
inline mat4 inverse (const mat4& mm) {
	float det = determinant (mm);
	/* there is no inverse if determinant is zero (not likely unless scale is
	broken) */
	if (0.0f == det) {
		fprintf (stderr, "WARNING. matrix has no determinant. can not invert.\n");
		return mm;
	}
	float inv_det = 1.0f / det;
	mat4 R;
	R.m[0] = inv_det * (
		mm.m[9] * mm.m[14] * mm.m[7] - mm.m[13] * mm.m[10] * mm.m[7] +
		mm.m[13] * mm.m[6] * mm.m[11] - mm.m[5] * mm.m[14] * mm.m[11] -
		mm.m[9] * mm.m[6] * mm.m[15] + mm.m[5] * mm.m[10] * mm.m[15]
	);
	R.m[1] = inv_det * (
		mm.m[13] * mm.m[10] * mm.m[3] - mm.m[9] * mm.m[14] * mm.m[3] -
		mm.m[13] * mm.m[2] * mm.m[11] + mm.m[1] * mm.m[14] * mm.m[11] +
		mm.m[9] * mm.m[2] * mm.m[15] - mm.m[1] * mm.m[10] * mm.m[15]
	);
	R.m[2] = inv_det * (
		mm.m[5] * mm.m[14] * mm.m[3] - mm.m[13] * mm.m[6] * mm.m[3] +
		mm.m[13] * mm.m[2] * mm.m[7] - mm.m[1] * mm.m[14] * mm.m[7] -
		mm.m[5] * mm.m[2] * mm.m[15] + mm.m[1] * mm.m[6] * mm.m[15]
	);
	R.m[3] = inv_det * (
		mm.m[9] * mm.m[6] * mm.m[3] - mm.m[5] * mm.m[10] * mm.m[3] -
		mm.m[9] * mm.m[2] * mm.m[7] + mm.m[1] * mm.m[10] * mm.m[7] +
		mm.m[5] * mm.m[2] * mm.m[11] - mm.m[1] * mm.m[6] * mm.m[11]
	);
	R.m[4] = inv_det * (
		mm.m[12] * mm.m[10] * mm.m[7] - mm.m[8] * mm.m[14] * mm.m[7] -
		mm.m[12] * mm.m[6] * mm.m[11] + mm.m[4] * mm.m[14] * mm.m[11] +
		mm.m[8] * mm.m[6] * mm.m[15] - mm.m[4] * mm.m[10] * mm.m[15]
	);
	R.m[5] = inv_det * (
		mm.m[8] * mm.m[14] * mm.m[3] - mm.m[12] * mm.m[10] * mm.m[3] +
		mm.m[12] * mm.m[2] * mm.m[11] - mm.m[0] * mm.m[14] * mm.m[11] -
		mm.m[8] * mm.m[2] * mm.m[15] + mm.m[0] * mm.m[10] * mm.m[15]
	);
	R.m[6] = inv_det * (
		mm.m[12] * mm.m[6] * mm.m[3] - mm.m[4] * mm.m[14] * mm.m[3] -
		mm.m[12] * mm.m[2] * mm.m[7] + mm.m[0] * mm.m[14] * mm.m[7] +
		mm.m[4] * mm.m[2] * mm.m[15] - mm.m[0] * mm.m[6] * mm.m[15]
	);
	R.m[7] = inv_det * (
		mm.m[4] * mm.m[10] * mm.m[3] - mm.m[8] * mm.m[6] * mm.m[3] +
		mm.m[8] * mm.m[2] * mm.m[7] - mm.m[0] * mm.m[10] * mm.m[7] -
		mm.m[4] * mm.m[2] * mm.m[11] + mm.m[0] * mm.m[6] * mm.m[11]
	);
	R.m[8] = inv_det * (
		mm.m[8] * mm.m[13] * mm.m[7] - mm.m[12] * mm.m[9] * mm.m[7] +
		mm.m[12] * mm.m[5] * mm.m[11] - mm.m[4] * mm.m[13] * mm.m[11] -
		mm.m[8] * mm.m[5] * mm.m[15] + mm.m[4] * mm.m[9] * mm.m[15]
	);
	R.m[9] = inv_det * (
		mm.m[12] * mm.m[9] * mm.m[3] - mm.m[8] * mm.m[13] * mm.m[3] -
		mm.m[12] * mm.m[1] * mm.m[11] + mm.m[0] * mm.m[13] * mm.m[11] +
		mm.m[8] * mm.m[1] * mm.m[15] - mm.m[0] * mm.m[9] * mm.m[15]
	);
	R.m[10] = inv_det * (
		mm.m[4] * mm.m[13] * mm.m[3] - mm.m[12] * mm.m[5] * mm.m[3] +
		mm.m[12] * mm.m[1] * mm.m[7] - mm.m[0] * mm.m[13] * mm.m[7] -
		mm.m[4] * mm.m[1] * mm.m[15] + mm.m[0] * mm.m[5] * mm.m[15]
	);
	R.m[11] = inv_det * (
		mm.m[8] * mm.m[5] * mm.m[3] - mm.m[4] * mm.m[9] * mm.m[3] -
		mm.m[8] * mm.m[1] * mm.m[7] + mm.m[0] * mm.m[9] * mm.m[7] +
		mm.m[4] * mm.m[1] * mm.m[11] - mm.m[0] * mm.m[5] * mm.m[11]
	);
	R.m[12] = inv_det * (
		mm.m[12] * mm.m[9] * mm.m[6] - mm.m[8] * mm.m[13] * mm.m[6] -
		mm.m[12] * mm.m[5] * mm.m[10] + mm.m[4] * mm.m[13] * mm.m[10] +
		mm.m[8] * mm.m[5] * mm.m[14] - mm.m[4] * mm.m[9] * mm.m[14]
	);
	R.m[13] = inv_det * (
		mm.m[8] * mm.m[13] * mm.m[2] - mm.m[12] * mm.m[9] * mm.m[2] +
		mm.m[12] * mm.m[1] * mm.m[10] - mm.m[0] * mm.m[13] * mm.m[10] -
		mm.m[8] * mm.m[1] * mm.m[14] + mm.m[0] * mm.m[9] * mm.m[14]
	);
	R.m[14] = inv_det * (
		mm.m[12] * mm.m[5] * mm.m[2] - mm.m[4] * mm.m[13] * mm.m[2] -
		mm.m[12] * mm.m[1] * mm.m[6] + mm.m[0] * mm.m[13] * mm.m[6] +
		mm.m[4] * mm.m[1] * mm.m[14] - mm.m[0] * mm.m[5] * mm.m[14]
	);
	R.m[15] = inv_det * (
		mm.m[4] * mm.m[9] * mm.m[2] - mm.m[8] * mm.m[5] * mm.m[2] +
		mm.m[8] * mm.m[1] * mm.m[6] - mm.m[0] * mm.m[9] * mm.m[6] -
		mm.m[4] * mm.m[1] * mm.m[10] + mm.m[0] * mm.m[5] * mm.m[10]
	);
	return R;
}

inline mat4 transpose (const mat4& mm) {
	mat4 M;
	M.m[0] = mm.m[0]; M.m[4] = mm.m[1]; M.m[8] = mm.m[2]; M.m[12] = mm.m[3];
	M.m[1] = mm.m[4]; M.m[5] = mm.m[5]; M.m[9] = mm.m[6]; M.m[13] = mm.m[7];
	M.m[2] = mm.m[8]; M.m[6] = mm.m[9]; M.m[10] = mm.m[10]; M.m[14] = mm.m[11];
	M.m[3] = mm.m[12]; M.m[7] = mm.m[13]; M.m[11] = mm.m[14]; M.m[15] = mm.m[15];
	return M;
}
/*--------------------------AFFINE MATRIX FUNCTIONS--------------------------*/
inline mat4 translate (const vec3& v) {
	mat4 m_t = identity_mat4 ();
	m_t.m[12] = v.x;
	m_t.m[13] = v.y;
	m_t.m[14] = v.z;
	return m_t;
}

inline mat4 rotate_x_deg (float deg) {
	// convert to radians
	float rad = deg * ONE_DEG_IN_RAD;
	mat4 m_r = identity_mat4 ();
	m_r.m[5] = cos (rad);
	m_r.m[9] = -sin (rad);
	m_r.m[6] = sin (rad);
	m_r.m[10] = cos (rad);
	return m_r;
}

inline mat4 rotate_y_deg (float deg) {
	// convert to radians
	float rad = deg * ONE_DEG_IN_RAD;
	mat4 m_r = identity_mat4 ();
	m_r.m[0] = cos (rad);
	m_r.m[8] = sin (rad);
	m_r.m[2] = -sin (rad);
	m_r.m[10] = cos (rad);
	return m_r;
}

inline mat4 rotate_z_deg (float deg) {
	// convert to radians
	float rad = deg * ONE_DEG_IN_RAD;
	mat4 m_r = identity_mat4 ();
	m_r.m[0] = cos (rad);
	m_r.m[4] = -sin (rad);
	m_r.m[1] = sin (rad);
	m_r.m[5] = cos (rad);
	return m_r;
}

inline mat4 scale (const vec3& v) {
	mat4 a = identity_mat4 ();
	a.m[0] = v.x;
	a.m[5] = v.y;
	a.m[10] = v.z;
	return a;
}

/*-----------------------VIRTUAL CAMERA MATRIX FUNCTIONS---------------------*/
// returns a view matrix using the GLU lookAt style.
// NOTE: targ_pos is not const& because using the overloaded "-" in here makes
// the compiler barf
inline mat4 look_at (const vec3& cam_pos, vec3 targ_pos, const vec3& up) {
	// inverse translation
	mat4 p = identity_mat4 ();
	p = translate (vec3 (-cam_pos.x, -cam_pos.y, -cam_pos.z));
	// distance vector
	vec3 d = targ_pos - cam_pos;
	// forward vector
	vec3 f = normalise (d);
	// right vector
	vec3 r = normalise (cross (f, up));
	// real up vector
	vec3 u = normalise (cross (r, f));
	mat4 ori = identity_mat4 ();
	ori.m[0] = r.x;
	ori.m[4] = r.y;
	ori.m[8] = r.z;
	ori.m[1] = u.x;
	ori.m[5] = u.y;
	ori.m[9] = u.z;
	ori.m[2] = -f.x;
	ori.m[6] = -f.y;
	ori.m[10] = -f.z;
	return ori * p;
}

// returns a perspective matrix mimicking the opengl projection style
// remeber if calculating aspect to do floating point division, not integer
inline mat4 perspective (float fovy, float aspect, float nr, float fr) {
	float fov_rad = fovy * ONE_DEG_IN_RAD;
	float range = tan (fov_rad / 2.0f) * nr;
	float sx = (2.0f * nr) / (range * aspect + range * aspect);
	float sy = nr / range;
	float sz = -(fr + nr) / (fr - nr);
	float pz = -(2.0f * fr * nr) / (fr - nr);
	mat4 m = zero_mat4 (); // make sure bottom-right corner is zero
	m.m[0] = sx;
	m.m[5] = sy;
	m.m[10] = sz;
	m.m[14] = pz;
	m.m[11] = -1.0f;
	return m;
}
/*----------------------------HAMILTON IN DA HOUSE!--------------------------*/
// create quaternion from normalised axis and angle in radians around axis
inline versor quat_from_axis_rad (float radians, const vec3& axis) {
	versor result;
	result.w = cosf (radians / 2.0f);
	result.x = sinf (radians / 2.0f) * axis.x;
	result.y = sinf (radians / 2.0f) * axis.y;
	result.z = sinf (radians / 2.0f) * axis.z;
	return result;
}

// create quaternion from normalised axis and angle in degrees around axis
inline versor quat_from_axis_deg (float degrees, const vec3& axis) {
	return quat_from_axis_rad (ONE_DEG_IN_RAD * degrees, axis);
}

// convert versor to rotation matrix
inline mat4 quat_to_mat4 (const versor& q) {
	float w = q.w;
	float x = q.x;
	float y = q.y;
	float z = q.z;
	mat4 R;
	R.m[0] = 1.0f - 2.0f * y * y - 2.0f * z * z;
	R.m[1] = 2.0f * x * y + 2.0f * w * z;
	R.m[2] = 2.0f * x * z - 2.0f * w * y;
	R.m[3] = 0.0f;
	R.m[4] = 2.0f * x * y - 2.0f * w * z;
	R.m[5] = 1.0f - 2.0f * x * x - 2.0f * z * z;
	R.m[6] = 2.0f * y * z + 2.0f * w * x;
	R.m[7] = 0.0f;
	R.m[8] = 2.0f * x * z + 2.0f * w * y;
	R.m[9] = 2.0f * y * z - 2.0f * w * x;
	R.m[10] = 1.0f - 2.0f * x * x - 2.0f * y * y;
	R.m[11] = 0.0f;
	R.m[12] = 0.0f;
	R.m[13] = 0.0f;
	R.m[14] = 0.0f;
	R.m[15] = 1.0f;
	return R;
}

// normalise a quaternion into a unit quaternion (versor) for use in rotation
inline versor normalise (versor q) {
	// norm(q) = q / magnitude (q)
	// magnitude (q) = sqrt (w*w + x*x...)
	// only compute sqrt if interior sum != 1.0
	float sum = q.w * q.w + q.x * q.x +
		q.y * q.y + q.z * q.z;
	// NB: floats have min 6 digits of precision
	const float thresh = 0.0001f;
	if (fabs (1.0f - sum) < thresh) {
		return q;
	}
	float mag = sqrt (sum);
	return q / mag;
}

// dot product of two quaternions
inline float dot (const versor& q, const versor& r) {
	return q.w * r.w + q.x * r.x + q.y * r.y + q.z * r.z;
}

// spherical linear interpolation between two quaternions
// factor t between 0.0 and 1.0
// returns interpolated versor
// NOTE: q is not const& here because i want a copy that i can modify
inline versor slerp (versor q, versor r, float t) {
	// angle between q0-q1
	float cos_half_theta = dot (q, r);
	// as found here http://stackoverflow.com/questions/2886606/flipping-issue-when-interpolating-rotations-using-quaternions
	// if dot product is negative then one quaternion should be negated, to make
	// it take the short way around, rather than the long way
	// yeah! and furthermore Susan, I had to recalculate the d.p. after this
	if (cos_half_theta < 0.0f) {
		q.w *= -1.0f;
		q.x *= -1.0f;
		q.y *= -1.0f;
		q.z *= -1.0f;
		cos_half_theta = dot (q, r);
	}
	// if qa=qb or qa=-qb then theta = 0 and we can return qa
	if (fabs (cos_half_theta) >= 1.0f) {
		return q;
	}
	// calculate temporary values
	float sin_half_theta = sqrt (1.0f - cos_half_theta * cos_half_theta);
	// if theta = 180 degrees then result is not fully defined
	// we could rotate around any axis normal to qa or qb
	versor result;
	if (fabs (sin_half_theta) < 0.001f) {
		result.w = (1.0f - t) * q.w + t * r.w;
		result.x = (1.0f - t) * q.x + t * r.x;
		result.y = (1.0f - t) * q.y + t * r.y;
		result.z = (1.0f - t) * q.z + t * r.z;
		return result;
	}
	float half_theta = acos (cos_half_theta);
	float a = sin ((1.0f - t) * half_theta) / sin_half_theta;
	float b = sin (t * half_theta) / sin_half_theta;
	result.w = q.w * a + r.w * b;
	result.x = q.x * a + r.x * b;
	result.y = q.y * a + r.y * b;
	result.z = q.z * a + r.z * b;
	return result;
}

