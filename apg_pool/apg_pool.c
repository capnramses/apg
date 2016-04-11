//
// c99 memory manager
// first v. anton gerdelan 21 march 2016
// WARNING: not tested!
//

#include "apg_pool.h"
#include <string.h>

bool allocate_pool( Mem_Pool* mp, size_t sz ){
	if( mp->mem ){ if( !release_pool( mp ) ){ return false; } }
	mp->mem = ( unsigned char* )calloc( 1, sz );
	if( !mp->mem ){ fprintf( stderr, "ERROR: OOM\n" ); return false; }
	mp->sz = sz;
	return true;
}

bool release_pool( Mem_Pool* mp ){
	if( !mp->mem ){ fprintf( stderr, "ERROR: not allocd\n" ); return false; }
	free( mp->mem );
	memset( mp, 0, sizeof( Mem_Pool ) );
	return true;
}

size_t get_file_sz( FILE* fptr ){
	if( !fptr ){ fprintf( stderr, "ERROR: fptr invalid\n" ); return 0; }
	fseek( fptr, 0, SEEK_END );
	size_t sz = ftell( fptr );
	rewind( fptr );
	return sz;
}

int malloc_in_pool( Mem_Pool* mp, size_t sz ){
	if( mp->entry_count >= MAX_POOL_ENTRIES ) {
		fprintf( stderr, "ERROR: pool at entry cap\n" );
		return -1;
	}
	if( ( mp->used + sz ) >= mp->sz ){
		fprintf( stderr, "ERROR: pool at sz cap\n" );
		return -1;
	}
	int entry_idx = mp->entry_count;
	mp->entry_addr[ entry_idx ] = &mp->mem[ mp->used ];
	mp->entry_sz[ entry_idx ] = sz;
	mp->used += sz;
	mp->entry_count++;
	return entry_idx;
}

int calloc_in_pool( Mem_Pool* mp, size_t sz ){
	int entry_idx = malloc_in_pool( mp, sz );
	if( entry_idx < 0 ) { return entry_idx; }
	void* addr = mp->entry_addr[ entry_idx ];
	memset( addr, 0, sz );
	return entry_idx;
}

int load_file_to_pool( Mem_Pool* mp, const char* fn ){
	FILE* fptr = fopen( fn, "rb" );
	if( !fptr ) {
		fprintf( stderr, "ERROR: opening file %s\n", fn );
		return - 1;
	}
	size_t sz = get_file_sz( fptr );
	int entry_idx = malloc_in_pool ( mp, sz );
	if (entry_idx < 0){ return entry_idx; }
	void* addr = mp->entry_addr[ entry_idx ];
	fread( addr, sz, 1, fptr );
	fclose( fptr );
	return entry_idx;
}
