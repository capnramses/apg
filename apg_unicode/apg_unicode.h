/* apg_unicode
Unicode <-> UTF-8 Multibyte String Functions for C & C++
Anton Gerdelan
Version: 0.2. 3 May 2020.
Licence: see bottom of file.
C89 ( Implementation is C99 )

TODO:
* Find out why some vars in bitwise ops are unused - feels like a copy-paste mistake between versions.
* Compile with all warnings and address unused vars etc.
* More tests. Arabic, reversed, etc
* CI tests.

Not Supported (Yet):
* Arabic (and other) ligatures

Further Reading:
* On special unicode "non-spacing" modifier codepoints: http://mts.io/2015/04/21/unicode-symbol-render-text-emoji/
* Skin tone modifier sequences https://unicode.org/emoji/charts/full-emoji-modifiers.html
* A good description of Arabic text formatting and styling https://w3c.github.io/alreq/
* I based Arabic cursive look-up table on the medial and final forms available in https://www.unicode.org/charts/PDF/UFE70.pdf

History:
* 0.2. 3 May 2020 Dug up unicode files from Project Storm and brushed off the dust.
*/

#pragma once
#include <stdint.h>

#define APG_UNICODE_MAX_STR 2048 // max length of string. for avoiding infinite loops.

/* Converts a unicode codepoint to an UTF-8 encoded multi-byte sequence.
PARAMS
mbs - Pointer to a buffer location that can hold the UTF-8 multi-byte sequence + null terminator (max 5 bytes).
RETURNS: Number of bytes in the sequence (1 - 4). On error the function returns 4 and mbs holds the question mark character <?>.
NOTE: UTF-8 mbs can be used with ASCII string functions that process byte-by-byte.
*/
int apg_cp_to_utf8( uint32_t codepoint, char* mbs );

/* Converts part of an utf-8 encoded multi-byte sequence (i.e. a string) to a unicode codepoint.
PARAMS
mbs - Pointer to first byte of UTF-8 sequence in a multi-byte array.
sz  - Sets number of bytes detected in sequence (1 - 4).
RETURNS: Unicode codepoint, or 0 on error or end of string.
*/
uint32_t apg_utf8_to_cp( const char* mbs, int* sz );

/* Get number of codepoints in an UTF8-encoded sequence. Note that strlen() on a MBS returns the number of bytes, not codepoints.
PARAMS
buf - must be nul-terminated
RETURNS < 0 on error, or the number of valid code points in the multi-byte sequence */
int apg_utf8_count_cp( const char* buf );

/* Reverse an UTF-8 encoded multi-byte string.
PARAMS
out - Output multi-byte string.
in  - Input multi-byte string. Must be nul-terminated.
*/
void apg_utf8_reverse( char* out, const char* in );

/* Conversion of Arabic codepoint to correct cursive form, based on cursive properties of neighbouring codepoints. Simply put: joins the lines together between
characters.
PARAMS: cp_left and cp_right are in order of writing/reading - right-to-left. So cp_right is the /earlier/ codepoint.
RETURNS: Unicode codepoint for correct cursive to use for an Arabic codepoint within a string.
NOTE: Memory order for right-to-left text is usually element [0] => rightmost, [len-1] => leftmost.
*/
uint32_t apg_cursive_arabic_cp( uint32_t cp_left, uint32_t cp, uint32_t cp_right );

/* Converts any general Unicode Arabic characters (U+0627 to U+0649) to cursive-aware equivalents.
PARAMS
in  - Input must be in standard right-to-left memory arrangement. Must be nul-terminated.
out - Output will be a string of the same size in range (U+FE70 to U+FEFF ).
WARNING: You must allow extra space in the output buffer in case bytes required per codepoint increases to 3.
*/
void apg_utf8_convert_to_arabic_cursive( char* out, const char* in, int* out_sz );

/* Trims the end n chars from a utf8-encoded string.
PARAMS
buf - Must be nul-terminated.
NOTE: Searches string from front-to-back so not efficient for long strings.
Could instead be implemented to progressively look at last 4 bytes for unicode indicators.
*/
void apg_utf8_trim_end( char* buf, int n );

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
