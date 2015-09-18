//
// apg_assman
// first v. 20 May 2015. Anton Gerdelan, Trinity College Dublin, Ireland
// <gerdela at scss dot tcd dot ie>
//

#include <stdio.h>
#include <assert.h>
#include <stdbool.h>
#include <fcntl.h> // open()
#include <unistd.h> // close()
#include <stdlib.h> // malloc
#include <string.h> // memset
#include <aio.h> // link with -lrt
#include <errno.h> // EINPROGRESS
#include <stdint.h>
#include <sys/stat.h> // stat

typedef uint8_t uint8; // most useful one for byte in memory

// an array of initiated reads
struct aiocb read_cbs[2];
bool read_states[2];
int read_cb_count;

off_t fsize(const char *filename) {
    struct stat st; 

    if (stat(filename, &st) == 0)
        return st.st_size;

    return -1; 
}

// open and read a file, return index number in read_cbs[]
int start_reading (const char* filename, uint8** buffer) {
	printf ("opening file %s...\n", filename);

	// get file descriptor/size
	int sz = fsize (filename);//1024*1024;
	int fd = open (filename, O_RDONLY);
	assert (fd >= 0);

	// create the control block structure
	memset (&read_cbs[read_cb_count], 0, sizeof (struct aiocb));
	*buffer = (uint8*)malloc (sz);
	read_cbs[read_cb_count].aio_buf = *buffer;
	assert (read_cbs[read_cb_count].aio_buf);
	read_cbs[read_cb_count].aio_fildes = fd;
  read_cbs[read_cb_count].aio_nbytes = sz;
  read_cbs[read_cb_count].aio_offset = 0;

  // start asynch read
  read_states[read_cb_count] = false;
  int ret = aio_read (&read_cbs[read_cb_count]);
  assert (ret >= 0);

  read_cb_count++;
  return read_cb_count - 1;
}

// check if all the reads are done
bool check_reads () {
	bool done = true;
	for (int i = 0; i < read_cb_count; i++) {
		if (!read_states[i]) {
			if (aio_error (&read_cbs[i]) != EINPROGRESS) {
				read_states[i] = true;
				printf ("asynch file read %i done\n", i);
				int ret;
				if ((ret = aio_return (&read_cbs[i])) > 0) {
			    /* got ret bytes on the read */
			    printf ("file loaded %i bytes\n", ret);
			  } else {
			    /* read failed, consult errno */
			    fprintf (stderr, "ERROR loading file\n");
			  }
			  close (read_cbs[i].aio_fildes);
			} else {
				done = false; // could return false early
			}
		}
	} // endfor
	return done;
}

int main () {
	printf ("apg_assman\n");

	uint8 *buff_a, *buff_b;
  int a = start_reading ("test1.txt", &buff_a);
  int b = start_reading ("test2.txt", &buff_b);

  while (!check_reads ()) {
  	printf (".");
  }

  for (int i = 0; i < 5; i++) {
  	printf ("\nbuff_a[%i]=%c", i, buff_a[i]);
	}
	for (int i = 0; i < 5; i++) {
  	printf ("\nbuff_b[%i]=%c", i, buff_b[i]);
	}
	free (buff_a);
  free (buff_b);

	return 0;
}
