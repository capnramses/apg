/*****************************************************************************\
apg_wav - Waveform Audio File Format read & write
Anton Gerdelan <antonofnote at gmail> 2022
C99
Licence: See bottom of this file or LICENSE file.

History
-------
0.2.1   - 17 Jan 2022 - Duration function. Degrated some file parsing errors to
                        warnings.
0.2     - 04 Jan 2022 - Tidied up and testing reading with a PortAudio example.
0.1     - ???         - First version on GitHub.
\*****************************************************************************/

#ifndef _APG_WAV_H_
#define _APG_WAV_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

#pragma pack( push, 1 )
/// Header from a .wav file. 44-bytes.
typedef struct apg_wav_header_t {
  char riff_magic_num[4];   // "RIFF"
  uint32_t file_sz;         // Overall file size in bytes.
  char wave_magic_num[4];   // "WAVE"
  char fmt_magic_num[4];    // "fmt\0" includes trailing null. Marks start of format subchunk.
  uint32_t fmt_sz;          // Size of this format subchunk in bytes.
  uint16_t fmt_type;        // 1=PCM. Other numbers indicate compression (not supported).
  uint16_t n_chans;         // Number of audio channels. 2 for stereo.
  uint32_t sample_rate_hz;  // 44100 (CD) 48000 (DAT) ...
  uint32_t byte_rate;       // ( sample_rate_hz * bytes per sample * n_chans )
  uint16_t block_align;     // (BitsPerSample * Channels) / 8. ->> 1 = 8-bit mono. 2 = 8-bit stereo. 3 = 8-bit stereo or 16-bit mono. 4 = 16-bit stereo.
  uint16_t bits_per_sample; // Usually 16 but can be 8.
  char data_magic_num[4];   // "data"
  uint32_t data_sz;         // Size of PCM data section in bytes.
} apg_wav_header_t;
#pragma pack( pop )

typedef struct apg_wav_t {
  apg_wav_header_t* header_ptr; // Use for playback settings.
  uint8_t* file_data_ptr;       // Pointer to entire file
  uint8_t* pcm_data_ptr;        // Pointer to the data section (44 bytes on from top of file data).
} apg_wav_t;

/// Information returned when reading a file that can be used for playing it back correctly.

bool apg_wav_write( const char* filename, const void* data_ptr, uint32_t data_sz, uint16_t n_chans, uint32_t sample_rate_hz, uint32_t n_samples, uint16_t bits_per_sample );

/**
 * @param wav_data_ptr Pointer to a previously unused struct to fill out, including allocating memory to store file contents.
 * @return Returns false on error.
 * @warning This function allocates memory. Call apg_wav_free() on your apg_wav_data_t when done with playback, to release memory.
 */
bool apg_wav_read( const char* filename, apg_wav_t* wav_data_ptr );

double apg_wav_duration( const apg_wav_t* wav_ptr );

bool apg_wav_free( apg_wav_t* wav_data_ptr );

#ifdef __cplusplus
}
#endif

#endif

/*
-------------------------------------------------------------------------------------
This software is available under two licences - you may use it under either licence.
-------------------------------------------------------------------------------------
FIRST LICENCE OPTION

>                                  Apache License
>                            Version 2.0, January 2004
>                         http://www.apache.org/licenses/
>    Copyright 2019 Anton Gerdelan.
>    Licensed under the Apache License, Version 2.0 (the "License");
>    you may not use this file except in compliance with the License.
>    You may obtain a copy of the License at
>        http://www.apache.org/licenses/LICENSE-2.0
>    Unless required by applicable law or agreed to in writing, software
>    distributed under the License is distributed on an "AS IS" BASIS,
>    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
>    See the License for the specific language governing permissions and
>    limitations under the License.
-------------------------------------------------------------------------------------
SECOND LICENCE OPTION

> This is free and unencumbered software released into the public domain.
>
> Anyone is free to copy, modify, publish, use, compile, sell, or
> distribute this software, either in source code form or as a compiled
> binary, for any purpose, commercial or non-commercial, and by any
> means.
>
> In jurisdictions that recognize copyright laws, the author or authors
> of this software dedicate any and all copyright interest in the
> software to the public domain. We make this dedication for the benefit
> of the public at large and to the detriment of our heirs and
> successors. We intend this dedication to be an overt act of
> relinquishment in perpetuity of all present and future rights to this
> software under copyright law.
>
> THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
> EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
> MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
> IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
> OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
> ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
> OTHER DEALINGS IN THE SOFTWARE.
>
> For more information, please refer to <http://unlicense.org>
-------------------------------------------------------------------------------------
*/
