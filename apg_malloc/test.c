#define DEBUG_MODE
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "apg_malloc.h"

int main () {
	printf ("#leak test#\n");

	printf ("\n##malloc##\n");
	char* ptr_a = (char*)malloc (strlen ("hello") + 1);
	strcpy (ptr_a, "hello");
	char* ptr_b = (char*)malloc (strlen ("world") + 1);
	strcpy (ptr_b, "world");
	char* ptr_c = (char*)malloc (strlen ("!") + 1);
	strcpy (ptr_c, "!");

	printf ("\n##contents of memory##\n");
	printf ("[%s][%s][%s]\n", ptr_a, ptr_b, ptr_c);
	
	printf ("\n##report##\n");
	apg_mem_report ();

	printf ("\n##free##\n");
	free (ptr_a);
	ptr_a = NULL;
	free (ptr_b);
	ptr_b = NULL;
	// deliberately did not free ptr_c

	printf ("\n##report##\n");
	apg_mem_report ();
	
	printf ("\n##contents of memory##\n");
	printf ("[%s][%s][%s]\n", ptr_a, ptr_b, ptr_c);

	return 0;
}
