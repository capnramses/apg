#include "apg_maths.h"
#include <stdio.h>

int main () {
	printf ("testing C99 version of maths library\n");

	vec2 v2, v2b;
	vec3 v3, v3b;
	vec4 v4, v4b;
	mat4 m4, m4b;
	versor q, qb;

	// constructors and assignment
	printf ("* vec2_from_2f\n");
	v2 = vec2_from_2f (99.0f, 98.0f);
	print_vec2 (v2);
	printf ("* vec2_from_vec2\n");
	v2b = vec2_from_vec2 (v2);
	print_vec2 (v2b);
	printf ("* vec3_from_3f\n");
	v3 = vec3_from_3f (1.0f, 2.0f, 3.0f);
	print_vec3 (v3);
	printf ("* vec3_from_vec3_vec2_f\n");
	v3 = vec3_from_vec2_f (v2, 97.0f);
	print_vec3 (v3);
	printf ("* vec3_from_vec3\n");
	v3b = vec3_from_vec3 (v3);
	print_vec3 (v3b);
	printf ("* vec4_from_4f\n");
	v4 = vec4_from_4f (1.0f, 2.0f, 4.0f, 8.0f);
	print_vec4 (v4);
	printf ("* vec4_from_vec3_f\n");
	v4 = vec4_from_vec3_f (v3, 8.0f);
	print_vec4 (v4);
	printf ("* vec4_from_vec4\n");
	v4b = vec4_from_vec4 (v4);
	print_vec4 (v4b);
	printf ("* zero_mat4\n");
	m4 = zero_mat4 ();
	print_mat4 (m4);
	printf ("* identity_mat4\n");
	m4 = identity_mat4 ();
	print_mat4 (m4);
	printf ("* mat4_from_mat4\n");
	m4b = mat4_from_mat4 (m4);
	print_mat4 (m4b);
	printf ("* versor_from_4f\n");
	q = versor_from_4f (0.1f, 0.2f, 0.3f, 0.4f);
	print_quat (q);
	printf ("* versor_from_quat\n");
	qb = versor_from_versor (q);
	print_quat (qb);

	printf ("* add_vec3_vec3\n");
	v3 = add_vec3_vec3 (v3, v3b);
	print_vec3 (v3);
	printf ("* sub_vec3_vec3\n");
	v3 = sub_vec3_vec3 (v3, v3b);
	print_vec3 (v3);

	//add_vec3_f
	//sub_vec3_f
	//mult_vec3_f
	//div_vec3_f
	//mult_vec3_vec3
	//div_vec3_vec3
	
	//length_vec3
	//length2_vec3

	return 0;
}