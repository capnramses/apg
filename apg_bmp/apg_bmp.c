/*****************************************************************************\
apg_bmp - BMP File Reader/Writer Implementation
Anton Gerdelan
Version: 3.4.0
Licence: see apg_bmp.h
C99
\*****************************************************************************/

#include "apg_bmp.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Maximum pixel dimensions of width or height of an image. Should accommodate max used in graphics APIs.
   NOTE: 65536*65536 is the biggest number storable in 32 bits.
   This needs to be multiplied by n_channels so actual memory indices are not uint32 but size_t to avoid overflow.
   Note this will crash stb_image_write et al at maximum size which use 32bits, so reduce max size to accom. */
#define _BMP_MAX_DIMS 65536U
#define _BMP_FILE_HDR_SZ 14ULL
#define _BMP_MIN_DIB_HDR_SZ 40ULL
#define _BMP_MIN_HDR_SZ ( _BMP_FILE_HDR_SZ + _BMP_MIN_DIB_HDR_SZ )

#pragma pack( push, 1 ) // Supported on GCC in addition to individual packing attribs.
/** All BMP files, regardless of type, start with this file header. */
typedef struct _bmp_file_header_t {
  char file_type[2];
  uint32_t file_sz;
  uint16_t reserved1;
  uint16_t reserved2;
  uint32_t image_data_offset;
} _bmp_file_header_t;

/** Following the file header is the BMP type header. this is the most commonly used format. */
typedef struct _bmp_dib_BITMAPINFOHEADER_t {
  uint32_t this_header_sz;
  int32_t w;                      // In older headers w & h these are shorts and may be unsigned.
  int32_t h;                      //
  uint16_t n_planes;              // Must be 1.
  uint16_t bpp;                   // bits per pixel. 1,4,8,16,24,32.
  uint32_t compression_method;    // 16 and 32-bit images must have a value of 3 here
  uint32_t image_uncompressed_sz; // Not consistently used in the wild, so ignored here.
  int32_t horiz_pixels_per_meter; // Not used.
  int32_t vert_pixels_per_meter;  // Not used.
  uint32_t n_colours_in_palette;  //
  uint32_t n_important_colours;   // Not used.
  /* NOTE(Anton) a DIB header may end here at 40-bytes. Be careful using sizeof(). */
  /* If 'compression' value, above, is set to 3 ie the image is 16 or 32-bit,
  then these colour channel masks follow the headers.
  These are big-endian order bit masks to assign bits of each pixel to different colours.
  Bits used must be contiguous and not overlap. */
  uint32_t bitmask_r;
  uint32_t bitmask_g;
  uint32_t bitmask_b;
  // bitmask_a; is here in v4 and v5, but not earlier.
  // Note(Anton) v4 and v5 have gamma curve and sRGB information here.
} _bmp_dib_BITMAPINFOHEADER_t;
#pragma pack( pop )

typedef enum _bmp_compression_t {
  BI_RGB            = 0,
  BI_RLE8           = 1,
  BI_RLE4           = 2,
  BI_BITFIELDS      = 3,
  BI_JPEG           = 4,  // Not supported.
  BI_PNG            = 5,  // Not supported.
  BI_ALPHABITFIELDS = 6,
  BI_CMYK           = 11, // Not supported.
  BI_CMYKRLE8       = 12, // Not supported.
  BI_CMYRLE4        = 13  // Not supported.
} _bmp_compression_t;

/** Convenience struct and file->memory function. */
typedef struct _entire_file_t {
  void* data;
  size_t sz;
} _entire_file_t;

/**
 * @returns true on success. record->data is allocated memory and must be freed by the caller.
 * Returns false on any error. Any allocated memory is freed if false is returned.
 */
static bool _read_entire_file( const char* filename, _entire_file_t* record ) {
  FILE* fp = fopen( filename, "rb" );
  if ( !fp ) { return false; }
  fseek( fp, 0L, SEEK_END );
  record->sz   = (size_t)ftell( fp );
  record->data = malloc( record->sz );
  if ( !record->data ) {
    fclose( fp );
    return false;
  }
  rewind( fp );
  size_t nr = fread( record->data, record->sz, 1, fp );
  fclose( fp );
  if ( 1 != nr ) { return false; }
  return true;
}

static bool _validate_file_hdr( _bmp_file_header_t* file_hdr_ptr, size_t file_sz ) {
  if ( !file_hdr_ptr ) { return false; }
  if ( file_hdr_ptr->file_type[0] != 'B' || file_hdr_ptr->file_type[1] != 'M' ) { return false; }
  if ( file_hdr_ptr->image_data_offset > file_sz ) { return false; }
  return true;
}

static bool _validate_dib_hdr( _bmp_dib_BITMAPINFOHEADER_t* dib_hdr_ptr, size_t file_sz ) {
  if ( !dib_hdr_ptr ) { return false; }
  if ( _BMP_FILE_HDR_SZ + dib_hdr_ptr->this_header_sz > file_sz ) { return false; }
  if ( ( 32 == dib_hdr_ptr->bpp || 16 == dib_hdr_ptr->bpp ) && ( BI_BITFIELDS != dib_hdr_ptr->compression_method && BI_ALPHABITFIELDS != dib_hdr_ptr->compression_method ) ) {
    return false;
  }
  if ( BI_RGB != dib_hdr_ptr->compression_method &&            //
       BI_BITFIELDS != dib_hdr_ptr->compression_method &&      //
       BI_ALPHABITFIELDS != dib_hdr_ptr->compression_method && //
       BI_RLE8 != dib_hdr_ptr->compression_method &&           //
       BI_RLE4 != dib_hdr_ptr->compression_method              //
  ) {
    return false;
  }
  // NOTE(Anton) using abs() in the if-statement was blowing up on large negative numbers. switched to labs()
  if ( 0 == dib_hdr_ptr->w || 0 == dib_hdr_ptr->h || labs( dib_hdr_ptr->w ) > _BMP_MAX_DIMS || labs( dib_hdr_ptr->h ) > _BMP_MAX_DIMS ) { return false; }

  /* NOTE(Anton) If images reliably used n_colours_in_palette we could have done a palette/file size integrity check here.
  because some always set 0 then we have to check every palette indexing as we read them. */
  return true;
}

/* NOTE(Anton) This could have ifdef branches on different compilers for the intrinsics versions for perf. */
static uint32_t _bitscan( uint32_t dword ) {
  for ( uint32_t i = 0; i < 32; i++ ) {
    if ( 1 & dword ) { return i; }
    dword = dword >> 1;
  }
  return 0;
}

unsigned char* apg_bmp_read( const char* filename, int* w, int* h, unsigned int* n_chans ) {
  _entire_file_t record = ( _entire_file_t ){ .data = NULL };
  uint8_t* dst_img_ptr  = NULL;

  if ( !filename || !w || !h || !n_chans ) { goto apg_bmp_read_error; }

  if ( !_read_entire_file( filename, &record ) ) { goto apg_bmp_read_error; }
  if ( record.sz < _BMP_MIN_HDR_SZ ) { goto apg_bmp_read_error; }

  // Grab and validate the first, file, header.
  _bmp_file_header_t* file_hdr_ptr = (_bmp_file_header_t*)record.data;
  if ( !_validate_file_hdr( file_hdr_ptr, record.sz ) ) { goto apg_bmp_read_error; }

  // Grab and validate the second, DIB, header.
  _bmp_dib_BITMAPINFOHEADER_t* dib_hdr_ptr = (_bmp_dib_BITMAPINFOHEADER_t*)( (uint8_t*)record.data + _BMP_FILE_HDR_SZ );
  if ( !_validate_dib_hdr( dib_hdr_ptr, record.sz ) ) { goto apg_bmp_read_error; }

  // Bitmaps can have negative dims to indicate the image should be flipped.
  uint32_t width = *w = abs( dib_hdr_ptr->w );
  uint32_t height = *h = abs( dib_hdr_ptr->h );

  // Channel count and palette are not well defined in the header so we make a good guess here.
  uint32_t n_dst_chans = 3, n_src_chans = 3;
  bool has_palette = false;
  switch ( dib_hdr_ptr->bpp ) {
  case 32: n_dst_chans = n_src_chans = 4; break; // Technically can be RGB but not supported.
  case 24: n_dst_chans = n_src_chans = 3; break; // Technically can be RGBA but not supported.
  case 8:                                        // Seems to always use a BGR0 palette, even for greyscale.
    n_dst_chans = 3;
    has_palette = true;
    n_src_chans = 1;
    break;
  case 4: // Always has a palette - needed for a MS-saved BMP.
    n_dst_chans = 3;
    has_palette = true;
    n_src_chans = 1;
    break;
  case 1: // 1-bpp means the palette has 3 colour channels with 2 colours i.e. monochrome but not always black & white.
    n_dst_chans = 3;
    has_palette = true;
    n_src_chans = 1;
    break;
  default: // This includes 0 (PNG and JPG) and 16.
    goto apg_bmp_read_error;
  }        // endswitch.

  *n_chans = n_dst_chans;
  // NOTE(Anton) Some image formats are not allowed a palette - could check for a bad header spec here also.
  if ( dib_hdr_ptr->n_colours_in_palette > 0 ) { has_palette = true; }

#ifdef APG_BMP_DEBUG_OUTPUT
  printf( "apg_bmp_debug: reading image\n|-filename `%s`\n|-dims %ux%u pixels\n|-bpp %u\n|-n_src_chans %u\n|-n_dst_chans %u\n", filename, *w, *h,
    dib_hdr_ptr->bpp, n_src_chans, n_dst_chans );
#endif

  uint32_t palette_offset = _BMP_FILE_HDR_SZ + dib_hdr_ptr->this_header_sz;
  bool has_bitmasks       = false;
  if ( BI_BITFIELDS == dib_hdr_ptr->compression_method || BI_ALPHABITFIELDS == dib_hdr_ptr->compression_method ) {
    has_bitmasks = true;
    palette_offset += 12;
  }
  if ( palette_offset > record.sz ) { goto apg_bmp_read_error; }

  // Work out if any padding how much to skip at end of each row.
  uint32_t unpadded_row_sz = width * n_src_chans;
  // Bit-encoded palette indices have different padding properties.
  if ( 4 == dib_hdr_ptr->bpp ) { unpadded_row_sz = width % 2 > 0 ? width / 2 + 1 : width / 2; } // Find how many whole bytes required for this bit width.
  if ( 1 == dib_hdr_ptr->bpp ) { unpadded_row_sz = width % 8 > 0 ? width / 8 + 1 : width / 8; } // Find how many whole bytes required for this bit width.
  uint32_t row_padding_sz = 0 == unpadded_row_sz % 4 ? 0 : 4 - ( unpadded_row_sz % 4 );         // NOTE(Anton) didn't expect operator precedence of - over %

  if ( BI_RLE8 != dib_hdr_ptr->compression_method && BI_RLE4 != dib_hdr_ptr->compression_method ) {
    // Another file size integrity check: partially validate source image data size.
    // 'image_data_offset' is by row padded to 4 bytes and is either colour data or palette indices.
    if ( file_hdr_ptr->image_data_offset + ( unpadded_row_sz + row_padding_sz ) * height > record.sz ) { goto apg_bmp_read_error; }
  }

  // Find which bit number each colour channel starts at, so we can separate colours out.
  uint32_t bitshift_rgba[4] = { 0, 0, 0, 0 };
  uint32_t bitmask_a        = 0;
  if ( has_bitmasks ) {
    bitmask_a        = ~( dib_hdr_ptr->bitmask_r | dib_hdr_ptr->bitmask_g | dib_hdr_ptr->bitmask_b );
    bitshift_rgba[0] = _bitscan( dib_hdr_ptr->bitmask_r );
    bitshift_rgba[1] = _bitscan( dib_hdr_ptr->bitmask_g );
    bitshift_rgba[2] = _bitscan( dib_hdr_ptr->bitmask_b );
    bitshift_rgba[3] = _bitscan( bitmask_a );
  }

  // Allocate memory for the output pixels block. Cast to size_t in case width and height are both the max of 65536 and n_dst_chans > 1.
  size_t dst_img_sz = (size_t)width * (size_t)height * (size_t)n_dst_chans;
  dst_img_ptr       = malloc( dst_img_sz );
  if ( !dst_img_ptr ) { goto apg_bmp_read_error; }

  uint8_t* palette_data_ptr = (uint8_t*)record.data + palette_offset;
  uint8_t* src_img_ptr      = (uint8_t*)record.data + file_hdr_ptr->image_data_offset;
  size_t src_img_sz         = record.sz - file_hdr_ptr->image_data_offset;
  size_t dst_stride_sz      = width * n_dst_chans;

  // == 32-bpp -> 32-bit RGBA. == 32-bit and 16-bit require bitmasks.
  if ( 32 == dib_hdr_ptr->bpp ) {
    // Check source image has enough data in it to read from.
    if ( (size_t)file_hdr_ptr->image_data_offset + (size_t)height * (size_t)width * (size_t)n_src_chans > record.sz ) { goto apg_bmp_read_error; }
    size_t src_byte_idx = 0;
    for ( uint32_t r = 0; r < height; r++ ) {
      size_t dst_pixels_idx = ( height - 1 - r ) * dst_stride_sz;
      for ( uint32_t c = 0; c < width; c++ ) {
        uint32_t pixel;
        memcpy( &pixel, &src_img_ptr[src_byte_idx], 4 );
        // NOTE(Anton) The below assumes 32-bits is always RGBA 1 byte per channel. 10,10,10 RGB exists though and isn't handled.
        dst_img_ptr[dst_pixels_idx++] = (uint8_t)( ( pixel & dib_hdr_ptr->bitmask_r ) >> bitshift_rgba[0] );
        dst_img_ptr[dst_pixels_idx++] = (uint8_t)( ( pixel & dib_hdr_ptr->bitmask_g ) >> bitshift_rgba[1] );
        dst_img_ptr[dst_pixels_idx++] = (uint8_t)( ( pixel & dib_hdr_ptr->bitmask_b ) >> bitshift_rgba[2] );
        dst_img_ptr[dst_pixels_idx++] = (uint8_t)( ( pixel & bitmask_a ) >> bitshift_rgba[3] );
        src_byte_idx += 4;
      }
      src_byte_idx += row_padding_sz;
    }

    // == 8-bpp -> 24-bit RGB ==
  } else if ( 8 == dib_hdr_ptr->bpp && has_palette ) {
    if ( BI_RLE8 == dib_hdr_ptr->compression_method ) {
      // RLE Compressed:
      size_t row = 0, col = 0, byte_idx = 0;
      size_t dst_pixels_idx = ( height - 1 - row ) * dst_stride_sz;
      // Iterate over the "Colour-index array".
      while ( byte_idx + 1 < record.sz ) {
        uint8_t byte_a = src_img_ptr[byte_idx++];
        uint8_t byte_b = src_img_ptr[byte_idx++];
        // Absolute_mode run:
        if ( 0x00 == byte_a && byte_b >= 0x03 ) {
          for ( int fol_i = 0; fol_i < byte_b; fol_i++ ) {
            if ( byte_idx >= src_img_sz ) { goto apg_bmp_read_error; }
            uint8_t colour_index = src_img_ptr[byte_idx++];
            if ( dst_pixels_idx + 3 > dst_img_sz ) { goto apg_bmp_read_error; }
            if ( col >= width ) { goto apg_bmp_read_error; }
            dst_img_ptr[dst_pixels_idx++] = palette_data_ptr[colour_index * 4 + 2]; // Red
            dst_img_ptr[dst_pixels_idx++] = palette_data_ptr[colour_index * 4 + 1]; // Green
            dst_img_ptr[dst_pixels_idx++] = palette_data_ptr[colour_index * 4 + 0]; // Blue
            col++;
          }
          // In absolute mode, each run must be zero-padded to end on a 16-bit word boundary.
          if ( 0 != byte_idx % 2 ) { byte_idx++; }
          continue;
        }
        // Encoded mode:
        //   - Escapes:
        if ( 0x00 == byte_a && 0x00 == byte_b ) { // "End line".
          col = 0;
          row++;
          if ( row >= height ) { goto apg_bmp_read_error; }
          dst_pixels_idx = ( height - 1 - row ) * dst_stride_sz;
          continue;
        } else if ( 0x00 == byte_a && 0x01 == byte_b ) { // "End of bitmap".
          break;                                         // break `Iterate over the "Colour-index array".`
        } else if ( 0x00 == byte_a && 0x02 == byte_b ) { // "Delta position".
          goto apg_bmp_read_error;                       // Not supported (yet) because I don't have any test images.

          // The 2 bytes following the escape contain unsigned values indicating the offset to the right and up of the next pixel from the current position.
          // uint8_t x = src_img_ptr[byte_idx++];
          // uint8_t y = src_img_ptr[byte_idx++];
          // I presume this means set dst_pixels_idx based on a new (row,col) for subsequent pixels until another delta escape?
          // I'm not clear from MS docs which direction horizontal and vertical are supposed to move.
        }

        //   - Normal 2-byte pair: First byte=count, second byte=index.
        for ( int count_i = 0; count_i < byte_a; count_i++ ) {
          if ( dst_pixels_idx + 3 > dst_img_sz ) { goto apg_bmp_read_error; }
          if ( col >= width ) { goto apg_bmp_read_error; }
          dst_img_ptr[dst_pixels_idx++] = palette_data_ptr[byte_b * 4 + 2]; // Red
          dst_img_ptr[dst_pixels_idx++] = palette_data_ptr[byte_b * 4 + 1]; // Green
          dst_img_ptr[dst_pixels_idx++] = palette_data_ptr[byte_b * 4 + 0]; // Blue
          col++;
        }

      } // endwhile Iterate over the "Colour-index array".

    } else {
      // Uncompressed:

      // Validate indices (body of image data) fits in file.
      if ( file_hdr_ptr->image_data_offset + height * width > record.sz ) { goto apg_bmp_read_error; }

      size_t src_byte_idx = 0;
      for ( uint32_t r = 0; r < height; r++ ) {
        size_t dst_pixels_idx = ( height - 1 - r ) * dst_stride_sz;
        for ( uint32_t c = 0; c < width; c++ ) {
          // "most palettes are 4 bytes in RGB0 order but 3 for..." - it was actually BRG0 in old images -- Anton
          uint8_t index = src_img_ptr[src_byte_idx]; // 8-bit index value per pixel.

          if ( palette_offset + index * 4 + 2 >= record.sz ) {
            free( record.data );
            return dst_img_ptr;
          }
          dst_img_ptr[dst_pixels_idx++] = palette_data_ptr[index * 4 + 2];
          dst_img_ptr[dst_pixels_idx++] = palette_data_ptr[index * 4 + 1];
          dst_img_ptr[dst_pixels_idx++] = palette_data_ptr[index * 4 + 0];
          src_byte_idx++;
        } // endfor col.
        src_byte_idx += row_padding_sz;
      }   // endfor row.
    }     // endif RLE/Uncompressed 24-bit RGB.

    // == 4-bpp (16-colour) -> 24-bit RGB ==
  } else if ( 4 == dib_hdr_ptr->bpp && has_palette ) {
    if ( BI_RLE4 == dib_hdr_ptr->compression_method ) {
      // RLE4 Compressed:

      size_t col = 0, row = 0, byte_idx = 0;
      size_t dst_pixels_idx = ( height - 1 - row ) * dst_stride_sz;
      // Iterate over the "Colour-index array".
      while ( byte_idx + 1 < record.sz ) {
        uint8_t byte_a = src_img_ptr[byte_idx++];
        uint8_t byte_b = src_img_ptr[byte_idx++];

        // The end-of-line, end-of-bitmap, and delta escapes described for BI_RLE8 also apply to BI_RLE4 compression.

        // Absolute_mode run:
        if ( 0x00 == byte_a && byte_b >= 0x03 ) {
          uint8_t n_pixels = 0; // The second byte contains the number of color indexes that follow.
          // Subsequent bytes contain color indexes in their high- and low-order 4 bits, one color index for each pixel.
          while ( n_pixels < byte_b ) {
            if ( byte_idx >= src_img_sz ) { goto apg_bmp_read_error; }
            uint8_t colour_index_duo = src_img_ptr[byte_idx++];
            uint8_t a_index          = ( 0xFF & colour_index_duo ) >> 4;
            uint8_t b_index          = 0xF & colour_index_duo;
            if ( dst_pixels_idx + 3 > dst_img_sz ) { goto apg_bmp_read_error; }
            if ( col >= width ) { goto apg_bmp_read_error; }
            dst_img_ptr[dst_pixels_idx++] = palette_data_ptr[a_index * 4 + 2]; // Red
            dst_img_ptr[dst_pixels_idx++] = palette_data_ptr[a_index * 4 + 1]; // Green
            dst_img_ptr[dst_pixels_idx++] = palette_data_ptr[a_index * 4 + 0]; // Blue
            col++;
            n_pixels++;
            if ( n_pixels < byte_b ) {
              if ( dst_pixels_idx + 3 > dst_img_sz ) { goto apg_bmp_read_error; }
              if ( col >= width ) { goto apg_bmp_read_error; }
              dst_img_ptr[dst_pixels_idx++] = palette_data_ptr[b_index * 4 + 2]; // Red
              dst_img_ptr[dst_pixels_idx++] = palette_data_ptr[b_index * 4 + 1]; // Green
              dst_img_ptr[dst_pixels_idx++] = palette_data_ptr[b_index * 4 + 0]; // Blue
              col++;
              n_pixels++;
            }
          }
          // In absolute mode, each run must be zero-padded to end on a 16-bit word boundary.
          if ( 0 != byte_idx % 2 ) { byte_idx++; }
          continue;
        } // endif absolute mode run.

        // RLE4 Encoded mode:
        //   - Escapes:
        if ( 0x00 == byte_a && 0x00 == byte_b ) { // "End line".
          col = 0;
          row++;
          if ( row >= height ) { goto apg_bmp_read_error; }
          dst_pixels_idx = ( height - 1 - row ) * dst_stride_sz;
          continue;
        } else if ( 0x00 == byte_a && 0x01 == byte_b ) { // "End of bitmap".
          break;                                         // break `Iterate over the "Colour-index array".`
        } else if ( 0x00 == byte_a && 0x02 == byte_b ) { // "Delta position".
          goto apg_bmp_read_error;                       // Not supported (yet) because I don't have any test images.

          // The 2 bytes following the escape contain unsigned values indicating the offset to the right and up of the next pixel from the current position.
          // uint8_t x = src_img_ptr[byte_idx++];
          // uint8_t y = src_img_ptr[byte_idx++];
          // I presume this means set dst_pixels_idx based on a new (row,col) for subsequent pixels until another delta escape?
          // I'm not clear from MS docs which direction horizontal and vertical are supposed to move.
          continue;
        }

        // The first of the pixels is drawn using the color specified by the high-order 4 bits,
        // the second is drawn using the color in the low-order 4 bits,
        // the third is drawn using the color in the high - order 4 bits,
        // and so on, until all the pixels specified by the first byte have been drawn.

        uint8_t n_pixels = 0; // The first byte of the pair contains the number of pixels to be drawn using the color indexes in the second byte.
        // The second byte contains two color indexes, one in its high-order 4 bits and one in its low-order 4 bits.
        uint8_t colour_index_duo = byte_b;
        uint8_t a_index          = ( 0xFF & colour_index_duo ) >> 4;
        uint8_t b_index          = 0xF & colour_index_duo;

        // NOTE: byte_a contains count in encoded mode, but in absolute it's byte_b.
        while ( n_pixels < byte_a ) {
          if ( dst_pixels_idx + 3 > dst_img_sz ) { goto apg_bmp_read_error; }
          if ( col >= width ) { goto apg_bmp_read_error; }
          dst_img_ptr[dst_pixels_idx++] = palette_data_ptr[a_index * 4 + 2]; // Red
          dst_img_ptr[dst_pixels_idx++] = palette_data_ptr[a_index * 4 + 1]; // Green
          dst_img_ptr[dst_pixels_idx++] = palette_data_ptr[a_index * 4 + 0]; // Blue
          col++;
          n_pixels++;
          if ( n_pixels < byte_a ) {
            if ( dst_pixels_idx + 3 > dst_img_sz ) { goto apg_bmp_read_error; }
            if ( col >= width ) { goto apg_bmp_read_error; }
            dst_img_ptr[dst_pixels_idx++] = palette_data_ptr[b_index * 4 + 2]; // Red
            dst_img_ptr[dst_pixels_idx++] = palette_data_ptr[b_index * 4 + 1]; // Green
            dst_img_ptr[dst_pixels_idx++] = palette_data_ptr[b_index * 4 + 0]; // Blue
            col++;
            n_pixels++;
          }
        } // endwhile still pixels in encoded run.

      }   // endwhile Iterate over the "Colour-index array".

    } else {
      // 4-bit Uncompressed:
      size_t src_byte_idx = 0;
      for ( uint32_t r = 0; r < height; r++ ) {
        size_t dst_pixels_idx = ( height - 1 - r ) * dst_stride_sz;
        for ( uint32_t c = 0; c < width; c++ ) {
          if ( file_hdr_ptr->image_data_offset + src_byte_idx > record.sz ) { goto apg_bmp_read_error; }
          // Handle 2 pixels at a time.
          uint8_t pixel_duo = src_img_ptr[src_byte_idx];
          uint8_t a_index   = ( 0xFF & pixel_duo ) >> 4;
          uint8_t b_index   = 0xF & pixel_duo;

          if ( palette_offset + a_index * 4 + 2 >= record.sz ) { // Invalid src image.
            free( record.data );
            return dst_img_ptr;
          }
          if ( dst_pixels_idx + 3 > width * height * n_dst_chans ) { // Done.
            free( record.data );
            return dst_img_ptr;
          }
          dst_img_ptr[dst_pixels_idx++] = palette_data_ptr[a_index * 4 + 2];
          dst_img_ptr[dst_pixels_idx++] = palette_data_ptr[a_index * 4 + 1];
          dst_img_ptr[dst_pixels_idx++] = palette_data_ptr[a_index * 4 + 0];
          if ( ++c >= width ) { // Advance a column.
            c = 0;
            r++;
            if ( r >= height ) { // Done. no need to get second pixel. eg a 1x1 pixel image.
              free( record.data );
              return dst_img_ptr;
            }
            dst_pixels_idx = ( height - 1 - r ) * dst_stride_sz;
          }

          if ( palette_offset + b_index * 4 + 2 >= record.sz ) { // Invalid src image.
            free( record.data );
            return dst_img_ptr;
          }
          if ( dst_pixels_idx + 3 > width * height * n_dst_chans ) { // Done. Probably redundant check since checking r >= height.
            free( record.data );
            return dst_img_ptr;
          }
          dst_img_ptr[dst_pixels_idx++] = palette_data_ptr[b_index * 4 + 2];
          dst_img_ptr[dst_pixels_idx++] = palette_data_ptr[b_index * 4 + 1];
          dst_img_ptr[dst_pixels_idx++] = palette_data_ptr[b_index * 4 + 0];
          src_byte_idx++;
        } // endfor col.
        src_byte_idx += row_padding_sz;
      }   // endfor row.
    }     // endif RLE4 or uncompressed 4-bit.

    // == 1-bpp -> 24-bit RGB ==
  } else if ( 1 == dib_hdr_ptr->bpp && has_palette ) {
    /* Encoding method for monochrome is not well documented.
    A 2x2 pixel image is stored as 4 1-bit palette indexes
    The palette is stored as any 2 RGB0 colours (not necessarily B&W)
    So for an image with indexes like so:
    1 1
    0 1
    it is bit-encoded as follows, starting at MSB:
    01000000 00000000 00000000 00000000 (first byte val  64)
    11000000 00000000 00000000 00000000 (first byte val 192)
    data is still split by row and each row padded to 4 byte multiples.
     */
    size_t src_byte_idx = 0;
    for ( uint32_t r = 0; r < height; r++ ) {
      uint8_t bit_idx       = 0; // Used in monochrome,
      size_t dst_pixels_idx = ( height - 1 - r ) * dst_stride_sz;
      for ( uint32_t c = 0; c < width; c++ ) {
        if ( 8 == bit_idx ) { // Start reading from the next byte,
          src_byte_idx++;
          bit_idx = 0;
        }
        if ( file_hdr_ptr->image_data_offset + src_byte_idx > record.sz ) {
          free( record.data );
          return dst_img_ptr;
        }
        uint8_t pixel_oct   = src_img_ptr[src_byte_idx];
        uint8_t bit         = 128 >> bit_idx;
        uint8_t masked      = pixel_oct & bit;
        uint8_t palette_idx = masked > 0 ? 1 : 0;

        if ( palette_offset + palette_idx * 4 + 2 >= record.sz ) {
          free( record.data );
          return dst_img_ptr;
        }
        dst_img_ptr[dst_pixels_idx++] = palette_data_ptr[palette_idx * 4 + 2];
        dst_img_ptr[dst_pixels_idx++] = palette_data_ptr[palette_idx * 4 + 1];
        dst_img_ptr[dst_pixels_idx++] = palette_data_ptr[palette_idx * 4 + 0];
        bit_idx++;
      }
      src_byte_idx += ( row_padding_sz + 1 ); // 1bpp is special here.
    }

    // == 24-bpp -> 24-bit RGB == (but also should handle some other n_chans cases).
  } else {
    // NOTE(Anton) this only supports 1 byte per channel.
    if ( file_hdr_ptr->image_data_offset + height * width * n_dst_chans > record.sz ) { goto apg_bmp_read_error; }
    size_t src_byte_idx = 0;
    for ( uint32_t r = 0; r < height; r++ ) {
      size_t dst_pixels_idx = ( height - 1 - r ) * dst_stride_sz;
      for ( uint32_t c = 0; c < width; c++ ) {
        // Re-orders from BGR to RGB.
        if ( n_dst_chans > 3 ) {
          if ( dst_pixels_idx > dst_img_sz || src_byte_idx + 3 > src_img_sz ) { goto apg_bmp_read_error; }
          dst_img_ptr[dst_pixels_idx++] = src_img_ptr[src_byte_idx + 3];
        }
        if ( n_dst_chans > 2 ) {
          if ( dst_pixels_idx > dst_img_sz || src_byte_idx + 2 > src_img_sz ) { goto apg_bmp_read_error; }
          dst_img_ptr[dst_pixels_idx++] = src_img_ptr[src_byte_idx + 2];
        }
        if ( n_dst_chans > 1 ) {
          if ( dst_pixels_idx > dst_img_sz || src_byte_idx + 1 > src_img_sz ) { goto apg_bmp_read_error; }
          dst_img_ptr[dst_pixels_idx++] = src_img_ptr[src_byte_idx + 1];
        }
        dst_img_ptr[dst_pixels_idx++] = src_img_ptr[src_byte_idx];
        src_byte_idx += n_src_chans;
      }
      src_byte_idx += row_padding_sz;
    }
  } // endif bpp

  free( record.data );
  return dst_img_ptr;

apg_bmp_read_error:
  if ( record.data ) { free( record.data ); }
  if ( dst_img_ptr ) { free( dst_img_ptr ); }
  return NULL;
}

void apg_bmp_free( unsigned char* pixels_ptr ) {
  if ( !pixels_ptr ) { return; }
  free( pixels_ptr );
}

unsigned int apg_bmp_write( const char* filename, unsigned char* pixels_ptr, int w, int h, unsigned int n_chans ) {
  if ( !filename || !pixels_ptr ) { return 0; }
  if ( 0 == w || 0 == h ) { return 0; }
  if ( labs( w ) > _BMP_MAX_DIMS || labs( h ) > _BMP_MAX_DIMS ) { return 0; }
  if ( n_chans != 3 && n_chans != 4 ) { return 0; }

  uint32_t height = labs( h );
  uint32_t width  = labs( w );
  // Work out if any padding how much to skip at end of each row.
  const uint32_t unpadded_row_sz      = width * n_chans;
  const uint32_t row_padding_sz       = 0 == unpadded_row_sz % 4 ? 0 : 4 - unpadded_row_sz % 4;
  const uint32_t row_sz               = unpadded_row_sz + row_padding_sz;
  const uint32_t dst_pixels_padded_sz = row_sz * height;

  const size_t dib_hdr_sz = sizeof( _bmp_dib_BITMAPINFOHEADER_t );
  _bmp_file_header_t file_hdr;
  {
    file_hdr.file_type[0]      = 'B';
    file_hdr.file_type[1]      = 'M';
    file_hdr.file_sz           = (uint32_t)( _BMP_FILE_HDR_SZ + dib_hdr_sz ) + dst_pixels_padded_sz;
    file_hdr.reserved1         = 0;
    file_hdr.reserved2         = 0;
    file_hdr.image_data_offset = (uint32_t)( _BMP_FILE_HDR_SZ + dib_hdr_sz );
  }
  _bmp_dib_BITMAPINFOHEADER_t dib_hdr;
  {
    dib_hdr.this_header_sz         = (uint32_t)_BMP_MIN_DIB_HDR_SZ; // NOTE: Must be 40 and not include the bitmask memory in size here.
    dib_hdr.w                      = w;
    dib_hdr.h                      = h;
    dib_hdr.n_planes               = 1;
    dib_hdr.bpp                    = 3 == n_chans ? 24 : 32;
    dib_hdr.compression_method     = 3 == n_chans ? BI_RGB : BI_BITFIELDS;
    dib_hdr.image_uncompressed_sz  = 0;
    dib_hdr.horiz_pixels_per_meter = 0;
    dib_hdr.vert_pixels_per_meter  = 0;
    dib_hdr.n_colours_in_palette   = 0;
    dib_hdr.n_important_colours    = 0;
    // Big-endian masks. only used in BI_BITFIELDS and BI_ALPHABITFIELDS ( 16 and 32-bit images ).
    // Important note: GIMP stores BMP data in this array order for 32-bit: [A][B][G][R].
    dib_hdr.bitmask_r = 0xFF000000;
    dib_hdr.bitmask_g = 0x00FF0000;
    dib_hdr.bitmask_b = 0x0000FF00;
  }

  uint8_t* dst_pixels_ptr = malloc( dst_pixels_padded_sz );
  if ( !dst_pixels_ptr ) { return 0; }
  {
    size_t dst_byte_idx = 0;
    uint8_t padding[4]  = { 0, 0, 0, 0 };
    uint8_t rgba[4]     = { 0, 0, 0, 0 };
    uint8_t bgra[4]     = { 0, 0, 0, 0 };

    for ( uint32_t row = 0; row < height; row++ ) {
      size_t src_byte_idx = ( height - 1 - row ) * n_chans * width;
      for ( uint32_t col = 0; col < width; col++ ) {
        for ( uint32_t chan = 0; chan < n_chans; chan++ ) { rgba[chan] = pixels_ptr[src_byte_idx++]; }
        if ( 3 == n_chans ) {
          bgra[0] = rgba[2];
          bgra[1] = rgba[1];
          bgra[2] = rgba[0];
        } else {
          /* NOTE(Anton) RGBA with alpha channel would be better supported with an extended DIB header. */
          bgra[0] = rgba[3];
          bgra[1] = rgba[2];
          bgra[2] = rgba[1];
          bgra[3] = rgba[0]; // Alpha.
        }
        memcpy( &dst_pixels_ptr[dst_byte_idx], bgra, n_chans );
        dst_byte_idx += (size_t)n_chans;
      } // endfor col
      if ( row_padding_sz > 0 ) {
        memcpy( &dst_pixels_ptr[dst_byte_idx], padding, row_padding_sz );
        dst_byte_idx += row_padding_sz;
      }
    } // endfor row
  }
  {
    FILE* fp = fopen( filename, "wb" );
    if ( !fp ) {
      free( dst_pixels_ptr );
      return 0;
    }
    if ( 1 != fwrite( &file_hdr, _BMP_FILE_HDR_SZ, 1, fp ) ) {
      free( dst_pixels_ptr );
      fclose( fp );
      return 0;
    }
    if ( 1 != fwrite( &dib_hdr, dib_hdr_sz, 1, fp ) ) {
      free( dst_pixels_ptr );
      fclose( fp );
      return 0;
    }
    if ( 1 != fwrite( dst_pixels_ptr, dst_pixels_padded_sz, 1, fp ) ) {
      free( dst_pixels_ptr );
      fclose( fp );
      return 0;
    }
    fclose( fp );
  }
  free( dst_pixels_ptr );

  return 1;
}
