#include "apg.h"
#include <stdio.h>

int main () {
	uint uval = maxu (110, 120);
	printf ("uval = %u\n", uval);
	uval = clampu (uval, 11, 12);
	printf ("uval = %u\n", uval);
	return 0;
}