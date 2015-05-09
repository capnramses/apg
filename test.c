#include "apg.h"
#include <stdio.h>

int g_argc;
char** g_argv;

int main (int argc, char** argv) {
	g_argc = argc;
	g_argv = argv;

	restart_apg_log ();

	int pn = check_param ("hello");
	printf ("the user param 'hello' was found at arg num %i\n", pn);

	uint uval = MAX (110, 120);
	printf ("uval = %u\n", uval);
	apg_log ("the value was %u\n", uval);

	uval = CLAMP (uval, 11, 12);
	printf ("uval = %u\n", uval);

	apg_log_err ("ERROR: this is a fake error\n");

	return 0;
}
