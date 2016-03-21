#include "apg_maths_2016.hpp"

int main () {
	printf ("hello maths\n");
	
	// constructors and print functions
	vec2 st = vec2 (0.0f, 1.0f);
	print (st);
	vec2 uv = vec2 (st);
	print (st);
	vec3 xyz = vec3 (0.0f, 1.0f, 2.0f);
	print (xyz);
	vec2 wx = vec2(xyz);
	print (wx);
	vec4 xyzw = vec4 (0.0f, 1.0f, 2.0f, 3.0f);
	print (xyzw);
	vec2 yz = vec2 (xyzw);
	print (yz);
	vec3 rst = vec3 (st, 1.0);
	print (rst);
	xyz = vec3 (rst);
	print (xyz);
	xyz = vec3 (xyzw);
	print (xyz);
	// overloaded arithmetic
	xyz = vec3 (1.0f, 1.0f, 1.0f) + vec3 (2.0f, 2.0f, 2.0f);
	print (xyz);
	xyz += vec3 (1.0f, 1.0f, 1.0f);
	print (xyz);
	xyz = vec3 (1.0f, 1.0f, 1.0f) - vec3 (2.0f, 2.0f, 2.0f);
	print (xyz);
	xyz -= vec3 (1.0f, 1.0f, 1.0f);
	print (xyz);
	xyz = vec3 (1.0f, 1.0f, 1.0f) / vec3 (2.0f, 2.0f, 2.0f);
	print (xyz);
	xyz /= vec3 (2.0f, 2.0f, 2.0f);
	print (xyz);
	xyz = xyz + 1.0f;
	print (xyz);
	xyz += 1.0f;
	print (xyz);
	xyz = xyz - 1.0f;
	print (xyz);
	xyz -= 1.0f;
	print (xyz);
	xyz = xyz * 2.0f;
	print (xyz);
	xyz *= 2.0f;
	print (xyz);
	xyz = xyz / 2.0f;
	print (xyz);
	xyz /= 2.0f;
	print (xyz);
	// and for vec4
	xyzw = vec4 (st, 1.0f, 2.0f);
	print (xyzw);
	xyzw = vec4 (xyz, 2.0f);
	print (xyzw);
	// and mat3
	mat3 N = identity_mat3 ();
	print (N);
	float e = N.get_el (0, 0);
	printf ("e = %f\n", e);
	N.set_el (2, 0, 2.0f);
	print (N);
	N = zero_mat3 ();
	print (N);
	// and mat4
	mat4 T = identity_mat4 ();
	print (T);
	e = T.get_el (3, 3);
	printf ("e = %f\n", e);
	T.set_el (3, 0, 2.0f);
	print (T);
	T = identity_mat4 ();
	print (T);
	T.set_el (2, 0, 2.0f);
	print (T);
	mat4 R = identity_mat4 ();
	R.set_el (0, 3, 3.0f);
	print (R);
	T = T * R;
	print (T);
	xyzw = T * vec4 (1.0f, 1.0f, 1.0f, 1.0f);
	print (xyzw);
	T = R;
	print (T);
	// and versor
	versor q;
	q.w = 0.0f;
	q.x = 1.0f;
	q.y = 2.0f;
	q.z = 3.0f;
	print (q);
	q = q / 2.0f;
	print (q);
	q = q * 2.0f;
	print (q);
	q = q * q;
	print (q);
	q = q + q;
	print (q);
	// vector functions
	float l = length (vec2 (1,1));
	float l2 = length2 (vec2 (1, 1));
	printf ("l = %f\n", l);
	printf ("l2 = %f\n", l2);
	vec2 nn = normalise (vec2 (2.0, 2.0));
	print (nn);
	l = length (vec3 (1,1,1));
	l2 = length2 (vec3 (1,1,1));
	printf ("l = %f\n", l);
	printf ("l2 = %f\n", l2);
	vec3 nnn = normalise (vec3 (2,2,2));
	print (nnn);
	float dp = dot (vec3 (0,0,1), normalise (vec3 (0.25,0,-0.75)));
	printf ("dp = %f\n", dp);
	vec3 cp = cross (vec3 (1,0,0),vec3 (0,1,0));
	print (cp);
	float det = determinant (identity_mat4 ());
	printf ("det = %f\n", det);
	T = translate (vec3 (1,-1,0));
	print (T);
	mat4 TI = inverse (T);
	print (TI);
	mat4 TT = transpose (TI);
	print (TT);
	R = rotate_x_deg (10.0f);
	print (R);
	R = rotate_y_deg (10.0f);
	print (R);
	R = rotate_z_deg (10.0f);
	print (R);
	mat4 S = scale (vec3 (10.0,10.0,10.0));
	print (S);
	R = S * R;
	print (R);
	mat4 V = look_at (vec3(0,10,0),vec3(0,0,0),vec3(0,0,-1));
	print (V);
	mat4 P = perspective (67.0, 800.0 / 600.0, 0.1, 100.0);
	print (P);
	// quaternions
	q = quat_from_axis_rad (2.00, vec3(0,1,0));
	print (q);
	q = quat_from_axis_deg (90, vec3(0,1,0));
	print (q);
	R = quat_to_mat4 (q);
	print (R);
	q.x = 10.0;
	print (q);
	q = normalise (q);
	print (q);
	// dot looks fine - no need to test
	versor qa = quat_from_axis_deg (0, vec3(0,1,0));
	versor qb = quat_from_axis_deg (90, vec3(0,1,0));
	versor qe = quat_from_axis_deg (45, vec3(0,1,0));
	versor qc = slerp (qa, qb, 0.5f);
	print (qa); // first
	print (qb); // second
	print (qc); // actual result
	print (qe); // expected results
	return 0;
}
