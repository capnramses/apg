/* ===============================================================================================
apg_plot
Author:   Anton Gerdelan  <antonofnote at gmail>  @capnramses
URL:      https://github.com/capnramses/apg
Licence:  See bottom of corresponding header file.
Language: C99.
Version:  0.2
==================================================================================================
*/

#include "apg_plot.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define _PI 3.14159265358979323846f
#define _2PI ( _PI * 2.0f )
#define _IMG_N_CHNS 3 // Number of colour channels per pixel.

static uint8_t _plot_colour[_IMG_N_CHNS]   = { 0xFF, 0xFF, 0xFF };
static uint8_t _line_colour[_IMG_N_CHNS]   = { 0x66, 0x66, 0x66 };
static uint8_t _x_axis_colour[_IMG_N_CHNS] = { 0x00, 0xAA, 0xAA };
static uint8_t _y_axis_colour[_IMG_N_CHNS] = { 0xAA, 0xAA, 0x00 };
static uint8_t _background_colour          = 0x00;

int _apg_plot_get_pixel_idx( apg_plot_t* chart_ptr, float x, float y ) {
  if ( !chart_ptr || !chart_ptr->rgb_ptr ) { return -1; }

  float x_fac = ( x - chart_ptr->params.min_x ) / ( chart_ptr->params.max_x - chart_ptr->params.min_x );
  float y_fac = ( y - chart_ptr->params.min_y ) / ( chart_ptr->params.max_y - chart_ptr->params.min_y );
  int x_int   = (int)roundf( x_fac * (float)chart_ptr->params.w );
  int y_int   = chart_ptr->params.h - 1 - (int)roundf( y_fac * (float)chart_ptr->params.h ); // Flip y so big values are at top of image.
  if ( x_int < 0 || x_int >= chart_ptr->params.w || y_int < 0 || y_int >= chart_ptr->params.h ) { return -1; }
  return ( y_int * chart_ptr->params.w + x_int ) * _IMG_N_CHNS;
}

// x0,x1 etc are allowed to be out of bounds of the drawing area. this function will draw up to the image edges towards OOB pixels.
static bool _draw_bresenham_line( apg_plot_t* chart_ptr, int x0, int y0, int x1, int y1 ) {
  if ( !chart_ptr || !chart_ptr->rgb_ptr ) { return false; }

  int x = x0, y = y0;
  int d_x = x1 - x0, d_y = y1 - y0; // Original deltas between start and end points.
  int i_x = 1, i_y = 1;             // Increase rate on each axis.
  // Remember direction of line on each axis.
  if ( d_x < 0 ) {
    i_x = -1;
    d_x = abs( d_x );
  }
  if ( d_y < 0 ) {
    i_y = -1;
    d_y = abs( d_y );
  }
  // Scaled deltas (used to allow integer comparison of <0.5).
  int d2_x = d_x * 2;
  int d2_y = d_y * 2;
  if ( d_x > d_y ) { // Identify major axis (remember these have been absoluted).
    int err = d2_y - d_x;
    for ( int i = 0; i <= d_x; i++ ) {
      bool out_of_bounds = ( x < 0 || x >= chart_ptr->params.w || y < 0 || y >= chart_ptr->params.h );
      if ( !out_of_bounds ) {
        int idx = ( y * chart_ptr->params.w + x );
        memcpy( &chart_ptr->rgb_ptr[idx * _IMG_N_CHNS], _line_colour, _IMG_N_CHNS * sizeof( uint8_t ) );
      }
      if ( err >= 0 ) {
        err -= d2_x;
        y += i_y;
      }
      err += d2_y;
      x += i_x;
    } // endfor
  } else {
    int err = d2_x - d_y;
    for ( int i = 0; i <= d_y; i++ ) {
      bool out_of_bounds = ( x < 0 || x >= chart_ptr->params.w || y < 0 || y >= chart_ptr->params.h );
      if ( !out_of_bounds ) {
        int idx = ( y * chart_ptr->params.w + x );
        memcpy( &chart_ptr->rgb_ptr[idx * _IMG_N_CHNS], _line_colour, _IMG_N_CHNS * sizeof( uint8_t ) );
      }
      if ( err >= 0 ) {
        err -= d2_y;
        x += i_x;
      }
      err += d2_x;
      y += i_y;
    } // endfor
  }   // endif
  return true;
}

bool apg_plot_plot_lines( apg_plot_t* chart_ptr, float* xy_ptr, int n ) {
  if ( !chart_ptr || !chart_ptr->rgb_ptr ) { return false; }

  for ( int i = 0; i < n - 1; i++ ) {
    int x0, y0, x1, y1;
    {
      int idx = i;
      float x = xy_ptr[idx * 2 + 0], y = xy_ptr[idx * 2 + 1];
      float x_fac = ( x - chart_ptr->params.min_x ) / ( chart_ptr->params.max_x - chart_ptr->params.min_x );
      float y_fac = ( y - chart_ptr->params.min_y ) / ( chart_ptr->params.max_y - chart_ptr->params.min_y );
      x0          = (int)roundf( x_fac * (float)chart_ptr->params.w );
      y0          = chart_ptr->params.h - 1 - (int)roundf( y_fac * (float)chart_ptr->params.h );
    }
    {
      int idx = i + 1;
      float x = xy_ptr[idx * 2 + 0], y = xy_ptr[idx * 2 + 1];
      float x_fac = ( x - chart_ptr->params.min_x ) / ( chart_ptr->params.max_x - chart_ptr->params.min_x );
      float y_fac = ( y - chart_ptr->params.min_y ) / ( chart_ptr->params.max_y - chart_ptr->params.min_y );
      x1          = (int)roundf( x_fac * (float)chart_ptr->params.w );
      y1          = chart_ptr->params.h - 1 - (int)roundf( y_fac * (float)chart_ptr->params.h );
    }
    if ( !_draw_bresenham_line( chart_ptr, x0, y0, x1, y1 ) ) { return false; }
  }
  return true;
}

bool apg_plot_plot_points( apg_plot_t* chart_ptr, float* xy_ptr, int n ) {
  if ( !chart_ptr || !chart_ptr->rgb_ptr ) { return false; }

  for ( int i = 0; i < n; i++ ) {
    float x = xy_ptr[i * 2 + 0], y = xy_ptr[i * 2 + 1];
    int plot_pixel_idx = _apg_plot_get_pixel_idx( chart_ptr, x, y );
    if ( plot_pixel_idx < 0 || plot_pixel_idx >= chart_ptr->params.h * chart_ptr->params.w * _IMG_N_CHNS ) { continue; }
    memcpy( &chart_ptr->rgb_ptr[plot_pixel_idx], _plot_colour, _IMG_N_CHNS ); // Or some symbol.
  }
  return true;
}

bool apg_plot_clear( apg_plot_t* chart_ptr ) {
  if ( !chart_ptr || !chart_ptr->rgb_ptr ) { return false; }
  memset( chart_ptr->rgb_ptr, _background_colour, chart_ptr->params.w * chart_ptr->params.h * _IMG_N_CHNS );
  return true;
}

bool apg_plot_x_axis_draw( apg_plot_t* chart_ptr, float y_value ) {
  if ( !chart_ptr || !chart_ptr->rgb_ptr ) { return false; }

  float y_fac = ( y_value - chart_ptr->params.min_y ) / ( chart_ptr->params.max_y - chart_ptr->params.min_y );
  int y_int   = chart_ptr->params.h - 1 - (int)roundf( y_fac * (float)chart_ptr->params.h ); // flip y so big values are at top of image
  if ( y_int < 0 || y_int >= chart_ptr->params.h ) { return false; }

  int idx_i = y_int * chart_ptr->params.w;
  int idx_f = ( y_int + 1 ) * chart_ptr->params.w;
  for ( int i = idx_i; i < idx_f; i++ ) { memcpy( &chart_ptr->rgb_ptr[i * _IMG_N_CHNS], _x_axis_colour, _IMG_N_CHNS ); }
  return true;
}

bool apg_plot_y_axis_draw( apg_plot_t* chart_ptr, float x_value ) {
  if ( !chart_ptr || !chart_ptr->rgb_ptr ) { return false; }
  if ( x_value < 0 || x_value >= chart_ptr->params.w ) { return false; }

  float x_fac = ( x_value - chart_ptr->params.min_x ) / ( chart_ptr->params.max_x - chart_ptr->params.min_x );
  int x_int   = (int)roundf( x_fac * (float)chart_ptr->params.w );
  if ( x_int < 0 || x_int >= chart_ptr->params.w ) { return false; }

  for ( int i = 0; i < chart_ptr->params.h; i++ ) {
    int idx = i * chart_ptr->params.w + x_int;
    memcpy( &chart_ptr->rgb_ptr[idx * _IMG_N_CHNS + 0], _y_axis_colour, _IMG_N_CHNS );
  }
  return true;
}

apg_plot_t apg_plot_init( apg_plot_params_t chart_params ) {
  apg_plot_t chart = ( apg_plot_t ){ .params = chart_params };
  chart.rgb_ptr    = calloc( chart_params.w * chart_params.h, _IMG_N_CHNS );
  apg_plot_clear( &chart );
  if ( !chart.rgb_ptr ) { return chart; }
  return chart;
}

bool apg_plot_free( apg_plot_t* chart_ptr ) {
  if ( !chart_ptr || !chart_ptr->rgb_ptr ) { return false; }
  free( chart_ptr->rgb_ptr );
  *chart_ptr = ( apg_plot_t ){ .rgb_ptr = NULL };
  return true;
}

void apg_plot_background_colour( uint8_t greyscale_value ) { _background_colour = greyscale_value; }

void apg_plot_line_colour( uint8_t r, uint8_t g, uint8_t b ) {
  _line_colour[0] = r;
  _line_colour[1] = g;
  _line_colour[2] = b;
}

void apg_plot_plot_colour( uint8_t r, uint8_t g, uint8_t b ) {
  _plot_colour[0] = r;
  _plot_colour[1] = g;
  _plot_colour[2] = b;
}

void apg_plot_x_axis_colour( uint8_t r, uint8_t g, uint8_t b ) {
  _x_axis_colour[0] = r;
  _x_axis_colour[1] = g;
  _x_axis_colour[2] = b;
}

void apg_plot_y_axis_colour( uint8_t r, uint8_t g, uint8_t b ) {
  _y_axis_colour[0] = r;
  _y_axis_colour[1] = g;
  _y_axis_colour[2] = b;
}

// Example/test program follows.
#ifdef _APG_PLOT_UNIT_TEST
// Was using my own TGA writer here but I think it's buggy.
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define _APG_PLOT_TEST_PX_X 512
#define _APG_PLOT_TEST_PX_Y 512
#define _APG_PLOT_TEST_N 32

int main() {
  // Init some test data. I want to plot n values, between 0 ant 2pi on x. Any value of n is fine.
  float xy_array[_APG_PLOT_TEST_N * 2];
  for ( int i = 0; i < _APG_PLOT_TEST_N; i++ ) {
    float x             = i * 1.0f / (float)( _APG_PLOT_TEST_N - 1 ) * _2PI;
    float y             = sinf( x );
    xy_array[i * 2 + 0] = x;
    xy_array[i * 2 + 1] = y;
  }
  // Create a chart bitmap with 512,512 pixels (any values are fine).
  // Have it plot between 0 and 2pi on x, and just over -1 to 1 on y, so the wave peaks are clearly visible.
  apg_plot_t chart =
    apg_plot_init( ( apg_plot_params_t ){ .w = _APG_PLOT_TEST_PX_X, .h = _APG_PLOT_TEST_PX_Y, .min_x = -0.1f, .max_x = _2PI + 0.1f, .min_y = -1.1f, .max_y = 1.1f } );
  if ( !chart.rgb_ptr ) {
    fprintf( stderr, "ERROR allocating chart.\n" );
    return 1;
  }
  // This is the horizontal "axis" line. I want it in the middle of the range hence 0 in a range of -1 to 1.
  if ( !apg_plot_x_axis_draw( &chart, 0.0f ) ) { return 1; }
  // This is the vertical "axis" line. I want it in the middle of the data range hence pi out of 2pi.
  if ( !apg_plot_y_axis_draw( &chart, _PI ) ) { return 1; }
  // Draw lines connecting the data series. I do this before the plot points, so they get drawn on top of the line.
  if ( !apg_plot_plot_lines( &chart, xy_array, _APG_PLOT_TEST_N ) ) { return 1; }
  // Draw plot points. These will be drawn over the lines.
  if ( !apg_plot_plot_points( &chart, xy_array, _APG_PLOT_TEST_N ) ) { return 1; }

  // Output the bitmap to a TGA image so we can see it. You might instead draw it onto your application window.
  if ( !stbi_write_png( "output_black.png", _APG_PLOT_TEST_PX_X, _APG_PLOT_TEST_PX_Y, 3, chart.rgb_ptr, _APG_PLOT_TEST_PX_X * 3 ) ) { return 1; }
  printf( "Wrote output_black.png.\n" );

  // Redraw the chart with a white background.
  apg_plot_background_colour( 0xFF );         // White background.
  apg_plot_line_colour( 0xAA, 0xAA, 0xAA );   // Light grey plot lines.
  apg_plot_plot_colour( 0x00, 0x00, 0x00 );   // Black plot points.
  apg_plot_x_axis_colour( 0x66, 0x66, 0x66 ); // Grey.
  apg_plot_y_axis_colour( 0x66, 0x66, 0x66 ); // Grey.

  // Clear _after_ changing the background colour, otherwise it will still be black.
  if ( !apg_plot_clear( &chart ) ) { return 1; }

  if ( !apg_plot_x_axis_draw( &chart, 0.0f ) ) { return 1; }
  if ( !apg_plot_y_axis_draw( &chart, _PI ) ) { return 1; }
  if ( !apg_plot_plot_lines( &chart, xy_array, _APG_PLOT_TEST_N ) ) { return 1; }
  if ( !apg_plot_plot_points( &chart, xy_array, _APG_PLOT_TEST_N ) ) { return 1; }

  // Output second bitmap for white chart.
  if ( !stbi_write_png( "output_white.png", _APG_PLOT_TEST_PX_X, _APG_PLOT_TEST_PX_Y, 3, chart.rgb_ptr, _APG_PLOT_TEST_PX_X * 3 ) ) { return 1; }
  printf( "Wrote output_white.png.\n" );

  // Free allocated chart memory.
  if ( !apg_plot_free( &chart ) ) {
    fprintf( stderr, "ERROR freeing chart.\n" );
    return 1;
  }
  printf( "Success.\n" );

  return 0;
}
#endif
