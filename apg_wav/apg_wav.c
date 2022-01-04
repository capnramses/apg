/*****************************************************************************\
Wave file read/write
Anton Gerdelan <antonofnote at gmail>
C99
Version 0.2
Licence: see header.
\*****************************************************************************/

#include "apg_wav.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Convenience struct and file->memory function.
typedef struct entire_file_t {
  void* data_ptr;
  size_t sz;
} entire_file_t;

static bool _read_entire_file( const char* filename, entire_file_t* record_ptr ) {
  FILE* fp = fopen( filename, "rb" );
  if ( !fp ) {
    fprintf( stderr, "ERROR: opening file for reading `%s`\n", filename );
    return false;
  }
  fseek( fp, 0L, SEEK_END );
  record_ptr->sz       = (size_t)ftell( fp );
  record_ptr->data_ptr = malloc( record_ptr->sz );
  if ( !record_ptr->data_ptr ) {
    fclose( fp );
    return false;
  }
  rewind( fp );
  size_t nr = fread( record_ptr->data_ptr, record_ptr->sz, 1, fp );
  fclose( fp );
  if ( 1 != nr ) { return false; }
  return true;
}

bool apg_wav_write( const char* filename, const void* data_ptr, uint32_t data_sz, uint16_t n_chans, uint32_t sample_rate_hz, uint32_t n_samples, uint16_t bits_per_sample ) {
  if ( !filename || !data_ptr || 0 == data_sz || 0 == n_chans || 0 == sample_rate_hz || 0 == bits_per_sample ) { return false; }
  if ( 0 != bits_per_sample % 8 ) { return false; }

  uint32_t bytes_per_sample   = bits_per_sample / 8;
  apg_wav_header_t wav_header = ( apg_wav_header_t ){
    .file_sz         = data_sz + 44,                                //
    .fmt_sz          = 16,                                          //
    .fmt_type        = 1,                                           //
    .n_chans         = n_chans,                                     //
    .sample_rate_hz  = sample_rate_hz,                              //
    .byte_rate       = sample_rate_hz * n_chans * bytes_per_sample, //
    .block_align     = ( bits_per_sample * n_chans ) / 8,           //
    .bits_per_sample = bits_per_sample,                             //
    .data_sz         = data_sz                                      //
  };
  memcpy( wav_header.riff_magic_num, "RIFF", 4 );
  memcpy( wav_header.wave_magic_num, "WAVE", 4 );
  memcpy( wav_header.fmt_magic_num, "fmt ", 4 ); // trailing space is required (erroneously described as a 'null' in other docs).
  memcpy( wav_header.data_magic_num, "data", 4 );

  assert( 44 == sizeof( apg_wav_header_t ) );
  assert( data_sz == n_samples * n_chans * bytes_per_sample );

  { // File I/O
    FILE* fp = fopen( filename, "wb" );
    if ( !fp ) { return false; }
    size_t n = fwrite( &wav_header, sizeof( apg_wav_header_t ), 1, fp );
    if ( n != 1 ) {
      fclose( fp );
      return false;
    }
    n = fwrite( data_ptr, data_sz, 1, fp );
    if ( n != 1 ) {
      fclose( fp );
      return false;
    }
    fclose( fp );
  }
  return true;
}

bool apg_wav_read( const char* filename, apg_wav_t* wav_ptr ) {
  if ( !filename || !wav_ptr ) { return 0; }

  memset( wav_ptr, 0, sizeof( apg_wav_t ) );

  entire_file_t record;
  bool ret = _read_entire_file( filename, &record );
  if ( !ret || 0 == record.sz ) {
    fprintf( stderr, "ERROR: Reading file\n" );
    return false;
  }
  // too small for any data
  if ( record.sz <= 44 ) {
    free( record.data_ptr );
    fprintf( stderr, "ERROR: file sz = %i, smaller than WAVE header of 44 bytes\n", (int)record.sz );
    return false;
  }
  uint8_t* byte_ptr = record.data_ptr;

  wav_ptr->header_ptr = (apg_wav_header_t*)record.data_ptr;
  // too small for reported data
  if ( record.sz < wav_ptr->header_ptr->file_sz || record.sz < wav_ptr->header_ptr->data_sz + 44 ) { return false; }
  // unsupported type
  if ( wav_ptr->header_ptr->fmt_type != 1 || wav_ptr->header_ptr->fmt_sz != 16 ) {
    fprintf( stderr, "ERROR: fmt_type = %i (expected 1), fmt_sz = %i (expected 16)\n", (int)wav_ptr->header_ptr->fmt_type, wav_ptr->header_ptr->fmt_sz );
    return false;
  }

  wav_ptr->file_data_ptr = byte_ptr;
  wav_ptr->pcm_data_ptr  = &byte_ptr[44];

  return true;
}

bool apg_wav_free( apg_wav_t* wav_ptr ) {
  if ( !wav_ptr || wav_ptr->file_data_ptr ) { return false; }

  free( wav_ptr->file_data_ptr );
  memset( wav_ptr, 0, sizeof( apg_wav_t ) );

  return true;
}
