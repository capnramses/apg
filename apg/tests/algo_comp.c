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

  apg_time_init();

  {
    printf( "Greedy BFS:\n" );
    double cumulative_time = 0.0;
    for ( int i = 0; i < 1000; i++ ) {
      double start_time = apg_time_s();
      int start_pixel   = 0;
      int target_pixel  = w * h - 1; // NOTE: not the mem addr: * n_chans to get that.
      success           = apg_gbfs( start_pixel, target_pixel, _h_cb_ptr, _neighs_cb_ptr, reverse_path, &reverse_path_n, MAZE_PATH_MAX );
      double end_time   = apg_time_s();
      double elapsed    = end_time - start_time;
      cumulative_time += elapsed;
    }
    printf( "Average time taken = %lfms (over 1000 runs)\n", ( cumulative_time * 1000 ) / 1000 );
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
