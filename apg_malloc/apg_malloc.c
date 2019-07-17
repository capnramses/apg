//
// apg_malloc wrapper for malloc() and free() to provide some accounting and
// leak checks
// it's basically a re-implementation / learning experiment remaking
// the stb_memleak library
// Written by Dr Anton Gerdelan, Trinity College Dublin, Ireland
// gerdela AT scss DOT tcd DOT ie
// First v 14 April 2015
// Public Domain - no warrenty implied; use at your own risk.
//

//#include "apg_malloc.h" // don't include this or our own malloc() will go bad
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>

//
// list nodes to account for each malloc
typedef struct APG_Mem_Block APG_Mem_Block;
struct APG_Mem_Block {
  // address in main memory
  void* addr;
  // size of memory allocation in bytes
  size_t sz;
  // source code file name of calling malloc()
  char file[32];
  // line in above file
  int line;
  // next accounting block in list
  APG_Mem_Block* next;
  APG_Mem_Block* prev;
};

//
// head of list of accounting nodes
APG_Mem_Block* apg_blocks_ptr;
unsigned long int apg_total_allocs_history;
//
// total memory allocated through this wrapper
// (not including accounting nodes)
size_t g_apg_total_mallocd;

void* apg_malloc( size_t sz, const char* file, int line ) {
  void* ptr = malloc( sz );
  if ( ptr ) {
    // accounting total mallocd
    g_apg_total_mallocd += sz;
    // leak checking data structure
    APG_Mem_Block* block_ptr = (APG_Mem_Block*)malloc( sizeof( APG_Mem_Block ) );
    block_ptr->addr          = ptr;
    block_ptr->sz            = sz;
    strncpy( block_ptr->file, file, 32 );
    block_ptr->line = line;
    block_ptr->next = apg_blocks_ptr;
    apg_blocks_ptr  = block_ptr;
    // printf ("malloc addr %p %i bytes at %s:%i\n", ptr, (int)sz, file, line);
    apg_total_allocs_history++;
  } else {
    fprintf( stderr, "ERROR: could not malloc OOM\n" );
  }
  return ptr;
}

void* apg_calloc( size_t num, size_t sz, const char* file, int line ) {
  void* ptr = calloc( num, sz );
  if ( ptr ) {
    // accounting total mallocd
    g_apg_total_mallocd += ( num * sz );
    // leak checking data structure
    APG_Mem_Block* block_ptr = (APG_Mem_Block*)malloc( sizeof( APG_Mem_Block ) );
    block_ptr->addr          = ptr;
    block_ptr->sz            = ( num * sz );
    strncpy( block_ptr->file, file, 32 );
    block_ptr->line = line;
    block_ptr->next = apg_blocks_ptr;
    apg_blocks_ptr  = block_ptr;
    // printf ("malloc addr %p %i bytes at %s:%i\n", ptr, (int)sz, file, line);
    apg_total_allocs_history++;
  } else {
    fprintf( stderr, "ERROR: could not malloc OOM\n" );
  }
  return ptr;
}

void* apg_realloc( void* ptr, size_t sz, const char* file, int line ) {
  void* rptr = realloc( ptr, sz );
  if ( rptr ) {
    // change old block
    if ( ptr ) {
      // find matching accounting node in list
      // NOTE: this is slow
      APG_Mem_Block* block_ptr = apg_blocks_ptr;
      bool found               = false;
      while ( block_ptr ) {
        if ( block_ptr->addr == ptr ) {
          found = true;
          g_apg_total_mallocd -= block_ptr->sz;
          block_ptr->sz = sz;
          g_apg_total_mallocd += sz;
          break;
        }
        block_ptr = block_ptr->next;
      } // endwhile
      if ( !found ) {
        fprintf( stderr,
          "WARNING: apg_realloc did not find addr %p in list. "
          "from %s:%i\n",
          ptr, file, line );
      }
      // entirely new block
    } else {
      // accounting total mallocd
      g_apg_total_mallocd += sz;
      // leak checking data structure
      APG_Mem_Block* block_ptr = (APG_Mem_Block*)malloc( sizeof( APG_Mem_Block ) );
      block_ptr->addr          = rptr;
      block_ptr->sz            = sz;
      strncpy( block_ptr->file, file, 32 );
      block_ptr->line = line;
      block_ptr->next = apg_blocks_ptr;
      apg_blocks_ptr  = block_ptr;
      // printf ("malloc addr %p %i bytes at %s:%i\n", ptr, (int)sz, file, line);
      apg_total_allocs_history++;
    }
  } else {
    fprintf( stderr, "ERROR: could not malloc OOM\n" );
  }
  return rptr;
}

void apg_free( void* ptr, const char* file, int line ) {
  if ( !ptr ) {
    fprintf( stderr, "WARNING: trying to free() null ptr at %s:%i\n", file, line );
  } else {
    // find matching accounting node in list
    // NOTE: this is slow
    APG_Mem_Block* block_ptr = apg_blocks_ptr;
    APG_Mem_Block* prev_ptr  = NULL;
    bool found               = false;
    while ( block_ptr ) {
      if ( block_ptr->addr == ptr ) {
        found = true;
        g_apg_total_mallocd -= block_ptr->sz;
        // printf ("free addr %p %i bytes at %s:%i\n", ptr, (int)block_ptr->sz,
        //	file, line);
        // remove from list
        if ( prev_ptr ) {
          prev_ptr->next = block_ptr->next;
        } else {
          apg_blocks_ptr = block_ptr->next;
        }
        free( block_ptr );
        block_ptr = NULL;
        break;
      }
      prev_ptr  = block_ptr;
      block_ptr = block_ptr->next;
    } // endwhile
    if ( !found ) {
      fprintf( stderr,
        "WARNING: apg_free did not find addr %p in list. from "
        "%s:%i\n",
        ptr, file, line );
    }
    free( ptr );
  } // endif(!ptr)
}

void apg_mem_report() {
  fprintf( stderr, "total allocs in lifetime: %lu\n", apg_total_allocs_history );
  fprintf( stderr, "counter reports %i bytes still allocated\n", (int)g_apg_total_mallocd );
  APG_Mem_Block* block_ptr = apg_blocks_ptr;
  while ( block_ptr ) {
    fprintf( stderr, "LEAK: addr: %p sz %i. alloc at %s:%i\n", block_ptr->addr, (int)block_ptr->sz, block_ptr->file, block_ptr->line );

    block_ptr = block_ptr->next;
  }
}
