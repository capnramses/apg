//
// apg_mframes.c - frame-based memory allocator
// first v. 2 sept 2015 - anton gerdelan
// Public Domain - no warrenty implied; use at your own risk.
// written for C99
//

#include "apg_mframes.h"
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

typedef struct APG_Frame APG_Frame;
struct APG_Frame {
	void* data;
	size_t sz;
};

// stack of frames
APG_Frame g_frames[128];
int g_frame_count;

// alloc new frame onto stack
bool apg_open_frame (size_t sz) {
	assert (g_frame_count < 128);
	assert (sz > 0);
	assert (!g_frames[g_frame_count].data);
	g_frames[g_frame_count].data = (void*)malloc (sz);
	assert (g_frames[g_frame_count].data);
	g_frames[g_frame_count].sz = sz;
	g_frame_count++;
	return true;
}

bool apg_close_frame () {
	assert (g_frame_count > 0);
	int n = g_frame_count - 1;
	assert (g_frames[n].data);
	free (g_frames[n].data);
	g_frames[n].sz = 0;
	g_frame_count--;
	return true;
}

void apg_print_stack_stats () {
	size_t tot_sz = 0;
	for (int i = 0; i < g_frame_count; i++) {
		tot_sz += g_frames[i].sz;
		printf ("frame %i: %lu bytes\n", i, g_frames[i].sz);
	}
	printf ("total sz: %lu bytes\n", tot_sz);
}
