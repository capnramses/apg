/*****************************************************************************\
apg_mod - description to go here
Anton Gerdelan <antonofnote at gmail> 2022
C99
Licence: See bottom of this file or LICENSE file.

What are Modules?
---------------------

Modules are music files started by Karsten Obarski's "Ultimate SoundTracker"
for Commodore Amiga in 1987. Similar to MIDI files, except they store the
sample sounds in the music file itself, rather than relying on sound banks
that come with the sound device. That means they always sound the same on
different devices, and can have custom digital samples.

A good description, by Norman Lin, of the Amiga MOD format, is found here:
https://www.ocf.berkeley.edu/~eek/index.html/tiny_examples/ptmod/ap12.html

Format details here:
http://fileformats.archiveteam.org/wiki/Amiga_Module

ToDo
---------------------

* S3M (Scream Tracker) https://en.wikipedia.org/wiki/S3M_(file_format)
* XM support (Extended Module - Triton's FastTracker 2) https://en.wikipedia.org/wiki/XM_(file_format)
* IT support (Impulse Tracker) https://en.wikipedia.org/wiki/Impulse_Tracker#IT_file_format
* Graphical tracker player demo (probably pulseaudio+allegro/sdl/opengl or similar).
* Interactive visual track editor demo.
* File writing.

History
---------------------

0.0 - 10 Jan 2022 - Added skeleton to apg_libraries.
\*****************************************************************************/

#ifndef _APG_MOD_H_
#define _APG_MOD_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

bool apg_mod_read_file( const char* filename );
bool apg_mod_write_file( const char* filename );

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
