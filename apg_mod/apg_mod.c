/*****************************************************************************\
apg_mod - description to go here
Anton Gerdelan <antonofnote at gmail>
C99
Version 0.0
Licence: see header.
\*****************************************************************************/

#include "apg_mod.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Finetune values are stored as 4-bit signed integers:
Value Finetune
 0      0
 1     +1
 2     +2
 3     +3
 4     +4
 5     +5
 6     +6
 7     +7
 8     -8
 9     -7
 A     -6
 B     -5
 C     -4
 D     -3
 E     -2
 F     -1
*/

// 30 bytes.
typedef struct sample_t {
  char _name[22];         // Pad with nul byte(s).
  uint16_t length;        // This is the number of 2-byte *words*. Multiply x2 to get bytes.
  uint8_t finetune;       // Lower 4 bits stored as signed 4-bit number. Upper 4-bits are set to 0.
  uint8_t volume;         // Range is 0x00-0x40 or 0-64 decimal.
  uint16_t repeat_point;  // Stored as n of words offset from start of sample. x2 = offset in bytes.
  uint16_t repeat_length; // Stored as n of words offset from start of sample. x2 = offset in bytes.
} sample_t;

// The MOD format is headerless so not technically a 'header' but I refer to it as one.
// This seems to reliably map to .mod files, but not "Extended Module" .xm files, which
// look like they have more data per field e.g. longer songname etc.
typedef struct protracker_1_1b_hdr_t {
  char _songname[20];          // Should include trailing nul byte(s).
  sample_t samples[31];        // Sample numbers are 1-31. Early versions had only 15 samples.
  uint8_t songlength;          // Range is 1-128
  uint8_t special;             // Set to 127 to make old trackers parse all patterns. Some trackers use to mean 'restart'.
  uint8_t song_positions[128]; // Positions 0-127.  Values are a number 0-63 to indicate pattern to play at that position.
  char magicletters[4];        // "M.K." (or "FLT4" or "FLT8"). If not here assume song uses only 15 samples or text was removed to obfuscate the music.
} protracker_1_1b_hdr_t;

typedef struct record_t {
  uint8_t* data_ptr;
  size_t sz;
} record_t;

// If returned record has a NULL ptr or sz == 0 then it failed to read.
static record_t _read_entire_file( const char* filename ) {
  record_t record = ( record_t ){ .sz = 0 };
  if ( !filename ) { return record; }
  FILE* f_ptr = fopen( filename, "rb" );
  if ( !f_ptr ) { return record; }
  fseek( f_ptr, 0L, SEEK_END );
  record.sz       = (size_t)ftell( f_ptr );
  record.data_ptr = malloc( record.sz );
  if ( !record.data_ptr ) {
    fclose( f_ptr );
    return record;
  }
  rewind( f_ptr );
  size_t nr = fread( record.data_ptr, record.sz, 1, f_ptr );
  fclose( f_ptr );
  if ( 1 != nr ) { // failed to read
    free( record.data_ptr );
    record = ( record_t ){ .sz = 0 };
  }
  return record;
}

bool apg_mod_read_file( const char* filename ) {
  record_t record = _read_entire_file( filename );
  if ( !record.data_ptr || 0 == record.sz ) { return false; }

  protracker_1_1b_hdr_t* hdr_ptr = (protracker_1_1b_hdr_t*)record.data_ptr;
  char songname[21];
  songname[20] = '\0'; // Song names are usually not nul-terminated.
  memcpy( songname, hdr_ptr->_songname, 20 );

  printf( "Song name:   \"%s\"\n", songname );
  printf( "Song length: %u\n", (uint32_t)hdr_ptr->songlength );
  for ( int i = 0; i < 31; i++ ) {
    char samplename[23];
    samplename[22] = '\0'; // Song names are usually not nul-terminated.
    memcpy( samplename, hdr_ptr->samples[i]._name, 22 );
    printf( "  Sample %i name: \"%s\"\n", i + 1, samplename );
  }

  return true;
}

bool apg_mod_write_file( const char* filename ) {
  if ( !filename ) { return false; }
  return true;
}
