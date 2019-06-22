#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

struct wav_chunk_descr_t {
  char riff_str[4];  // "RIFF"
  uint32_t chunk_sz; // Size of file in bytes, not including 8: riff_str and chunk_sz
  char wave_str[4];  // "WAVE"
};

struct wav_fmt_subchunk_t {
  char fmt_str[4];          // "fmt " - last char
  uint32_t subchunk_1_sz;   // PCM is 16 ( sz of this subchunk following this var )
  uint16_t audio_fmt;       // 1 = PCM. other values indicate a compression alg
  uint16_t n_chans;         // 1,2,...
  uint32_t sample_rate;     // 8000, 44100 etc
  uint32_t byte_rate;       // sample rate * n chans * bits_per_sample/8
  uint16_t block_align;     // NOTE(Anton) not sure the calc for this is correct. probably shd be eg 4 or 1
  uint16_t bits_per_sample; // 8, 16 etc.
  // NOTE: non-PCM have additional params here
};

// of size given by subchunk_2_sz
struct wav_data_subchunk_t {
  char data_str[4]; // "data"
  uint32_t subchunk_2_sz;

  // includes size of uint8_t* data;
};

/* convenience struct and file->memory function */
struct entire_file_t {
  void* data;
  size_t sz;
};

/* this could be replaced with a project-specific method */
static bool _read_entire_file( const char* filename, struct entire_file_t* record ) {
  FILE* fp = fopen( filename, "rb" );
  if ( !fp ) {
    fprintf( stderr, "ERROR: opening file for reading `%s`\n", filename );
    return false;
  }
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

int apg_write_wav( const char* filename, const void* data, int n_chans, int sample_rate, int n_samples, int bits_per_sample ) {
  if ( !filename || n_samples <= 0 || bits_per_sample <= 0 ) { return 0; }
  if ( 0 != bits_per_sample % 8 ) { return 0; }

  struct wav_chunk_descr_t chunk_descr;
  struct wav_fmt_subchunk_t fmt_subchunk;
  struct wav_data_subchunk_t data_subchunk;
  const char* riff_str = "RIFF";
  const char* wave_str = "WAVE";
  const char* fmt_str  = "fmt ";
  const char* data_str = "data";
  int bytes_per_sample = bits_per_sample / 8;
  { // build format subchunk
    memcpy( fmt_subchunk.fmt_str, fmt_str, 4 );
    fmt_subchunk.subchunk_1_sz   = 16;
    fmt_subchunk.audio_fmt       = 1;
    fmt_subchunk.n_chans         = n_chans;
    fmt_subchunk.sample_rate     = sample_rate;
    fmt_subchunk.byte_rate       = sample_rate * n_chans * bytes_per_sample;
    fmt_subchunk.block_align     = n_chans * bytes_per_sample;
    fmt_subchunk.bits_per_sample = bits_per_sample;
  }
  { // build data subchunk
    memcpy( data_subchunk.data_str, data_str, 4 );
    data_subchunk.subchunk_2_sz = n_samples * n_chans * bytes_per_sample;
  }
  { // build description subchunk
    memcpy( chunk_descr.riff_str, riff_str, 4 );
    chunk_descr.chunk_sz = 4 + ( 8 + fmt_subchunk.subchunk_1_sz ) + ( 8 + data_subchunk.subchunk_2_sz );
    memcpy( chunk_descr.wave_str, wave_str, 4 );
  }
  { // File I/O
    FILE* fp = fopen( filename, "wb" );
    if ( !fp ) { return 0; }
    size_t n = fwrite( &chunk_descr, sizeof( struct wav_chunk_descr_t ), 1, fp );
    if ( !n ) {
      fclose( fp );
      return 0;
    }
    n = fwrite( &fmt_subchunk, sizeof( struct wav_fmt_subchunk_t ), 1, fp );
    if ( !n ) {
      fclose( fp );
      return 0;
    }
    n = fwrite( &data_subchunk, sizeof( struct wav_data_subchunk_t ), 1, fp );
    if ( !n ) {
      fclose( fp );
      return 0;
    }
    n = fwrite( data, data_subchunk.subchunk_2_sz, 1, fp );
    if ( !n ) {
      fclose( fp );
      return 0;
    }
    fclose( fp );
  }
  return 1;
}

unsigned char* apg_read_wav( const char* filename, int* n_chans, int* sample_rate, int* n_samples, int* bits_per_sample ) {
  if ( !filename || !n_chans || !sample_rate || !n_samples || !bits_per_sample ) { return 0; }
  struct entire_file_t record;
  bool ret = _read_entire_file( filename, &record );
  if ( !ret || 0 == record.sz ) { return 0; }
  // TODO validate each subchunk
  struct wav_chunk_descr_t* chunk_descr   = (struct wav_chunk_descr_t*)record.data;
  struct wav_fmt_subchunk_t* fmt_subchunk = (struct wav_fmt_subchunk_t*)( (uint8_t*)record.data + sizeof( struct wav_chunk_descr_t ) );
  struct wav_data_subchunk_t* data_subchunk =
    (struct wav_data_subchunk_t*)( (uint8_t*)record.data + sizeof( struct wav_fmt_subchunk_t ) + sizeof( struct wav_chunk_descr_t ) );
  unsigned char* wav_data =
    (unsigned char*)( (uint8_t*)record.data + sizeof( struct wav_fmt_subchunk_t ) + sizeof( struct wav_chunk_descr_t ) + sizeof( struct wav_data_subchunk_t ) );
  *n_chans         = fmt_subchunk->n_chans;
  *sample_rate     = fmt_subchunk->sample_rate;
  *bits_per_sample = fmt_subchunk->bits_per_sample;
  // TODO derive *n_samples = fmt_subchunk->n_samples;

  // TODO malloc the data separately
  return wav_data;
}
