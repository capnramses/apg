/* apg_mod - test/example - play a .mod song using PortAudio.
By Anton Gerdelan

WORK IN PROGRESS

Ubuntu: `sudo apt install portaudio19-dev`

Compile e.g.:
gcc examples/play_song.c apg_mod.c -o play_song.bin -I ./ -lm -lportaudio


TODO

beside every row print:
* ?preload samples into audio buffers e.g. line 29 in tests/dump_wavs.c but into raw buffers?
* play the sample indicated without any extra effect
* work out correct frequency
* remove buffered print statements to improve timing
* do proper channel mixing
* and mix up to 4 channels together
* add advanced effects (maybe loops first)
*/

#include "portaudio.h"
#include "apg_mod.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

PaStream* stream        = NULL;    // Usually just 1 stream per device.
int n_stream_chans      = 1;       // Mono or stereo.
PaSampleFormat fmt      = paInt16; // I can't remember if this is correct but its 16int in dump_wavs.
uint32_t sample_rate_hz = 11025;

/* Callback that feeds wave data into PortAudio's stream during playback. */
static int _pa_cb(                               //
  const void* input_buffer_ptr,                  //
  void* output_buffer_ptr,                       //
  unsigned long frames_per_buffer,               //
  const PaStreamCallbackTimeInfo* time_info_ptr, //
  PaStreamCallbackFlags status_flags,            //
  void* user_data_ptr                            //
) {
  (void)input_buffer_ptr; // suppress unused var warning

  apg_mod_t* mod_ptr = (apg_mod_t*)user_data_ptr;
  assert( mod_ptr );

#if 0 // TODO! Fetch correct row->channel->note->sample

  // because some encoders like ffmpeg break this WAV variable we need to calculate it from the file size.
  //  uint32_t actual_data_sz = src_ptr->wav.header_ptr->file_sz - 44; // 44 is size of WAV PCM header.

  uint32_t sample_sz = src_ptr->wav.header_ptr->bits_per_sample / 8;

  if ( src_ptr->wav_data_idx + frames_per_buffer * src_ptr->wav.header_ptr->n_chans * sample_sz < actual_data_sz ) {
    memcpy( output_buffer_ptr, &src_ptr->wav.pcm_data_ptr[src_ptr->wav_data_idx], frames_per_buffer * src_ptr->wav.header_ptr->n_chans * sample_sz );
  } else {
    memset( output_buffer_ptr, 0, frames_per_buffer * src_ptr->wav.header_ptr->n_chans * sample_sz );
  }
  src_ptr->wav_data_idx += frames_per_buffer * src_ptr->wav.header_ptr->n_chans * sample_sz;
#endif
#if 0
  // TODO WIP Create buffers for sound samples?.
  //
  for ( int i = 0; i < APG_MOD_N_SAMPLES; i++ ) {
    if ( 0 == mod.sample_sz_bytes[i] ) { continue; }
    uint32_t n_samples = mod.sample_sz_bytes[i] / sizeof( int16_t ); // not sure why this exists as a param if it can be derived from other params.

    // TODO put this stuff in a buffer: (I guess 'samples' here is the actual data-points on the sound waveform encoding.

    // bool ret           = apg_wav_write( tmp, mod.sample_data_ptrs[i], mod.sample_sz_bytes[i], 1, sample_rate_hz, n_samples, 16 );
  }
#endif

  return 0;
}

int main( int argc, char** argv ) {
  if ( argc < 2 ) {
    printf( "Usage: %s MYFILE.mod\n", argv[0] );
    return 0;
  }
  const char* filename_ptr = argv[1];

  apg_mod_t mod = ( apg_mod_t ){ .sample_data_ptrs[0] = NULL };
  if ( !apg_mod_read_file( filename_ptr, &mod ) ) {
    fprintf( stderr, "ERROR: could not open file %s\n", filename_ptr );
    return 1;
  }

  printf( "Loaded song: %s\n", mod.song_name );

  // Start PortAudio
  printf( "%s\n", Pa_GetVersionText() );
  PaError err = Pa_Initialize();
  if ( err != paNoError ) {
    printf( "PortAudio error: %s\n", Pa_GetErrorText( err ) );
    return 1;
  }
  // printf( "BPS = %i\n", (int)audio_source.wav.header_ptr->bits_per_sample );

  err = Pa_OpenDefaultStream( //
    &stream,                  //
    0,                        // no input channels (mic/record etc)
    n_stream_chans,           // mono/stereo
    fmt,                      // 8-bit, 16-bit int or 32-bit float supported in this demo
    sample_rate_hz,           //
    256,                      // frames per buffer to request from callback (can use paFramesPerBufferUnspecified)
    _pa_cb,                   //
    &mod                      //
  );
  if ( err != paNoError ) {
    printf( "PortAudio error: %s\n", Pa_GetErrorText( err ) );
    return 1;
  }

  Pa_StartStream( stream );
  if ( err != paNoError ) {
    printf( "PortAudio error: %s\n", Pa_GetErrorText( err ) );
    return 1;
  }

  // Loop over rows in song
  for ( int o_idx = 0; o_idx < mod.n_orders; o_idx++ ) {
    uint8_t p_idx = mod.orders_ptr[o_idx];
    printf( "o%i/%i --p%i--\n", o_idx, mod.n_orders, p_idx );
    for ( int r_idx = 0; r_idx < APG_MOD_N_PATTERN_ROWS; r_idx++ ) {
      printf( "r%2i] ", r_idx );
      // TODO reset channel buffers
      for ( int c_idx = 0; c_idx < mod.n_chans; c_idx++ ) {
        apg_mod_note_t note = ( apg_mod_note_t ){ .sample_idx = 0 };
        if ( apg_mod_fetch_note( &mod, p_idx, r_idx, c_idx, &note ) ) {
          int idx = apg_mod_find_period_table_idx( note.period_value_12b );
          char tmp[4];
          strcpy( tmp, "..." );
          if ( idx > -1 ) { strcpy( tmp, _note_names[idx] ); }
          printf( "%s %2i %2i %2i, ", tmp, note.sample_idx, note.effect_type_4b, note.effect_params );

          // TODO copy note buffer into a channel buffer
        }
      }
      // TODO mix all channel buffers together
      // TODO queue the mixed buffer to the end of the playing feed/buffer
      printf( "\n" );
    }
  }

  Pa_Sleep( 5000 ); // Wait for last note(s) to finish.
  {                 // Shut down PortAudio
    err = Pa_StopStream( stream );
    if ( err != paNoError ) {
      printf( "PortAudio error: %s\n", Pa_GetErrorText( err ) );
      return 1;
    }
    err = Pa_CloseStream( stream );
    if ( err != paNoError ) {
      printf( "PortAudio error: %s\n", Pa_GetErrorText( err ) );
      return 1;
    }
    err = Pa_Terminate();
    if ( err != paNoError ) {
      printf( "PortAudio error: %s\n", Pa_GetErrorText( err ) );
      return 1;
    }
  }

  if ( !apg_mod_free( &mod ) ) {
    fprintf( stderr, "ERROR: could not free mod\n" );
    return 1;
  }

  printf( "Normal exit\n" );
  return 0;
}
