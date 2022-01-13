#define APG_IMPLEMENTATION
#include "apg.h"
#include <stdbool.h>
#include <stdio.h>

typedef struct _entire_file_t {
  void* data;
  size_t sz;
} _entire_file_t;

static bool _read_entire_file( const char* filename, _entire_file_t* record ) {
  FILE* fp = fopen( filename, "rb" );
  if ( !fp ) { return false; }
  fseek( fp, 0L, SEEK_END );
  record->sz   = (size_t)ftell( fp );
  record->data = malloc( record->sz );
  if ( !record->data ) {
    fclose( fp );
    return false;
  }
  rewind( fp );
  size_t nr = fread( record->data, record->sz, 1, fp );
  fclose( fp );
  if ( 1 != nr ) { return false; }
  return true;
}

int main( int argc, char** argv ) {
  if ( argc < 3 ) {
    printf( "usage: compress.exe myfile.bmp output.rle\n" );
    return 0;
  }
  _entire_file_t record;
  if ( !_read_entire_file( argv[1], &record ) ) {
    printf( "error reading file %s\n", argv[1] );
    return 1;
  }
  size_t out_sz;
  apg_rle_compress( record.data, record.sz, NULL, &out_sz );
  printf( "record sz %i. compressed sz %i\n%.2f%% size when compressed\n", (int)record.sz, (int)out_sz, ( (float)out_sz / (float)record.sz ) * 100 );
  uint8_t* out_ptr = malloc( out_sz );
  apg_rle_compress( record.data, record.sz, out_ptr, &out_sz );

  FILE* fptr = fopen( argv[2], "wb" );
  if ( !fptr ) {
    printf( "error writing file %s\n", argv[2] );
    return 1;
  }
  fwrite( out_ptr, out_sz, 1, fptr );
  fclose( fptr );

  return 0;
}
