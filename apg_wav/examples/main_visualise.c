/** Example program that plays a sound and also draws the waveform into an image file `out_vis.png`.
 * If the wave file has 2 channels then the second channel is also written, to `out_vis2.png`.
 *
 * For this example install the PortAudio library first:
 * http://portaudio.com/
 * Ubuntu: `sudo apt install portaudio19-dev`

Compile e.g.:

  gcc examples\main_visualise.c apg_wav.c ..\apg_plot\apg_plot.c -o test_visualise_file.exe
  -I . -I ..\apg_plot\ -I ..\third_party\ -I ..\third_party\portaudio\include\ -lm -lportaudio -L .
*/

#include "portaudio.h"
#include "apg_wav.h"
#include "apg_plot.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"
#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// User data struct
typedef struct audio_source_t {
  apg_wav_t wav; // Audio data loaded by apg_wav
  double duration_s;
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

  uint32_t sample_sz = src_ptr->wav.header_ptr->bits_per_sample / 8;

  if ( src_ptr->wav_data_idx + frames_per_buffer * src_ptr->wav.header_ptr->n_chans * sample_sz < actual_data_sz ) {
    memcpy( output_buffer_ptr, &src_ptr->wav.pcm_data_ptr[src_ptr->wav_data_idx], frames_per_buffer * src_ptr->wav.header_ptr->n_chans * sample_sz );
  } else {
    memset( output_buffer_ptr, 0, frames_per_buffer * src_ptr->wav.header_ptr->n_chans * sample_sz );
  }
  src_ptr->wav_data_idx += frames_per_buffer * src_ptr->wav.header_ptr->n_chans * sample_sz;

  return 0;
}

static void _wav_bitmap( apg_wav_t* wav_ptr ) {
  uint32_t actual_data_sz = wav_ptr->header_ptr->file_sz - 44; // 44 is size of WAV PCM header.
  uint32_t sample_sz      = wav_ptr->header_ptr->bits_per_sample / 8;
  int n_channels          = wav_ptr->header_ptr->n_chans;
  uint32_t n_samples      = actual_data_sz / sample_sz;
  printf( "sample size %u, n samples %u\n", sample_sz, n_samples );
  float duration_s = apg_wav_duration( wav_ptr );
  int img_width    = (int)( duration_s * 512.0f );
  int img_height   = 256;

  apg_plot_t chart =
    apg_plot_init( ( apg_plot_params_t ){ .h = img_height, .w = img_width, .max_y = 1.0f, .min_y = -1.0f, .max_x = n_samples / n_channels, .min_x = 0 } );
  apg_plot_t chart2 =
    apg_plot_init( ( apg_plot_params_t ){ .h = img_height, .w = img_width, .max_y = 1.0f, .min_y = -1.0f, .max_x = n_samples / n_channels, .min_x = 0 } );
  apg_plot_background_colour( 0xCC );
  apg_plot_line_colour( 0x00, 0x00, 0xFF );
  apg_plot_x_axis_colour( 0, 0, 0 );
  apg_plot_clear( &chart );
  apg_plot_clear( &chart2 );

  float* xy_ptr = malloc( n_samples / n_channels * wav_ptr->header_ptr->bits_per_sample * 2.0f );
  if ( !xy_ptr ) { return; }
  float* xy_ptr2 = malloc( n_samples / n_channels * wav_ptr->header_ptr->bits_per_sample * 2.0f );
  if ( !xy_ptr2 ) { return; }

  for ( uint32_t i = 0; i < n_samples / n_channels; i++ ) {
    xy_ptr[i * 2 + 0] = (float)i;
    if ( 1 == sample_sz ) { // uint8_t
      xy_ptr[i * 2 + 1] = (float)wav_ptr->pcm_data_ptr[i * sample_sz * n_channels] / 127.5f - 1.0f;
    } else if ( 2 == sample_sz ) { // int16_t signed
      int16_t sample = 0;
      memcpy( &sample, &wav_ptr->pcm_data_ptr[i * sample_sz * n_channels], sample_sz );
      xy_ptr[i * 2 + 1] = (float)sample / 32768.0f;
    }
  }
  if ( n_channels > 1 ) {
    for ( uint32_t i = 0; i < n_samples / n_channels; i++ ) {
      xy_ptr2[i * 2 + 0] = (float)i;
      if ( 1 == sample_sz ) { // uint8_t
        xy_ptr2[i * 2 + 1] = (float)wav_ptr->pcm_data_ptr[( i * n_channels + 1 ) * sample_sz] / 127.5f - 1.0f;
      } else if ( 2 == sample_sz ) { // int16_t signed
        int16_t sample = 0;
        memcpy( &sample, &wav_ptr->pcm_data_ptr[( i * n_channels + 1 ) * sample_sz], sample_sz );
        xy_ptr2[i * 2 + 1] = (float)sample / 32768.0f;
      }
    }
  }

  apg_plot_plot_lines( &chart, xy_ptr, n_samples / n_channels );
  apg_plot_x_axis_draw( &chart, 0.0f );
  stbi_write_png( "out_vis.png", img_width, img_height, 3, chart.rgb_ptr, 3 * img_width );
  if ( n_channels > 1 ) {
    apg_plot_plot_lines( &chart2, xy_ptr2, n_samples / n_channels );
    apg_plot_x_axis_draw( &chart2, 0.0f );
    stbi_write_png( "out_vis2.png", img_width, img_height, 3, chart2.rgb_ptr, 3 * img_width );
  }

  free( xy_ptr );
  free( xy_ptr2 );
  apg_plot_free( &chart );
  apg_plot_free( &chart2 );
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
    fprintf( stderr, "ERROR loading file %s %i\n", filename );
    return 1;
  }
  audio_source.duration_s = apg_wav_duration( &audio_source.wav );

  _wav_bitmap( &audio_source.wav );

  // Start PortAudio
  printf( "%s\n", Pa_GetVersionText() );
  PaError err = Pa_Initialize();
  if ( err != paNoError ) {
    printf( "PortAudio error: %s\n", Pa_GetErrorText( err ) );
    return 1;
  }
  printf( "BPS = %i\n", (int)audio_source.wav.header_ptr->bits_per_sample );
  printf( "#chans = %i\n", (int)audio_source.wav.header_ptr->n_chans );

  PaSampleFormat fmt = paFloat32;
  switch ( audio_source.wav.header_ptr->bits_per_sample ) {
  case 16: fmt = paInt16; break;
  case 8: fmt = paUInt8; break; // 8-bit is unsigned in wav, but 16 is signed.
  default:
    fprintf( stderr, "WARNING: unhandled bits-per-sample of %i detected.\n", audio_source.wav.header_ptr->bits_per_sample );
    fmt = paInt16;
    break; // guess
  }

  PaStream* stream = NULL; // usually just 1 stream per device
  err              = Pa_OpenDefaultStream( &stream,
                 0,                                    // no input channels (mic/record etc)
                 audio_source.wav.header_ptr->n_chans, // mono/stereo
                 fmt,                                  // 8-bit, 16-bit int or 32-bit float supported in this demo
                 audio_source.wav.header_ptr->sample_rate_hz,
                 256, // frames per buffer to request from callback (can use paFramesPerBufferUnspecified)
                 antons_pa_cb, &audio_source );
  if ( err != paNoError ) {
    printf( "PortAudio error: %s\n", Pa_GetErrorText( err ) );
    return 1;
  }

  printf( "sample Hz: %i\n", audio_source.wav.header_ptr->sample_rate_hz );
  printf( "wav duration %lf\n", audio_source.duration_s );

  Pa_StartStream( stream );
  if ( err != paNoError ) {
    printf( "PortAudio error: %s\n", Pa_GetErrorText( err ) );
    return 1;
  }

  Pa_Sleep( audio_source.duration_s * 1000 );

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
