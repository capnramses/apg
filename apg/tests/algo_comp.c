/*
An amortised comparison of search algorithms finding a path through a maze drawn into an image, for easy visualisation.
by Anton Gerdelan

gcc algo_comp.c -I ../ -I ../../third_party/stb/ -lm -Wall -Wextra -pedantic
*/
#define APG_NO_BACKTRACES
#define APG_IMPLEMENTATION
#include "apg.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

// maze image and dims
static uint8_t* img_ptr;
static int w, h, n_chans;

// get distance heuristic for a given node/key/pixel
static int _h_cb_ptr( int key ) {
  int x      = key % h;
  int y      = key / h;
  int x_dist = ( w - 1 ) - x;
  int y_dist = ( h - 1 ) - y;
  return x_dist + y_dist; // manhattan
}

// get an array of valid non-obstacle neighbours for a given node/key/pixel
static int _neighs_cb_ptr( int key, int* neighs ) {
  int x        = key % h;
  int y        = key / h;
  int n_neighs = 0;
  if ( x < w - 1 ) {
    int neigh_key = y * w + x + 1;
    if ( img_ptr[neigh_key * n_chans] == 0xFF ) { neighs[n_neighs++] = neigh_key; } // just check first colour byte ~ non-black ->> path.
  }
  if ( x > 0 ) {
    int neigh_key = y * w + x - 1;
    if ( img_ptr[neigh_key * n_chans] == 0xFF ) { neighs[n_neighs++] = neigh_key; } // just check first colour byte ~ non-black ->> path.
  }
  if ( y < h - 1 ) {
    int neigh_key = ( y + 1 ) * w + x;
    if ( img_ptr[neigh_key * n_chans] == 0xFF ) { neighs[n_neighs++] = neigh_key; } // just check first colour byte ~ non-black ->> path.
  }
  if ( y > 0 ) {
    int neigh_key = ( y - 1 ) * w + x;
    if ( img_ptr[neigh_key * n_chans] == 0xFF ) { neighs[n_neighs++] = neigh_key; } // just check first colour byte ~ non-black ->> path.
  }
  return n_neighs;
}

#define MAZE_PATH_MAX APG_MEGABYTES( 256 )
static int reverse_path[MAZE_PATH_MAX];
static uint64_t reverse_path_n;

///////////////////////////////////////////////////////////

// Called whenever an item is _inserted_ into the queue - with biggest h towards the start (reverse order).
static int _apg_gbfs_sort_queue_comp_cb( const void* a_ptr, const void* b_ptr ) {
  apg_gbfs_node_t a_node = *(apg_gbfs_node_t*)a_ptr;
  apg_gbfs_node_t b_node = *(apg_gbfs_node_t*)b_ptr;
  return b_node.h - a_node.h;
}

// Called whenever an item is _inserted_ into the visited set - with smallest parent_key towards the start (correct order for bsearch).
static int _apg_gbfs_sort_vset_comp_cb( const void* a_ptr, const void* b_ptr ) { return *(int*)a_ptr - *(int*)b_ptr; }

// Called whenever we check if an item has been visited already. should return -ve if key < element.
static int _apg_gbfs_search_vset_comp_cb( const void* key_ptr, const void* element_ptr ) { return *(int*)key_ptr - *(int*)element_ptr; }

// A 'clever' version using sorting and searching algorithms to replace linear arrays.
bool apg_gbfs2( int start_key, int target_key, int ( *h_cb_ptr )( int key ), int ( *neighs_cb_ptr )( int key, int* neighs ), int* reverse_path_ptr,
  uint64_t* path_n, uint64_t max_path_steps, int* visited_set_ptr, int visited_set_max, apg_gbfs_node_t* evaluated_nodes_ptr, int evaluated_nodes_max,
  apg_gbfs_node_t* queue_ptr, int queue_max ) {
  // apg_gbfs_node_t queue[APG_GBFS_ARRAY_MAX]; // ~96kB. Descending-order sorted by h O(n log n) to avoid the need to search the queue.
  //  apg_gbfs_node_t evaluated_nodes[APG_GBFS_ARRAY_MAX]; // ~96kB. Used to recreate path on success. Only includes nodes that had childen added to the queue.
  //  int visited_set_keys[APG_GBFS_ARRAY_MAX];            // ~32kB. Sorted in ascending order by key O(n log n) to allow binary search O(log n).
  int n_visited_set = 1, n_queue = 1, n_evaluated_nodes = 0;
  visited_set_ptr[0] = start_key;                                                                                 // Mark start as visited
  queue_ptr[0]       = ( apg_gbfs_node_t ){ .h = h_cb_ptr( start_key ), .parent_idx = -1, .our_key = start_key }; // and add to queue.
  while ( n_queue > 0 ) {
    apg_gbfs_node_t curr = queue_ptr[--n_queue]; // curr is vertex in queue w/ smallest h. Smallest h is always at the end of the queue for easy deletion.

    int neigh_keys[APG_GBFS_NEIGHBOURS_MAX];
    int n_neighs = neighs_cb_ptr( curr.our_key, neigh_keys );
    if ( n_neighs > APG_GBFS_NEIGHBOURS_MAX ) {
      printf( "neigh max\n" );
      return false;
    }
    bool neigh_added = false, found_path = false;
    for ( int neigh_idx = 0; neigh_idx < n_neighs; neigh_idx++ ) {
      if ( neigh_keys[neigh_idx] == target_key ) {
        found_path = neigh_added = true; // Resolve path including the final item's key. Break here and flag so that we add the final node.
        break;
      }
      if ( bsearch( &neigh_keys[neigh_idx], visited_set_ptr, n_visited_set, sizeof( int ), _apg_gbfs_search_vset_comp_cb ) != NULL ) { continue; }

      if ( n_visited_set >= visited_set_max ) {
        printf( "n_visited_set max \n" );
        return false;
      }
      if ( n_queue >= queue_max ) {
        printf( " n_queue max\n" );
        return false;
      }
      visited_set_ptr[n_visited_set++] = neigh_keys[neigh_idx]; // If not already visited then mark as visited and add n to queue.
      // parent_idx is n_evaluated_nodes because we /will/ add the parent to the end of that list shortly.
      queue_ptr[n_queue++] = ( apg_gbfs_node_t ){ .h = h_cb_ptr( neigh_keys[neigh_idx] ), .parent_idx = n_evaluated_nodes, .our_key = neigh_keys[neigh_idx] };
      qsort( visited_set_ptr, n_visited_set, sizeof( int ), _apg_gbfs_sort_vset_comp_cb );
      qsort( queue_ptr, n_queue, sizeof( apg_gbfs_node_t ), _apg_gbfs_sort_queue_comp_cb );
      neigh_added = true;
    } // endfor neighbours
    if ( neigh_added ) {
      if ( n_evaluated_nodes >= evaluated_nodes_max ) {
        printf( "n_evaluated_nodes max\n" );
        return false;
      }
      evaluated_nodes_ptr[n_evaluated_nodes++] = curr;
    }
    if ( found_path ) {
      uint64_t tmp_path_n            = 0;
      int parent_eval_idx            = n_evaluated_nodes - 1;
      reverse_path_ptr[tmp_path_n++] = target_key;
      for ( int i = 0; i < n_evaluated_nodes; i++ ) { // Some sort of timeout in case of logic error.
        if ( tmp_path_n >= max_path_steps ) {
          printf( "tmp_path_n >= max_path_steps \n" );
          return false;
        } // Maxed out path length.
        apg_gbfs_node_t path_tmp       = evaluated_nodes_ptr[parent_eval_idx];
        reverse_path_ptr[tmp_path_n++] = path_tmp.our_key;
        parent_eval_idx                = path_tmp.parent_idx;
        if ( path_tmp.parent_idx == -1 ) {
          *path_n = tmp_path_n;
          //	printf("stats: n_evaluated_nodes = %lu, n_visited_set = %lu, n_queue = %lu\n", n_evaluated_nodes, n_visited_set, n_queue );
          return true;
        }
      }
      assert( false && "failed to find path back to start" );
      return false;
    }
  } // endwhile queue not empty
  return false;
}
/////////////////////////////////////////////////////////////

// Supply an image to load where black pixels are obstacles and white pixels are valid paths.
// Search starts at a white pixel in the top left and tries to find a path to the white pixel in the bottom-right.
// The path is output as a red line in out_path.png that can be overlaid on the source image.
int main( int argc, char** argv ) {
  if ( argc < 2 ) {
    printf( "Usage: %s image.PNG\n", argv[0] );
    return 0;
  }
  img_ptr = stbi_load( argv[1], &w, &h, &n_chans, 0 );
  if ( !img_ptr ) {
    fprintf( stderr, "ERROR: failed to load image %s\n", img_ptr );
    return 1;
  }
  printf( "Loaded image %s\n", argv[1] );
  bool success = false;

  // for 128px version, at end (queue may be bigger at run-time): stats: n_evaluated_nodes = 2351, n_visited_set = 3346, n_queue = 306
  size_t visted_set_max                = 1024 * 1024;
  int* visited_set_ptr                 = malloc( visted_set_max * sizeof( int ) ); // 32MB
  size_t evaluated_nodes_max           = 1024 * 1024;
  apg_gbfs_node_t* evaluated_nodes_ptr = malloc( evaluated_nodes_max * sizeof( apg_gbfs_node_t ) ); //
  int queue_max                        = 1024 * 1024;
  apg_gbfs_node_t* queue_ptr           = malloc( queue_max * sizeof( apg_gbfs_node_t ) );
  // the above is ~30MB
  printf( "total heap aux memory: %lu\n", visted_set_max * sizeof( int ) + evaluated_nodes_max * sizeof( apg_gbfs_node_t ) + queue_max * sizeof( apg_gbfs_node_t ) );

#define N_RUNS 1000
  apg_time_init();
  {
    printf( "Greedy BFS #1 (basic array iteration from apg.h):\n" );
    double cumulative_time = 0.0;
    for ( int i = 0; i < N_RUNS; i++ ) {
      double start_time = apg_time_s();
      int start_pixel   = 0;
      int target_pixel  = w * h - 1; // NOTE: not the mem addr: * n_chans to get that.
      success           = apg_gbfs( start_pixel, target_pixel, _h_cb_ptr, _neighs_cb_ptr, reverse_path, &reverse_path_n, MAZE_PATH_MAX, evaluated_nodes_ptr,
        evaluated_nodes_max, visited_set_ptr, visted_set_max, queue_ptr, queue_max );
      double end_time   = apg_time_s();
      double elapsed    = end_time - start_time;
      cumulative_time += elapsed;
    }
    printf( "Average time taken = %lfms (over %i runs)\n", ( cumulative_time * 1000 ) / N_RUNS, N_RUNS );
  }

  {
    printf( "Greedy BFS #2 (using binary search & working array sorting):\n" );
    double cumulative_time = 0.0;
    for ( int i = 0; i < N_RUNS; i++ ) {
      double start_time = apg_time_s();
      int start_pixel   = 0;
      int target_pixel  = w * h - 1; // NOTE: not the mem addr: * n_chans to get that.
      success = apg_gbfs2( start_pixel, target_pixel, _h_cb_ptr, _neighs_cb_ptr, reverse_path, &reverse_path_n, MAZE_PATH_MAX, visited_set_ptr, visted_set_max,
        evaluated_nodes_ptr, evaluated_nodes_max, queue_ptr, queue_max );
      double end_time = apg_time_s();
      double elapsed  = end_time - start_time;
      cumulative_time += elapsed;
    }
    printf( "Average time taken = %lfms (over %i runs)\n", ( cumulative_time * 1000 ) / N_RUNS, N_RUNS );
  }

  free( visited_set_ptr );
  free( evaluated_nodes_ptr );

  {
    // TODO!
    printf( "TODO Greedy BFS #3 (using hash table from apg.h for visited set and queue):\n" );
  }
  {
    /*
     TODO Let the user supply the working sets (queue and visited set) with sizes (doesn't necessarily sacrifice fast stack memory, and can solve bigger searches
    * and avoid syscalls for repeated searches that can reuse any allocated memory
    */
    printf( "TODO Greedy BFS #4 (with dynamic memory to allow for BIG maze search completion):\n" );
  }
  if ( success ) {
    printf( "Path found.\n" );
    uint8_t* out_img_ptr = calloc( w * h * 4, 1 );
    if ( !out_img_ptr ) {
      fprintf( stderr, "ERROR: OOM for output!\n" );
      return 1;
    }

    for ( uint64_t i = 0; i < reverse_path_n; i++ ) {
      int key                  = reverse_path[i]; // pixel index. multiply by 4 for byte.
      out_img_ptr[key * 4 + 0] = 0xFF;            // RED channel.
      out_img_ptr[key * 4 + 3] = 0xFF;            // ALPHA channel.
    }
    if ( !stbi_write_png( "out_path.png", w, h, 4, out_img_ptr, w * 4 ) ) { fprintf( stderr, "WARNING: could not write out_path.png\n" ); }

    free( out_img_ptr );
  } else {
    printf( "Path NOT found.\n" );
  }

  free( img_ptr );
  return 0;
}
