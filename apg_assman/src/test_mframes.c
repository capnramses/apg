#include "apg_mframes.h"
#include <stdio.h>

int main () {
	printf ("allocating frame size 1024\n");
	apg_open_frame (1024);
	apg_open_frame (256);
	apg_open_frame (64);
	apg_print_stack_stats ();
	apg_close_frame ();
	apg_print_stack_stats ();
	return 0;
}
