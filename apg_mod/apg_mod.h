/*****************************************************************************\
apg_mod - description to go here
Anton Gerdelan <antonofnote at gmail> 2022
C99
Licence: See bottom of this file or LICENSE file.

Status: Not functional yet.

What is this library?
---------------------

This is an Amiga music module (.mod) loader I am making out of curiosity. I
never had an Amiga or played module files but I am finding audio programming
(see apg_wav), and dealing with old file formats (see also apg_bmp, apg_tga)
interesting.

The design philosophy of the library is similar to my other apg library projects:

* Simple C code with no dependencies and a stable ABI.
* Minimal run-time memory allocation. Just one memory allocation call is made
  when loading the file.
* Some convenience variables or functions to convert to C-strings and
  little-endian data-types.
* Otherwise just index directly into the file's memory during playback, as you
  could in the original trackers.

What are Modules?
---------------------

Modules are music files started by Karsten Obarski's "Ultimate SoundTracker"
for Commodore Amiga in 1987. They are similar to MIDI files, except they
store the sample sounds (called "instruments" in MIDI) in the music file
itself, rather than relying on sound banks that come with the sound device.
That means they always sound the same on different devices, and can have
custom digital samples such as vocal recordings.

Format details here:
http://fileformats.archiveteam.org/wiki/Amiga_Module

ToDo
---------------------

* Move raw sample rip into a test/ or examples/ program.
* Figure out correct frequency for each sample and create a .wav rip test/
  program.
* Figure out the playback row advance speed and whether to pin it to PAL or NTSC.
* Write a full tracker/player example program using PortAudio.
* Add repeat/loop features to samples.
* Add effects and finetune values to samples.
* Graphical tracker player demo (probably pulseaudio+allegro/sdl/opengl or similar).
* Interactive visual track editor demo.
* File writing.
* Maybe try web asm -> web player?
* Support other module formats:
  * IT support (Impulse Tracker) https://en.wikipedia.org/wiki/Impulse_Tracker#IT_file_format
  * S3M (Scream Tracker) https://en.wikipedia.org/wiki/S3M_(file_format)
  * XM support (Extended Module - Triton's FastTracker 2) https://en.wikipedia.org/wiki/XM_(file_format)
  * Other formats here https://en.wikipedia.org/wiki/Module_file
History
---------------------

0.0 - 10 Jan 2022 - Added skeleton to apg_libraries.
\*****************************************************************************/

#ifndef _APG_MOD_H_
#define _APG_MOD_H_

#ifdef _WIN32
/** Explicit symbol export for building .DLLs with MSVC so it generates a corresponding .LIB. */
#define APG_MOD_EXPORT __declspec( dllexport )
#else
#define APG_MOD_EXPORT
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

#define APG_MOD_N_SAMPLES 31       // Default
#define APG_MOD_SONG_NAME_LEN 20   // Default
#define APG_MOD_ORDERS_MAX 128     // Max song length. Default 128.
#define APG_MOD_N_PATTERN_ROWS 64  // Default
#define APG_MOD_SAMPLE_NAME_LEN 22 // Default
#define APG_MOD_N_NOTE_BYTES 4     // Default

typedef enum apg_mod_fmt_t {
  APG_MOD_FMT_STD_4CH, // Original M.K. 4-channel .MOD
  APG_MOD_FMT_FASTTRACKER_2CH,
  APG_MOD_FMT_FASTTRACKER_6CH,
  APG_MOD_FMT_FASTTRACKER_8CH,
  APG_MOD_FMT_FALCON_8CH,
  APG_MOD_FMT_STARTREKKER_4CH,
  APG_MOD_FMT_STARTREKKER_8CH,
  APG_MOD_FMT_PROTRACKER_GT64CH,
  APG_MOD_FMT_OCTA_8CH,
  APG_MOD_FMT_TAKETRACKER_xCH,
  APG_MOD_FMT_UNKNOWN,
  APG_MOD_FMT_MAX
} apg_mod_fmt_t;

APG_MOD_EXPORT typedef struct apg_mod_t {
  // Module
  void* mod_data_ptr;                        // The entire file loaded into memory. Motorola byte-format (big-endian).
  uint32_t mod_data_sz;                      // Size of the entire module file.
  apg_mod_fmt_t mod_fmt;                     // What type of module was detected. Default APG_MOD_FMT_STD_4CH.
  int n_chans;                               // Number of channels (columns) in each pattern. Default 4.
  char song_name[APG_MOD_SONG_NAME_LEN + 1]; // Song name with nul-terminator appended so it can be used as a C-string.

  // Orders Table - which order to play the patterns (verses). They can repeat.
  uint8_t* orders_ptr; // Points into mod_data_ptr.
  uint8_t n_orders;    // Song length in number of patterns/verses.

  // Patterns (verses).
  int n_patterns; // Number of patterns used in the song. Use apg_mod_fetch_note() to iterate over patterns and retrieve notes.

  // Samples (instruments).
  int8_t* sample_data_ptrs[APG_MOD_N_SAMPLES];                       // PCM 8-bit signed samples for *Paula* Amiga chip. These point into mod_data_ptr.
  uint32_t sample_sz_bytes[APG_MOD_N_SAMPLES];                       // Size of each sample in bytes. Converted from 16-bit big-endian 'word' lengths.
  char sample_names[APG_MOD_N_SAMPLES][APG_MOD_SAMPLE_NAME_LEN + 1]; // Sample names with nul-terminator appended so they can be used as C-strings.
} apg_mod_t;

APG_MOD_EXPORT typedef struct apg_mod_note_t {
  uint8_t sample_idx;        // Which sample index to play from sample_data_ptrs.
  uint16_t period_value_12b; // 12-bit 'period' value for sample timing.
  uint8_t effect_type_4b;    // 4-bit 'effect' code to apply to sample.
  uint16_t effect_params;    // Paramters to effect applied.
} apg_mod_note_t;

/** Read in a module file from disk. Call apg_mod_free() to release allocated memory. */
APG_MOD_EXPORT bool apg_mod_read_file( const char* filename, apg_mod_t* mod_ptr );

/** Decode the details for a note (sample and applied effects) to play at a channel in particular row in a given pattern.
 * This function can be called whilst iterating over the pattern indices contained in orders_ptr.
 * @param mod_ptr       Pass it a loaded module. Must not be NULL.
 * @param pattern_idx   Up to n_patterns.
 * @param row_idx       Up to APG_MOD_N_PATTERN_ROWS.
 * @param channel_idx   Up to n_chans.
 * @param note_ptr      Output written into a struct. Must not be NULL.
 * @return              Returns false on error.
 */
APG_MOD_EXPORT bool apg_mod_fetch_note( const apg_mod_t* mod_ptr, int pattern_idx, int row_idx, int channel_idx, apg_mod_note_t* note_ptr );

APG_MOD_EXPORT bool apg_mod_free( apg_mod_t* mod_ptr );

// APG_MOD_EXPORT bool apg_mod_write_file( const char* filename );

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
