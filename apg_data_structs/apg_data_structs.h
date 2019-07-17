// apg_data_structures: light-weight C99 data structures
// Licence: see bottom of file.
// ChangeLog:     0.1    13 April 2015    First v. Lists, simple hash.
//                0.11   21 March 2017    Refactored, .clang-format etc.


#pragma once

#include <assert.h>
#include <limits.h> // UINT_MAX
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// size in bytes of all memory so far allocated with this library
size_t g_apg_ds_mem_allocd;
// size in butes of memory in all linked list nodes
size_t g_apg_ll_mem_allocd;

/////////////////////////////// Linked Lists //////////////////////////////////
// Indexing:                   O(n) vs. array which is O(1)
// Insert/delete at beginning: O(1) -- best!
// Insert/delete at end:       O(n) or O(1) if we keep an end ptr
// Insert/delete in middle:    O(1) + search time (shorter for doubly linked)
// Wasted space (avg):         O(n) vs. array which is O(0)

// singly-linked list node, size 24 bytes
typedef struct llist_node_t llist_node_t;
struct llist_node_t {
  void* data;
  size_t sz;
  llist_node_t* next;
};

// doubly-linked list node, size 32 bytes
typedef struct dllist_node_t dllist_node_t;
struct dllist_node_t {
  void* data;
  size_t sz;
  dllist_node_t *next, *prev;
};

// binary tree node
typedef struct binarytree_node_t binarytree_node_t;
struct binarytree_node_t {
  void* data;
  size_t sz;
  binarytree_node_t *left, *right;
};

// quad tree node
typedef struct quadtree_node_t quadtree_node_t;
struct quadtree_node_t {
  void* data;
  size_t sz;
  quadtree_node_t* children[4];
};

///////////////////////////////// accounting //////////////////////////////////
void print_mem_allocd();
//////////////////////////// singly linked lists //////////////////////////////
llist_node_t* llist_add_to_front( llist_node_t** list_ptr, const void* data, size_t sz );
llist_node_t* llist_insert_after( llist_node_t* prev_ptr, const void* data, size_t sz );
bool llist_delete_node( llist_node_t** list_ptr, llist_node_t* ptr );
llist_node_t* llist_find_end_node( llist_node_t* list_ptr );
bool llist_recursive_delete( llist_node_t** ptr );
//////////////////////////// doubly linked lists //////////////////////////////
dllist_node_t* dllist_add_to_front( dllist_node_t** list_ptr, const void* data, size_t sz );
dllist_node_t* dllist_insert_after( dllist_node_t* prev_ptr, const void* data, size_t sz );
bool dllist_delete_node( dllist_node_t** list_ptr, dllist_node_t* ptr );
dllist_node_t* dllist_find_end_node( dllist_node_t* list_ptr );
bool dllist_recursive_delete( dllist_node_t** ptr );
//////////////////////////////// hash tables //////////////////////////////////
unsigned int hash_index( const char* str, int k );

//
// prints memory stats
//
void print_mem_allocd() {
  printf( "memory allocated in apg_data_structures:\n" );
  printf( "g_apg_ds_mem_allocd: %16i bytes\n", (int)g_apg_ds_mem_allocd );
  printf( "  g_apg_ll_mem_allocd: %13i bytes\n", (int)g_apg_ds_mem_allocd );
}

// add a node to the front of a singly-linked list
// list_ptr is the pointer to the front of the list or null
// data is the contents to hold in the node
// sz is the size of that data in bytes
// returns ptr to new node or NULL on error
// note: data pointer is not freed by this function
inline llist_node_t* llist_add_to_front( llist_node_t** list_ptr, const void* data, size_t sz ) {
  llist_node_t* node = (llist_node_t*)malloc( sizeof( llist_node_t ) );
  if ( !node ) {
    fprintf( stderr, "ERROR: could not alloc memory for ssl node struct\n" );
    return NULL;
  }
  g_apg_ll_mem_allocd += sizeof( llist_node_t );
  node->next = *list_ptr;
  node->data = malloc( sz );
  g_apg_ll_mem_allocd += sz;
  node->sz            = sz;
  node->data          = memcpy( node->data, data, sz );
  *list_ptr           = node;
  g_apg_ds_mem_allocd = g_apg_ds_mem_allocd + sizeof( llist_node_t ) + sz;
  return node;
}

//
// add a node after another node of a singly-linked list
// prev_ptr is the pointer to the node to go after
// data is the contents to hold in the node
// sz is the size of that data in bytes
// returns ptr to new node or NULL on error
//
inline llist_node_t* llist_insert_after( llist_node_t* prev_ptr, const void* data, size_t sz ) {
  // this is far more likely to be a user mistake than anything - should warn
  if ( !prev_ptr ) {
    fprintf( stderr, "ERROR: could not insert llist node, prev_ptr was NULL\n" );
    return NULL;
  }
  llist_node_t* node = (llist_node_t*)malloc( sizeof( llist_node_t ) );
  if ( !node ) {
    fprintf( stderr, "ERROR: could not alloc memory for llist node struct\n" );
    return NULL;
  }
  g_apg_ll_mem_allocd += sizeof( llist_node_t );
  node->next = prev_ptr->next;
  node->data = malloc( sz );
  g_apg_ll_mem_allocd += sz;
  node->sz            = sz;
  node->data          = memcpy( node->data, data, sz );
  prev_ptr->next      = node;
  g_apg_ds_mem_allocd = g_apg_ds_mem_allocd + sizeof( llist_node_t ) + sz;
  return node;
}

//
// delete node pointed to by ptr
// function searches list from list_ptr to ptr to find prev node
// prev->next will then point to ptr->next
// ptr is freed and then set to NULL
// list_ptr is a pointer to the start of the list (or any node prior to ptr)
// ptr is the node to delete
// returns false on error
//
inline bool llist_delete_node( llist_node_t** list_ptr, llist_node_t* ptr ) {
  if ( !*list_ptr ) {
    fprintf( stderr, "ERROR: can not delete llist node, list_ptr is NULL\n" );
    return false;
  }
  if ( !ptr ) {
    fprintf( stderr, "ERROR: can not delete llist node, ptr is NULL\n" );
    return false;
  }
  size_t sz = ptr->sz;

  // find prev node to ptr so can adjust
  llist_node_t* p = *list_ptr;
  while ( p ) {
    // p is first node in list, so adjust list ptr
    if ( p == ptr ) {
      *list_ptr = ptr->next;
      break;
    }
    // make prev->next equal to ptr->next
    if ( p->next == ptr ) {
      p->next = ptr->next;
      break;
    }
    p = p->next;
  } // endwhile

  free( ptr->data );
  ptr->data = NULL; // pointless, not used again
  free( ptr );
  ptr                 = NULL; // pointless, not used again
  g_apg_ll_mem_allocd = g_apg_ll_mem_allocd - sz - sizeof( llist_node_t );
  g_apg_ds_mem_allocd = g_apg_ds_mem_allocd - sz - sizeof( llist_node_t );

  return true;
}

//
// list_ptr is any node in the list - the farther along, the shorter the
// search
// returns NULL if list is empty
//
inline llist_node_t* llist_find_end_node( llist_node_t* list_ptr ) {
  if ( !list_ptr ) { return NULL; }
  llist_node_t* p = list_ptr;
  while ( p ) {
    if ( !p->next ) { break; }
    p = p->next;
  }
  return p;
}

// recursively deletes and entire list, starting from ptr
// sets ptr to NULL afterwards
// returns false on error
// note: figured there was no point inlining this
bool llist_recursive_delete( llist_node_t** ptr ) {
  if ( !*ptr ) {
    fprintf( stderr, "ERROR: could not recursive delete llist, node was NULL\n" );
    return false;
  }
  if ( ( *ptr )->next ) { llist_recursive_delete( &( *ptr )->next ); }
  size_t sz = ( *ptr )->sz;
  free( ( *ptr )->data );
  ( *ptr )->data = NULL; // pointless, not used again
  free( *ptr );
  *ptr                = NULL; // pointless, not used again
  g_apg_ll_mem_allocd = g_apg_ll_mem_allocd - sz - sizeof( llist_node_t );
  g_apg_ds_mem_allocd = g_apg_ds_mem_allocd - sz - sizeof( llist_node_t );
  return true;
}

///////////////////////////////////////////////////////////////////////////////

inline dllist_node_t* dllist_add_to_front( dllist_node_t** list_ptr, const void* data, size_t sz ) {
  dllist_node_t* node = (dllist_node_t*)malloc( sizeof( dllist_node_t ) );
  if ( !node ) {
    fprintf( stderr, "ERROR: could not alloc memory for dll node struct\n" );
    return NULL;
  }
  g_apg_ll_mem_allocd += sizeof( dllist_node_t );
  node->next = *list_ptr;
  if ( node->next ) { node->next->prev = node; }
  node->prev = NULL;
  node->data = malloc( sz );
  g_apg_ll_mem_allocd += sz;
  node->sz            = sz;
  node->data          = memcpy( node->data, data, sz );
  *list_ptr           = node;
  g_apg_ds_mem_allocd = g_apg_ds_mem_allocd + sizeof( dllist_node_t ) + sz;
  return node;
}

inline dllist_node_t* dllist_insert_after( dllist_node_t* prev_ptr, const void* data, size_t sz ) {
  // this is far more likely to be a user mistake than anything - should warn
  if ( !prev_ptr ) {
    fprintf( stderr, "ERROR: could not insert dll node, prev_ptr was NULL\n" );
    return NULL;
  }
  dllist_node_t* node = (dllist_node_t*)malloc( sizeof( dllist_node_t ) );
  if ( !node ) {
    fprintf( stderr, "ERROR: could not alloc memory for dll node struct\n" );
    return NULL;
  }
  g_apg_ll_mem_allocd += sizeof( dllist_node_t );
  dllist_node_t* next = prev_ptr->next;
  prev_ptr->next      = node;
  node->next          = next;
  if ( next ) { next->prev = node; }
  node->prev = prev_ptr;
  node->data = malloc( sz );
  g_apg_ll_mem_allocd += sz;
  node->sz            = sz;
  node->data          = memcpy( node->data, data, sz );
  g_apg_ds_mem_allocd = g_apg_ds_mem_allocd + sizeof( dllist_node_t ) + sz;
  return node;
}

inline bool dllist_delete_node( dllist_node_t** list_ptr, dllist_node_t* ptr ) {
  if ( !*list_ptr ) {
    fprintf( stderr, "ERROR: can not delete dll node, list_ptr is NULL\n" );
    return false;
  }
  if ( !ptr ) {
    fprintf( stderr, "ERROR: can not delete dll node, ptr is NULL\n" );
    return false;
  }
  size_t sz = ptr->sz;

  // p is first node in list, so adjust list ptr
  if ( !ptr->prev ) {
    *list_ptr = ptr->next;
    if ( ptr->next ) { ptr->next->prev = NULL; }
  } else {
    ptr->prev->next = ptr->next;
    // this only applies if not deleting last node
    if ( ptr->next ) { ptr->next->prev = ptr->prev; }
  }
  free( ptr->data );
  ptr->data = NULL; // pointless, not used again
  free( ptr );
  ptr                 = NULL; // pointless, not used again
  g_apg_ll_mem_allocd = g_apg_ll_mem_allocd - sz - sizeof( dllist_node_t );
  g_apg_ds_mem_allocd = g_apg_ds_mem_allocd - sz - sizeof( dllist_node_t );

  return true;
}

inline dllist_node_t* dllist_find_end_node( dllist_node_t* list_ptr ) {
  if ( !list_ptr ) { return NULL; }
  dllist_node_t* p = list_ptr;
  while ( p ) {
    if ( !p->next ) { break; }
    p = p->next;
  }
  return p;
}

bool dllist_recursive_delete( dllist_node_t** ptr ) {
  if ( !*ptr ) {
    fprintf( stderr, "ERROR: could not recursive delete dll, node was NULL\n" );
    return false;
  }
  if ( ( *ptr )->next ) { dllist_recursive_delete( &( *ptr )->next ); }
  size_t sz = ( *ptr )->sz;
  free( ( *ptr )->data );
  ( *ptr )->data = NULL; // pointless, not used again
  free( *ptr );
  *ptr                = NULL; // pointless, not used again
  g_apg_ll_mem_allocd = g_apg_ll_mem_allocd - sz - sizeof( dllist_node_t );
  g_apg_ds_mem_allocd = g_apg_ds_mem_allocd - sz - sizeof( dllist_node_t );
  return true;
}

///////////////////////////////////////////////////////////////////////////////

#if 0 // placeholder -- replace this with a good impl
// k is size of hash table
inline unsigned int hash_index( const char *str, int k ) {
  unsigned int len = strlen( str );
  unsigned int sum = 0;
  unsigned int p = UINT_MAX;
  for ( unsigned int i = 0; i < len; i++ ) {
    // multiply by index so god != dog
    // and p so returned values are sufficiently large and well spaced
    unsigned int val = str[i] * ( i + p );
    sum += val;
  }
  unsigned int h = sum % k;
  // could check here for table collision and move to next index along (and
  // record that somewhere in a look-up list for this function)
  return h;
}
#endif

///////////////////////////////////////////////////////////////////////////////

// Euclid's algorithm to find greatest common denominator of fraction
inline int gcd( int u, int v ) {
  if ( u < 0 ) { u = -u; } // doesn't play well with negatives so convert
  if ( v < 0 ) { v = -v; }
  if ( u == 0 || v == 0 ) { return 0; }
  int t;
  while ( u > 0 ) {
    if ( u < v ) {
      t = u;
      u = v;
      v = t;
    }
    u = u % v;
  }
  return v;
}

// reduce a fraction to smallest terms. e.g 10/20 -> 1/2
void reduce_frac( int* u, int* v ) {
  assert( u && v );
  int div = gcd( *u, *v );
  *u /= div;
  *v /= div;
}

///////////////////////////////////////////////////////////////////////////////

// quadtree assumptions
//   - square regions of 2D space
//   - maximum bucket size of n
//   - when n exceeded, node splits into 4 equally spaced nodes
//   - if an item intersects >1 region it exists in both/all
//
// alternative - precalc quadtree subdivisions down to a certain size eg 1m x 1m
//             - work out which leaf node cell(s) everything is in - e.g.  use
//               integer division on AABB corners - and store ptr in those cells
//             - recursive frustum v node intersection tests - completely eliminate
//               any subtrees not in frustum, leaving a set of leaf nodes
//             - itemise unique stuff in set of remaining leaf nodes
//             - frustum test can also be on AABB around frustum in 2D
// L2 test vs 4/4 of root's children      				 4
// L3 eliminated 2, test 8/16 children    	    		12
// L4 eliminated 4, test 16/64 children					28 -- total box/box tests
// L5 elimated 2, sorted list everything in 14 nodes (avoid dupes)
// * > number of levels dramatically increases comp complexity
// * 28 tests is really good compared to ~1000 tests on CPU
// * but CPU bound isnt really a problem atm, especially cache-coherent arrays
// ~n=100
// * older article with code notes:
// https://www.gamedev.net/resources/_/technical/graphics-programming-and-theory/quadtrees-r1303

/*
-------------------------------------------------------------------------------------
This software is available under two licences - you may use it under either licence.
-------------------------------------------------------------------------------------
FIRST LICENCE OPTION

>                                  Apache License
>                            Version 2.0, January 2004
>                         http://www.apache.org/licenses/
>    Copyright 2019 Anton Gerdelan.
>    Licensed under the Apache License, Version 2.0 (the "License");
>    you may not use this file except in compliance with the License.
>    You may obtain a copy of the License at
>        http://www.apache.org/licenses/LICENSE-2.0
>    Unless required by applicable law or agreed to in writing, software
>    distributed under the License is distributed on an "AS IS" BASIS,
>    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
>    See the License for the specific language governing permissions and
>    limitations under the License.
-------------------------------------------------------------------------------------
SECOND LICENCE OPTION

> This is free and unencumbered software released into the public domain.
>
> Anyone is free to copy, modify, publish, use, compile, sell, or
> distribute this software, either in source code form or as a compiled
> binary, for any purpose, commercial or non-commercial, and by any
> means.
>
> In jurisdictions that recognize copyright laws, the author or authors
> of this software dedicate any and all copyright interest in the
> software to the public domain. We make this dedication for the benefit
> of the public at large and to the detriment of our heirs and
> successors. We intend this dedication to be an overt act of
> relinquishment in perpetuity of all present and future rights to this
> software under copyright law.
>
> THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
> EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
> MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
> IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
> OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
> ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
> OTHER DEALINGS IN THE SOFTWARE.
>
> For more information, please refer to <http://unlicense.org>
-------------------------------------------------------------------------------------
*/