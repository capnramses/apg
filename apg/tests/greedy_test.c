#define APG_NO_BACKTRACES
#define APG_IMPLEMENTATION
#include "apg.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

typedef struct node_t {
  int key;
  int h;
  int neighbours[4];
  int n_neighbours;
} node_t;

node_t nodes[6] = {
  { .key = 0, .h = 100, .neighbours = { 1, 2, 3, 4 }, .n_neighbours = 4 }, //
  { .key = 1, .h = 75, .neighbours = { 0 }, .n_neighbours = 1 },           //
  { .key = 2, .h = 50, .neighbours = { 0 }, .n_neighbours = 1 },           //
  { .key = 3, .h = 66, .neighbours = { 0 }, .n_neighbours = 1 },           //
  { .key = 4, .h = 69, .neighbours = { 5 }, .n_neighbours = 1 },           //
  { .key = 5, .h = 0, .neighbours = { 4 }, .n_neighbours = 1 }             //
};

static int _h_cb( int key ) {
  for ( int i = 0; i < 6; i++ ) {
    if ( key == nodes[i].key ) { return nodes[i].h; }
  }
  assert( false && "couldn't get h for key" );
  return -1;
}

static int _get_neighbours_cb( int key, int* neighs ) {
  for ( int i = 0; i < 6; i++ ) {
    if ( key == nodes[i].key ) {
      memcpy( neighs, nodes[i].neighbours, sizeof( int ) * nodes[i].n_neighbours );
      return nodes[i].n_neighbours;
    }
  }
  assert( false && "couldn't get neighbour count" );
  return -1;
}

int main() {
  int reversed_path[2048], path_n = 0;
  bool ret = apg_gbfs( 0, 5, _h_cb, _get_neighbours_cb, reversed_path, &path_n, 2048 );
  if ( ret ) {
    printf( "FOUND PATH from keys 0 to 5 with %i steps\n", path_n );
    for ( int i = path_n - 1, j = 0; i >= 0; i--, j++ ) { printf( "%i) key: %i\n", j, reversed_path[i] ); }
  } else {
    printf( "NO PATH FOUND!\n" );
  }

  return 0;
}
