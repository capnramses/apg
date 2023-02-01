/*****************************************************************************\
apg_bmp - A BMP File Reader/Writer Library
Original author: Anton Gerdelan
Project URL:     https://github.com/capnramses/apg
Licence:         See bottom of file.
Language:        C89 ( Implementation is C99 )

Contributors
-------------------------------------------------------------------------------
  Anton Gerdelan - Initial code.
  Saija Sorsa    - Fuzz testing.

Instructions
-------------------------------------------------------------------------------
- Just drop this header, and the matching .c file into your project.
- If in a C++ project set these files to build as C, not C++.
- To get debug printouts during parsing define APG_BMP_DEBUG_OUTPUT.

Advantages
-------------------------------------------------------------------------------
- The implementation is fast, simple, and supports more formats than most
  BMP reader libraries.
- The reader function is fuzzed with AFL https://lcamtuf.coredump.cx/afl/.
- The reader is robust to large files and malformed files, and will return
  any valid partial data in an image.
- Reader supports 32bpp (with alpha channel), 24bpp, 8bpp, 4bpp, and 1bpp
  monochrome BMP images.
- Reader handles indexed BMP images using a colour palette.
- Writer supports 32bpp RGBA and 24bpp uncompressed RGB images.

Current Limitations
-------------------------------------------------------------------------------
- 16-bit images not supported (don't have any samples to test on).
- No support for interleaved channel bit layouts;
  e.g. RGB101010 RGB555 RGB565.
- No support for compressed BMP images, although in practice these are
  not used.
- Images with alpha channel are written in BITMAPINFOHEADER format for maximum
  backwards-compatibility. For wider alpha support in other apps the 124-bit v5
  header could be used instead. Your own apps using apg_bmp_read() will still
  read the alpha channel correctly.

FAQ
-------------------------------------------------------------------------------
Q. What makes this image loader special? Why would I use it?

This library started as a curiosity project, to see if I could read really old
BMP files, and understand the format. It was then used as an example for a
security class learning fuzzing. Because it was fuzzed it was used in some very
large projects as an image loader. There are many other BMP loaders out there,
but this one is pretty small and fast, and can handle some very old formats
that are not broadly supported. There is a blog post about it here:
https://antongerdelan.net/blog/formatted/2020_03_24_apg_bmp.html

Q. Why won't this compile in my C++ project?

This is a C library, just make sure the apg_bmp.c file is set to compile as C,
not C++, and the compiled object file will compile in with your C++ program.

Q. Are you open to pull requests?

Yes, but it's not being actively worked on, so turn-around time may be slow.
If the PR is accepted, I'll add you to the Contributors list.

Welcome:       Bug fixes, BMP feature-handling improvement.
Not desired:   Build systems, language & code style changes, large PRs.

Version History
-------------------------------------------------------------------------------
  3.3.1   - 2023 Feb.  1. Fixed type casting warnings from MSVC.
  3.3     - 2023 Jan. 11. Fixed bug: images with alpha channel were y-flipped.
  3.2     - 2022 Mar. 22. Minor signed/unsigned tweaks to constants.
  3.1     - 2020 Mar. 18.
\*****************************************************************************/

#ifndef APG_BMP_H_
#define APG_BMP_H_

#ifdef _WIN32
/** Explicit symbol export for building .DLLs with MSVC so it generates a corresponding .LIB. */
#define APG_BMP_EXPORT __declspec( dllexport )
#else
#define APG_BMP_EXPORT
#endif

#ifdef __cplusplus
extern "C" {
#endif /* CPP */

/** Reads a bitmap from a file, allocates memory for the raw image data, and returns it.
 * @param w,h     Retrieves the width and height of the BMP in pixels.
 * @param n_chans Retrieves the number of channels in the BMP.
 * @returns       Tightly-packed pixel memory in RGBA order. The caller must call free() on the memory.
 *                NULL on any error. Any allocated memory is freed before returning NULL.
 */
APG_BMP_EXPORT unsigned char* apg_bmp_read( const char* filename, int* w, int* h, unsigned int* n_chans );

/** Calls free() on memory created by apg_bmp_read. */
APG_BMP_EXPORT void apg_bmp_free( unsigned char* pixels_ptr );

/** Writes a bitmap to a file.
 * @param filename   e.g."my_bitmap.bmp". Must not be NULL.
 * @param pixels_ptr Pointer to tightly-packed pixel memory in RGBA order. Must not be NULL.
 *                   There must be abs(w)*abs(h)*n_chans bytes in the memory pointed to.
 * @param w,h        Width and height of the image in pixels.
 * @param n_chans    The number of channels in the BMP. 3 or 4 supported for writing,
 *                   which means RGB or RGBA memory, respectively.
 * @returns Zero on any error, non zero on success.
 */
APG_BMP_EXPORT unsigned int apg_bmp_write( const char* filename, unsigned char* pixels_ptr, int w, int h, unsigned int n_chans );

#ifdef __cplusplus
}
#endif /* CPP */

#endif /*_APG_BMP_H_ */

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
