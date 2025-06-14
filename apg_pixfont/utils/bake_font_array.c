/**
 * @file bake_font_array.c
 *
 * @author Anton Gerdelan - @capnramses - <antongdl@protonmail.com>.
 *
 * @brief Stand-alone utility program for updating apg_pixfont.c with a new hard-coded font image.
 *
 * Example use case:
 *   "Ah! I forgot to add ê to the typeface image...let's paint that into the reference image and then
 *   update the hard-coded version in the library."
 *
 * Input
 *   some_font_image.png
 *
 * Output
 *   A C array in text is written to stdout that can be copy-pasted over the font array in apg_pixfont.c code.
 *
 * Build
 *   cc -o bake_font bake_font_array.c -lm -I ../../third_party/
 *
 * Usage
 *   ./bake_font MY_IMAGE.png > some_code.c
 *
 * Licence: See bottom of this file.
 */

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <stdio.h>

int main( int argc, char** argv ) {
  if ( argc < 2 ) {
    printf( "Usage: ./bake_font MY_IMAGE.png > some_code.c\n" );
    return 0;
  }
  int w = 0, h = 0, n = 0;
  unsigned char* img_ptr = stbi_load( argv[1], &w, &h, &n, 1 );
  if ( !img_ptr ) {
    fprintf( stderr, "ERROR loading font from image file `%s`\n", argv[1] );
    return 1;
  }
  printf( "// clang-format off\n" );
  printf( "static const int _font_img_w = %i;\n", w );
  printf( "static const int _font_img_h = %i;\n", h );
  printf( "static const unsigned char _font_img[] = { " );
  int max = w * h;
  for ( int idx = 0; idx < max - 1; idx++ ) { printf( "%i,", img_ptr[idx] ); } // 1 byte per pixel, hence '++'.
  printf( "%i", img_ptr[max - 1] );                                            // No comma on last element.
  printf( " };\n" );
  printf( "// clang-format on\n" );

  free( img_ptr );
  return 0;
}

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
