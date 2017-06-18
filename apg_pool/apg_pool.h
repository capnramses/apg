//
// c99 memory manager
// first v. anton gerdelan 21 march 2016
// Public Domain - no warrenty implied; use at your own risk.
// WARNING: not tested!
//
// TODO
// * bounds checking functions that only build into debug builds
// * leak checking functions
//

#pragma once
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

#define MAX_POOL_ENTRIES 1024

struct Mem_Pool {
	unsigned char* mem;
	size_t sz;
	size_t used;
	void* entry_addr[MAX_POOL_ENTRIES];
	size_t entry_sz[MAX_POOL_ENTRIES];
	int entry_count;
};
typedef struct Mem_Pool Mem_Pool;

// returns false if out of mem
bool allocate_pool( Mem_Pool* mp, size_t sz );

// returns false if mem not allocated
bool release_pool( Mem_Pool* mp );

// note: does not open or close file stream
size_t get_file_sz( FILE* fp );

int malloc_in_pool( Mem_Pool* mp, size_t sz );

int calloc_in_pool( Mem_Pool* mp, size_t sz );

// returns index in pool or -1 on err
int load_file_to_pool( Mem_Pool* mp, const char* fn );
