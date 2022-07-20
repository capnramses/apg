#ifdef _APG_LINE_CHART_UNIT_TEST
#define APG_TGA_IMPLEMENTATION
#include "apg_tga.h"
#endif
#include "apg_line_chart.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define _PI 3.14159265358979323846f
#define _2PI ( _PI * 2.0f )

#define _IMG_N_CHNS 3

int _apg_line_chart_get_pixel_idx( apg_line_chart_t* chart_ptr, float x, float y ) {
  if ( !chart_ptr || !chart_ptr->rgb_ptr ) { return -1; }

  float x_fac = ( x - chart_ptr->params.min_x ) / ( chart_ptr->params.max_x - chart_ptr->params.min_x );
  float y_fac = ( y - chart_ptr->params.min_y ) / ( chart_ptr->params.max_y - chart_ptr->params.min_y );
  int x_int   = (int)roundf( x_fac * (float)chart_ptr->params.w );
  int y_int   = chart_ptr->params.h - 1 - (int)roundf( y_fac * (float)chart_ptr->params.h ); // flip y so big values are at top of image
  if ( x_int < 0 || x_int >= chart_ptr->params.w || y_int < 0 || y_int >= chart_ptr->params.h ) { return -1; }
  return ( y_int * chart_ptr->params.w + x_int ) * _IMG_N_CHNS;
}

// x0,x1 etc are allowed to be out of bounds of the drawing area. this function will draw up to the image edges towards oob pixels.
static bool _draw_bresenham_line( apg_line_chart_t* chart_ptr, int x0, int y0, int x1, int y1 ) {
  if ( !chart_ptr || !chart_ptr->rgb_ptr ) { return false; }

  int x = x0;
  int y = y0;
  // original deltas between start and end points
  int d_x = x1 - x0;
  int d_y = y1 - y0;
  // increase rate on each axis
  int i_x = 1;
  int i_y = 1;
  // remember direction of line on each axis
  if ( d_x < 0 ) {
    i_x = -1;
    d_x = abs( d_x );
  }
  if ( d_y < 0 ) {
    i_y = -1;
    d_y = abs( d_y );
  }
  // scaled deltas (used to allow integer comparison of <0.5)
  int d2_x = d_x * 2;
  int d2_y = d_y * 2;
  // identify major axis (remember these have been absoluted)
  if ( d_x > d_y ) {
    // initialise error term
    int err = d2_y - d_x;
    for ( int i = 0; i <= d_x; i++ ) {
      {
        if ( x < 0 || x >= chart_ptr->params.w || y < 0 || y >= chart_ptr->params.h ) {
        } else { // ignore OOB pixel
          int idx                                   = ( y * chart_ptr->params.w + x );
          chart_ptr->rgb_ptr[idx * _IMG_N_CHNS + 0] = 0xFF;
          chart_ptr->rgb_ptr[idx * _IMG_N_CHNS + 1] = 0xFF;
          chart_ptr->rgb_ptr[idx * _IMG_N_CHNS + 2] = 0xFF;
        }
      }
      if ( err >= 0 ) {
        err -= d2_x;
        y += i_y;
      }
      err += d2_y;
      x += i_x;
    } // endfor
  } else {
    // initialise error term
    int err = d2_x - d_y;
    for ( int i = 0; i <= d_y; i++ ) {
      {
        if ( x < 0 || x >= chart_ptr->params.w || y < 0 || y >= chart_ptr->params.h ) {
        } else { // ignore OOB pixel
          int idx                                   = ( y * chart_ptr->params.w + x );
          chart_ptr->rgb_ptr[idx * _IMG_N_CHNS + 0] = 0xFF;
          chart_ptr->rgb_ptr[idx * _IMG_N_CHNS + 1] = 0xFF;
          chart_ptr->rgb_ptr[idx * _IMG_N_CHNS + 2] = 0xFF;
        }
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
} // endfunc

bool apg_line_chart_plot_lines( apg_line_chart_t* chart_ptr, float* xy_ptr, int n ) {
  if ( !chart_ptr || !chart_ptr->rgb_ptr ) { return false; }
  for ( int i = 0; i < n - 1; i++ ) {
    int x0, y0, x1, y1;
    {
      int idx     = i;
      float x     = xy_ptr[idx * 2 + 0];
      float y     = xy_ptr[idx * 2 + 1];
      float x_fac = ( x - chart_ptr->params.min_x ) / ( chart_ptr->params.max_x - chart_ptr->params.min_x );
      float y_fac = ( y - chart_ptr->params.min_y ) / ( chart_ptr->params.max_y - chart_ptr->params.min_y );
      x0          = (int)roundf( x_fac * (float)chart_ptr->params.w );
      y0          = chart_ptr->params.h - 1 - (int)roundf( y_fac * (float)chart_ptr->params.h );
    }
    {
      int idx     = i + 1;
      float x     = xy_ptr[idx * 2 + 0];
      float y     = xy_ptr[idx * 2 + 1];
      float x_fac = ( x - chart_ptr->params.min_x ) / ( chart_ptr->params.max_x - chart_ptr->params.min_x );
      float y_fac = ( y - chart_ptr->params.min_y ) / ( chart_ptr->params.max_y - chart_ptr->params.min_y );
      x1          = (int)roundf( x_fac * (float)chart_ptr->params.w );
      y1          = chart_ptr->params.h - 1 - (int)roundf( y_fac * (float)chart_ptr->params.h );
    }
    if ( !_draw_bresenham_line( chart_ptr, x0, y0, x1, y1 ) ) { return false; }
  }
  return true;
}

bool apg_line_chart_plot_points( apg_line_chart_t* chart_ptr, float* xy_ptr, int n ) {
  if ( !chart_ptr || !chart_ptr->rgb_ptr ) { return false; }

  for ( int i = 0; i < n; i++ ) {
    float x            = xy_ptr[i * 2 + 0];
    float y            = xy_ptr[i * 2 + 1];
    int plot_pixel_idx = _apg_line_chart_get_pixel_idx( chart_ptr, x, y );
    if ( plot_pixel_idx < 0 || plot_pixel_idx >= chart_ptr->params.h * chart_ptr->params.w * _IMG_N_CHNS ) { continue; }
    chart_ptr->rgb_ptr[plot_pixel_idx + 0] = chart_ptr->rgb_ptr[plot_pixel_idx + 1] = chart_ptr->rgb_ptr[plot_pixel_idx + 2] = 0xFF; // or some symbol.
  }
  return true;
}

bool apg_line_chart_clear( apg_line_chart_t* chart_ptr ) {
  if ( !chart_ptr || !chart_ptr->rgb_ptr ) { return false; }
  memset( chart_ptr->rgb_ptr, 0, chart_ptr->params.w * chart_ptr->params.h * _IMG_N_CHNS );
  return true;
}

bool apg_line_chart_x_axis_draw( apg_line_chart_t* chart_ptr, float y_value ) {
  if ( !chart_ptr || !chart_ptr->rgb_ptr ) { return false; }

  float y_fac = ( y_value - chart_ptr->params.min_y ) / ( chart_ptr->params.max_y - chart_ptr->params.min_y );
  int y_int   = chart_ptr->params.h - 1 - (int)roundf( y_fac * (float)chart_ptr->params.h ); // flip y so big values are at top of image
  if ( y_int < 0 || y_int >= chart_ptr->params.h ) { return false; }

  int idx_i = y_int * chart_ptr->params.w;
  int idx_f = ( y_int + 1 ) * chart_ptr->params.w;
  for ( int i = idx_i; i < idx_f; i++ ) {
    chart_ptr->rgb_ptr[i * _IMG_N_CHNS + 0] = 0x00;
    chart_ptr->rgb_ptr[i * _IMG_N_CHNS + 1] = 0xFF;
    chart_ptr->rgb_ptr[i * _IMG_N_CHNS + 2] = 0xFF;
  }
  return true;
}

bool apg_line_chart_y_axis_draw( apg_line_chart_t* chart_ptr, float x_value ) {
  if ( !chart_ptr || !chart_ptr->rgb_ptr ) { return false; }
  if ( x_value < 0 || x_value >= chart_ptr->params.w ) { return false; }

  float x_fac = ( x_value - chart_ptr->params.min_x ) / ( chart_ptr->params.max_x - chart_ptr->params.min_x );
  int x_int   = (int)roundf( x_fac * (float)chart_ptr->params.w );
  if ( x_int < 0 || x_int >= chart_ptr->params.w ) { return false; }

  for ( int i = 0; i < chart_ptr->params.h; i++ ) {
    int idx                                   = i * chart_ptr->params.w + x_int;
    chart_ptr->rgb_ptr[idx * _IMG_N_CHNS + 0] = 0xFF;
    chart_ptr->rgb_ptr[idx * _IMG_N_CHNS + 1] = 0xFF;
    chart_ptr->rgb_ptr[idx * _IMG_N_CHNS + 2] = 0x00;
  }
  return true;
}

apg_line_chart_t apg_line_chart_init( apg_line_chart_params_t chart_params ) {
  apg_line_chart_t chart = ( apg_line_chart_t ){ .params = chart_params };
  chart.rgb_ptr          = calloc( chart_params.w * chart_params.h, _IMG_N_CHNS );
  if ( !chart.rgb_ptr ) { return chart; }
  return chart;
}

bool apg_line_chart_free( apg_line_chart_t* chart_ptr ) {
  if ( !chart_ptr || !chart_ptr->rgb_ptr ) { return false; }
  free( chart_ptr->rgb_ptr );
  *chart_ptr = ( apg_line_chart_t ){ .rgb_ptr = NULL };
  return true;
}

#ifdef _APG_LINE_CHART_UNIT_TEST
#define TEST_X 256
#define TEST_Y 256

int main() {
  // Init some test data
  float xy_array[256 * 2];
  for ( int i = 0; i < 256; i++ ) {
    float x             = i * 1.0f / 256.0f * _2PI;
    float y             = sinf( x );
    xy_array[i * 2 + 0] = x;
    xy_array[i * 2 + 1] = y;
  }

  apg_line_chart_t chart = apg_line_chart_init( ( apg_line_chart_params_t ){ .w = TEST_X, .h = TEST_Y, .min_x = 0.0f, .max_x = _2PI, .min_y = -1.0f, .max_y = 1.0f } );
  if ( !chart.rgb_ptr ) {
    fprintf( stderr, "ERROR allocating chart.\n" );
    return 1;
  }

  apg_line_chart_plot_points( &chart, xy_array, 256 );

  apg_line_chart_x_axis_draw( &chart, TEST_Y / 2.0f );
  apg_line_chart_y_axis_draw( &chart, 0.0f );

  apg_tga_write_file( "output.tga", chart.rgb_ptr, TEST_X, TEST_Y, 3 );
  printf( "wrote output.tga\n" );

  if ( !apg_line_chart_free( &chart ) ) {
    fprintf( stderr, "ERROR freeing chart.\n" );
    return 1;
  }
  printf( "Freed chart.\n" );

  return 0;
}
#endif
