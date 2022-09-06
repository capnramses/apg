#define APG_IMPLEMENTATION
#define APG_NO_BACKTRACES
#include "../apg.h"
#include <stdio.h>

int main( int argc, char** argv ) {
  if ( argc < 2 ) {
    printf( "Usage: ./is_file PATH\n" );
    return 0;
  }
  apg_dirent_t* list_ptr = NULL;
  int n_list             = 0;
  bool res               = apg_dir_contents( argv[1], &list_ptr, &n_list );
  if ( !res ) {
    fprintf( stderr, "ERROR: reading directory %s\n", argv[1] );
    return 1;
  }
  printf( "Contents of %s:\n", argv[1] );
  for ( int i = 0; i < n_list; i++ ) { printf( "[%i] type=%i path=%s\n", i, list_ptr[i].type, list_ptr[i].path ); }

  res = apg_free_dir_contents_list( &list_ptr, n_list );
  if ( !res ) {
    fprintf( stderr, "ERROR: freeing directory contents list.\n" );
    return 1;
  }

  printf( "\nNormal exit\n" );

  return 0;
}
