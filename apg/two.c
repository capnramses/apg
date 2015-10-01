#include "two.h"
#include "apg.h" // making sure it works when included in two files
#include <stdio.h>

void two () {
	// open a file
	char buff[1024];
	if (!apg_file_to_str ("test_file.txt", 1024, buff)) {
		return;
	}
	printf ("file contents:\n[%s]\n", buff);
}
