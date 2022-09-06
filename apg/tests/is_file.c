#define APG_IMPLEMENTATION
#define APG_NO_BACKTRACES
#include "../apg.h"
#include <stdio.h>

int main( int argc, char** argv ) {
  if ( argc < 2 ) {
    printf( "Usage: ./is_file PATH\n" );
    return 0;
  }
  bool is_file = apg_is_file( argv[1] );
  bool is_dir  = apg_is_dir( argv[1] );

  printf( "Path `%s`\n  is_file=%i\n  is_dir =%i\n", argv[1], is_file, is_dir );

  return 0;
}
