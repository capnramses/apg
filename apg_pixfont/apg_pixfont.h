/**
 * @file apg_pixfont.h
 * @brief C Pixel Font Utility
 * @author Anton Gerdelan - @capnramses - <antongdl@protonmail.com>
 * LICENCE: see bottom of this file
 *
 * What Does It Do?
 * ==============================================================
 * - Given a string of text, renders it into an image, in pre-allocated memory, using an embedded pixel font.
 * - Useful for quick & dirty text output in graphics software. eg FPS counters or debug-on-screen text, or pixel-art games.
 * - Tells you how much memory to allocate for an image to fit a given string.
 * - Can set the number of colour channels, and those colours, for the text.
 * - Can add pixel art outlines to the text.
 *
 * What Doesn't It Do?
 * ==============================================================
 * - Contain a file output API. If you want to save to a file use stb_image_write or libpng etc.
 * - Characters rendered are limited to those in the font - 1-byte-per-codepoint ASCII, and a limited selection of other Latin alphabet glyphs (basic accents,
 * umlauts).
 *
 * Instructions:
 * ==============================================================
 * 1. Copy apg_pixfont.c and apg_pixfont.h into your project's source code files.
 *    You don't need to copy the image file for the font - it's embedded in the C file.
 *
 * 2. Find out the required image dimensions
 *
 *     int w, h;              // Image dimensions.
 *     int n_channels = 1;    // Greyscale image output.
 *     int thickness = 1;     // Single-pixel thick glyph lines.
 *     int col_max = 0;       // Maximum characters in a line. Zero means no limit.
 *     bool outlines = false; // If true, also draw a partial outline.
 *
 *     apg_pixfont_image_size_for_str( "my_string", &w, &h, thickness, outlines, col_max );
 *
 * 3. Allocate the memory
 *
 *     unsigned char* img_mem = (unsigned char*)malloc( w * h * n_channels );
 *     memset( img_mem, 0x00, w * h * n_channels );
 *
 * 4. Then paint the string onto the memory
 *
 *     apg_pixfont_str_into_image( "my_string", img_mem, w, h, n_channels, 0xFF, 0x7F, 0x00, 0xFF, thickness, outlines, col_max );
 *
 * Advanced Tips:
 * ==============================================================
 * *. If you want characters to wrap around at some line limit, you can set `col_max` to some number. This will just do the following:
 *
 * From:
 *
 *      col max
 *        |
 * A verylongword.
 *        |
 *
 * To:
 *        |
 * A verylo
 * ngword.
 *        |
 *
 * *. If you want words to wrap neatly at the end of a line, you can call the replace whitespace preceding
 *   overlapping words with linebreaks. Do this before calling the functions in steps 2 and 3.
 *
 *     apg_pixfont_word_wrap_str( my_string, 60 );
 *
 * This will achieve the following:
 *
 * From:
 *
 *          col max
 *             |
 * A verylongword.
 *             |
 *
 * To:
 *             |
 * A
 * verylongword.
 *             |
 *
 * ==============================================================
 *
 * Technical Details:
 * ==============================================================
 * - Implementation is C99, interface is C89.
 *
 * History:
 * ==============================================================
 * 0.5.1 - 2025 Jun 15 - Tiny font support. Various bug-fixes.
 * 0.4.4 - 2024 Oct 18 - Outline is now a darker shade of text colour, not black.
 * 0.4.0 - 2024 Jun 15 - Most of Latin-1 character set included.
 * 0.3.0 - 2023 Mar 09 - Bold/italic/underline/strikethrough style. '\n's at the end of strings trimmed. Atlas generation tools. Basic Unicode UTF-8 support.
 * 0.2.1 - 2022 Sep 26 - Readme correction.
 * 0.2.0 - 2022 Sep 25 - Word-wrap function.
 * 0.1.1 - 2022 Sep 22 - Tidied comments. Character-based wrap option.
 * 0.1.0 - 2022 Apr 23 - Carriage return \r is ignored.
 * 0.0.5 - 2021 Jan 31 - apg_pixfont_image_size_for_str() always returns even dimensions, padding a pixel if required. Helps image alignment.
 * 0.0.4 - 2021 Jan 05 - Removed vflip again, to simplify code in several places.
 * 0.0.3 - 2019 Aug 11 - Added vflip option back in, and reconciled with outline drawing block.
 * 0.0.2 - 2019 Aug 11 - Thickness (scaling) API, revised outlines code to suite, removed vertical flip option from interface.
 * 0.0.1 - 2019 Aug 9  - Split into stand-alone library code.
 * 0.0.0 - 2018        - First version by Anton, as part of voxel game project
 */

#ifndef _APG_PIXFONT_H_
#define _APG_PIXFONT_H_

#ifdef __cplusplus
extern "C" {
#endif

#define APGPF_FAILURE 0
#define APGPF_SUCCESS 1
#define APGPF_MAX_STRLEN 2048

typedef enum apgpf_style_t {
  APGPF_STYLE_REGULAR = 0,
  APGPF_STYLE_BOLD,
  APGPF_STYLE_ITALIC,
  APGPF_STYLE_UNDERLINE,
  APGPF_STYLE_STRIKETHROUGH
} apgpf_style_t;

typedef enum apgpf_typeface_t { APGPF_TYPEFACE_STANDARD = 0, APGPF_TYPEFACE_SHORT } apgpf_typeface_t;

/** Word-wrap a string at col_max by converting white-space to line breaks between words, where appropriate.
 *  Note that very long words are not split by this function; breaking those is handled inside the other functions, when drawing.
 *  The original string is modified, but the length of the string is never changed.
 *  The combination of the above two points means that sometimes gaps are left at the end of lines, as this function does not (yet) anticipate
 *  long words being split later. This should be an edge-case, as whole-words typically not not exceed a line length.
 */
void apg_pixfont_word_wrap_str( char* str_ptr, int col_max );

/** Get image dimensions required for writing full string into with str_into_image().
 *
 * @param ascii_str
 * Null-terminated string to render. ASCII plus a few basic UTF-8 encoded Unicode Latin characters are supported.
 * @param w,h
 * Minimum dimensions of image require to fix the whole string.
 * @param thickness
 * A scaling factor for the text ( default is 1px thick glyph stems ).
 * @param add_outline
 * If the text will add an outline to the right and bottom of glyph pixels 0=no, 1=yes.
 * @param style
 * 0 = Regular, 1 = Italic, 2 = Bold, 3 = Underline, 4 = Strikethrough.
 * @param col_max
 * If a number greater than 0 is provided, then the text will limit line width to that many characters.
 * If the current line exceeds the character count then it will wrap to the next line.
 *
 * @return
 * `APGPF_FAILURE` on error (zero-length strings, NULL pointer args), otherwise success.
 *
 * @note
 * Image widths returned will always tightly fit text size. You can, however, use a large image than this.
 * If the provided image is smaller than the reported size required then text will appear cut off on the image boundary.
 * Very short strings, eg '|' will produce images that are smaller than 4-byte-aligned memory expected by eg OpenGL textures.
 * To address this you could pad the image, or set OpenGL byte alignment to 1.
 * If you want power-of-two sized images then allocate the next power-of-two size up.
 */
int apg_pixfont_image_size_for_str( //
  const char* ascii_str,            //
  int* w,                           //
  int* h,                           //
  int thickness,                    //
  int add_outline,                  //
  apgpf_style_t style,              //
  int col_max,                      //
  apgpf_typeface_t typeface         //
);

/** Writes an ASCII string into an image using the pixel font.
 * Allocate image memory first and clear to a background colour as desired. The pixel font text writes over the top in white.
 *
 * @param ascii_str
 * Null-terminated string to render. ASCII plus a few basic UTF-8 encoded Unicode Latin characters are supported.
 * @param image
 * A pre-allocated block of memory to draw the image into. Use image_size_for_str() to find the size required for this.
 * @param w,h
 * Dimensions of image to write into. If text pixels extend out of these bounds it won't attempt to write anything there.
 * @param vertically_flip
 * Image can be vertically flipped (0=no,1=yes) for e.g. OpenGL textures.
 * @param n_channels
 * 1 for greyscale, 2 for RG, 3 for RGB, 4 for RGBA. if using 2-4 then size memory allocated for image should reflect that.
 * @param r,g,b,a
 * Colour of the text. 1-channel uses only red colour. 2-channel uses only red and alpha. 3-channel uses r,g,b. 4-channel uses all.
 * @param thickness
 * A scaling factor for the text ( default is 1px thick glyph stems ). Useful for making big headings.
 * @param add_outline
 * If the text will add an outline to the right and bottom of glyph pixels 0=no, 1=yes.
 * Note that the outline will not scale, it stays 1px thick, which gives a consistent look.
 * @param style
 * 0 = Regular, 1 = Italic, 2 = Bold, 3 = Underline, 4 = Strikethrough.
 * @param col_max
 * If a number greater than 0 is provided, then the text will limit line width the that many characters.
 * If the current line exceeds the character count then it will wrap to the next line.
 *
 * @return
 *  Returns `APGPF_FAILURE` on error, otherwise success.
 */
int apg_pixfont_str_into_image(                                       //
  const char* ascii_str,                                              //
  unsigned char* image,                                               //
  int w, int h,                                                       //
  int n_channels,                                                     //
  unsigned char r, unsigned char g, unsigned char b, unsigned char a, //
  int thickness,                                                      //
  int add_outline,                                                    //
  apgpf_style_t style,                                                //
  int col_max,                                                        //
  apgpf_typeface_t typeface                                           //
);

#ifdef __cplusplus
}
#endif /* extern C */

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
