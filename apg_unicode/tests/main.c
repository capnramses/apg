// test program
// COMPILE:
// gcc main.c ../apg_unicode.c -I ../
// RUN:
// a.out ./test.txt (or any UTF-8 file)
// only the first 2048 or fewer bytes are read from the file.
// output: out.txt (UTF-8)

#include "apg_unicode.h"
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define APG_MIN( a, b ) ( ( a ) < ( b ) ? ( a ) : ( b ) )
#define APG_MAX( a, b ) ( ( a ) > ( b ) ? ( a ) : ( b ) )
#define APG_CLAMP( x, lo, hi ) ( APG_MIN( hi, APG_MAX( lo, x ) ) )

/* convenience struct and file->memory function */
typedef struct _entire_file_t {
  void* data;
  size_t sz;
} _entire_file_t;

/*
RETURNS
- true on success. record->data is allocated memory and must be freed by the caller.
- false on any error. Any allocated memory is freed if false is returned */
static bool _read_entire_file( const char* filename, _entire_file_t* record ) {
  FILE* fp = fopen( filename, "rb" );
  if ( !fp ) { return false; }
  fseek( fp, 0L, SEEK_END );
  record->sz   = (size_t)ftell( fp );
  record->data = malloc( record->sz + 1 ); // + 1 to allow for a \0 to be appended
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

int apg_strnlen( const char* str, int maxlen ) {
  int i = 0;
  while ( i < maxlen && str[i] ) { i++; }
  return i;
}

int main( int argc, char** argv ) {
  if ( argc < 2 ) {
    printf( "usage: a.out UTF8INPUTFILE.txt\n" );
    return 0;
  }

  // read whole file into a buffer
  _entire_file_t record = { .sz = 0 };
  if ( !_read_entire_file( argv[1], &record ) ) {
    fprintf( stderr, "ERROR opening file `%s`\n", argv[1] );
    return 1;
  }
  char *ptr = (char*)record.data;
  ptr[record.sz] = '\0';

  // just read the first N chars from the file as a string. ignore the rest.
  int sz    = strnlen( ptr, APG_MIN( record.sz, APG_UNICODE_MAX_STR ) );

  FILE* fp = fopen( "out.txt", "w" );
  while ( sz > 0 ) {
    // convert a sequence of up to 4 UTF-8 bytes into a 32-bit codepoint
    uint32_t cp = apg_utf8_to_cp( ptr, &sz );
    // store the sequence of UTF-8 bytes in a temporary string.
    char tmp[5];
    tmp[0] = 0;
    strncat( tmp, ptr, sz );
    char tmpb[5];
    // convert the codepoint back to a UTF-8 byte sequence
    int szb = apg_cp_to_utf8( cp, tmpb );
    // compare the new output with the original bytes and report an error if they did not match
    if ( 0 != strcmp( tmp, tmpb ) ) {
      fprintf( fp, "ERROR [%s] vs [%s] cp = %u (U+%04X), sz = %i,%i\n", tmp, tmpb, cp, cp, sz, szb );
    }
    // write to out.txt:
    // 1. the UTF-8 byte sequence as a string,
    // 2. then the codepoint decimal value,
    // 3. then the UTF-8 unicode hex value,
    // 4. then the size in bytes.
    fprintf( fp, "[%s] cp = %u (U+%04X), sz = %i\n", tmp, cp, cp, sz );
    ptr += sz;
  }
  fclose( fp );

  return 0;
}
