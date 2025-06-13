#define APG_NO_BACKTRACES
#define APG_IMPLEMENTATION
#include "apg.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

typedef struct node_t {
  int64_t key;
  int64_t h;
  int64_t neighbours[4];
  int64_t n_neighbours;
} node_t;

// TODO(anton) generate a muuuuch larger set to test timing from top-left to bottom-right...somehow. maybe from an image where black pixels = no neighbour?

node_t nodes[6] = {
  { .key = 0, .h = 100, .neighbours = { 1, 2, 3, 4 }, .n_neighbours = 4 }, //
  { .key = 1, .h = 75, .neighbours = { 0 }, .n_neighbours = 1 },           //
  { .key = 2, .h = 50, .neighbours = { 0 }, .n_neighbours = 1 },           //
  { .key = 3, .h = 66, .neighbours = { 0 }, .n_neighbours = 1 },           //
  { .key = 4, .h = 69, .neighbours = { 5 }, .n_neighbours = 1 },           //
  { .key = 5, .h = 0, .neighbours = { 4 }, .n_neighbours = 1 }             //
};

static int64_t _h_cb( int64_t key, int64_t target_key ) {
  (void)target_key;
  for ( int i = 0; i < 6; i++ ) {
    if ( key == nodes[i].key ) { return nodes[i].h; }
  }
  assert( false && "couldn't get h for key" );
  return -1;
}

static int64_t _get_neighbours_cb( int64_t key, int64_t target_key, int64_t* neighs ) {
  (void)target_key;
  for ( int i = 0; i < 6; i++ ) {
    if ( key == nodes[i].key ) {
      memcpy( neighs, nodes[i].neighbours, sizeof( int64_t ) * nodes[i].n_neighbours );
      return nodes[i].n_neighbours;
    }
  }
  assert( false && "couldn't get neighbour count" );
  return -1;
}

#define N 1024

apg_gbfs_node_t eval_ptr[512];
int64_t visited_ptr[512];
apg_gbfs_node_t queue_ptr[128];

int main( void ) {
  int64_t reversed_path[2048], path_n = 0;
  apg_time_init();
  double cumulative_time = 0.0;
  for ( int runi = 0; runi < N; runi++ ) {
    double start   = apg_time_s();
    bool ret       = apg_gbfs( 0, 5, _h_cb, _get_neighbours_cb, reversed_path, &path_n, 2048LL, eval_ptr, 512LL, visited_ptr, 512LL, queue_ptr, 128LL );
    double elapsed = apg_time_s() - start;
    cumulative_time += elapsed;
    if ( ret ) {
      printf( "FOUND PATH from keys 0 to 5 with %lli steps.\n", path_n );
      for ( int64_t i = path_n - 1, j = 0; i >= 0; i--, j++ ) { printf( "%lli) key: %lli\n", j, reversed_path[i] ); }
    } else {
      printf( "NO PATH FOUND!\n" );
    }
    printf( "Time elapsed in search: %lfms\n", elapsed * 1000.0 );
  }
  printf( "Cumulative search time for %u searches %lfms\nAverage search time %lfms\n", N, cumulative_time * 1000.0, ( cumulative_time * 1000.0 ) / (double)N );
  return 0;
}
