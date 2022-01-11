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

// 30 bytes.
typedef struct sample_t {
  char _name[22];       // Pad with nul byte(s).
  uint16_t length;      // This is the number of 2-byte *words*. Multiply x2 to get bytes.
  uint8_t finetune;     // Lower 4 bits stored as signed 4-bit number. Upper 4-bits are set to 0.
  uint8_t volume;       // Range is 0x00-0x40 or 0-64 decimal.
  uint16_t loop_start;  // Stored as n of words offset from start of sample. x2 = offset in bytes.
  uint16_t loop_length; // Stored as n of words offset from start of sample. x2 = offset in bytes.
} sample_t;

// The MOD format is headerless so not technically a 'header' but I refer to it as one.
// This seems to reliably map to .mod files, but not "Extended Module" .xm files, which
// look like they have more data per field e.g. longer songname etc.
typedef struct protracker_1_1b_hdr_t {
  char _songname[20];        // Should include trailing nul byte(s).
  sample_t samples[31];      // Sample numbers are 1-31. Early versions had only 15 samples.
  uint8_t song_length;       // Range is 1-128. This is the number of pattern orders (from orders_table) to play in the song, including repeated patterns.
  uint8_t unused;            // Set to 127 to make old trackers parse all patterns. PT used it to mean 'restart'.
  uint8_t orders_table[128]; // Positions 0-127.  Values are a number 0-63 to indicate pattern to play at that position.
  char magicletters[4];      // "M.K." (or "FLT4" or "FLT8"). If not here assume song uses only 15 samples or text was removed to obfuscate the music.
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

/* "To get the real value in bytes, calculate it with (byte1*100h + byte2) * 2"
 * Applies to sample name, sample length, loop start, loop length.
 */
static int _words_val_to_bytes( uint16_t words_val ) {
  uint8_t lsb = (uint8_t)( words_val & 0xFF ); // Mask the lower byte.
  uint8_t msb = (uint8_t)( words_val >> 8 );   // Shift the higher byte.
}

/* Sample finetune values are stored as 4-bit signed integers: 0,1,2,3,4,5,6,7,-8,-7,-6,-5,-4,-3,-2,-1
 * NOTE(Anton) this could be a #define for an inline. */
static int _fine_tune_bits_to_int( uint8_t finetune ) { return finetune < 8 ? (int)finetune : (int)finetune - 16; }

bool apg_mod_read_file( const char* filename ) {
  record_t record = _read_entire_file( filename );
  if ( !record.data_ptr ) { return false; }

  if ( record.sz < 1084 ) {
    free( record.data_ptr );
    return false;
  }

  // Get offset at 1080 to determine MOD format.
  char magic_str[5];
  magic_str[4] = '\0';
  memcpy( magic_str, &record.data_ptr[1080], 4 );
  int n_chans = 0;

  if ( 0 == strcmp( magic_str, "M.K." ) ) {
    printf( "Standard 4-channel module\n" );
    n_chans = 4;
  } else if ( 0 == strcmp( magic_str, "2CHN" ) ) {
    printf( "2-channel FastTracker module\n" );
    n_chans = 2;
  } else if ( 0 == strcmp( magic_str, "6CHN" ) ) {
    printf( "6-channel FastTracker module\n" );
    n_chans = 6;
  } else if ( 0 == strcmp( magic_str, "8CHN" ) ) {
    printf( "8-channel FastTracker module\n" );
    n_chans = 8;
  } else if ( 0 == strcmp( magic_str, "CD81" ) ) {
    printf( "8-channel Falcon module\n" );
    n_chans = 8;
  } else if ( 0 == strcmp( magic_str, "FLT4" ) ) {
    printf( "4-channel Startrekker module\n" );
    n_chans = 4;
  } else if ( 0 == strcmp( magic_str, "FLT8" ) ) {
    printf( "8-channel Startrekker module\n" );
    n_chans = 8;
  } else if ( 0 == strcmp( magic_str, "M!K!" ) ) {
    printf( ">64 pattern ProTracker module\n" );
  } else if ( 0 == strcmp( magic_str, "OCTA" ) ) {
    printf( "8-channel module\n" );
    n_chans = 48;
  } else if ( magic_str[0] == 'T' && magic_str[1] == 'D' && magic_str[2] == 'Z' ) {
    sscanf( magic_str, "TDZ%i", &n_chans );
    printf( "%i-channel TakeTracker module\n", n_chans );
  } else if ( magic_str[1] == 'C' && magic_str[2] == 'H' && magic_str[3] == 'N' ) {
    sscanf( magic_str, "%iCHN", &n_chans );
    printf( "%i-channel TakeTracker module\n", n_chans );
  } else if ( magic_str[2] == 'C' && magic_str[3] == 'H' ) {
    sscanf( magic_str, "%iCH", &n_chans );
    printf( "%i-channel TakeTracker module\n", n_chans );
  } else {
    fprintf( stderr, "ERROR: Could not detect module type from \"%s\".\n", magic_str );
    free( record.data_ptr );
    return false;
  }

  protracker_1_1b_hdr_t* hdr_ptr = (protracker_1_1b_hdr_t*)record.data_ptr;
  char songname[21];
  songname[20] = '\0'; // Song names are usually not nul-terminated.
  memcpy( songname, hdr_ptr->_songname, 20 );

  printf( "Song name:   \"%s\"\n", songname );
  printf( "Song length: %u\n", (uint32_t)hdr_ptr->song_length );
#ifdef PRINT_SAMPLE_INFO
  for ( int i = 0; i < 31; i++ ) {
    char samplename[23];
    samplename[22] = '\0'; // Song names are usually not nul-terminated.
    memcpy( samplename, hdr_ptr->samples[i]._name, 22 );
    printf( "  Sample %i name: \"%s\"\n", i + 1, samplename );
    printf( "    Length:         %u\n", (uint32_t)hdr_ptr->samples[i].length );
    printf( "    Finetune:       %u\n", (uint32_t)hdr_ptr->samples[i].finetune );
    printf( "    Volume:         %u\n", (uint32_t)hdr_ptr->samples[i].finetune );
    printf( "    Loop point:     %u\n", (uint32_t)hdr_ptr->samples[i].loop_start );
    printf( "    Loop length:    %u\n", (uint32_t)hdr_ptr->samples[i].loop_length );
  }
#endif

  // Determine n of patterns stored in file by looking through order table for biggest pattern index played.
  int max_pattern = 0;
  printf( "Orders:\n" );
  // NOTE(Anton) could probably stop at song_length here, not the full 128
  for ( int i = 0; i < 128; i++ ) {
    if ( hdr_ptr->orders_table[i] > max_pattern ) { max_pattern = hdr_ptr->orders_table[i]; }
    printf( "%02i", hdr_ptr->orders_table[i] );
    if ( ( 0 == ( i + 1 ) % 32 ) ) {
      printf( "\n" );
    } else {
      printf( " " );
    }
  }
  int n_patterns = max_pattern + 1;
  printf( "# Patterns:  %u\n", n_patterns );

  // TODO(Anton) UP TO HERE*********

  // Load Pattern Data
  // Note formats newer than MOD/S3M use patterns of variable extended sizes.
  // "at most a pattern could be is 8192 bytes! -
  // and that's a 32 channel pattern with 64 rows and 4 bytes per note)"

  // mem per pattern = n_chans * 4 * 64 * n_patterns
	// pattern i offset = n_chans * 4 * 64 * i; 

	// A note is stored in the actual file as 4 bytes:
	// byte 0   byte 1   byte 2   byte 3
	// aaaaBBBB cccccccc DDDDeeee FFFFFFFF
	// where:
	// aaaaDDDD     = sample number
	// BBBBCCCCCCCC = sample period value (12 bits?) -> convert to a note number
	// eeee         = effect number
	// FFFFFFFF     = effect params ( can later be split into 2 parts for certain effects )

	// loop over all n_patterns
	// - loop over each pattern: 64 * n_chans
	//   - read a 4-byte note
	//   - store sample_number as ( byte[0] & 0xF0 ) + ( byte[2] >> 4 )
	//   - store period_freq   as ( ( byte[0] & 0x0F ) << 8 ) + byte[1]
	//   - store effect_num    as byte[2] & 0x0F 
	//   - store effect_params as byte[3]
	//   - increment to next 4-byte-note

  free( record.data_ptr );
  return true;
}

bool apg_mod_write_file( const char* filename ) {
  if ( !filename ) { return false; }
  return true;
}
