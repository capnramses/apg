// For this example install the PortAudio library first:
// http://portaudio.com/
// Ubuntu: `sudo apt install portaudio19-dev`
// Compile e.g.:
// gcc examples/main_play_file.c apg_wav.c -o test_play_file.bin -I ./ -lm -lportaudio

#include "portaudio.h"
#include "apg_wav.h"
#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// User data struct
typedef struct audio_source_t {
  apg_wav_t wav;         // Audio data loaded by apg_wav
  uint32_t wav_data_idx; // Counter of where my wav is up to in playback of bytes.
} audio_source_t;

// Callback that feeds wave data into port audio's stream during playback
static int antons_pa_cb( const void* input_buffer_ptr, void* output_buffer_ptr, unsigned long frames_per_buffer, const PaStreamCallbackTimeInfo* time_info_ptr,
  PaStreamCallbackFlags status_flags, void* user_data_ptr ) {
  (void)input_buffer_ptr; // suppress unused var warning

  audio_source_t* src_ptr = (audio_source_t*)user_data_ptr;
  assert( src_ptr );

  // because some encoders like ffmpeg break this WAV variable we need to calculate it from the file size.
  uint32_t actual_data_sz = src_ptr->wav.header_ptr->file_sz - 44; // 44 is size of WAV PCM header.

  uint32_t sample_sz = 4;
  if ( src_ptr->wav.header_ptr->bits_per_sample == 16 ) { sample_sz = 2; }

  if ( src_ptr->wav_data_idx + frames_per_buffer * src_ptr->wav.header_ptr->n_chans * sample_sz < actual_data_sz ) {
    memcpy( output_buffer_ptr, &src_ptr->wav.pcm_data_ptr[src_ptr->wav_data_idx], frames_per_buffer * src_ptr->wav.header_ptr->n_chans * sample_sz );
  } else {
    memset( output_buffer_ptr, 0, frames_per_buffer * src_ptr->wav.header_ptr->n_chans * sample_sz );
  }
  src_ptr->wav_data_idx += frames_per_buffer * src_ptr->wav.header_ptr->n_chans * sample_sz;

  return 0;
}

int main( int argc, char** argv ) {
  if ( argc < 2 ) {
    printf( "usage: %s FILE.wav\n", argv[0] );
    return 0;
  }
  const char* filename = argv[1];

  // Load a WAV file from disk
  printf( "loading `%s`\n", filename );
  audio_source_t audio_source = ( audio_source_t ){ .wav_data_idx = 0 };
  bool res                    = apg_wav_read( filename, &audio_source.wav );
  if ( !res ) {
    fprintf( stderr, "ERROR loading file %s\n", filename );
    return 1;
  }

  // Start PortAudio
  printf( "%s\n", Pa_GetVersionText() );
  PaError err = Pa_Initialize();
  if ( err != paNoError ) {
    printf( "PortAudio error: %s\n", Pa_GetErrorText( err ) );
    return 1;
  }

  PaSampleFormat fmt = paFloat32;
  if ( 16 == audio_source.wav.header_ptr->bits_per_sample ) { fmt = paInt16; }
  PaStream* stream = NULL; // usually just 1 stream per device

  err = Pa_OpenDefaultStream( &stream,
    0,                                    // no input channels (mic/record etc)
    audio_source.wav.header_ptr->n_chans, // mono/stereo
    fmt,                                  // 16-bit int or 32-bit float supported in this demo
    audio_source.wav.header_ptr->sample_rate_hz,
    256, // frames per buffer to request from callback (can use paFramesPerBufferUnspecified)
    antons_pa_cb, &audio_source );
  if ( err != paNoError ) {
    printf( "PortAudio error: %s\n", Pa_GetErrorText( err ) );
    return 1;
  }

  Pa_StartStream( stream );
  if ( err != paNoError ) {
    printf( "PortAudio error: %s\n", Pa_GetErrorText( err ) );
    return 1;
  }

  // TODO(Anton) make a function work out actual duration of wav here (work out n_samples then -> n_samples / sample_rate in Hz = duration in s ).
  int n_seconds = 10;
  Pa_Sleep( n_seconds * 1000 );

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

  res = apg_wav_free( &audio_source.wav );
  if ( !res ) {
    fprintf( stderr, "ERROR freeing wav resources\n" );
    return false;
  }

  printf( "normal exit\n" );

  return 0;
}
