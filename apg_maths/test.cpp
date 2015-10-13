#include "apg_maths.hpp"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main () {
	// memory set matrix internals
	mat4 M;
	print (M);
	M = identity_mat4 ();
	print (M);
	memset (&M, 0, 16 * sizeof (float));
	print (M);
	// TODO (anton): everything else
	return 0;
}
