/* JavaScript 3D maths library.
	 
   Original Author: Anton Gerdelan - antongerdelan.net
	 Licence:         see bottom of file.
	 
	 Version History & Contributors:
		 06 Dec 2022 - Anton Gerdelan - "mult_" -> "mul". Updated some comment documentation.
									                  API-breaking change - matrix scale/rotate/translation functions no longer accept another matrix as an argument, and function names include "mat4".
		 23 jun 2015 - Anton Gerdelan - dot_versor() and slerp() added.
		 15 may 2015 - Anton Gerdelan - Added rotate_z_deg(), length2().
		 24 mar 2015 - Anton Gerdelan - Added look_at(). swapped argument order in mul_quat_quat() and transposed the multiplications in mul_mat4_vec4() for column-major.
		 19 mar 2015 - Emma Carrigan  - Fixed bugs in quat_quat_mult, scale_mat4.
	   23 dec 2014 - Anton Gerdelan - First conversion from C++ maths library.
*/

/*--------------------------------CONSTANTS----------------------------------*/

var TAU = 2.0 * Math.PI;
var ONE_DEG_IN_RAD = (2.0 * Math.PI) / 360.0; //  0.017444444
var ONE_RAD_IN_DEG = 360.0 / (2.0 * Math.PI); // 57.2957795

/*-----------------------------VECTOR FUNCTIONS------------------------------*/

function mul_vec2_scal(s, x) {
	var v = new Array();
	v[0] = s[0] * x;
	v[1] = s[1] * x;
	return v;
}

function mul_vec3_scal(s, x) {
	var v = new Array();
	v[0] = s[0] * x;
	v[1] = s[1] * x;
	v[2] = s[2] * x;
	return v;
}

function add_vec2_vec2(s, t) {
	var v = new Array();
	v[0] = s[0] + t[0];
	v[1] = s[1] + t[1];
	return v;
}

function add_vec3_vec3(s, t) {
	var v = new Array();
	v[0] = s[0] + t[0];
	v[1] = s[1] + t[1];
	v[2] = s[2] + t[2];
	return v;
}

function sub_vec2_vec2(s, t) {
	var v = new Array();
	v[0] = s[0] - t[0];
	v[1] = s[1] - t[1];
	return v;
}

function sub_vec3_vec3(s, t) {
	var v = new Array();
	v[0] = s[0] - t[0];
	v[1] = s[1] - t[1];
	v[2] = s[2] - t[2];
	return v;
}

function mul_vec2_vec2(s, t) {
	var v = new Array();
	v[0] = s[0] * t[0];
	v[1] = s[1] * t[1];
	return v;
}

function mul_vec3_vec3(s, t) {
	var v = new Array();
	v[0] = s[0] * t[0];
	v[1] = s[1] * t[1];
	v[2] = s[2] * t[2];
	return v;
}

function length2_vec2(v) {
	return v[0] * v[0] + v[1] * v[1];
}

function length_vec2(v) {
	return Math.sqrt(v[0] * v[0] + v[1] * v[1]);
}

function length2_vec3(v) {
	return v[0] * v[0] + v[1] * v[1] + v[2] * v[2];
}

function length_vec3(v) {
	return Math.sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
}

function normalise_vec2(v) {
	var length = length_vec2(v);
	return [v[0] / length, v[1] / length];
}

function normalise_vec3(v) {
	var length = length_vec3(v);
	return [v[0] / length, v[1] / length, v[2] / length];
}

function dot_vec2(v, w) {
	return v[0] * w[0] + v[1] * w[1];
}

function dot_vec3(v, w) {
	return v[0] * w[0] + v[1] * w[1] + v[2] * w[2];
}

function cross_vec3(a, b) {
	var v = new Array();
	v[0] = a[1] * b[2] - a[2] * b[1];
	v[1] = a[2] * b[0] - a[0] * b[2];
	v[2] = a[0] * b[1] - a[1] * b[0];
	return v;
}

/*-----------------------------MATRIX FUNCTIONS------------------------------*/

//
// return the identity 4x4 matrix
//
function identity_mat4() {
	return [
		1.0, 0.0, 0.0, 0.0,
		0.0, 1.0, 0.0, 0.0,
		0.0, 0.0, 1.0, 0.0,
		0.0, 0.0, 0.0, 1.0
	];
}

//
// return the zero 4x4 matrix
//
function zero_mat4() {
	return [
		0.0, 0.0, 0.0, 0.0,
		0.0, 0.0, 0.0, 0.0,
		0.0, 0.0, 0.0, 0.0,
		0.0, 0.0, 0.0, 0.0
	];
}

//
// multiply 2 4x4 matrices together
//
function mul_mat4_mat4(a, b) {
	var r = [
		0.0, 0.0, 0.0, 0.0,
		0.0, 0.0, 0.0, 0.0,
		0.0, 0.0, 0.0, 0.0,
		0.0, 0.0, 0.0, 0.0
	];
	var r_index = 0;
	for (row = 0; row < 4; row++) {
		for (col = 0; col < 4; col++) {
			var sum = 0.0;
			for (i = 0; i < 4; i++) {
				sum += b[i + row * 4] * a[col + i * 4];
			}
			r[r_index] = sum;
			r_index++;
		}
	}
	return r;
}

//
// multiply 4d vector with 4x4 matrix. return resultant 4d vector
// 24 mar 2015 - anton - found out this was transposed and fixed it
//
function mul_mat4_vec4(m, v) {
	var x = m[0] * v[0] + m[4] * v[1] + m[8] * v[2] + m[12] * v[3];
	var y = m[1] * v[0] + m[5] * v[1] + m[9] * v[2] + m[13] * v[3];
	var z = m[2] * v[0] + m[6] * v[1] + m[10] * v[2] + m[14] * v[3];
	var w = m[3] * v[0] + m[7] * v[1] + m[11] * v[2] + m[15] * v[3];
	return [x, y, z, w];
}

//
// returns a scalar value with the determinant for a 4x4 matrix
// see http://www.euclideanspace.com/maths/algebra/matrix/functions/determinant/fourD/index.htm
//
function determinant(m) {
	// clang-format off
	return m[12] * m[9] * m[6] * m[3] -
		m[8] * m[13] * m[6] * m[3] -
		m[12] * m[5] * m[10] * m[3] +
		m[4] * m[13] * m[10] * m[3] +
		m[8] * m[5] * m[14] * m[3] -
		m[4] * m[9] * m[14] * m[3] -
		m[12] * m[9] * m[2] * m[7] +
		m[8] * m[13] * m[2] * m[7] +
		m[12] * m[1] * m[10] * m[7] -
		m[0] * m[13] * m[10] * m[7] -
		m[8] * m[1] * m[14] * m[7] +
		m[0] * m[9] * m[14] * m[7] +
		m[12] * m[5] * m[2] * m[11] -
		m[4] * m[13] * m[2] * m[11] -
		m[12] * m[1] * m[6] * m[11] +
		m[0] * m[13] * m[6] * m[11] +
		m[4] * m[1] * m[14] * m[11] -
		m[0] * m[5] * m[14] * m[11] -
		m[8] * m[5] * m[2] * m[15] +
		m[4] * m[9] * m[2] * m[15] +
		m[8] * m[1] * m[6] * m[15] -
		m[0] * m[9] * m[6] * m[15] -
		m[4] * m[1] * m[10] * m[15] +
		m[0] * m[5] * m[10] * m[15];
	// clang-format on
}

//
// returns a 16-element array that is the inverse of a 16-element array (4x4
// matrix).
// see http://www.euclideanspace.com/maths/algebra/matrix/functions/inverse/fourD/index.htm
//
function inverse_mat4(m) {
	var det = determinant(m);
	// there is no inverse if determinant is zero (not likely unless scale is broken)
	if (0.0 == det) {
		console.error("WARNING. matrix has no determinant. can not invert\n");
		return m;
	}
	var inv_det = 1.0 / det;

	var im = new Array();
	// clang-format off
	im = [
		inv_det * (
			m[9] * m[14] * m[7] - m[13] * m[10] * m[7] +
			m[13] * m[6] * m[11] - m[5] * m[14] * m[11] -
			m[9] * m[6] * m[15] + m[5] * m[10] * m[15]
		),
		inv_det * (
			m[13] * m[10] * m[3] - m[9] * m[14] * m[3] -
			m[13] * m[2] * m[11] + m[1] * m[14] * m[11] +
			m[9] * m[2] * m[15] - m[1] * m[10] * m[15]
		),
		inv_det * (
			m[5] * m[14] * m[3] - m[13] * m[6] * m[3] +
			m[13] * m[2] * m[7] - m[1] * m[14] * m[7] -
			m[5] * m[2] * m[15] + m[1] * m[6] * m[15]
		),
		inv_det * (
			m[9] * m[6] * m[3] - m[5] * m[10] * m[3] -
			m[9] * m[2] * m[7] + m[1] * m[10] * m[7] +
			m[5] * m[2] * m[11] - m[1] * m[6] * m[11]
		),
		inv_det * (
			m[12] * m[10] * m[7] - m[8] * m[14] * m[7] -
			m[12] * m[6] * m[11] + m[4] * m[14] * m[11] +
			m[8] * m[6] * m[15] - m[4] * m[10] * m[15]
		),
		inv_det * (
			m[8] * m[14] * m[3] - m[12] * m[10] * m[3] +
			m[12] * m[2] * m[11] - m[0] * m[14] * m[11] -
			m[8] * m[2] * m[15] + m[0] * m[10] * m[15]
		),
		inv_det * (
			m[12] * m[6] * m[3] - m[4] * m[14] * m[3] -
			m[12] * m[2] * m[7] + m[0] * m[14] * m[7] +
			m[4] * m[2] * m[15] - m[0] * m[6] * m[15]
		),
		inv_det * (
			m[4] * m[10] * m[3] - m[8] * m[6] * m[3] +
			m[8] * m[2] * m[7] - m[0] * m[10] * m[7] -
			m[4] * m[2] * m[11] + m[0] * m[6] * m[11]
		),
		inv_det * (
			m[8] * m[13] * m[7] - m[12] * m[9] * m[7] +
			m[12] * m[5] * m[11] - m[4] * m[13] * m[11] -
			m[8] * m[5] * m[15] + m[4] * m[9] * m[15]
		),
		inv_det * (
			m[12] * m[9] * m[3] - m[8] * m[13] * m[3] -
			m[12] * m[1] * m[11] + m[0] * m[13] * m[11] +
			m[8] * m[1] * m[15] - m[0] * m[9] * m[15]
		),
		inv_det * (
			m[4] * m[13] * m[3] - m[12] * m[5] * m[3] +
			m[12] * m[1] * m[7] - m[0] * m[13] * m[7] -
			m[4] * m[1] * m[15] + m[0] * m[5] * m[15]
		),
		inv_det * (
			m[8] * m[5] * m[3] - m[4] * m[9] * m[3] -
			m[8] * m[1] * m[7] + m[0] * m[9] * m[7] +
			m[4] * m[1] * m[11] - m[0] * m[5] * m[11]
		),
		inv_det * (
			m[12] * m[9] * m[6] - m[8] * m[13] * m[6] -
			m[12] * m[5] * m[10] + m[4] * m[13] * m[10] +
			m[8] * m[5] * m[14] - m[4] * m[9] * m[14]
		),
		inv_det * (
			m[8] * m[13] * m[2] - m[12] * m[9] * m[2] +
			m[12] * m[1] * m[10] - m[0] * m[13] * m[10] -
			m[8] * m[1] * m[14] + m[0] * m[9] * m[14]
		),
		inv_det * (
			m[12] * m[5] * m[2] - m[4] * m[13] * m[2] -
			m[12] * m[1] * m[6] + m[0] * m[13] * m[6] +
			m[4] * m[1] * m[14] - m[0] * m[5] * m[14]
		),
		inv_det * (
			m[4] * m[9] * m[2] - m[8] * m[5] * m[2] +
			m[8] * m[1] * m[6] - m[0] * m[9] * m[6] -
			m[4] * m[1] * m[10] + m[0] * m[5] * m[10]
		)
	];
	// clang-format on
	return im;
}

//
// print the contents of a 4x4 matrix in a readable way
//
function print_mat4(m) {
	console.log("mat4:");
	console.log(m[0].toFixed(2) + " " + m[4].toFixed(2) + " " +
		m[8].toFixed(2) + " " + m[12].toFixed(2));
	console.log(m[1].toFixed(2) + " " + m[5].toFixed(2) + " " +
		m[9].toFixed(2) + " " + m[13].toFixed(2));
	console.log(m[2].toFixed(2) + " " + m[6].toFixed(2) + " " +
		m[10].toFixed(2) + " " + m[14].toFixed(2));
	console.log(m[3].toFixed(2) + " " + m[7].toFixed(2) + " " +
		m[11].toFixed(2) + " " + m[15].toFixed(2));
}

/*---------------------------AFFINE TRANSFORMS-------------------------------*/

// Returns a 4x4 column-major translation matrix.
//   xyz - 3-value array representing a 3D vector. e.g. [0,2,6].
function translate_mat4(xyz) {
	var mat = identity_mat4();
	mat[12] = xyz[0];
	mat[13] = xyz[1];
	mat[14] = xyz[2];
	return mat;
}

// Returns a 4x4 column-major matrix for a rotation about the X-axis.
//   deg - An angle in degrees to rotate about. e.g. -1.3
function rotate_mat4_x_deg(deg) {
	var rad = deg * ONE_DEG_IN_RAD; // Convert to radians.
	var mat = identity_mat4();
	mat[5] = Math.cos(rad);
	mat[9] = -Math.sin(rad);
	mat[6] = Math.sin(rad);
	mat[10] = Math.cos(rad);
	return mat;
}

// Returns a 4x4 column-major matrix for a rotation about the Y-axis.
//   deg - An angle in degrees to rotate about. e.g. -1.3
function rotate_mat4_y_deg(deg) {
	var rad = deg * ONE_DEG_IN_RAD; // Convert to radians.
	var mat = identity_mat4();
	mat[0] = Math.cos(rad);
	mat[8] = Math.sin(rad);
	mat[2] = -Math.sin(rad);
	mat[10] = Math.cos(rad);
	return mat;
}

// Returns a 4x4 column-major matrix for a rotation about the Z-axis.
//   deg - An angle in degrees to rotate about. e.g. -1.3
function rotate_mat4_z_deg(deg) {
	var rad = deg * ONE_DEG_IN_RAD; // Convert to radians.
	var mat = identity_mat4();
	mat[0] = Math.cos(rad);
	mat[4] = -Math.sin(rad);
	mat[1] = Math.sin(rad);
	mat[5] = Math.cos(rad);
	return mat;
}

// Returns a 4x4 matrix for scaling, based on 3D vector xyz.
function scale_mat4(xyz) {
	var mat = identity_mat4();
	mat[0] = xyz[0];
	mat[5] = xyz[1];
	mat[10] = xyz[2];
	return mat;
}

/*-----------------------------VIRTUAL CAMERA--------------------------------*/

//
// Returns a 4x4 column-major virtual camera "view" matrix in gluLookAt() style.
//   cam_pos  - Camera position in world units as a 3D array. e.g. [0.0, 0.0, 0.0] to place at the world origin..
//   targ_pos - Camera focus point in world units as a 3D array. e.g. [0.0, 0.0, -10.0] to gaze down the -Z axis.
//   up       - Camera vertical direction in world coordinates as a 3D array. e.g. [0.0, 1.0, 0.0] for orientation with no roll or pitch.
function look_at(cam_pos, targ_pos, up) {
	var p = translate_mat4([-cam_pos[0], -cam_pos[1], -cam_pos[2]]); // Inverse translation.
	var d = sub_vec3_vec3(targ_pos, cam_pos); // Distance vector.
	var f = normalise_vec3(d); 	// Forward vector.
	var r = normalise_vec3(cross_vec3(f, up)); // Right vector.
	var u = normalise_vec3(cross_vec3(r, f)); // Real up vector.
	var ori = identity_mat4();
	ori[0] = r[0];
	ori[4] = r[1];
	ori[8] = r[2];
	ori[1] = u[0];
	ori[5] = u[1];
	ori[9] = u[2];
	ori[2] = -f[0];
	ori[6] = -f[1];
	ori[10] = -f[2];
	return mul_mat4_mat4(ori, p);
}

// Returns a 4x4 column-major perspective projection matrix.
//   fovy   - Vertical field of view angle in degrees. e.g. 66.6.
//   aspect - Aspect ratio. e.g. viewport width / height.
//   near   - Near clip plane distance. e.g. 0.1.
//   far    - Far clip plane distance. e.g. 1000.0.
function perspective(fovy, aspect, near, far) {
	var fov_rad = fovy * ONE_DEG_IN_RAD;
	var range = Math.tan(fov_rad / 2.0) * near;
	var sx = (2.0 * near) / (range * aspect + range * aspect);
	var sy = near / range;
	var sz = -(far + near) / (far - near);
	var pz = -(2.0 * far * near) / (far - near);
	var m = zero_mat4();
	m[0] = sx;
	m[5] = sy;
	m[10] = sz;
	m[11] = -1.0;
	m[14] = pz;
	return m;
}

/*--------------------------HAMILTON'S QUATERNIONS---------------------------*/

// Returns a versor from an angle in radians and an axis.
function quat_from_axis_rad(radians, x, y, z) {
	var result = new Array();
	result[0] = Math.cos(radians / 2.0);
	result[1] = Math.sin(radians / 2.0) * x;
	result[2] = Math.sin(radians / 2.0) * y;
	result[3] = Math.sin(radians / 2.0) * z;
	return result;
}

// Returns a versor from an angle in degrees and an axis.
function quat_from_axis_deg(degrees, x, y, z) {
	return quat_from_axis_rad(ONE_DEG_IN_RAD * degrees, x, y, z);
}

// Returns a quaternion, produced as a multiplication of two quaterions s, and r.
// e.g. For use in e.g. skeletal animation:
//   var result_quat = mul_quat_quat(parent_orientation_quat, child_orientation_quat).
function mul_quat_quat(s, r) {
	var t = new Array();
	t[0] = r[0] * s[0] - r[1] * s[1] - r[2] * s[2] - r[3] * s[3];
	t[1] = r[0] * s[1] + r[1] * s[0] - r[2] * s[3] + r[3] * s[2];
	t[2] = r[0] * s[2] + r[1] * s[3] + r[2] * s[0] - r[3] * s[1];
	t[3] = r[0] * s[3] - r[1] * s[2] + r[2] * s[1] + r[3] * s[0];
	return t;
}

// Returns a column-major 4x4 matrix form from a versor.
function quat_to_mat4(q) {
	var w = q[0];
	var x = q[1];
	var y = q[2];
	var z = q[3];
	var m = new Array();
	m[0] = 1.0 - 2.0 * y * y - 2.0 * z * z;
	m[1] = 2.0 * x * y + 2.0 * w * z;
	m[2] = 2.0 * x * z - 2.0 * w * y;
	m[3] = 0.0;
	m[4] = 2.0 * x * y - 2.0 * w * z;
	m[5] = 1.0 - 2.0 * x * x - 2.0 * z * z;
	m[6] = 2.0 * y * z + 2.0 * w * x;
	m[7] = 0.0;
	m[8] = 2.0 * x * z + 2.0 * w * y;
	m[9] = 2.0 * y * z - 2.0 * w * x;
	m[10] = 1.0 - 2.0 * x * x - 2.0 * y * y;
	m[11] = 0.0;
	m[12] = 0.0;
	m[13] = 0.0;
	m[14] = 0.0;
	m[15] = 1.0;
	return m;
}

// Returns the dot product of two versors q, and r.
function dot_versor(q, r) {
	return q[0] * r[0] + q[1] * r[1] + q[2] * r[2] + q[3] * r[3];
}

// Spherical linear interpolation between two quaternions.
// Returns an interpolated versor.
//   q - The versor to interpolate from.
//   r - The versor to interpolate to.
//   t - A factor of interpolation between q, and r, of between 0.0 and 1.0.
function slerp(q, r, t) {
	var cos_half_theta = dot_versor(q, r); // Angle between q0-q1.
	// As found here http://stackoverflow.com/questions/2886606/flipping-issue-when-interpolating-rotations-using-quaternions
	// If dot product is negative then one quaternion should be negated, to make it take the short way around, rather than the long way.
	// This requires also recalculating the dot product.
	if (cos_half_theta < 0.0) {
		for (var i = 0; i < 4; i++) { q[i] *= -1.0; }
		cos_half_theta = dot_versor(q, r);
	}
	// If qa=qb or qa=-qb then theta = 0 and we can return qa.
	if (Math.abs(cos_half_theta) >= 1.0) { return q; }
	// Calculate temporary values
	var sin_half_theta = Math.sqrt(1.0 - cos_half_theta * cos_half_theta);
	// If theta == 180 degrees then the result is not fully defined.
	// We could rotate around any axis normal to qa or qb.
	var result = [];
	if (Math.abs(sin_half_theta) < 0.001) {
		for (var i = 0; i < 4; i++) { result[i] = (1.0 - t) * q[i] + t * r[i]; }
		return result;
	}
	var half_theta = Math.acos(cos_half_theta);
	var a = Math.sin((1.0 - t) * half_theta) / sin_half_theta;
	var b = Math.sin(t * half_theta) / sin_half_theta;
	for (var i = 0; i < 4; i++) { result[i] = q[i] * a + r[i] * b; }
	return result;
}

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
