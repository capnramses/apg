/*****************************************************************************\
apg_mod - description to go here
Anton Gerdelan <antonofnote at gmail>
C99
Version 0.0
Licence: see header.
\*****************************************************************************/

#include "apg_mod.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// note: samples have no fixed frequency. there's a formula to work it out based on period, and a magic number for PAL or NSTC

// https://github.com/DaveyPocket/Amigo/blob/master/mod-spec.txt

// Note the module files are Motorola format (big-endian) so any variables >1 byte need a conversion (there are only a few).
// 30 bytes.
typedef struct sample_t {
  char _name[APG_MOD_SAMPLE_NAME_LEN]; // WARNING: Does not have a terminating \0.
  uint16_t length_be;                  // This is the number of 2-byte *words* - multiply x2 to get bytes. WARNING: Big-endian - swap the bytes for LE.
  uint8_t finetune;                    // Lower 4 bits stored as signed 4-bit number. Upper 4-bits are set to 0.
  uint8_t volume;                      // Range is 0x00-0x40 or 0-64 decimal.
  uint16_t loop_start_be;              // n words offset from start of sample. x2 = offset in bytes. WARNING: Big-endian - swap the bytes for LE.
  uint16_t loop_length_be;             // n of words. x2 = offset in bytes.  WARNING: Big-endian - swap the bytes for LE.
} sample_t;

// The MOD format is headerless so not technically a 'header' but I refer to it as one.
// This seems to reliably map to .mod files, but not "Extended Module" .xm files, which
// look like they have more data per field e.g. longer songname etc.
// 1084 bytes.
typedef struct protracker_1_1b_hdr_t {
  char _songname[APG_MOD_SONG_NAME_LEN]; // Should include trailing nul byte(s).
  sample_t samples[APG_MOD_N_SAMPLES];   // Sample numbers are 1-31. Early versions had only 15 samples.
  uint8_t song_length; // Range is 1-128. This is the number of pattern orders (from orders_table) to play in the song, including repeated patterns.
  uint8_t unused;      // Set to 127 to make old trackers parse all patterns. PT used it to mean 'restart'.
  uint8_t orders_table[APG_MOD_ORDERS_MAX]; // Positions 0-127.  Values are a number 0-63 to indicate pattern to play at that position.
  char magicletters[4]; // Address 1080. "M.K." (or "FLT4" or "FLT8"). If not here assume song uses only 15 samples or text was removed to obfuscate the music.
} protracker_1_1b_hdr_t;

typedef struct record_t {
  void* data_ptr;
  size_t sz;
} record_t;

// TODO(Anton) if M.K. not there at all assume it's older 15-samples?
static char _magic_strs[APG_MOD_FMT_MAX][4]    = { "M.K.", "2CHN", "6CHN", "8CHN", "CD81", "FLT4", "FLT8", "M!K!", "OCTA", "TDZx", "????" };
static int _magic_str_n_chans[APG_MOD_FMT_MAX] = { 4, 2, 6, 8, 8, 4, 8, 0, 8, 0, 0 };

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

static apg_mod_fmt_t _mod_type( char magic_str[4], int* n_chans ) {
  for ( int i = 0; i < APG_MOD_FMT_MAX; i++ ) {
    if ( strncmp( magic_str, _magic_strs[i], 4 ) == 0 ) {
      *n_chans = _magic_str_n_chans[i];
      return (apg_mod_fmt_t)i;
    }
  }
  if ( magic_str[0] == 'T' && magic_str[1] == 'D' && magic_str[2] == 'Z' && isdigit( magic_str[3] ) ) {
    *n_chans = magic_str[3] - '0';
    return APG_MOD_FMT_TAKETRACKER_xCH;
  } else if ( isdigit( magic_str[0] ) && magic_str[1] == 'C' && magic_str[2] == 'H' && magic_str[3] == 'N' ) {
    *n_chans = magic_str[0] - '0';
    return APG_MOD_FMT_TAKETRACKER_xCH;
  } else if ( isdigit( magic_str[0] ) && isdigit( magic_str[1] ) && magic_str[2] == 'C' && magic_str[3] == 'H' ) {
    *n_chans = atoi( magic_str );
    return APG_MOD_FMT_TAKETRACKER_xCH;
  }
  return APG_MOD_FMT_UNKNOWN;
}

/* "To get the real value in bytes, calculate it with (byte1*100h + byte2) * 2"
 * Applies to sample name, sample length, loop start, loop length.
 */
static uint32_t _words_val_to_bytes_le( uint16_t word_be ) {
  uint16_t le = ( word_be << 8 ) | ( word_be >> 8 );
  return (uint32_t)le * 2; // and x2 to get value in bytes
}

bool apg_mod_fetch_note( const apg_mod_t* mod_ptr, int pattern_idx, int row_idx, int channel_idx, apg_mod_note_t* note_ptr ) {
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

  // Each note is stored as 4 bytes, and all four notes at each position in the pattern are stored after each other.

  // example code for notes is here https://github.com/DaveyPocket/Amigo/blob/master/mod-spec.txt

  if ( !mod_ptr || !mod_ptr->mod_data_ptr || !note_ptr ) { return false; }

  uint32_t offset = 1084 + pattern_idx * row_idx * channel_idx * APG_MOD_N_NOTE_BYTES;
  if ( offset + 4 > mod_ptr->mod_data_sz ) {
    fprintf( stderr, "ERROR fetching note - past EOF.\n" );
    return false;
  }

  const uint8_t* byte_ptr    = mod_ptr->mod_data_ptr;
  const uint8_t* note_bytes  = &byte_ptr[offset];
  note_ptr->sample_idx       = ( note_bytes[0] & 0xF0 ) + ( note_bytes[2] >> 4 );
  note_ptr->period_value_12b = ( ( note_bytes[0] & 0x0F ) << 8 ) + note_bytes[1];
  note_ptr->effect_type_4b   = note_bytes[2] & 0x0F;
  note_ptr->effect_params    = note_bytes[3];

  return true;
}

/* Sample finetune values are stored as 4-bit signed integers: 0,1,2,3,4,5,6,7,-8,-7,-6,-5,-4,-3,-2,-1
 * NOTE(Anton) this could be a #define for an inline. */
// static int _fine_tune_bits_to_int( uint8_t finetune ) { return finetune < 8 ? (int)finetune : (int)finetune - 16; }

bool apg_mod_read_file( const char* filename, apg_mod_t* mod_ptr ) {
  if ( !filename || !mod_ptr ) { return false; }

  record_t record = _read_entire_file( filename );
  if ( !record.data_ptr ) { return false; }
  uint8_t* byte_ptr = record.data_ptr;

  if ( record.sz < 1084 ) {
    fprintf( stderr, "ERROR: File too small to be a song module with data\n" );
    free( record.data_ptr );
    return false;
  }

  mod_ptr->mod_data_ptr = record.data_ptr;
  mod_ptr->mod_data_sz  = record.sz;

  protracker_1_1b_hdr_t* hdr_ptr = (protracker_1_1b_hdr_t*)record.data_ptr;

  mod_ptr->n_chans = 0;
  mod_ptr->mod_fmt = _mod_type( hdr_ptr->magicletters, &mod_ptr->n_chans );
  if ( mod_ptr->mod_fmt == APG_MOD_FMT_UNKNOWN ) {
    fprintf( stderr, "Module format %c%c%c%c unknown.\n", hdr_ptr->magicletters[0], hdr_ptr->magicletters[1], hdr_ptr->magicletters[2], hdr_ptr->magicletters[3] );
    free( record.data_ptr );
    return false;
  }
  printf( "Module format type %i: %c%c%c%c.\n", (int)mod_ptr->mod_fmt, hdr_ptr->magicletters[0], hdr_ptr->magicletters[1], hdr_ptr->magicletters[2],
    hdr_ptr->magicletters[3] );
  printf( "# Channels:  %i\n", mod_ptr->n_chans );

  mod_ptr->song_name[APG_MOD_SONG_NAME_LEN] = '\0'; // Song names are usually not nul-terminated.
  memcpy( mod_ptr->song_name, hdr_ptr->_songname, APG_MOD_SONG_NAME_LEN );

  printf( "Song name:   \"%s\"\n", mod_ptr->song_name );
  printf( "Song length: %u\n", (uint32_t)hdr_ptr->song_length );

  mod_ptr->orders_ptr = hdr_ptr->orders_table;
  mod_ptr->n_orders   = hdr_ptr->song_length;

  // Determine n of patterns stored in file by looking through order table for biggest pattern index played.
  int max_pattern = 0;
  printf( "Orders Table:\n" );
  // NOTE(Anton) could probably stop at song_length here, not the full 128
  for ( int i = 0; i < APG_MOD_ORDERS_MAX; i++ ) {
    if ( hdr_ptr->orders_table[i] > max_pattern ) { max_pattern = hdr_ptr->orders_table[i]; }
    printf( "%02i", hdr_ptr->orders_table[i] );
    if ( ( 0 == ( i + 1 ) % 32 ) ) {
      printf( "\n" );
    } else {
      printf( " " );
    }
  }
  mod_ptr->n_patterns = max_pattern + 1;
  printf( "# Patterns:  %u\n", mod_ptr->n_patterns );

  // samples are stored after the patterns.
  // samples always start with 2 zeroes

  // TODO(Anton)! if older type then offset is only + 600 not + 1084!
  uint32_t offset = 1084 + mod_ptr->n_patterns * APG_MOD_N_PATTERN_ROWS * mod_ptr->n_chans * APG_MOD_N_NOTE_BYTES; // 1024 * n_patterns + header's offset of 1084

  printf( "Samples:\n" );
  // offset &or size is slightly off here somehow
  // some samples i output seemed to be correct as 8-bit signed 1200Hz or 8000Hz PCM waves.
  for ( int i = 0; i < APG_MOD_N_SAMPLES; i++ ) {
    mod_ptr->sample_names[i][APG_MOD_SAMPLE_NAME_LEN] = '\0';
    memcpy( mod_ptr->sample_names[i], hdr_ptr->samples[i]._name, APG_MOD_SAMPLE_NAME_LEN );

    mod_ptr->sample_data_ptrs[i] = (int8_t*)&byte_ptr[offset];
    mod_ptr->sample_sz_bytes[i]  = _words_val_to_bytes_le( hdr_ptr->samples[i].length_be );
    if ( offset + mod_ptr->sample_sz_bytes[i] > record.sz ) {
      fprintf( stderr, "Sample is outside range of file memory - looks like a corrupted file or wrong format.\n" );
      return false;
    }
#define PRINT_SAMPLE_NAMES
#ifdef PRINT_SAMPLE_NAMES
    if ( mod_ptr->sample_sz_bytes[i] > 0 ) { printf( "  Sample %i name: \"%s\"\n", i + 1, mod_ptr->sample_names[i] ); }
#endif
#ifdef DUMP_RAW_SAMPLES
    if ( mod_ptr->sample_sz_bytes[i] != 0 ) {
      if ( offset + mod_ptr->sample_sz_bytes[i] > record.sz ) {
        fprintf( stderr, "sample to write is outside range of file memory.\n" );
        return false;
      }
      printf( "address at data_ptr[%u] is %p\n", offset, (void*)&byte_ptr[offset] );
      char tmp[64];
      sprintf( tmp, "sample%i.raw", i );
      FILE* of_ptr = fopen( tmp, "wb" );
      if ( !of_ptr ) { return false; }
      printf( "writing %s size %u\n", tmp, mod_ptr->sample_sz_bytes[i] );
      int n = fwrite( &byte_ptr[offset], mod_ptr->sample_sz_bytes[i], 1, of_ptr );
      if ( 1 != n ) { return false; }
      fclose( of_ptr );
    }
#endif
#ifdef PRINT_SAMPLE_INFO
    printf( "    Length (bytes): %u\n", sample_sz );
    printf( "    Finetune:       %u\n", (uint32_t)hdr_ptr->samples[i].finetune );
    printf( "    Volume:         %u\n", (uint32_t)hdr_ptr->samples[i].finetune );
    printf( "    Loop point:     %u\n", (uint32_t)hdr_ptr->samples[i].loop_start_be );
    printf( "    Loop length:    %u\n", (uint32_t)hdr_ptr->samples[i].loop_length_be );
#endif

    offset += mod_ptr->sample_sz_bytes[i];
  } // endfor N_SAMPLES

//#define PRINT_NOTES
#ifdef PRINT_NOTES
  {
    for ( int p = 0; p < mod_ptr->n_patterns; p++ ) {
      for ( int r = 0; r < APG_MOD_N_PATTERN_ROWS; r++ ) {
        apg_mod_note_t note;
        for ( int c = 0; c < mod_ptr->n_chans; c++ ) {
          bool ret = apg_mod_fetch_note( mod_ptr, p, r, c, &note );
          if ( !ret ) { return false; }
          if ( mod_ptr->sample_sz_bytes[note.sample_idx] > 0 ) {
            printf( "Note p%i r%i c%i = sam%u per%u eff%u par%u\n", p, r, c, (uint32_t)note.sample_idx, (uint32_t)note.period_value_12b,
              (uint32_t)note.effect_type_4b, (uint32_t)note.effect_params );
          }
        }
      }
    }
  }
#endif

  return true;
}

bool apg_mod_free( apg_mod_t* mod_ptr ) {
  if ( !mod_ptr || !mod_ptr->mod_data_ptr ) { return false; }

  free( mod_ptr->mod_data_ptr );
  *mod_ptr = ( apg_mod_t ){ .mod_data_ptr = NULL };

  return true;
}

/*
bool apg_mod_write_file( const char* filename ) {
  if ( !filename ) { return false; }
  return true;
}
*/
