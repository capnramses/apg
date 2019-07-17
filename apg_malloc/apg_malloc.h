//
// apg_malloc wrapper for malloc() and free() to provide some accounting and
// leak checks
// Written by Dr Anton Gerdelan, Trinity College Dublin, Ireland
// gerdela AT scss DOT tcd DOT ie
// First v 14 April 2015
// Public Domain - no warrenty implied; use at your own risk.
//

#ifndef _APG_MALLOC_H_
#define _APG_MALLOC_H_
// only compiles in if DEBUG_MODE is defined
#ifdef DEBUG_MODE
#include <stdlib.h>

void* apg_malloc( size_t sz, const char* file, int line );
void* apg_calloc( size_t num, size_t sz, const char* file, int line );
void* apg_realloc( void* ptr, size_t sz, const char* file, int line );
void apg_free( void* ptr, const char* file, int line );
void apg_mem_report();

//
// wrap malloc and free
#define malloc( sz ) apg_malloc( sz, __FILE__, __LINE__ )
#define calloc( num, sz ) apg_calloc( num, sz, __FILE__, __LINE__ )
#define realloc( ptr, sz ) apg_realloc( ptr, sz, __FILE__, __LINE__ )
#define free( ptr ) apg_free( ptr, __FILE__, __LINE__ )

#endif // DEBUG_MODE
#endif // HEADER
