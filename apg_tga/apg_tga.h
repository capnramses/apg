/*==============================================================
Single-Header TGA image file reader/writer
Language: C89 header, C99 implementation.
Author:   Anton Gerdelan - @capnramses
Contact:  <antonofnote@gmail.com>
Website:  https://github.com/capnramses/apg - antongerdelan.net/
Licence:  See bottom of this file.

Instructions:
1. Include this header in one, and only one, source file.
2. #define APG_TGA_IMPLEMENTATION above #include "apg_tga.h"
3.
unsigned int w,h,n;
uint8_t* img_ptr = apg_tga_read_file("my_file.tga", &w, &h, &n, 0);

// ... use the BGR or BGRA (when n == 4) image memory here ...

free( img_ptr );

Define APG_TGA_DEBUG_OUTPUT to get extra information printed to stdout.

Limitations:
* Only reads/writes true colour uncompressed BGR and BGRA images.
* Note - There are inconsistent vertical flip conventions between users of TGA. We do our best here.

Todo:
* fuzzing
* validate range of dims etc - uint16_t internally so check range
* could allow malloc/free override

History:
09/09/2019 - First version.
24/09/2019 - Published to apg repository.
14/11/2019 - Fixes for MSVC warnings (CPP compat)
06/04/2020 - Tidy-up between repos. Added BGR<->RGB utility function. Bugfix: Writing. Y direction for GIMP etc. APG_TGA_DEBUG_OUTPUT option.
==============================================================*/

#ifndef APG_TGA_H
#define APG_TGA_H

#ifdef __cplusplus
extern "C" {
#endif

/* RETURNS A pointer to tightly-packed 8-bpp BGR or BGRA memory, or NULL on error or unsupported TGA subtype. */
unsigned char* apg_tga_read_file( const char* filename, unsigned int* w, unsigned int* h, unsigned int* n, unsigned int vert_flip );
/* RETURNS 1 on success, 0 on error. */
unsigned int apg_tga_write_file( const char* filename, unsigned char* bgr_img_ptr, unsigned int w, unsigned int h, unsigned int n );

/* Flips BGR[A] to RGB[A] or vice versa
RETURNS 1 on success, 0 on error. */
unsigned int apg_tga_bgr_to_rgb( unsigned char* img_ptr, unsigned int w, unsigned int h, unsigned int n );

#ifdef __cplusplus
}
#endif

#endif /* APG_TGA_H */
/*==============================================================
End of Header
==============================================================*/

#ifdef APG_TGA_IMPLEMENTATION
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#pragma pack( push, 1 )
struct tga_header_t {
  uint8_t id_length;       /* bytes in the image ID field. */
  uint8_t colour_map_type; /* Whether a color map is included. 0-no, 1-yes, 2-127 reserved, 128-255 custom. */
  uint8_t image_type;      /* Compression and color types */

  uint16_t colour_map_first_entry_idx; /* index of first color map entry that is included in the file */
  uint16_t colour_map_length;          /* number of entries of the color map that are included in the file */
  uint8_t colour_map_bpp;              /* size of each colour map entry */

  /* Image dimensions and format. */
  uint16_t x_origin, y_origin; /* absolute coordinate of lower-left corner for displays where origin is at the lower left */
  uint16_t w, h;               /* img dims in pixels */
  uint8_t bpp;
  uint8_t img_descriptor;
  /* bits 3-0 give the alpha channel depth, bits 5-8 give direction

from http://www.gamers.org/dEngine/quake3/TGA.txt:
|   17   |     1  |  Image Descriptor Byte.                                    |
|        |        |  Bits 3-0 - number of attribute bits associated with each  |
|        |        |             pixel.                                         |
|        |        |  Bit 4    - reserved.  Must be set to 0.                   |
|        |        |  Bit 5    - screen origin bit.                             | <-- important for eg GIMP
|        |        |             0 = Origin in lower left-hand corner.          |
|        |        |             1 = Origin in upper left-hand corner.          |
|        |        |             Must be 0 for Truevision images.               |
|        |        |  Bits 7-6 - Data storage interleaving flag.                |
|        |        |             00 = non-interleaved.                          |
|        |        |             01 = two-way (even/odd) interleaving.          |
|        |        |             10 = four way interleaving.                    |
|        |        |             11 = reserved.                                 |
|        |        |  This entire byte should be set to 0.  Don't ask me.       |

  */
};
#pragma pack( pop )

struct file_record_t {
  uint8_t* data;
  size_t sz; /* in bytes */
};

unsigned char* apg_tga_read_file( const char* filename, unsigned int* w, unsigned int* h, unsigned int* n, unsigned int vert_flip ) {
  struct file_record_t record;
  uint8_t* img_ptr     = NULL;
  size_t img_id_offset = 0, colour_map_offset = 0, img_data_offset = 0, img_data_sz = 0;
  bool vflip = 0;

  if ( !filename || !w || !h || !n ) { return NULL; }
  {
    FILE* fptr = fopen( filename, "rb" );
    if ( !fptr ) { return NULL; }
    fseek( fptr, 0L, SEEK_END );
    record.sz   = (size_t)ftell( fptr );
    record.data = (uint8_t*)malloc( record.sz );
    if ( !record.data ) {
      fclose( fptr );
      return NULL;
    }
    rewind( fptr );
    size_t nr = fread( record.data, record.sz, 1, fptr );
    fclose( fptr );
    if ( nr != 1 ) { return NULL; }
  }
  {
    struct tga_header_t* hdr_ptr;

    if ( record.sz < sizeof( struct tga_header_t ) ) {
      free( record.data );
      return NULL;
    }
    hdr_ptr = (struct tga_header_t*)record.data;
#ifdef APG_TGA_DEBUG_OUTPUT
    printf( "TGA hdr for `%s`\n", filename );
    printf( " |-id_length: %u\n", hdr_ptr->id_length );
    printf( " |-colour_map_type: %u\n", hdr_ptr->colour_map_type );
    printf( " |-image_type: %u\n", hdr_ptr->image_type );
    printf( " |-colour_map_first_entry_idx: %u\n", hdr_ptr->colour_map_first_entry_idx );
    printf( " |-colour_map_length: %u\n", hdr_ptr->colour_map_length );
    printf( " |-colour_map_bpp: %u\n", hdr_ptr->colour_map_bpp );
    printf( " |-x_origin: %u\n", hdr_ptr->x_origin );
    printf( " |-y_origin: %u\n", hdr_ptr->y_origin );
    printf( " |-w: %u\n", hdr_ptr->w );
    printf( " |-h: %u\n", hdr_ptr->h );
    printf( " |-bpp: %u\n", hdr_ptr->bpp );
    printf( " |-img_descriptor: %u\n", hdr_ptr->img_descriptor );
#endif
    *w                = hdr_ptr->w;
    *h                = hdr_ptr->h;
    *n                = 32 == hdr_ptr->bpp ? 4 : 3;
    img_id_offset     = sizeof( struct tga_header_t );
    colour_map_offset = img_id_offset + hdr_ptr->id_length;
    img_data_offset   = colour_map_offset;
    if ( hdr_ptr->colour_map_bpp % 8 > 0 || hdr_ptr->bpp % 8 > 0 || hdr_ptr->bpp == 0 ) {
      free( record.data );
      return NULL;
    }
    /* only supporting RGB right now */
    if ( hdr_ptr->colour_map_bpp > 0 && hdr_ptr->colour_map_length > 0 ) {
      img_data_offset = img_data_offset + ( hdr_ptr->colour_map_length * hdr_ptr->colour_map_bpp ) / 8;
    }
    /* check if file too small for data */
    img_data_sz = hdr_ptr->w * hdr_ptr->h * *n;
    if ( img_data_sz + img_data_offset > record.sz ) {
      free( record.data );
      return NULL;
    }
    /* only supports truecolour uncompressed */
    if ( 2 != hdr_ptr->image_type ) {
      free( record.data );
      return NULL;
    }
    /* vertical flip so 0,0 is bottom-left */
    if ( 0 == hdr_ptr->y_origin ) { vflip = 1; }
  }
  {
    img_ptr = (uint8_t*)malloc( img_data_sz );
    if ( !img_ptr ) {
      free( record.data );
      return NULL;
    }
  }
  if ( vert_flip ) { vflip = true; }
  if ( vflip ) {
    size_t row_stride = *w * *n;
    for ( unsigned int row = 0; row < *h; row++ ) {
      uint8_t* src_ptr = record.data + img_data_offset + row * row_stride;
      uint8_t* dst_ptr = img_ptr + ( *h - row - 1 ) * row_stride;
      memcpy( dst_ptr, src_ptr, row_stride );
    }
  } else {
    memcpy( img_ptr, &record.data[img_data_offset], img_data_sz );
  }
  free( record.data );
  return img_ptr;
}

unsigned int apg_tga_write_file( const char* filename, unsigned char* bgr_img_ptr, unsigned int w, unsigned int h, unsigned int n ) {
  struct tga_header_t hdr;

  if ( !filename || !bgr_img_ptr ) { return 0; }

  {
    memset( &hdr, 0, sizeof( struct tga_header_t ) );
    hdr.image_type     = 2;
    hdr.w              = (uint16_t)w;
    hdr.h              = (uint16_t)h;
    hdr.y_origin       = (uint16_t)h;
    hdr.bpp            = ( uint8_t )( 8 * n );
    hdr.img_descriptor = 40; /* NOTE(Anton) if wrong, eg set to zero, then image may be upside-down.
    bits 3-0 give the alpha channel depth, bits 5-8 give direction */
  }
  {
    size_t nw     = 0;
    size_t img_sz = w * h * n;
    FILE* fptr    = fopen( filename, "wb" );
    if ( !fptr ) { return 0; }
    nw = fwrite( &hdr, sizeof( struct tga_header_t ), 1, fptr );
    if ( 1 != nw ) {
      fclose( fptr );
      return 0;
    }
    nw = fwrite( bgr_img_ptr, img_sz, 1, fptr );
    if ( 1 != nw ) {
      fclose( fptr );
      return 0;
    }
    fclose( fptr );
  };
  return 1;
}

unsigned int apg_tga_bgr_to_rgb( unsigned char* img_ptr, unsigned int w, unsigned int h, unsigned int n ) {
  if ( !img_ptr || !w || !h || !n ) { return 0; }
  if ( n != 3 && n != 4 ) { return 0; }
  for ( unsigned int y = 0; y < h; y++ ) {
    for ( unsigned int x = 0; x < w; x++ ) {
      unsigned int idx  = y * w * n + x * n;
      unsigned char tmp = img_ptr[idx + 0];
      img_ptr[idx + 0]  = img_ptr[idx + 2];
      img_ptr[idx + 2]  = tmp;
    }
  }
  return 1;
}

#endif /* APG_TGA_IMPLEMENTATION */

/*
-------------------------------------------------------------------------------------
This software is available under two licences - you may use it under either licence.
-------------------------------------------------------------------------------------
FIRST LICENCE OPTION

                                 Apache License
                           Version 2.0, January 2004
                        http://www.apache.org/licenses/
   Copyright 2019 Anton Gerdelan.
   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at
       http://www.apache.org/licenses/LICENSE-2.0
   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
-------------------------------------------------------------------------------------
SECOND LICENCE OPTION

Public Domain (www.unlicense.org)
This is free and unencumbered software released into the public domain.
Anyone is free to copy, modify, publish, use, compile, sell, or distribute this
software, either in source code form or as a compiled binary, for any purpose,
commercial or non-commercial, and by any means.
In jurisdictions that recognize copyright laws, the author or authors of this
software dedicate any and all copyright interest in the software to the public
domain. We make this dedication for the benefit of the public at large and to
the detriment of our heirs and successors. We intend this dedication to be an
overt act of relinquishment in perpetuity of all present and future rights to
this software under copyright law.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
-------------------------------------------------------------------------------------
*/
