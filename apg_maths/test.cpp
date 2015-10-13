#include "apg_maths.hpp"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main () {
	// print functions
	vec2 xy = vec2 (0.0f, 1.0f);
	print (xy);
	vec3 xyz = vec3 (0.0f, 1.0f, 2.0f);
	print (xyz);
	vec4 xyzw = vec4 (0.0f, 1.0f, 2.0f, 3.0f);
	print (xyzw);
	mat3 Mxyz= identity_mat3 ();
	print (Mxyz);
	// and matrix memory test
	mat4 Mxyzw;
	print (Mxyzw);
	Mxyzw = identity_mat4 ();
	print (Mxyzw);
	memset (&Mxyzw, 0, 16 * sizeof (float));
	print (Mxyzw);
	versor q = quat_from_axis_rad (0.0f, 0.0f, 1.0f, 0.0f);
	print (q);
/*	
float length (const vec3& v);
float length2 (const vec3& v);
vec3 normalise (const vec3& v);
float dot (const vec3& a, const vec3& b);
inline vec3 cross (const vec3& a, const vec3& b);
inline float get_squared_dist (vec3 from, vec3 to);
float direction_to_heading (vec3 d);
vec3 heading_to_direction (float degrees);
*/

	// memory set matrix internals
	// TODO (anton): everything else
	return 0;
}
